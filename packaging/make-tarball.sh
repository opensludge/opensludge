#!/bin/sh
# Make the source tarball, sludge-<version>.tar.gz, from the commit HEAD.
#
# Usage: make-tarball.sh <version> <output folder>
#
# We leave out the files marked export-ignore in .gitattributes.

set -eu

version=$1 output=$2

mkdir -p "$output"
git -C "$(dirname "$0")/.." archive --format=tar.gz --prefix="sludge-$version/" HEAD \
	> "$output/sludge-$version.tar.gz"
