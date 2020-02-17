import os
import subprocess
import sys
import shutil

src_dir = os.path.dirname(os.path.abspath(__file__))
src_dir = os.path.join(src_dir, "..", "src")
src_dir = os.path.abspath(src_dir)

objects = []

optimize = "-o" in sys.argv

def compile_file(path, cpp):
    args = ["emcc", path, "-c", "-I" + src_dir]
    if optimize:
        args += ["-O2"]
    else:
        args += ["-g"]
    if cpp:
        args.append("-std=c++11")

    outname = os.path.basename(path)
    outname = os.path.splitext(outname)[0] + ".o"

    if outname in objects:
        raise RuntimeError("Duplicated .o file: " + outname)

    objects.append(outname)

    try:
        o_time = os.path.getmtime(outname)
        c_time = os.path.getmtime(path)
        if o_time > c_time:
            print("Up to date: " + path)
            return
    except:
        pass

    print("$ " + " ".join(args))
    subprocess.check_call(args, shell=True)

def link_files():
    args = ["emcc"] + objects
    if optimize:
        args += ["-O2"]
    else:
        args += ["-g"]
    args += ["-s", "WASM=1", "-o" "spear.js"]
    args += ["-s", "TOTAL_MEMORY=268435456"]

    print("$ " + " ".join(args))
    subprocess.check_call(args, shell=True)

for root,dirs,files in os.walk(src_dir):
    for f in files:
        f = os.path.join(root, f)
        if f.endswith(".cpp"):
            compile_file(f, True)
        elif f.endswith(".c"):
            compile_file(f, False)

link_files()

