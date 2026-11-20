#!/bin/sh
# MissionLedger release: VERSION, CHANGELOG.md, commit and tag in one step (docs/VERSIONING.md)
#
#   ./release.sh 1.1.0          check, edit, commit, tag (stops before pushing)
#   ./release.sh 1.1.0 push     the same, then pushes main and the tag
#   ./release.sh                asks for the version
#
# Same as: cmake -DVERSION=1.1.0 -P scripts/release.cmake   (Windows: release.bat)
# After pushing, run the Jenkins build: it uploads the release to Google Drive.

cd "$(dirname "$0")" || exit 1

version="$1"
push=OFF
[ "$2" = "push" ] && push=ON
if [ -z "$version" ]; then
    echo "Current version: $(tr -d '\r\n' < VERSION)"
    printf "New version (e.g. 1.0.1 / 1.1.0 / 2.0.0): "
    read -r version
fi

exec cmake "-DVERSION=$version" "-DPUSH=$push" -P scripts/release.cmake
