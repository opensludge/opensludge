#!/bin/sh
# Check that a Windows executable links dynamically only to DLLs that are
# part of Windows, so that it runs without other DLLs next to it.
#
# Usage: check-dlls.sh <objdump> <executable>

set -eu

objdump=$1 executable=$2
"$objdump" -p "$executable" | grep "DLL Name"
if "$objdump" -p "$executable" | grep "DLL Name" | grep -i -v -E \
	'Name: (advapi32|comdlg32|gdi32|imm32|kernel32|msvcrt|ole32|oleaut32|opengl32|setupapi|shell32|user32|version|winmm|ucrtbase|api-ms-win-[a-z0-9-]+)\.dll'; then
	echo "The executable links to a DLL that is not part of Windows."
	exit 1
fi
