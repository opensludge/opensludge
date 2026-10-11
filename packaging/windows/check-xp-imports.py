#!/usr/bin/env python3
"""Check that a Windows executable imports only functions that Windows XP
has.

Usage: check-xp-imports.py <objdump> <executable> <folder for export lists>

We read the imported functions with objdump -p, and compare them with the
export lists (.spec files) of ReactOS, a free reimplementation of Windows
XP and Server 2003. In these lists, a function that only some versions of
Windows have is marked with the versions, for example -version=0x600+ for
Vista and later. Windows XP is version 0x501. We download the lists from a
fixed ReactOS commit into the given folder, unless they are already there.
"""

import os
import re
import subprocess
import sys
import urllib.request

XP = 0x501

REACTOS_COMMIT = "7b47e058fec87f64baab489d26787a105ce52b9a"

# The path of the export list of each DLL in the ReactOS source.
SPEC_PATHS = {
    "advapi32": "dll/win32/advapi32/advapi32.spec",
    "comdlg32": "dll/win32/comdlg32/comdlg32.spec",
    "gdi32": "win32ss/gdi/gdi32/gdi32.spec",
    "imm32": "win32ss/user/imm32/imm32.spec",
    "kernel32": "dll/win32/kernel32/kernel32.spec",
    "msvcrt": "dll/win32/msvcrt/msvcrt.spec",
    "ole32": "dll/win32/ole32/ole32.spec",
    "oleaut32": "dll/win32/oleaut32/oleaut32.spec",
    "opengl32": "dll/opengl/opengl32/opengl32.spec",
    "setupapi": "dll/win32/setupapi/setupapi.spec",
    "shell32": "dll/win32/shell32/shell32.spec",
    "user32": "win32ss/user/user32/user32.spec",
    "version": "dll/win32/version/version.spec",
    "winmm": "dll/win32/winmm/winmm.spec",
}


def available_on_xp(flags):
    """Return whether a function with these .spec flags exists on XP."""
    match = re.search(r"-version=(0x[0-9A-Fa-f]+)(\+|-(0x[0-9A-Fa-f]+))?", flags)
    if not match:
        return True
    low = int(match.group(1), 16)
    if match.group(2) == "+":
        return XP >= low
    if match.group(3):
        return low <= XP <= int(match.group(3), 16)
    return XP == low


def export_list(dll, folder):
    """Return the names of the functions of a DLL that XP has, or None when
    we have no export list for the DLL."""
    if dll not in SPEC_PATHS:
        return None
    path = os.path.join(folder, dll + ".spec")
    if not os.path.exists(path):
        url = f"https://raw.githubusercontent.com/reactos/reactos/{REACTOS_COMMIT}/{SPEC_PATHS[dll]}"
        with urllib.request.urlopen(url) as response, open(path, "wb") as f:
            f.write(response.read())

    names = set()
    for line in open(path, encoding="utf-8", errors="replace"):
        fields = line.split("#", 1)[0].split()
        # Lines of functions: "@" or an ordinal, a calling convention,
        # flags that start with "-", then the name and arguments.
        if len(fields) < 3 or not (fields[0] == "@" or fields[0].isdigit()):
            continue
        flags = " ".join(field for field in fields[2:] if field.startswith("-"))
        name = next((field for field in fields[2:] if not field.startswith("-")), None)
        if name and available_on_xp(flags):
            names.add(name.split("(", 1)[0])
    return names


def read_imports(objdump, executable):
    """Return the imported functions as a list of (DLL, function)."""
    output = subprocess.run([objdump, "-p", executable], check=True, capture_output=True, text=True).stdout
    imports = []
    dll = None
    for line in output.splitlines():
        if "DLL Name:" in line:
            dll = os.path.splitext(line.split("DLL Name:", 1)[1].strip().lower())[0]
        elif not line.strip():
            dll = None
        elif dll:
            # Lines of functions: address, hint, name.
            fields = line.split()
            if len(fields) >= 3 and re.fullmatch(r"[0-9a-f]+", fields[0]) and fields[1].isdigit():
                imports.append((dll, fields[-1]))
    return imports


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    objdump, executable, folder = sys.argv[1:]
    os.makedirs(folder, exist_ok=True)

    problems = set()
    lists = {}
    for dll, function in read_imports(objdump, executable):
        if dll not in lists:
            lists[dll] = export_list(dll, folder)
        if lists[dll] is None:
            problems.add(f"{dll}.dll: we have no export list to check {function}")
        elif function not in lists[dll]:
            problems.add(f"{dll}.dll: {function}")

    if problems:
        print("These imports are not available on Windows XP:")
        for problem in sorted(problems):
            print("  " + problem)
        sys.exit(1)
    print("Every import is available on Windows XP.")


if __name__ == "__main__":
    main()
