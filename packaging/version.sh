#!/bin/sh
# Print the version for the names of the release files: the tag without its
# v when GitHub Actions builds a tag that starts with v, and otherwise the
# version in CMakeLists.txt.

set -eu

case "${GITHUB_REF:-}" in
	refs/tags/v*) echo "${GITHUB_REF#refs/tags/v}" ;;
	*) sed -n 's/^project(OpenSLUDGE VERSION \([0-9.]*\).*/\1/p' "$(dirname "$0")/../CMakeLists.txt" ;;
esac
