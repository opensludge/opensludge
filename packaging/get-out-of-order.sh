#!/bin/sh
# Download the data file of Out Of Order from the Ubuntu archive, check it,
# and unpack the package into a folder. The game is freeware, and anyone may
# give it away together with its readme and licence.
#
# Usage: get-out-of-order.sh <folder>
#
# In the folder, the data file is usr/share/out-of-order/gamedata, the
# manual and licence are in usr/share/doc/out-of-order, and the icon is
# usr/share/pixmaps/out-of-order.xpm.

set -eu

url=http://archive.ubuntu.com/ubuntu/pool/multiverse/o/out-of-order/out-of-order_1.0-3_all.deb
sha256=fcf4e47f947d767ce55b4be718b0c10e591fdf1a8bc99e884f7e3e988e757a4f

folder=$1
mkdir -p "$folder"
cd "$folder"
curl -fsSL -o out-of-order.deb "$url"
if command -v sha256sum > /dev/null; then
	echo "$sha256  out-of-order.deb" | sha256sum -c -
else
	echo "$sha256  out-of-order.deb" | shasum -a 256 -c -
fi
# A Debian package is an ar archive, with the files in data.tar.xz.
ar x out-of-order.deb data.tar.xz
tar -xf data.tar.xz
rm out-of-order.deb data.tar.xz
