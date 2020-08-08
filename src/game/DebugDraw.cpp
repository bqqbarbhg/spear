#include "DebugDraw.h"

#include "sf/Mutex.h"
#include "sf/Array.h"

struct DebugDrawDataImp
{
	sf::Array<DebugLine> lines;
	sf::Array<DebugSphere> spheres;
};

struct DebugDrawThread
{
	sf::Mat34 transform;
	sf::SmallArray<sf::Mat34, 8> transformStack;
};

static sf::StaticMutex g_mutex;
static DebugDrawDataImp g_appendData, g_renderData;
thread_local DebugDrawThread *t_debugDrawData;

static sf::Mat34 getDebugDrawTransform()
{
	if (!t_debugDrawData) return { };
	return t_debugDrawData->transform;
}

void debugDrawPushTransform(const sf::Mat34 &transform)
{
	if (!t_debugDrawData) t_debugDrawData = new DebugDrawThread();
	t_debugDrawData->transformStack.push(t_debugDrawData->transform);
	t_debugDrawData->transform = t_debugDrawData->transform * transform;
}

void debugDrawPopTransform()
{
	if (!t_debugDrawData) t_debugDrawData = new DebugDrawThread();
	t_debugDrawData->transform = t_debugDrawData->transformStack.popValue();
}

void debugDrawLine(const DebugLine &line)
{
	sf::Mat34 transform = getDebugDrawTransform();
	sf::MutexGuard mg(g_mutex);
	DebugLine &dst = g_appendData.lines.push();
	dst.a = sf::transformPoint(transform, line.a);
	dst.b = sf::transformPoint(transform, line.b);
	dst.color = line.color;
}

void debugDrawLine(const sf::Vec3 &a, const sf::Vec3 &b, const sf::Vec3 &color)
{
	sf::Mat34 transform = getDebugDrawTransform();
	sf::MutexGuard mg(g_mutex);
	DebugLine &line = g_appendData.lines.push();
	line.a = sf::transformPoint(transform, a);
	line.b = sf::transformPoint(transform, b);
	line.color = color;
}

void debugDrawBox(const sf::Mat34 &boxTransform, const sf::Vec3 &color)
{
	sf::MutexGuard mg(g_mutex);

	sf::Mat34 transform = getDebugDrawTransform() * boxTransform;

	sf::Vec3 c000 = transform.cols[3] - transform.cols[0] - transform.cols[1] - transform.cols[2];
	sf::Vec3 c100 = transform.cols[3] + transform.cols[0] - transform.cols[1] - transform.cols[2];
	sf::Vec3 c010 = transform.cols[3] - transform.cols[0] + transform.cols[1] - transform.cols[2];
	sf::Vec3 c110 = transform.cols[3] + transform.cols[0] + transform.cols[1] - transform.cols[2];
	sf::Vec3 c001 = transform.cols[3] - transform.cols[0] - transform.cols[1] + transform.cols[2];
	sf::Vec3 c101 = transform.cols[3] + transform.cols[0] - transform.cols[1] + transform.cols[2];
	sf::Vec3 c011 = transform.cols[3] - transform.cols[0] + transform.cols[1] + transform.cols[2];
	sf::Vec3 c111 = transform.cols[3] + transform.cols[0] + transform.cols[1] + transform.cols[2];
	DebugLine *lines = g_appendData.lines.pushUninit(12);
	lines[ 0] = { c000, c100, color };
	lines[ 1] = { c100, c110, color };
	lines[ 2] = { c110, c010, color };
	lines[ 3] = { c010, c000, color };
	lines[ 4] = { c001, c101, color };
	lines[ 5] = { c101, c111, color };
	lines[ 6] = { c111, c011, color };
	lines[ 7] = { c011, c001, color };
	lines[ 8] = { c000, c001, color };
	lines[ 9] = { c100, c101, color };
	lines[10] = { c010, c011, color };
	lines[11] = { c110, c111, color };
}

void debugDrawBox(const sf::Bounds3 &bounds, const sf::Vec3 &color)
{
	debugDrawBox(
		sf::mat::translate(bounds.origin) *
		sf::mat::scale(bounds.extent), color);
}

void debugDrawSphere(const sf::Mat34 &transform, const sf::Vec3 &color)
{
	sf::MutexGuard mg(g_mutex);
	g_appendData.spheres.push({ getDebugDrawTransform() * transform, color });
}

void debugDrawSphere(const sf::Vec3 &origin, float radius, const sf::Vec3 &color)
{
	debugDrawSphere(sf::mat::translate(origin) * sf::mat::scale(radius), color);
}

void debugDrawSphere(const sf::Sphere &sphere, const sf::Vec3 &color)
{
	debugDrawSphere(sf::mat::translate(sphere.origin) * sf::mat::scale(sphere.radius), color);
}

void debugDrawFlipBuffers()
{
	sf::MutexGuard mg(g_mutex);
	sf::impSwap(g_appendData, g_renderData);
	g_appendData.lines.clear();
	g_appendData.spheres.clear();
}

DebugDrawData debugDrawGetData()
{
	sf::MutexGuard mg(g_mutex);
	DebugDrawData dd;
	dd.lines = g_renderData.lines;
	dd.spheres = g_renderData.spheres;
	return dd;
}
