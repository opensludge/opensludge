#!/bin/sh
# Make the Windows zips: OpenSLUDGE, the engine alone, and Out Of Order,
# the engine with the game.
#
# Usage: make-zips.sh <engine .exe> <out-of-order folder> <version> <label> <output folder>
#
# The out-of-order folder is the one from packaging/get-out-of-order.sh. We
# put the label, for example x64, in the names of the zips. Set STRIP to the
# strip program for the executable, when it is not strip.

set -eu

engine=$1 ooo=$2 version=$3 label=$4 output=$5
here=$(cd "$(dirname "$0")" && pwd)
stage=$(mktemp -d)

mkdir -p "$stage/OpenSLUDGE" "$stage/Out Of Order/ooodoc" "$output"
cp "$engine" "$stage/OpenSLUDGE/OpenSLUDGE.exe"
${STRIP:-strip} "$stage/OpenSLUDGE/OpenSLUDGE.exe"
cp "$here/README-Windows.txt" "$stage/OpenSLUDGE/README.txt"

cp "$stage/OpenSLUDGE/OpenSLUDGE.exe" "$stage/Out Of Order/Out Of Order.exe"
cp "$ooo/usr/share/out-of-order/gamedata" "$stage/Out Of Order/gamedata"
cp "$here/../out-of-order/controller.ini" "$stage/Out Of Order/"
# When the player chooses Help, the game opens ooodoc/contents.html in the
# folder that contains its data file.
cp "$ooo"/usr/share/doc/out-of-order/*.html "$stage/Out Of Order/ooodoc/"
cp "$ooo/usr/share/doc/out-of-order/copyright" "$stage/Out Of Order/ooodoc/LICENSE.txt"
cp "$here/README-Windows.txt" "$stage/Out Of Order/README.txt"

output=$(cd "$output" && pwd)
(cd "$stage" && zip -q -r -9 "$output/OpenSLUDGE-$version-windows-$label.zip" OpenSLUDGE)
(cd "$stage" && zip -q -r -9 "$output/OutOfOrder-$version-windows-$label.zip" "Out Of Order")
rm -rf "$stage"
ls -la "$output"
