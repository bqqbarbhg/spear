import os
import subprocess
import sys
import shutil
import time
import re

src_dir = os.path.dirname(os.path.abspath(__file__))
src_dir = os.path.join(src_dir, "..", "src")
src_dir = os.path.abspath(src_dir)

objects = []
processes = []

optimize = "-o" in sys.argv or "-O" in sys.argv
webgl2 = "--webgl2" in sys.argv
threads = "--threads" in sys.argv
clean = "--clean" in sys.argv
jobs = "-j" in sys.argv
profiling = "--profiling" in sys.argv

begin_time = time.time()

def compile_file(path, cpp):
    args = ["emcc", path, "-c", "-I" + src_dir]
    if optimize:
        args += ["-O2", "-DNDEBUG"]
    else:
        args += ["-g"]
    if cpp:
        args += ["-std=c++14"]
    if webgl2:
        args += ["-s", "MAX_WEBGL_VERSION=2"]
        args += ["-DSP_USE_WEBGL2=1"]
    if threads:
        args += ["-s", "USE_PTHREADS=1"]
    if profiling:
        args += ["--profiling"]

    outname = os.path.basename(path)
    outname = os.path.splitext(outname)[0] + ".o"

    if outname in objects:
        raise RuntimeError("Duplicated .o file: " + outname)

    objects.append(outname)

    try:
        o_time = os.path.getmtime(outname)
        c_time = os.path.getmtime(path)
        if o_time > c_time and not clean:
            if not jobs:
                print("Up to date: " + path)
            return
    except:
        pass

    if jobs:
        print(".",end='')
        sys.stdout.flush()
        p = subprocess.Popen(args, shell=True, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True)
        processes.append((path, p))
    else:
        print("$ " + " ".join(args))
        subprocess.check_call(args, shell=True)

def link_files():
    args = ["emcc"] + objects
    if optimize:
        args += ["-O2"]
    else:
        args += ["-g"]
    if webgl2:
        args += ["-s", "MAX_WEBGL_VERSION=2"]
    args += ["-s", "WASM=1", "-o" "spear.js"]
    args += ["-s", "TOTAL_MEMORY=268435456"]
    if threads:
        args += ["-s", "USE_PTHREADS=1"]
    if profiling:
        args += ["--profiling"]

    if not jobs:
        print("$ " + " ".join(args))
    else:
        print("  Linking\r", end="")

    subprocess.check_call(args, shell=True)

for root,dirs,files in os.walk(src_dir):
    for f in files:
        f = os.path.join(root, f)
        if f.endswith(".cpp"):
            compile_file(f, True)
        elif f.endswith(".c"):
            compile_file(f, False)

all_ok = True

RE_ERR_END = re.compile(r"\d+ errors generated")

if jobs:
    print()

    done_processes = set()
    print("  {}/{}\r".format(0, len(processes)), end="")

    while len(done_processes) < len(processes):
        for f, p in processes:
            code = p.poll()
            if code == None:
                continue
            if p in done_processes:
                continue
            done_processes.add(p)

            print("  {}/{}\r".format(len(done_processes), len(processes)), end="")

            if code != 0:
                err_str = str(p.stderr.read(), encoding="utf-8")
                m = RE_ERR_END.search(err_str)
                if m:
                    err_str = err_str[:m.start()]
                print("FAIL: " + f)
                print()
                print(err_str)
                all_ok = False

        time.sleep(0.1)

    if not processes:
        print("All files up to date")
        all_ok = False

if all_ok:
    build_end_time = time.time()

    link_files()

    link_end_time = time.time()
    print("Built in {:.1f} seconds, link took {:.1f} seocnds".format(
        link_end_time - begin_time, link_end_time - build_end_time))

