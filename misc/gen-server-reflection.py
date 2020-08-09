#!/usr/bin/env python3

import os
import subprocess
import sys
import re
from collections import namedtuple

source_name = "../src/server/ServerState.h"
output_name = "../src/server/ServerStateRefletion.cpp"

self_path = os.path.dirname(os.path.abspath(__file__))
source_path = os.path.join(self_path, source_name)
output_path = os.path.join(self_path, output_name)

Struct = namedtuple("Component", "type_name, enum_name, fields")
Field = namedtuple("Field", "type_name, name")

RE_COMPONENT = re.compile(r"struct (\w+) : ComponentBase<Component::(\w+)>")
RE_EVENT = re.compile(r"struct (\w+) : EventBase<Event::(\w+)>")
RE_EDIT = re.compile(r"struct (\w+) : EditBase<Edit::(\w+)>")
RE_REFLECT = re.compile(r"struct (\w+).*sv_reflect.*")
RE_FIELD = re.compile(r"([A-Za-z_][A-Za-z0-9<>:_*]*)\s+([A-Za-z0-9]+).*")

components = []
events = []
edits = []
reflects = []
last_struct = None
with open(source_path) as f:
    for line in f:
        line = line.strip()
        if line == "};":
            last_struct = None
            continue
        m = RE_COMPONENT.match(line)
        if m:
            last_struct = Struct(m.group(1), m.group(2), [])
            components.append(last_struct)
            continue
        m = RE_EVENT.match(line)
        if m:
            last_struct = Struct(m.group(1), m.group(2), [])
            events.append(last_struct)
            continue
        m = RE_EDIT.match(line)
        if m:
            last_struct = Struct(m.group(1), m.group(2), [])
            edits.append(last_struct)
            continue
        m = RE_REFLECT.match(line)
        if m:
            last_struct = Struct(m.group(1), m.group(1), [])
            reflects.append(last_struct)
            continue
        m = RE_FIELD.match(line)
        if m and last_struct:
            last_struct.fields.append(Field(m.group(1), m.group(2)))
            continue

lines = []

def push(line):
    lines.append(line)

push("#include \"ServerState.h\"")
push("#include \"sf/Reflection.h\"")
push("")
push("namespace sf {")
push("using namespace sv;")
push("")

push("template<> void initType<Component>(Type *t)")
push("{")
push("\tstatic PolymorphType polys[] = {")
for c in components:
    push(f"\t\tsf_poly(Component, {c.enum_name}, {c.type_name}),")
push("\t};")
push("\tsf_struct_poly(t, Component, type, { }, polys);")
push("}")
push("")

push("template<> void initType<Event>(Type *t)")
push("{")
push("\tstatic PolymorphType polys[] = {")
for c in events:
    push(f"\t\tsf_poly(Event, {c.enum_name}, {c.type_name}),")
push("\t};")
push("\tsf_struct_poly(t, Event, type, { }, polys);")
push("}")
push("")

push("template<> void initType<Edit>(Type *t)")
push("{")
push("\tstatic PolymorphType polys[] = {")
for c in edits:
    push(f"\t\tsf_poly(Edit, {c.enum_name}, {c.type_name}),")
push("\t};")
push("\tsf_struct_poly(t, Edit, type, { }, polys);")
push("}")
push("")

for s in components:
    push(f"template<> void initType<{s.type_name}>(Type *t)")
    push("{")
    push("\tstatic Field fields[] = {")
    for f in s.fields:
        push(f"\t\tsf_field({s.type_name}, {f.name}),")
    push("\t};")
    push(f"\tsf_struct_base(t, {s.type_name}, Component, fields);")
    push("}")
    push("")

for s in events:
    push(f"template<> void initType<{s.type_name}>(Type *t)")
    push("{")
    push("\tstatic Field fields[] = {")
    for f in s.fields:
        push(f"\t\tsf_field({s.type_name}, {f.name}),")
    push("\t};")
    push(f"\tsf_struct_base(t, {s.type_name}, Event, fields);")
    push("}")
    push("")

for s in edits:
    push(f"template<> void initType<{s.type_name}>(Type *t)")
    push("{")
    push("\tstatic Field fields[] = {")
    for f in s.fields:
        push(f"\t\tsf_field({s.type_name}, {f.name}),")
    push("\t};")
    push(f"\tsf_struct_base(t, {s.type_name}, Event, fields);")
    push("}")
    push("")

for s in reflects:
    push(f"template<> void initType<{s.type_name}>(Type *t)")
    push("{")
    push("\tstatic Field fields[] = {")
    for f in s.fields:
        push(f"\t\tsf_field({s.type_name}, {f.name}),")
    push("\t};")
    push(f"\tsf_struct(t, {s.type_name}, fields);")
    push("}")
    push("")

push("}")
push("")
push("")

with open(output_path, "w") as f:
    f.write("\n".join(lines))
