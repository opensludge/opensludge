#!/bin/bash
# Sign a macOS application bundle.
#
# Usage: sign-app.sh <.app>
#
# When MACOS_SIGNING_IDENTITY is set, for example to "Developer ID
# Application: Name (TEAMID)", and that identity is in the keychain, we sign
# the app with it and the hardened runtime, as Apple requires to notarise.
# Otherwise we give the app an ad-hoc signature, without which an Apple
# Silicon Mac does not run it at all.

set -euo pipefail

app=$1
here=$(cd "$(dirname "$0")" && pwd)

if [ -n "${MACOS_SIGNING_IDENTITY:-}" ]; then
	codesign --force --timestamp --options runtime \
		--entitlements "$here/entitlements.plist" \
		--sign "$MACOS_SIGNING_IDENTITY" "$app"
else
	codesign --force --sign - "$app"
fi
codesign --verify --strict --verbose=2 "$app"
