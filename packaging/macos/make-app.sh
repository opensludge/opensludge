#!/bin/bash
# Make a macOS application bundle that contains the SLUDGE engine.
#
# Usage: make-app.sh <engine executable> <output .app> <app name> <bundle id>
#                    <icon .icns> <minimum macOS version> [<game data file> [<extra file>...]]
#
# With a game data file, we start that game when the player opens the app.
# We copy the data file and any extra files, such as the readme and licence
# of the game, into the Resources folder of the app. Without a game data
# file, we show a file chooser, in which the player picks a game.

set -euo pipefail

engine=$1 app=$2 name=$3 bundle_id=$4 icon=$5 min_macos=$6
shift 6

here=$(cd "$(dirname "$0")" && pwd)
# The version in Info.plist must be numbers and dots, so we leave out a
# suffix such as -beta.1.
version=$("$here/../version.sh")
version=${version%%-*}
copyright=${APP_COPYRIGHT:-"SLUDGE engine © Tim Furnish, Rikard Peterson, Tobias Hansen and contributors"}

rm -rf "$app"
mkdir -p "$app/Contents/MacOS" "$app/Contents/Resources"

cp "$engine" "$app/Contents/MacOS/sludge-engine"
chmod 755 "$app/Contents/MacOS/sludge-engine"
cp "$icon" "$app/Contents/Resources/AppIcon.icns"

sed -e "s|@BUNDLE_ID@|$bundle_id|g" \
    -e "s|@APP_NAME@|$name|g" \
    -e "s|@VERSION@|$version|g" \
    -e "s|@MIN_MACOS@|$min_macos|g" \
    -e "s|@COPYRIGHT@|$copyright|g" \
    "$here/Info.plist.in" > "$app/Contents/Info.plist"
printf 'APPL????' > "$app/Contents/PkgInfo"

if [ $# -gt 0 ]; then
	cp "$1" "$app/Contents/Resources/gamedata"
	shift
	for extra in "$@"; do
		cp -R "$extra" "$app/Contents/Resources/"
	done
fi

echo "Created $app"
