import os
import subprocess
import sys
import shutil

self_path = os.path.dirname(os.path.abspath(__file__))
shdc_exe = os.path.join(self_path, "sokol-shdc.exe")
src_dir = os.path.join(self_path, "..", "src")
src_dir = os.path.abspath(src_dir)

objects = []

force = "-f" in sys.argv

def compile_file(path):
    outname = os.path.splitext(path)[0] + ".h"

    args = [shdc_exe, "-i", path, "-o", outname, "-b"]
    args += ["-l", "glsl100:glsl300es:glsl330:hlsl5:metal_macos:metal_ios"]
    args += ["-f", "sokol_impl"]

    try:
        o_time = os.path.getmtime(outname)
        c_time = os.path.getmtime(path)
        if o_time > c_time and not force:
            print("Up to date: " + path)
            return
    except:
        pass

    print("$ " + " ".join(args))
    subprocess.check_call(args, shell=True)

for root,dirs,files in os.walk(src_dir):
    for f in files:
        f = os.path.join(root, f)
        if f.endswith(".glsl"):
            compile_file(f)

