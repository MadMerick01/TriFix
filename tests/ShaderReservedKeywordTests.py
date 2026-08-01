#!/usr/bin/env python3
"""Prevent HLSL reserved words from being introduced as identifiers."""

import re
from pathlib import Path


ROOT = Path(__file__).parents[1]

# Keywords which can be mistaken for descriptive variable names. HLSL reserves
# these for primitive topology, interpolation, storage, and shader declarations.
RESERVED_IDENTIFIERS = {
    "asm", "asm_fragment", "break", "case", "catch", "centroid", "class",
    "column_major", "compile", "compile_fragment", "const", "continue",
    "default", "discard", "do", "else", "export", "extern", "false", "for",
    "fxgroup", "geometryshader", "groupshared", "if", "in", "inline", "inout",
    "interface", "line", "lineadj", "linear", "namespace", "nointerpolation",
    "noperspective", "out", "packoffset", "pass", "pixelfragment",
    "pixelshader", "point", "precise", "register", "return", "row_major",
    "sample", "shared", "stateblock", "stateblock_state", "static", "struct",
    "switch", "tbuffer", "technique", "technique10", "technique11", "true",
    "try", "typedef", "triangle", "triangleadj", "uniform", "vertexfragment",
    "vertexshader", "void", "volatile", "while",
}

# A reserved word used illegally as an identifier follows a type in variable,
# parameter, field, or function declarations. Keeping this source-level check
# platform-independent lets Linux CI catch errors otherwise found only by FXC.
DECLARATION = re.compile(
    r"\b(?:bool|double|dword|half|int|uint|float)(?:[1-4](?:x[1-4])?)?"
    r"\s+([A-Za-z_]\w*)\b"
)
COMMENTS = re.compile(r"//[^\n]*|/\*.*?\*/", re.DOTALL)


violations = []
for shader_path in sorted((ROOT / "shaders").glob("*.hlsl")):
    source = COMMENTS.sub("", shader_path.read_text(encoding="utf-8"))
    for match in DECLARATION.finditer(source):
        if match.group(1) in RESERVED_IDENTIFIERS:
            line = source.count("\n", 0, match.start(1)) + 1
            violations.append(f"{shader_path.relative_to(ROOT)}:{line}: {match.group(1)}")

assert not violations, "reserved HLSL keyword used as an identifier:\n" + "\n".join(violations)
print("HLSL reserved-keyword identifier checks passed.")
