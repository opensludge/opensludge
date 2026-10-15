#!/bin/sh
# Print the version for the names of the release files: the tag when GitHub
# Actions builds a tag, and otherwise the version in CMakeLists.txt.

set -eu

case "${GITHUB_REF:-}" in
	refs/tags/*) echo "${GITHUB_REF#refs/tags/}" ;;
	*) sed -n 's/^project(OpenSLUDGE VERSION \([0-9.]*\).*/\1/p' "$(dirname "$0")/../CMakeLists.txt" ;;
esac
