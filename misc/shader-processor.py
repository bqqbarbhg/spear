import pcpp
import zstd
import re
from collections import namedtuple
import io
import os
import subprocess
import sys

tmp_name = "__sp__temp__"
tmp_include = "__sp__temp__include__"

top_directives = []
stmt_directives = []

class directive(object):
    def __init__(self, regex):
        self.regex = re.compile(regex)

    def __call__(self, func):
        self.func = func

class top_directive(directive):
    def __init__(self, regex):
        super().__init__(regex)
        top_directives.append(self)

class stmt_directive(directive):
    def __init__(self, regex):
        super().__init__(regex)
        stmt_directives.append(self)

Type = namedtuple("Type", "name size align, hlsl")
types = [
    Type("float", 1, 1, "float"),
    Type("vec2", 2, 2, "float2"),
    Type("vec3", 3, 4, "float3"),
    Type("vec4", 4, 4, "float4"),
    Type("uint", 1, 1, "uint"),
    Type("uvec2", 2, 2, "uint2"),
    Type("uvec3", 3, 4, "uint3"),
    Type("uvec4", 4, 4, "uint4"),
    Type("int", 1, 1, "int"),
    Type("ivec2", 2, 2, "int2"),
    Type("ivec3", 3, 4, "int3"),
    Type("ivec4", 4, 4, "int4"),
    Type("mat4", 16, 4, "float4x4"),
]
name_to_type = { t.name: t for t in types }

Attrib = namedtuple("Attribute", "location type name")
Varying = namedtuple("Varying", "interp type name")
Output = namedtuple("Output", "type name")
Sampler = namedtuple("Sampler", "type name")
Uniform = namedtuple("Uniform", "type name num")
UniformBlock = namedtuple("UniformBlock", "name uniforms size")
Permutation = namedtuple("Permutation", "name value")
Reflection = namedtuple("Reflection", "permutations uniforms")

class GLSL:

    def __init__(self, version):
        self.version = version

    def get_extension(self):
        return "glsl"

    def get_defines(self):
        return { "SP_GLSL": "1" }

    def do_header(self, c):
        lines = []

        lines.append("#version {}".format(self.version))
        lines.append("")
        lines.append("#define mul(a, b) ((b) * (a))")
        lines.append("#define lerp(a, b, t) mix(a, b, t)")
        lines.append("#define saturate(a) clamp(a, 0.0, 1.0)")
        lines.append("#define asVec2(a) vec2(a)")
        lines.append("#define asVec3(a) vec3(a)")
        lines.append("#define asVec4(a) vec4(a)")

        if self.version.endswith("es"):
            lines.append("precision mediump float;")
            if any(s.type == "sampler2DArray" for s in c.samplers):
                lines.append("precision lowp sampler2DArray;")
            if any(s.type == "sampler3D" for s in c.samplers):
                lines.append("precision lowp sampler3D;")

        if c.vert:
            if c.attribs:
                lines.append("")

            for a in c.attribs:
                lines.append("layout(location={}) in {} {};".format(a.location, a.type.name, a.name))

        if c.varyings:
            lines.append("")

        for ix, v in enumerate(c.varyings):
            interp = v.interp + " " if v.interp else ""
            kw = "in" if c.frag else "out"
            if self.version == "450":
                lines.append("layout(location={}) {}{} {} {};".format(ix, interp, kw, v.type.name, v.name))
            else:
                lines.append("{}{} {} {};".format(interp, kw, v.type.name, v.name))

        if c.frag:
            lines.append("")

            for ix, o in enumerate(c.outputs):
                lines.append("layout(location={}) out {} {};".format(ix, o.type.name, o.name))

        if c.samplers:
            lines.append("")

        for ix, s in enumerate(c.samplers):
            if self.version == "450":
                lines.append("layout(binding={}) uniform {} {};".format(ix, s.type, s.name))
            else:
                lines.append("uniform {} {};".format(s.type, s.name))

        for ix, block in enumerate(c.uniform_blocks):
            lines.append("")
            if self.version == "450":
                lines.append("layout(binding={}) layout(std140) uniform {} {{".format(ix, block.name))
            else:
                lines.append("layout(std140) uniform {} {{".format(block.name))
            offset = 0
            pad = 0
            for u in block.uniforms:
                while offset % u.type.align != 0:
                    lines.append("\tfloat sp_{}_pad{};".format(block.name, pad))
                    pad += 1
                    offset += 1
                if u.num:
                    lines.append("\t{} {}[{}];".format(u.type.name, u.name, u.num))
                    offset += u.type.size * u.num
                else:
                    lines.append("\t{} {};".format(u.type.name, u.name))
                    offset += u.type.size

            lines.append("};")

        lines.append("")
        return lines

    def do_main(self, c):
        return "void main() {"

    def do_main_end(self, c):
        return "}"

    def do_gl_position(self, c, indent, expr):
        if c.glsl_flip:
            return [
                "{}vec4 sp_tmpFlip = {};".format(indent, expr),
                "{}sp_tmpFlip.y = -sp_tmpFlip.y;".format(indent),
                "{}gl_Position = sp_tmpFlip;".format(indent),
                "",
            ]
        else:
            return indent + "gl_Position = " + expr + ";"

    def do_gl_frag_color(self, c, indent, expr):
        return indent + "sp_fragColor = " + expr + ";"

class HLSL:

    def __init__(self, version):
        self.version = version

    def get_extension(self):
        return "hlsl"

    def get_defines(self):
        return { "SP_HLSL": "1" }

    def do_header(self, c):
        lines = []

        for t in types:
            if t.hlsl != t.name:
                lines.append("#define {} {}".format(t.name, t.hlsl))

        lines.append("#define asVec2(a) (a)")
        lines.append("#define asVec3(a) (a)")
        lines.append("#define asVec4(a) (a)")
        lines.append("#define texture(s, p) s.Sample(s##_spSampler, p)")
        lines.append("#define textureLod(s, p, l) s.SampleLevel(s##_spSampler, p, l)")

        interp_map = {
            "smooth": "linear",
            "flat": "nointerpolation",
            "noperspective": "noperspective",
        }

        sampler_map = {
            "sampler2D": "Texture2D<float4>",
            "sampler2DArray": "Texture2DArray<float4>",
            "sampler3D": "Texture3D<float4>",
            "samplerCube": "TextureCube<float4>",
        }

        if c.vert:
            lines.append("")
            lines.append("struct SP_Attribs {")
            for a in c.attribs:
                lines.append("\t{} {}{} : TEXCOORD{};".format(a.type.name, tmp_name, a.name, a.location))
            lines.append("};")

            lines.append("")
            lines.append("struct SP_Varyings {")
            loc = 0
            for v in c.varyings:
                interp = interp_map[v.interp] + " " if v.interp else ""
                lines.append("\t{}{} {}{} : TEXCOORD{};".format(interp, v.type.name, tmp_name, v.name, loc))
                loc += 1
            lines.append("\tvec4 sp_vertexPosition : SV_Position;")
            lines.append("};")

            lines.append("")
            for a in c.attribs:
                lines.append("#define {} sp_in.{}{}".format(a.name, tmp_name, a.name))
            for a in c.varyings:
                lines.append("#define {} sp_out.{}{}".format(a.name, tmp_name, a.name))
        else:
            lines.append("")
            lines.append("struct SP_Varyings {")
            loc = 0
            for v in c.varyings:
                interp = interp_map[v.interp] + " " if v.interp else ""
                lines.append("\t{}{} {}{} : TEXCOORD{};".format(interp, v.type.name, tmp_name, v.name, loc))
                loc += 1
            lines.append("};")

            lines.append("")
            for a in c.varyings:
                lines.append("#define {} sp_in.{}{}".format(a.name, tmp_name, a.name))

            lines.append("")
            lines.append("struct SP_Targets {")
            for ix, o in enumerate(c.outputs):
                lines.append("\t{} {} : SV_Target{};".format(o.type.name, o.name, ix))
            lines.append("};")

            for a in c.outputs:
                lines.append("#define {} sp_out.{}{}".format(a.name, tmp_name, a.name))

        for ix, s in enumerate(c.samplers):
            typ = sampler_map[s.type]
            lines.append("{} {} : register(t{});".format(typ, s.name, ix))
            lines.append("SamplerState {}_spSampler : register(s{});".format(s.name, ix))

        for ix, block in enumerate(c.uniform_blocks):
            lines.append("")
            lines.append("cbuffer {} : register(b{}) {{".format(block.name, ix))
            offset = 0
            pad = 0
            for u in block.uniforms:
                while offset % u.type.align != 0:
                    lines.append("\tfloat sp_{}_pad{};".format(block.name, pad))
                    pad += 1
                    offset += 1

                type_name = u.type.name
                if type_name == "mat4": type_name = "row_major float4x4 "

                if u.num:
                    lines.append("\t{} {}[{}];".format(type_name, u.name, u.num))
                    offset += u.type.size * u.num
                else:
                    lines.append("\t{} {};".format(type_name, u.name))
                    offset += u.type.size

            lines.append("};")

        lines.append("")
        return lines

    def do_main(self, c):
        if c.vert:
            return [
                "SP_Varyings main(SP_Attribs sp_in) {",
                "\tSP_Varyings sp_out;",
            ]
        else:
            return [
                "SP_Targets main(SP_Varyings sp_in) {",
                "\tSP_Targets sp_out;",
            ]

    def do_main_end(self, c):
        return [
            "\treturn sp_out;",
            "}",
        ]

    def do_gl_position(self, c, indent, expr):
        return indent + "sp_out.sp_vertexPosition  = " + expr + ";"

    def do_gl_frag_color(self, c, indent, expr):
        return indent + "sp_out.sp_fragColor = " + expr + ";"

re_uniform = re.compile(r"\s*(\w+)\s*(\w+)(?:\[(.*)])?\s*;?\s*")

def get_ubo_size(uniforms):
    offset = 0
    for u in uniforms:
        while offset % u.type.align != 0:
            offset += 1
        if u.num:
            offset += u.type.size * u.num
        else:
            offset += u.type.size
    return offset

class Compiler:
    def __init__(self, lang, frag):
        self.uniform_blocks = []
        self.attribs = []
        self.varyings = []
        self.outputs = []
        self.uniform_name = ""
        self.current_uniforms = []
        self.in_main = False
        self.samplers = []
        self.lang = lang
        self.glsl_flip = False
        self.vert = not frag
        self.frag = frag
        self.skip_block = False

    def do_line(self, line):
        if self.uniform_name:
            m = re_uniform.match(line)
            if m:
                typ, name, num = m.groups()
                if num is not None: num = int(eval(num))
                self.current_uniforms.append(Uniform(name_to_type[typ], name, num))
            if line.strip() == "};":
                self.uniform_blocks.append(UniformBlock(self.uniform_name, tuple(self.current_uniforms), get_ubo_size(self.current_uniforms)))
                self.uniform_name = ""
                self.current_uniforms = []
            return []
        elif self.in_main:
            if self.skip_block:
                if line.strip() == "{": self.skip_block = False
                return []
            for d in stmt_directives:
                m = d.regex.match(line)
                if not m: continue
                return d.func(self, *m.groups())
            if line == "}":
                self.in_main = False
                return self.lang.do_main_end(self)
        else:
            for d in top_directives:
                m = d.regex.match(line)
                if not m: continue
                return d.func(self, *m.groups())

@top_directive(r"\s*(?:layout\s*\(\s*location\s*=\s*(\d+)\s*\))?\s*attribute\s+(\w+)\s+(\w+)\s*;?\s*")
def do_attribute(c, location, typ, name):
    c.attribs.append(Attrib(location, name_to_type[typ], name))
    return []

@top_directive(r"\s*(smooth|flat|noperspective|)\s*varying\s+(\w+)\s+(\w+)\s*;?\s*")
def do_varying(c, interp, typ, name):
    c.varyings.append(Varying(interp, name_to_type[typ], name))
    return []

@top_directive(r"\s*out\s+(\w+)\s+(\w+)\s*;?\s*")
def do_out(c, typ, name):
    c.outputs.append(Output(name_to_type[typ], name))
    return []

@top_directive(r"\s*uniform\s+(sampler2D|sampler3D|samplerCube|sampler2DArray)\s+(\w+)\s*;\s*")
def do_uniform_texture(c, typ, name):
    c.samplers.append(Sampler(typ, name))
    return []

@top_directive(r"\s*uniform\s+(\w+)\s*")
def do_uniform_block(c, name):
    c.uniform_name = name
    return []

@top_directive(r"\s*void\s+main\s*\(\s*\)\s*")
def do_main(c):
    c.in_main = True
    c.skip_block = True
    return c.lang.do_main(c)

@top_directive(r"\s*#pragma\s+glsl_flip\s*")
def do_pragma_glsl_flip(c):
    c.glsl_flip = True
    return []

@top_directive(r"\s*#pragma\s+permutation.*")
def do_pragma_permutation(c):
    return []

@stmt_directive(r"(\s*)gl_Position\s+=\s+(.*);\s*")
def do_gl_position(c, indent, expr):
    return c.lang.do_gl_position(c, indent, expr)

@stmt_directive(r"(\s*)gl_FragColor\s+=\s+(.*);\s*")
def do_gl_frag_color(c, indent, expr):
    return c.lang.do_gl_frag_color(c, indent, expr)

class Preprocessor(pcpp.Preprocessor):
    def __init__(self, ignore_unknown):
        super().__init__()
        self.ignore_unknown = ignore_unknown

    def on_directive_unknown(self, directive, toks, ifpassthru, precedingtoks):
        if self.ignore_unknown:
            return None
        else:
            return super().on_directive_unknown(directive, toks, ifpassthru, precedingtoks)

def preprocess(src, name=None, defines={}, includes=[], ignore_unknown=False):
    pp = Preprocessor(ignore_unknown)
    for name, value in defines.items():
        pp.define(name + " " + value)
    for path in includes:
        pp.add_path(path)
    pp.parse(src)
    f = io.StringIO()
    pp.write(f)
    return f.getvalue()

self_path = os.path.dirname(os.path.abspath(__file__))
g_root = os.path.join(self_path, "..", "src", "game", "shader2")
g_temp = os.path.join(self_path, "..", "temp", "shaders")
if sys.platform == "win32":
    g_tool_root = os.path.join(self_path, "..", "tool", "win32")
elif sys.platform == "darwin":
    g_tool_root = os.path.join(self_path, "..", "tool", "macos")
else:
    g_tool_root = os.path.join(self_path, "..", "tool", "linux")

re_permutation = re.compile(r"\s*#pragma\s+permutation\s+(\w+)\s+(\w+)\s*")

class Shader:
    def __init__(self, filename):
        self.filename = filename
        self.name = os.path.splitext(os.path.basename(filename))[0]
        self.path = os.path.join(g_root, "shader", filename)

    def base_defines_includes(self, lang, frag):
        defines = { }
        includes = [ ]
        if frag:
            defines["SP_FS"] = "1"
        else:
            defines["SP_VS"] = "1"
        includes.append(g_root)
        return defines, includes

    def get_permutations(self, lang, frag):
        with open(self.path) as f:
            main_source = f.read()

        permutations = []

        defines, includes = self.base_defines_includes(lang, frag)
        defines.update(lang.get_defines())
        src = preprocess(main_source, self.path, defines=defines, includes=includes, ignore_unknown=True)
        for line in src.splitlines():
            m = re_permutation.match(line)
            if m:
                name, num = m.groups()
                permutations.append(Permutation(name, int(eval(num))))
        return permutations

    def compile(self, lang, frag, permutations):
        with open(self.path) as f:
            main_source = f.read()
        defines, includes = self.base_defines_includes(lang, frag)
        c = Compiler(lang, frag)
        defines.update(c.lang.get_defines())
        for perm in permutations:
            defines[perm.name] = str(perm.value)

        src = preprocess(main_source, self.path, defines=defines, includes=includes, ignore_unknown=False)
        lines = []
        for line in src.splitlines():
            line = line.rstrip("\r\n")
            res = c.do_line(line)
            if isinstance(res, str):
                lines.append(res)
            elif res is not None:
                lines += res
            else:
                lines.append(line)

        header = c.lang.do_header(c)

        tmp_src = "\n".join(header + lines)

        new_src = preprocess(tmp_src)

        new_lines = []

        prev = ""
        for line in new_src.splitlines():
            s = line.strip()
            if s.startswith("#line"):
                if prev: new_lines.append("")
                prev = ""
                continue
            if not s and not prev:
                continue
            prev = s

            line = line.replace(tmp_name, "")

            new_lines.append(line)

        source = "\n".join(new_lines) + "\n"
        attribs = [] if frag else c.attribs
        return source, c.uniform_blocks, c.samplers, attribs

def compile_permutations(shader, lang, frag, perms_left, permutations=tuple()):
    if not perms_left:
        return [(permutations, shader.compile(lang, frag, permutations))]
    else:
        result = []
        perm = perms_left[0]
        for val in range(perm.value):
            new_p = permutations + (Permutation(perm.name, val),)
            result += compile_permutations(shader, lang, frag, perms_left[1:], new_p)
        return result

Config = namedtuple("Config", "name lang translate")

def run(exe, args):
    if sys.platform == "win32":
        exe += ".exe"
    exe_path = os.path.join(g_tool_root, exe)
    subprocess.check_call([exe_path] + args)

def translate_metal(frag, src_path, ios):
    dst_path = os.path.splitext(src_path)[0] + ".metal"

    temp_path = src_path + ".spv"

    args = []
    args += ["-V"]
    args += ["-e", "main"]
    args += ["-o", temp_path]
    args += ["-S", ["vert", "frag"][frag]]
    args += [src_path]
    run("glslangValidator", args)

    args = []
    args += ["--msl"]
    if ios:
        args += ["--msl-ios"]
    args += ["--entry", "main"]
    args += ["--output", dst_path]
    args += ["--stage", ["vert", "frag"][frag]]
    args += [temp_path]
    run("spirv-cross", args)

    return dst_path

def translate_metal_ios(frag, src_path):
    return translate_metal(frag, src_path, True)

def translate_metal_macos(frag, src_path):
    return translate_metal(frag, src_path, False)

configs = [
    Config("gles", GLSL("300 es"), None),
    Config("glsl", GLSL("300"), None),
    Config("hlsl", HLSL("5"), None),
    Config("macos", GLSL("450"), translate_metal_macos),
    Config("ios", GLSL("450"), translate_metal_ios),
]

shader_root = os.path.join(g_root, "shader")

shaders = []
for root,dirs,files in os.walk(shader_root):
    for f in files:
        if not f.endswith("glsl"): continue
        path = os.path.join(root, f)
        shaders.append(Shader(path))

ShaderInfo = namedtuple("ShaderInfo", "name permutations base num")
PermutationInfo = namedtuple("PermutationInfo", "permutation offset size uniforms samplers attribs")

for config in configs:
    lang = config.lang
    shader_infos = []
    permutation_infos = []
    shader_datas = []
    shader_data_offset = 0

    header_name = "GameShaders_{}.h".format(config.name)
    source_name = "GameShadersImp_{}.h".format(config.name)

    perm_index = { }
    permutations = []

    uniform_index = { }
    uniforms = []

    sampler_index = { }
    samplers = []

    attrib_index = { }
    attribs = []

    def add_uniform(ubo):
        if ubo.name not in uniform_index:
            ix = len(uniforms)
            uniform_index[ubo.name] = ix
            uniforms.append(ubo)
        else:
            ix = uniform_index[ubo.name]
            assert uniforms[ix] == ubo
        return ix + 1

    def add_sampler(sampler):
        if sampler.name not in sampler_index:
            ix = len(samplers)
            sampler_index[sampler.name] = ix
            samplers.append(sampler)
        else:
            ix = sampler_index[sampler.name]
            assert samplers[ix] == sampler
        return ix + 1

    def add_attrib(attrib):
        if attrib.name not in attrib_index:
            ix = len(attribs)
            attrib_index[attrib.name] = ix
            attribs.append(attrib)
        else:
            ix = attrib_index[attrib.name]
            assert attribs[ix] == attrib
        return ix + 1

    for shader in shaders:
        infos = [None, None]
        for frag in [False, True]:
            perms = shader.get_permutations(lang, frag)

            for perm in perms:
                if perm.name not in perm_index:
                    perm_index[perm.name] = len(permutations)
                    permutations.append(perm.name)

            result = compile_permutations(shader, lang, frag, perms)

            infos[int(frag)] = ShaderInfo(shader.name, perms, len(permutation_infos), len(result))
            for ix, (permutation, (source, ubos, smps, atts)) in enumerate(result):

                suffix = ["vs", "fs"][frag]
                temp_name = "{}_{}_{}.{}".format(shader.name, suffix, ix, lang.get_extension())
                temp_dir = os.path.join(g_temp, config.name)
                os.makedirs(temp_dir, exist_ok=True)
                temp_path = os.path.join(temp_dir, temp_name)
                with open(temp_path, "w") as f:
                    f.write(source)

                if config.translate:
                    dst_path = config.translate(frag, temp_path)
                    with open(dst_path, "r") as f:
                        source = f.read()

                data = source.encode("utf-8") + b"\0"
                ubos = tuple(add_uniform(u) for u in ubos)
                smps = tuple(add_sampler(s) for s in smps)
                atts = tuple(add_attrib(s) for s in atts)
                permutation_infos.append(PermutationInfo(permutation, shader_data_offset, len(data), ubos, smps, atts))
                shader_data_offset += len(data)
                shader_datas.append(data)
        shader_infos.append(tuple(infos))

    total_data = b"".join(shader_datas)
    compressed_data = zstd.ZSTD_compress(total_data, 10, 1)

    header_lines = []
    source_lines = []

    header_lines.append("#pragma once")

    header_lines.append("")

    header_lines.append("#include \"sf/Vector.h\"")
    header_lines.append("#include \"sf/Matrix.h\"")

    header_lines.append("")

    for ix, infos in enumerate(shader_infos):
        header_lines.append("#define SpShader_{} {}".format(infos[0].name, ix))
    header_lines.append("#define SpShaderDataSize {}".format(len(total_data)))

    header_lines.append("")
    for ix, perm in enumerate(permutations):
        header_lines.append("#define {} {}".format(perm, ix))
    header_lines.append("#define SP_NUM_PERMUTATIONS {}".format(len(permutations)))

    header_lines.append("")

    type_mapping = {
        "float": "float",
        "uint": "uint32_t",
        "vec2": "sf::Vec2",
        "vec3": "sf::Vec3",
        "vec4": "sf::Vec4",
        "int": "int32_t",
        "ivec2": "sf::Vec2i",
        "ivec3": "sf::Vec3i",
        "ivec4": "sf::Vec4i",
        "mat4": "sf::Mat44",
    }

    for ubo_ix, ubo in enumerate(uniforms):

        header_lines.append("struct UBO_{} {{".format(ubo.name))

        header_lines.append("\tstatic const constexpr uint32_t UboIndex = {};".format(ubo_ix + 1))
        header_lines.append("")

        offset = 0
        pad = 0
        for u in ubo.uniforms:
            cur_pad = 0
            while offset % u.type.align != 0:
                offset += 1
                cur_pad += 1
            if cur_pad:
                header_lines.append("\tuint32_t _sp_pad{}[{}];".format(pad, cur_pad))
                pad += 1
            typename = type_mapping[u.type.name]
            if u.num:
                header_lines.append("\t{} {}[{}];".format(typename, u.name, u.num))
                offset += u.type.size * u.num
            else:
                header_lines.append("\t{} {};".format(typename, u.name))
                offset += u.type.size

        header_lines.append("};")
        header_lines.append("")

    for smp_ix, smp in enumerate(samplers):
        header_lines.append("#define TEX_{} {}".format(smp.name, smp_ix + 1))

    header_lines.append("")

    header_lines.append("struct SpShaderInfo;")
    header_lines.append("struct SpPermutationInfo;")
    header_lines.append("struct SpUniformBlockInfo;")
    header_lines.append("struct SpSamplerInfo;")
    header_lines.append("struct SpAttribInfo;")
    header_lines.append("extern const SpShaderInfo spShaders[{}];".format(len(shader_infos)))
    header_lines.append("extern const SpPermutationInfo spPermutations[{}];".format(len(permutation_infos)))
    header_lines.append("extern const SpUniformBlockInfo spUniformBlock[{}];".format(len(uniforms) + 1))
    header_lines.append("extern const SpSamplerInfo spSamplers[{}];".format(len(samplers) + 1))
    header_lines.append("extern const SpAttribInfo spAttribs[{}];".format(len(attribs) + 1))
    header_lines.append("extern const char spShaderData[{}];".format(len(compressed_data) + 1))

    source_lines.append("#include \"{}\"".format(header_name))
    source_lines.append("#include \"{}\"".format("game/ShadersDesc.h"))
    source_lines.append("")

    source_lines.append("const SpShaderInfo spShaders[] = {")
    for infos in shader_infos:
        source_lines.append("\t{")
        source_lines.append("\t\t\"{}\",".format(infos[0].name))
        source_lines.append("\t\t{ {")
        for info in infos:
            perms = ", ".join("{{{},{}}}".format(perm_index[perm.name], perm.value) for perm in info.permutations)
            source_lines.append("\t\t\t{{ {} }},".format(perms))
            source_lines.append("\t\t\t{}, {},".format(info.base, info.num))
            if info is infos[0]:
                source_lines.append("\t\t}, {")
        source_lines.append("\t\t} }")
        source_lines.append("\t},")
    source_lines.append("};")

    source_lines.append("")

    source_lines.append("const SpPermutationInfo spPermutations[] = {")
    for info in permutation_infos:
        ubos = ",".join("{}".format(ubo) for ubo in info.uniforms)
        smps = ",".join("{}".format(smp) for smp in info.samplers)
        atts = ",".join("{}".format(att) for att in info.attribs)
        source_lines.append("\t{{ {{ {} }}, {{ {} }}, {{ {} }}, {}, {} }},".format(ubos, smps, atts, info.offset, info.size))
    source_lines.append("};")

    source_lines.append("")

    source_lines.append("const SpUniformBlockInfo spUniformBlocks[] = {")
    source_lines.append("\t{ }, // Null uniform block")
    for ubo in uniforms:
        source_lines.append("\t{{ \"{}\", {} }},".format(ubo.name, ubo.size * 4))
    source_lines.append("};")

    sampler_mapping = {
        "sampler2D": "SG_IMAGETYPE_2D",
        "sampler3D": "SG_IMAGETYPE_3D",
        "sampler2DArray": "SG_IMAGETYPE_ARRAY",
        "samplerCube": "SG_IMAGETYPE_CUBE",
    }

    source_lines.append("const SpSamplerInfo spSamplers[] = {")
    source_lines.append("\t{ }, // Null sampler")
    for smp in samplers:
        source_lines.append("\t{{ \"{}\", (uint32_t){} }},".format(smp.name, sampler_mapping[smp.type]))
    source_lines.append("};")

    source_lines.append("const SpAttribInfo spAttribs[] = {")
    source_lines.append("\t{ }, // Null attrib")
    for att in attribs:
        source_lines.append("\t{{ \"{}\", {} }},".format(att.name, att.location))
    source_lines.append("};")

    source_lines.append("")

    cols = 32
    source_lines.append("const char spShaderData[] = ")
    for base in range(0, len(compressed_data), cols):
        block = compressed_data[base:base+cols]
        line = "".join("\\x{:02x}".format(b) for b in block)
        source_lines.append("\t\"{}\"".format(line))
    source_lines.append(";")

    with open(os.path.join(g_root, header_name), "wb") as f:
        f.write("\n".join(header_lines).encode("utf-8"))

    with open(os.path.join(g_root, source_name), "wb") as f:
        f.write("\n".join(source_lines).encode("utf-8"))
