#include "Processing.h"

#include "sf/Process.h"

#include <stdarg.h>

#include "sf/Array.h"
#include "sf/String.h"
#include "sf/File.h"
#include "sf/Symbol.h"
#include "sf/HashMap.h"
#include "sf/HashSet.h"
#include "sf/Box.h"

#include "sp/Asset.h"

#include "ext/sokol/sokol_time.h"

// For std::thread::hardware_concurrency()
// TODO: Reimplement this
#include <thread>

sf::Symbol symf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	sf::SmallStringBuf<1024> buf;
	buf.vformat(fmt, args);

	va_end(args);

	return std::move(sf::Symbol(buf));
}

sf::Symbol operator "" _sym(const char *data, size_t size)
{
	return sf::Symbol(data, size);
}

enum class JobPriority
{
	Normal,
	Background,
	Count,
};

struct Task;
struct Processor;

struct Job
{
	enum Status
	{
		Running,
		Succeeded,
		Failed,
	};

	sf::StringBuf description;

	virtual bool isNotable() const { return false; }
	virtual Status begin(Processor &p) = 0;
	virtual Status getStatus() { return Status::Succeeded; }
};

struct ExecJob : Job
{
	sf::Symbol exeName;
	sf::Array<sf::StringBuf> args;
	sf::Process *process = nullptr;

	ExecJob(const sf::String &exeName, sf::Array<sf::StringBuf> &&args)
		: exeName(exeName), args(std::move(args))
	{
	}

	~ExecJob()
	{
		if (process) {
			sf::joinProcess(process);
		}
	}

	virtual bool isNotable() const { return true; }

	virtual Status begin(Processor &p);
	virtual Status getStatus();
};

struct CopyJob : Job
{
	sf::StringBuf src, dst;

	CopyJob(sf::String src, sf::String dst)
		: src(src), dst(dst)
	{
	}

	virtual Status begin(Processor &p)
	{
		description.format("cp %s %s", src.data, dst.data);
		sf::Array<char> data;
		if (!sf::readFile(data, src)) return Failed;
		if (!sf::writeFile(dst, data)) return Failed;
		return Succeeded;
	}
};

struct MoveJob : Job
{
	sf::StringBuf src, dst;

	MoveJob(sf::String src, sf::String dst)
		: src(src), dst(dst)
	{
	}

	virtual Status begin(Processor &p)
	{
		description.format("mv %s %s", src.data, dst.data);
		if (sf::replaceFile(dst, src)) {
			return Succeeded;
		} else {
			return Failed;
		}
	}
};

struct MkdirsJob : Job
{
	sf::StringBuf path;

	MkdirsJob(sf::String path)
		: path(path)
	{
	}

	virtual Status begin(Processor &p)
	{
		description.format("mkdir -p %s", path.data);
		if (sf::createDirectories(path)) {
			return Succeeded;
		} else {
			return Failed;
		}
	}
};

struct TaskInstance;

struct JobQueue
{
	TaskInstance *taskInstance;
	sf::Array<sf::Box<Job>> jobs;

	void exec(sf::String exeName, sf::Array<sf::StringBuf> &&args)
	{
		jobs.push(sf::box<ExecJob>(exeName, std::move(args)));
	}

	void copy(sf::String src, sf::String dst)
	{
		jobs.push(sf::box<CopyJob>(src, dst));
	}

	void move(sf::String src, sf::String dst)
	{
		jobs.push(sf::box<MoveJob>(src, dst));
	}

	void mkdirsToFile(sf::String path)
	{
		size_t len = 0;
		for (size_t i = 0; i < path.size; i++) {
			if (path.data[i] == '/' || path.data[i] == '\\') len = i;
		}
		if (len > 0 && !sf::isDirectory(sf::String(path.data, len))) {
			jobs.push(sf::box<MkdirsJob>(sf::String(path.data, len)));
		}
	}

	bool isEmpty()
	{
		return jobs.size == 0;
	}

	sf::Box<Job> dequeue()
	{
		sf::Box<Job> job = std::move(jobs[0]);
		jobs.removeOrdered(0);
		return job;
	}
};

struct TaskInstance
{
	Task *task = nullptr;
	sf::Symbol key;
	sf::HashMap<sf::Symbol, sf::Symbol> inputs;
	sf::HashMap<sf::Symbol, sf::Symbol> params;
	sf::HashMap<sf::Symbol, sf::Symbol> outputs;
	sf::HashSet<sf::Symbol> assets;

	bool dirty = false;
	bool processing = false;
	uint64_t lastMonitoredUpdateStm = 0;
	int failCounter = 0;

	void merge(const TaskInstance &rhs)
	{
		sf_assert(key == rhs.key);
		for (auto pair : rhs.inputs) inputs[pair.key] = pair.val;
		for (auto pair : rhs.params) params[pair.key] = pair.val;
		for (auto pair : rhs.outputs) outputs[pair.key] = pair.val;
	}
};

struct Task
{
	sf::StringBuf name;
	sf::Array<sf::StringBuf> tools;

	~Task() { }
	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) = 0;
	virtual void process(Processor &p, TaskInstance &ti) = 0;
};

struct TaskInstanceKey
{
	sf::Symbol key;
	Task *task;

	bool operator==(const TaskInstanceKey &rhs) const { return key == rhs.key && task == rhs.task; }
	bool operator!=(const TaskInstanceKey &rhs) const { return !(*this == rhs); }
};

uint32_t hash(const TaskInstanceKey &key)
{
	return sf::hashCombine(sf::hash(key.key), sf::hashPointer(key.task));
}

struct ActiveJobQueue
{
	JobQueue queue;
	sf::Box<Job> job;
};

struct Processor
{
	bool verbose = false;

	int level = 10;
	sf::StringBuf dataRoot;
	sf::StringBuf tempRoot;
	sf::StringBuf buildRoot;
	sf::StringBuf toolRoot;

	sf::HashSet<sf::Symbol> assetsToReload;
	sf::HashMap<sf::Symbol, ProcessingAsset> processingAssets;

	sf::DirectoryMonitor dataMonitor;

	sf::Array<sf::Box<Task>> tasks;
	sf::HashMap<TaskInstanceKey, sf::Box<TaskInstance>> taskInstances;
	sf::HashMap<sf::Symbol, sf::Array<sf::Box<TaskInstance>>> tasksForInput;
	sf::Array<TaskInstance*> dirtyTaskInstances;

	uint32_t maxActiveJobs = 1;
	sf::Array<ActiveJobQueue> activeJobs;
	sf::Array<JobQueue> jobQueues[(uint32_t)JobPriority::Count];

	void addJobs(JobPriority priority, TaskInstance &ti, JobQueue jobs)
	{
		jobs.taskInstance = &ti;
		jobQueues[(uint32_t)priority].push(std::move(jobs));
	}

	void addInputFile(const sf::Symbol &path);

	void updateTasks();
	void updateJobs();
};

void Processor::addInputFile(const sf::Symbol &path)
{
	sf::Array<sf::Box<TaskInstance>> &inputTasks = tasksForInput[path];
	inputTasks.clear();

	for (sf::Box<Task> &task : tasks) {
		TaskInstance ti;
		ti.task = task;
		if (task->addInput(ti, path)) {
			sf_assert(ti.outputs.size() > 0);
			if (!ti.key) {
				sf_assert(ti.outputs.size() == 1);
				ti.key = ti.outputs.data[0].val;
			}

			TaskInstanceKey key;
			key.key = ti.key;
			key.task = task;
			sf::Box<TaskInstance> &sharedInstance = taskInstances[key];
			if (sharedInstance) {
				sharedInstance->merge(ti);
			} else {
				sharedInstance = sf::box<TaskInstance>(std::move(ti));
				sharedInstance->task = task;
			}
			inputTasks.push(sharedInstance);
		}
	}
}

void Processor::updateTasks()
{
	uint64_t now = stm_now();
	for (uint32_t i = 0; i < dirtyTaskInstances.size; i++) {
		TaskInstance *ti = dirtyTaskInstances[i];
		if (ti->processing) continue;
		if (ti->lastMonitoredUpdateStm && stm_sec(stm_diff(now, ti->lastMonitoredUpdateStm)) < 3.0) {
			continue;
		}

		ti->dirty = false;
		ti->processing = true;
		for (const sf::Symbol &sym : ti->assets) {
			assetsToReload.insert(sym);
			processingAssets[sym].tasksPending++;
		}
		ti->task->process(*this, *ti);
		dirtyTaskInstances.removeSwap(i--);
	}
}

static Job::Status processActiveQueue(Processor &p, ActiveJobQueue &active)
{
	Job::Status status = Job::Succeeded;
	if (active.job) status = active.job->getStatus();
	for (;;) {
		switch (status) {
		case Job::Failed: {
			TaskInstance *ti = active.queue.taskInstance;
			ti->processing = false;

			sf::debugPrintLine("FAIL: %s (%s)", ti->key.data, ti->task->name.data);
			sf::debugPrintLine("FAIL> %s", active.job->description.data);

			if (++ti->failCounter <= 3) {
				ti->lastMonitoredUpdateStm = stm_now();
				if (!ti->dirty) {
					ti->dirty = true;
					p.dirtyTaskInstances.push(ti);
				}
			} else {
				for (const sf::Symbol &sym : ti->assets) {
					p.processingAssets[sym].tasksDone++;
				}
			}

			return Job::Failed;
		}
		case Job::Running: return Job::Running;
		case Job::Succeeded:
			if (active.queue.isEmpty()) {
				TaskInstance *ti = active.queue.taskInstance;
				ti->failCounter = 0;
				ti->processing = false;
				for (const sf::Symbol &sym : ti->assets) {
					p.processingAssets[sym].tasksDone++;
				}
				return Job::Succeeded;
			}
			active.job = active.queue.dequeue();
			status = active.job->begin(p);
			if (p.verbose || active.job->isNotable()) {
				sf::debugPrintLine("> %s", active.job->description.data);
			}
			break;
		}
	}

	return Job::Running;
}

void Processor::updateJobs()
{
	// Remove/advance active job queues
	for (uint32_t i = 0; i < activeJobs.size; i++) {
		Job::Status status = processActiveQueue(*this, activeJobs[i]);
		if (status != Job::Running) {
			activeJobs.removeSwap(i--);
		}
	}

	// Start new jobs
	for (sf::Array<JobQueue> &prioQueues : jobQueues) {
		if (activeJobs.size >= maxActiveJobs) break;
		for (uint32_t i = 0; i < prioQueues.size; i++) {
			if (activeJobs.size >= maxActiveJobs) break;

			ActiveJobQueue active;
			active.queue = std::move(prioQueues[i]);
			prioQueues.removeSwap(i--);
			Job::Status status = processActiveQueue(*this, active);
			if (status == Job::Running) {
				activeJobs.push(std::move(active));
			}
		}
	}
}

Job::Status ExecJob::begin(Processor &p)
{
	sf::SmallArray<sf::String, 32> argsRef;
	argsRef.reserve(args.size);
	for (sf::StringBuf &arg : args) argsRef.push(arg);

	sf::ProcessStartOpts opts;

	sf::StringBuf exePath;
	sf::appendPath(exePath, p.toolRoot, exeName);

	process = sf::startProcess(exePath, argsRef, opts, &description);
	return process ? Running : Failed;
}

Job::Status ExecJob::getStatus()
{
	sf_assert(process);
	uint32_t code;
	if (sf::getProcessCompleted(process, &code)) {
		return code == 0 ? Succeeded : Failed;
	} else {
		return Running;
	}
}

static const sf::Symbol s_metallic{"metallic"};
static const sf::Symbol s_ao{"ao"};
static const sf::Symbol s_roughness{"roughness"};
static const sf::Symbol s_emissive{"emissive"};
static const sf::Symbol s_albedo{"albdo"};
static const sf::Symbol s_normal{"normal"};
static const sf::Symbol s_normal_gl{"normal_gl"};
static const sf::Symbol s_normal_dx{"normal_dx"};
static const sf::Symbol s_src{"src"};
static const sf::Symbol s_dst{"dst"};
static const sf::Symbol s_drop_arr[] = {
	sf::Symbol("drop0"),  sf::Symbol("drop1"),  sf::Symbol("drop2"),  sf::Symbol("drop3"),
	sf::Symbol("drop4"),  sf::Symbol("drop5"),  sf::Symbol("drop6"),  sf::Symbol("drop7"),
	sf::Symbol("drop8"),  sf::Symbol("drop9"),  sf::Symbol("drop10"), sf::Symbol("drop11"),
	sf::Symbol("drop12"), sf::Symbol("drop13"), sf::Symbol("drop14"), sf::Symbol("drop15"),
};
static const sf::Symbol s_face_arr[] = {
	sf::Symbol("face0"),  sf::Symbol("face1"), sf::Symbol("face2"),  sf::Symbol("face3"),
	sf::Symbol("face4"),  sf::Symbol("face5"), 
};

bool endsWithStrip(sf::Symbol &base, sf::String path, sf::String suffix)
{
	if (!sf::endsWith(path, suffix)) return false;
	base = sf::Symbol(path.data, path.size - suffix.size);
	return true;
}

static const sf::Symbol s_bc3{"bc3"};
static const sf::Symbol s_bc7{"bc7"};
static const sf::Symbol s_rgba8{"rgba8"};
static const sf::Symbol s_astc4x4{"astc4x4"};
static const sf::Symbol s_astc8x8{"astc8x8"};

static JobPriority getPriorityForTextureFormat(sf::String format)
{
	if (format == s_bc3) return JobPriority::Normal;
	if (format == s_bc7) return JobPriority::Normal;
	if (format == s_rgba8) return JobPriority::Normal;
	return JobPriority::Background;
}

struct GuiTextureTask : Task
{
	sf::Symbol format;
	int maxExtent;
	sf::SmallStringBuf<32> maxExtentStr;
	sf::StringBuf directory;

	GuiTextureTask(sf::String format, sf::String directory, int maxExtent)
		: format(format), directory(directory), maxExtent(maxExtent)
	{
		name.format("GuiTextureTask (%s) %s %d", directory.data, format.data, maxExtent);
		maxExtentStr.format("%d", maxExtent);
		tools.push("sp-texcomp");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (!sf::containsDirectory(path, directory)) return false;
		if (!sf::endsWith(path, ".png")) return false;
		ti.outputs[s_dst] = symf("%s.%s.sptex", path.data, format.data);
		ti.inputs[s_src] = path;
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf tempFile, dstFile;
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--format");
		args.push(sf::String(format));

		args.push("--max-extent");
		args.push(maxExtentStr);

		{
			sf::SmallStringBuf<512> path;
			sf::appendPath(path, p.dataRoot, ti.inputs[s_src]);
			args.push("--input");
			args.push(path);
		}

		args.push("--output");
		args.push(tempFile);

		args.push("--premultiply");
		args.push("--crop-alpha");

		JobPriority priority = getPriorityForTextureFormat(format);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-texcomp", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(priority, ti, jq);
	}
};

struct MiscTextureTask : Task
{
	sf::Symbol format;
	sf::SmallStringBuf<32> maxExtentStr;
	sf::StringBuf directory;

	MiscTextureTask(sf::String format, sf::String directory)
		: format(format), directory(directory)
	{
		name.format("MiscTextureTask (%s) %s", directory.data, format.data);
		tools.push("sp-texcomp");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (!sf::containsDirectory(path, directory)) return false;
		if (!sf::endsWith(path, ".png")) return false;
		ti.outputs[s_dst] = symf("%s.sptex", path.data);
		ti.inputs[s_src] = path;
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf tempFile, dstFile;
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--format");
		args.push(sf::String(format));

		{
			sf::SmallStringBuf<512> path;
			sf::appendPath(path, p.dataRoot, ti.inputs[s_src]);
			args.push("--input");
			args.push(path);
		}

		args.push("--linear");

		args.push("--output");
		args.push(tempFile);

		JobPriority priority = getPriorityForTextureFormat(format);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-texcomp", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(priority, ti, jq);
	}
};

struct ParticleTextureTask : Task
{
	sf::Symbol format;
	int maxExtent;
	sf::SmallStringBuf<32> maxExtentStr;
	sf::StringBuf directory;

	ParticleTextureTask(sf::String format, sf::String directory, int maxExtent)
		: format(format), directory(directory), maxExtent(maxExtent)
	{
		name.format("ParticleTextureTask (%s) %s %d", directory.data, format.data, maxExtent);
		maxExtentStr.format("%d", maxExtent);
		tools.push("sp-texcomp");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (!sf::containsDirectory(path, directory, 1)) return false;
		if (!sf::endsWith(path, ".png")) return false;
		ti.outputs[s_dst] = symf("%s.%s.sptex", path.data, format.data);
		ti.inputs[s_src] = path;
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf tempFile, dstFile;
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--format");
		args.push(sf::String(format));

		args.push("--max-extent");
		args.push(maxExtentStr);

		{
			sf::SmallStringBuf<512> path;
			sf::appendPath(path, p.dataRoot, ti.inputs[s_src]);
			args.push("--input");
			args.push(path);
		}

		args.push("--output");
		args.push(tempFile);

		JobPriority priority = getPriorityForTextureFormat(format);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-texcomp", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(priority, ti, jq);
	}
};

struct EnvmapTask : Task
{
	sf::Symbol format;
	int maxExtent;
	sf::SmallStringBuf<32> maxExtentStr;
	sf::StringBuf directory;

	EnvmapTask(sf::String format, sf::String directory, int maxExtent)
		: format(format), directory(directory), maxExtent(maxExtent)
	{
		name.format("ParticleTextureTask (%s) %s %d", directory.data, format.data, maxExtent);
		maxExtentStr.format("%d", maxExtent);
		tools.push("sp-envmap");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (!sf::containsDirectory(path, directory)) return false;
		static const sf::String suffixes[] = {
			"_3.exr", "_1.exr", "_5.exr", "_4.exr", "_6.exr", "_2.exr", 
		};
		for (uint32_t i = 0; i < 6; i++) {
			sf::String suffix = suffixes[i];
			if (!sf::endsWith(path, suffix)) continue;
			ti.key = sf::Symbol(sf::String(path).slice().dropRight(suffix.size));
			ti.inputs[s_face_arr[i]] = path;
			ti.outputs[s_dst] = symf("%s.%s.sptex", ti.key.data, format.data);
			ti.assets.insert(ti.key);
			return true;
		}
		return false;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		if (ti.inputs.size() != 6) return;

		sf::Array<sf::StringBuf> args;

		sf::StringBuf tempFile, dstFile;
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		if (p.level < 10) {
			args.push("--samples");
			args.push().format("%d", 64);
		}

		args.push("--format");
		args.push(sf::String(format));

		args.push("--resolution");
		args.push(maxExtentStr);

		{
			args.push("--input");
			for (uint32_t i = 0; i < 6; i++) {
				sf::SmallStringBuf<512> path;
				sf::appendPath(path, p.dataRoot, ti.inputs[s_face_arr[i]]);
				args.push(path);
			}
		}

		args.push("--output");
		args.push(tempFile);

		JobPriority priority = getPriorityForTextureFormat(format);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-envmap", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(priority, ti, jq);
	}
};


struct MaterialTextureTask : Task
{
	sf::Symbol format;
	sf::Symbol suffix;
	int resolution;
	int mipDrops;
	sf::SmallStringBuf<16> resolutionString;
	sf::SmallStringBuf<16> mipDropString;

	MaterialTextureTask(sf::String type, sf::String suffix, sf::String format, int resolution, int mipDrops)
		: format(format), suffix(suffix), resolution(resolution), mipDrops(mipDrops)
	{
		name.append(type, "TextureTask ", format);
		resolutionString.format("%d", resolution);
		mipDropString.format("%d", mipDrops);
		tools.push("sp-texcomp");
	}

	void postAddInput(TaskInstance &ti)
	{
		for (int drop = 0; drop < mipDrops; drop++) {
			ti.outputs[s_drop_arr[drop]] = symf("%s_%s.%s.%d.sptex", ti.key.data, suffix.data, format.data, resolution >> drop);
		}
		ti.params[s_dst] = symf("%s_%s.%s.:width:.sptex", ti.key.data, suffix.data, format.data);
		ti.assets.insert(ti.key);
	}

	void addCommonTexcompArgs(Processor &p, TaskInstance &ti, sf::Array<sf::StringBuf> &args)
	{
		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--format");
		args.push(sf::String(format));

		args.push("--flip-y");

		args.push("--resolution");
		args.push(resolutionString);
		args.push(resolutionString);

		sf::StringBuf tempPattern;
		sf::appendPath(tempPattern, p.tempRoot, ti.params[s_dst]);

		args.push("--output");
		args.push(tempPattern);

		args.push("--mip-drop-copies");
		args.push(mipDropString);
	}

	void addTexcompInputArg(Processor &p, sf::Array<sf::StringBuf> &args, const sf::Symbol &name)
	{
		sf::StringBuf path;
		sf::appendPath(path, p.dataRoot, name);
		args.push("--input");
		args.push(std::move(path));
	}

	void pushJobs(Processor &p, TaskInstance &ti, sf::Array<sf::StringBuf> &&args)
	{
		JobQueue jq;
		{
			sf::SmallStringBuf<256> tempFile, dstFile;
			sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_drop_arr[0]]);
			sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_drop_arr[0]]);
			jq.mkdirsToFile(tempFile);
			jq.mkdirsToFile(dstFile);
		}
		jq.exec("sp-texcomp", std::move(args));
		for (int i = 0; i < mipDrops; i++) {
			sf::SmallStringBuf<256> tempFile, dstFile;
			sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_drop_arr[i]]);
			sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_drop_arr[i]]);
			jq.move(tempFile, dstFile);
		}

		JobPriority priority = getPriorityForTextureFormat(format);
		p.addJobs(priority, ti, jq);
	}
};

struct AlbedoTextureTask : MaterialTextureTask
{
	AlbedoTextureTask(sf::String format, int resolution, int mipDrops)
		: MaterialTextureTask("Albedo", "albedo", format, resolution, mipDrops)
	{
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (endsWithStrip(ti.key, path, "_BaseColor.png")) {
			ti.inputs[s_albedo] = path;
		} else if (endsWithStrip(ti.key, path, "_Base_Color.png")) {
			ti.inputs[s_albedo] = path;
		} else {
			return false;
		}
		postAddInput(ti);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		addCommonTexcompArgs(p, ti, args);
		addTexcompInputArg(p, args, ti.inputs[s_albedo]);

		args.push("--output-ignores-alpha");

		pushJobs(p, ti, std::move(args));
	}
};

struct NormalTextureTask : MaterialTextureTask
{
	bool remap;

	NormalTextureTask(sf::String format, bool remap, int resolution, int mipDrops)
		: MaterialTextureTask("Normal", "normal", format, resolution, mipDrops)
		, remap(remap)
	{
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (endsWithStrip(ti.key, path, "_Normal.png")) {
			ti.inputs[s_normal] = path;
		} else if (endsWithStrip(ti.key, path, "_Normal_OpenGL.png")) {
			ti.inputs[s_normal_gl] = path;
		} else if (endsWithStrip(ti.key, path, "_Normal_DirectX.png")) {
			ti.inputs[s_normal_dx] = path;
		} else {
			return false;
		}
		postAddInput(ti);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		addCommonTexcompArgs(p, ti, args);

		args.push("--linear");
		args.push("--normal-map");
		if (remap) args.push("--decorrelate-remap");

		if (sf::Symbol *dx = ti.inputs.findValue(s_normal_dx)) {
			args.push("--invert-g");
			addTexcompInputArg(p, args, *dx);
		} else if (sf::Symbol *gl = ti.inputs.findValue(s_normal_gl)) {
			addTexcompInputArg(p, args, *gl);
		} else {
			addTexcompInputArg(p, args, ti.inputs[s_normal]);
		}

		pushJobs(p, ti, std::move(args));
	}
};

struct MaskTextureTask : MaterialTextureTask
{
	MaskTextureTask(sf::String format, int resolution, int mipDrops)
		: MaterialTextureTask("Mask", "mask", format, resolution, mipDrops)
	{
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (endsWithStrip(ti.key, path, "_Metallic.png")) {
			ti.inputs[s_metallic] = path;
		} else if (endsWithStrip(ti.key, path, "_Mixed_AO.png")) {
			ti.inputs[s_ao] = path;
		} else if (endsWithStrip(ti.key, path, "_Roughness.png")) {
			ti.inputs[s_roughness] = path;
		} else if (endsWithStrip(ti.key, path, "_Emissive.png")) {
			ti.inputs[s_emissive] = path;
		} else {
			return false;
		}
		postAddInput(ti);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		addCommonTexcompArgs(p, ti, args);

		args.push("--linear");

		if (auto pair = ti.inputs.find(s_metallic)) {
			sf::SmallStringBuf<512> path;
			sf::appendPath(path, p.dataRoot, pair->val);
			args.push("--input-r");
			args.push(path);
		}

		if (auto pair = ti.inputs.find(s_ao)) {
			sf::SmallStringBuf<512> path;
			sf::appendPath(path, p.dataRoot, pair->val);
			args.push("--input-g");
			args.push(path);
		} else {
			sf::SmallStringBuf<512> path;
			sf::appendPath(path, p.dataRoot, "Utility", "White.png");
			args.push("--input-g");
			args.push(path);
		}

		if (auto pair = ti.inputs.find(s_emissive)) {
			sf::SmallStringBuf<512> path;
			sf::appendPath(path, p.dataRoot, pair->val);
			args.push("--input-b");
			args.push(path);
		}

		if (auto pair = ti.inputs.find(s_roughness)) {
			sf::SmallStringBuf<512> path;
			sf::appendPath(path, p.dataRoot, pair->val);
			args.push("--input-a");
			args.push(path);
		}

		pushJobs(p, ti, std::move(args));
	}
};

struct GIAlbedoTextureTask : Task
{
	sf::Symbol format;
	int resolution;
	sf::SmallStringBuf<16> resolutionString;

	GIAlbedoTextureTask(sf::String format, int resolution)
		: format(format), resolution(resolution)
	{
		name.append("GIAlbedoTextureTask ", format);
		resolutionString.format("%d", resolution);
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (sf::endsWith(path, "_GI_Color.png")) {
			ti.inputs[s_albedo] = path;
		} else {
			return false;
		}
		ti.outputs[s_dst] = symf("%s.%s.sptex", path.data, format.data);
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--format");
		args.push(sf::String(format));

		args.push("--flip-y");

		args.push("--resolution");
		args.push(resolutionString);
		args.push(resolutionString);

		sf::StringBuf tempPattern;
		sf::appendPath(tempPattern, p.tempRoot, ti.outputs[s_dst]);

		args.push("--output");
		args.push(tempPattern);

		sf::StringBuf path;
		sf::appendPath(path, p.dataRoot, ti.inputs[s_albedo]);
		args.push("--input");
		args.push(std::move(path));

		args.push("--output-ignores-alpha");

		JobQueue jq;
		{
			sf::SmallStringBuf<256> tempFile, dstFile;
			sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
			sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);
			jq.mkdirsToFile(tempFile);
			jq.mkdirsToFile(dstFile);
		}
		jq.exec("sp-texcomp", std::move(args));
		{
			sf::SmallStringBuf<256> tempFile, dstFile;
			sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
			sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);
			jq.move(tempFile, dstFile);
		}

		JobPriority priority = getPriorityForTextureFormat(format);
		p.addJobs(priority, ti, jq);
	}
};


struct AnimationTask : Task
{
	AnimationTask()
	{
		name = "AnimatationTask";
		tools.push("sp-model");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (sf::endsWith(path, ".fbx") && sf::contains(path, "_anim_")) {
			ti.inputs[s_src] = path;
		} else {
			return false;
		}
		ti.outputs[s_dst] = symf("%s.spanim", path.data);
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf srcFile, tempFile, dstFile;
		sf::appendPath(srcFile, p.dataRoot, ti.inputs[s_src]);
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--vertex");
		args.push("pos_rgb32f,bonei_rgba8u,bonew_rgba8");

		args.push("--anim");

		args.push("--retain-prefix");
		args.push("bnd_");

		args.push("--remove-namespaces");

		args.push("--input");
		args.push(srcFile);

		args.push("--output");
		args.push(tempFile);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-model", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(JobPriority::Normal, ti, jq);
	}
};

struct CharacterModelTask : Task
{
	CharacterModelTask()
	{
		name = "CharacterModelTask";
		tools.push("sp-model");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (sf::endsWith(path, ".fbx") && sf::contains(path, "_character")) {
			ti.inputs[s_src] = path;
		} else {
			return false;
		}
		ti.outputs[s_dst] = symf("%s.spmdl", path.data);
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf srcFile, tempFile, dstFile;
		sf::appendPath(srcFile, p.dataRoot, ti.inputs[s_src]);
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--vertex");
		args.push("pos_rgb32f,uv_rg32f,nrm_rgb16sn,pad_r16sn,tan_rgba16sn,bonei_rgba8u,bonew_rgba8");

		args.push("--combine-materials");
		args.push("--mesh");

		args.push("--input");
		args.push(srcFile);

		args.push("--output");
		args.push(tempFile);

		args.push("--retain-prefix");
		args.push("bnd_");

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-model", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(JobPriority::Normal, ti, jq);
	}
};

struct CharacterModelGITask : Task
{
	CharacterModelGITask()
	{
		name = "CharacterModelGITask";
		tools.push("sp-model");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (sf::endsWith(path, ".fbx") && sf::contains(path, "_gicharacter")) {
			ti.inputs[s_src] = path;
		} else {
			return false;
		}
		ti.outputs[s_dst] = symf("%s.spmdl", path.data);
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf srcFile, tempFile, dstFile;
		sf::appendPath(srcFile, p.dataRoot, ti.inputs[s_src]);
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--vertex");
		args.push("pos_rgb32f,nrm_rgb32f,uv_rg32f,bonei_rgba8u,bonew_rgba8");

		args.push("--combine-materials");
		args.push("--mesh");

		args.push("--input");
		args.push(srcFile);

		args.push("--output");
		args.push(tempFile);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-model", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(JobPriority::Normal, ti, jq);
	}
};

struct TileModelTask : Task
{
	TileModelTask()
	{
		name = "TileModelTask";
		tools.push("sp-model");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (sf::endsWith(path, ".fbx") && sf::containsDirectory(path, "Tiles")) {
			ti.inputs[s_src] = path;
		} else {
			return false;
		}
		ti.outputs[s_dst] = symf("%s.spmdl", path.data);
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf srcFile, tempFile, dstFile;
		sf::appendPath(srcFile, p.dataRoot, ti.inputs[s_src]);
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--vertex");
		args.push("pos_rgb32f,nrm_rgb32f,tan_rgba32f,uv_rg32f");

		args.push("--transform-to-root");
		args.push("--mesh");

		args.push("--bvh");

		args.push("--input");
		args.push(srcFile);

		args.push("--output");
		args.push(tempFile);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-model", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(JobPriority::Normal, ti, jq);
	}
};

struct ObjectModelTask : Task
{
	ObjectModelTask()
	{
		name = "ObjectModelTask";
		tools.push("sp-model");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (sf::endsWith(path, ".fbx") && sf::containsDirectory(path, "Objects", 1)) {
			ti.inputs[s_src] = path;
		} else {
			return false;
		}
		ti.outputs[s_dst] = symf("%s.spmdl", path.data);
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf srcFile, tempFile, dstFile;
		sf::appendPath(srcFile, p.dataRoot, ti.inputs[s_src]);
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		args.push("--vertex");
		args.push("pos_rgb32f,nrm_rgba16sn,tan_rgba16sn,uv_rg32f");

		args.push("--combine-materials");
		args.push("--transform-to-root");
		args.push("--mesh");

		args.push("--bvh");

		args.push("--input");
		args.push(srcFile);

		args.push("--output");
		args.push(tempFile);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-model", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(JobPriority::Normal, ti, jq);
	}
};

struct FontTask : Task
{
	FontTask()
	{
		name = "FontTask";
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (sf::endsWith(path, ".ttf")) {
			ti.inputs[s_src] = path;
		} else {
			return false;
		}
		ti.outputs[s_dst] = ti.inputs[s_src];
		ti.assets.insert(path);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf srcFile, tempFile, dstFile;
		sf::appendPath(srcFile, p.dataRoot, ti.inputs[s_src]);
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.copy(srcFile, tempFile);
		jq.move(tempFile, dstFile);
		p.addJobs(JobPriority::Normal, ti, jq);
	}
};

struct SoundTask : Task
{
	SoundTask()
	{
		name = "SoundTask";
		tools.push("sp-sound");
	}

	virtual bool addInput(TaskInstance &ti, const sf::Symbol &path) 
	{
		if (path.size() < 4) return false;
		size_t endIx = sf::indexOf(path, "_alt_");
		if (endIx == SIZE_MAX) {
			endIx = path.size() - 4;
		}
		if (sf::endsWith(path, ".ogg") || sf::endsWith(path, ".wav")) {
			sf::Symbol alt = sf::Symbol(path.data + endIx, path.size() - endIx);
			ti.inputs[alt] = path;
		} else {
			return false;
		}

		sf::Symbol baseName = sf::Symbol(path.data, endIx);
		ti.outputs[s_dst] = symf("%s.spsnd", baseName.data);
		ti.assets.insert(baseName);
		return true;
	}

	virtual void process(Processor &p, TaskInstance &ti)
	{
		sf::Array<sf::StringBuf> args;

		sf::StringBuf tempFile, dstFile;
		sf::appendPath(tempFile, p.tempRoot, ti.outputs[s_dst]);
		sf::appendPath(dstFile, p.buildRoot, ti.outputs[s_dst]);

		args.push("--level");
		args.push().format("%d", p.level);

		for (auto &pair : ti.inputs) {
			sf::StringBuf srcFile;
			sf::appendPath(srcFile, p.dataRoot, pair.val);
			args.push("-i");
			args.push(srcFile);
		}

		args.push("--output");
		args.push(tempFile);

		JobQueue jq;
		jq.mkdirsToFile(tempFile);
		jq.mkdirsToFile(dstFile);
		jq.exec("sp-sound", std::move(args));
		jq.move(tempFile, dstFile);
		p.addJobs(JobPriority::Normal, ti, jq);
	}
};

Processor g_processor;

static void findResourcesImp(Processor &p, sf::String root, sf::StringBuf &prefix)
{
	sf::Array<sf::FileInfo> files;
	files.reserve(128);

	{
		sf::SmallStringBuf<256> dir;
		sf::appendPath(dir, root, prefix);
		sf::listFiles(dir, files);
	}

	for (sf::FileInfo &file : files) {
		if (file.isDirectory) {
			uint32_t len = prefix.size;
			sf::appendPath(prefix, file.name);
			findResourcesImp(p, root, prefix);
			prefix.resize(len);
			continue;
		}

		sf::StringBuf path;
		sf::appendPath(path, prefix, file.name);
		p.addInputFile(sf::Symbol(path));
	}
}

void initializeProcessing(const ProcessingDesc &desc)
{
	Processor &p = g_processor;

	// TODO: -j argument
	#if defined(SP_DEDICATED_PROCESSOR)
		p.maxActiveJobs = sf::max(1u, (uint32_t)std::thread::hardware_concurrency() / 2);
	#else
		p.maxActiveJobs = sf::max(1u, (uint32_t)std::thread::hardware_concurrency() / 4);
	#endif

	if (desc.threads > 0) p.maxActiveJobs = (uint32_t)desc.threads;

	p.level = desc.level;

	sf::appendPath(p.dataRoot, "Assets");
	sf::appendPath(p.tempRoot, "Temp");
	sf::appendPath(p.buildRoot, "Build");
	sf::appendPath(p.toolRoot, "Tools");
#if SF_OS_WINDOWS
	sf::appendPath(p.toolRoot, "win32");
#elif SF_OS_APPLE
	sf::appendPath(p.toolRoot, "macos");
#else
	sf::appendPath(p.toolRoot, "linux");
#endif

	bool doAstc = true;
	bool doRgba = true;
	bool doBc3Normal = true;

#if SF_OS_WINDOWS
	if (desc.localProcessing) {
		doAstc = false;
		doRgba = false;
		doBc3Normal = false;
	}
#endif

	int materialResolution = 1024;
	int materialMips = 4;
	p.tasks.push(sf::box<AlbedoTextureTask>("bc1", materialResolution, materialMips));
	p.tasks.push(sf::box<AlbedoTextureTask>("bc7", materialResolution, materialMips));
	if (doAstc) p.tasks.push(sf::box<AlbedoTextureTask>("astc4x4", materialResolution, materialMips));
	if (doRgba) p.tasks.push(sf::box<AlbedoTextureTask>("rgba8", materialResolution, materialMips));

	p.tasks.push(sf::box<NormalTextureTask>("bc5", false, materialResolution, materialMips));
	if (doBc3Normal) p.tasks.push(sf::box<NormalTextureTask>("bc3", true, materialResolution, materialMips));
	if (doAstc) p.tasks.push(sf::box<NormalTextureTask>("astc4x4", true, materialResolution, materialMips));
	if (doRgba) p.tasks.push(sf::box<NormalTextureTask>("rgba8", false, materialResolution, materialMips));

	p.tasks.push(sf::box<MaskTextureTask>("bc3", materialResolution, materialMips));
	if (doAstc) p.tasks.push(sf::box<MaskTextureTask>("astc8x8", materialResolution, materialMips));
	if (doRgba) p.tasks.push(sf::box<MaskTextureTask>("rgba8", materialResolution, materialMips));

	int giMaterialResolution = 32;
	// p.tasks.push(sf::box<GIAlbedoTextureTask>("bc1", giMaterialResolution));
	// p.tasks.push(sf::box<GIAlbedoTextureTask>("astc8x8", giMaterialResolution));
	p.tasks.push(sf::box<GIAlbedoTextureTask>("rgba8", giMaterialResolution));

	int maxGuiExtent = 512;
	int maxCardExtent = 256;
	int maxIconExtent = 256;
	int maxBillboardExtent = 512;
	p.tasks.push(sf::box<GuiTextureTask>("rgba8", "Gui", maxGuiExtent));
	p.tasks.push(sf::box<GuiTextureTask>("rgba8", "Cards", maxCardExtent));
	p.tasks.push(sf::box<GuiTextureTask>("rgba8", "Character_Icons", maxCardExtent));
	p.tasks.push(sf::box<GuiTextureTask>("rgba8", "Billboards", maxBillboardExtent));

	int maxParticleExtent = 1024;
	p.tasks.push(sf::box<ParticleTextureTask>("bc3", "Particles", maxParticleExtent));
	if (doAstc) p.tasks.push(sf::box<ParticleTextureTask>("astc4x4", "Particles", maxParticleExtent));
	if (doRgba) p.tasks.push(sf::box<ParticleTextureTask>("rgba8", "Particles", maxParticleExtent));

	int envmapExtent = 128;
	p.tasks.push(sf::box<EnvmapTask>("r11g11b10f", "Envmaps", envmapExtent));

	p.tasks.push(sf::box<MiscTextureTask>("rgba8", "Misc_RGBA"));

	p.tasks.push(sf::box<AnimationTask>());
	p.tasks.push(sf::box<CharacterModelTask>());
	p.tasks.push(sf::box<CharacterModelGITask>());
	p.tasks.push(sf::box<TileModelTask>());
	p.tasks.push(sf::box<ObjectModelTask>());

	p.tasks.push(sf::box<SoundTask>());

	p.tasks.push(sf::box<FontTask>());

	p.dataMonitor.begin(p.dataRoot);

	sf::SmallStringBuf<256> prefix;
	findResourcesImp(p, p.dataRoot, prefix);

	// Check for files that need to be updated
	for (auto &pair : p.taskInstances) {
		sf::Box<TaskInstance> ti = pair.val;
		uint64_t oldestOutput = UINT64_MAX;
		uint64_t newestInput = 0;

		for (auto &tool : ti->task->tools) {
			sf::SmallStringBuf<1024> path;
			sf::appendPath(path, p.toolRoot, tool);
			#if SF_OS_WINDOWS
				path.append(".exe");
			#endif
			uint64_t ts = sf::getFileTimestamp(path);
			if (ts) newestInput = sf::max(newestInput, ts);
		}

		for (auto &input : ti->inputs) {
			sf::SmallStringBuf<1024> path;
			sf::appendPath(path, p.dataRoot, input.val);
			uint64_t ts = sf::getFileTimestamp(path);
			if (ts) newestInput = sf::max(newestInput, ts);
		}

		for (auto &output : ti->outputs) {
			sf::SmallStringBuf<1024> path;
			sf::appendPath(path, p.buildRoot, output.val);
			uint64_t ts = sf::getFileTimestamp(path);
			if (ts) oldestOutput = sf::min(oldestOutput, ts);
		}

		if (newestInput > oldestOutput || oldestOutput == UINT64_MAX) {
			ti->dirty = true;
			p.dirtyTaskInstances.push(ti);
		}
	}
}

void closeProcessing()
{
	g_processor.dataMonitor.end();
}

bool updateProcessing()
{
	Processor &p = g_processor;

	p.updateTasks();
	p.updateJobs();

	uint64_t now = stm_now();
	sf::SmallArray<sf::StringBuf, 128> updates;
	p.dataMonitor.getUpdates(updates);
	for (sf::StringBuf &update : updates) {
		sf::Symbol name = sf::Symbol(update);

		sf::SmallStringBuf<512> path;
		sf::appendPath(path, p.dataRoot, update);
		if (sf::fileExists(path)) {
			p.addInputFile(name);
		}

		auto tasks = p.tasksForInput.find(name);
		if (tasks) {
			for (sf::Box<TaskInstance> &ti : tasks->val) {
				ti->lastMonitoredUpdateStm = now;
				if (!ti->dirty) {
					ti->dirty = true;
					p.dirtyTaskInstances.push(ti);
				}
			}
		}
	}

	if (p.dirtyTaskInstances.size > 0 || p.activeJobs.size > 0) {
		return true;
	}

	if (p.assetsToReload.size() > 0) {
		sf::Array<sf::Symbol> fixedNames;
		sf::SmallStringBuf<1024> fixed;
		for (const sf::Symbol &name : p.assetsToReload) {
			fixed.clear();
			fixed.append("Assets/");
			for (size_t i = 0; i < name.size(); i++) {
				char c = name.data[i];
				fixed.append(c == '\\' ? '/' : c);
			}
			fixedNames.push(sf::Symbol(fixed));
		}

		sp::Asset::reloadAssetsByName(fixedNames);
		p.assetsToReload.clear();
	}

	p.processingAssets.clear();

	return false;
}

void queryProcessingAssets(sf::Array<ProcessingAsset> &assets)
{
	Processor &p = g_processor;

	for (auto &pair : p.processingAssets) {
		if (pair.val.tasksDone == pair.val.tasksPending) continue;
		ProcessingAsset &asset = assets.push();
		asset.name = pair.key;
		asset.tasksPending = pair.val.tasksPending;
		asset.tasksDone = pair.val.tasksDone;
	}
}

