#!/bin/sh
# Installs the general tools a macOS build node needs, only the missing ones: CMake.
# Run once by hand on the node, as the user the Jenkins agent runs as (the pipeline only checks, it never installs):
#   sh jenkins/scripts/node_setup_macos.sh
# Safe to run again: tools that are present are skipped. Xcode is only checked (App Store install).
# CMake is the official pinned release in /Applications/CMake.app, not Homebrew (which installs the
# newest version, not tested with this project). See docs/CI_PLAN.md S5, S8
set -eu

# Pinned version (update together with the hash; SHA-256 from the release page)
CMAKE_VERSION=3.31.12
CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-macos-universal.tar.gz"
CMAKE_SHA256=799af7fd545db9bf1b9cfe72f8095880e727a2d4e0df0e3dffc3bc7b95c2d3b0
CMAKE_APP=/Applications/CMake.app
CMAKE_BIN=$CMAKE_APP/Contents/bin
TODO=""

echo "--- CMake (pinned $CMAKE_VERSION)"
if [ -x "$CMAKE_BIN/cmake" ]; then
    echo "  already installed: $("$CMAKE_BIN/cmake" --version | head -1)  ($CMAKE_APP)"
else
    if command -v cmake >/dev/null 2>&1; then
        echo "  note: another cmake is on the PATH ($(command -v cmake)); installing $CMAKE_VERSION to $CMAKE_APP anyway"
    fi
    if [ ! -w /Applications ]; then
        echo "ERROR: /Applications is not writable for $(whoami). Run as an administrator user, or install"
        echo "       cmake-$CMAKE_VERSION-macos-universal.dmg from https://cmake.org/download/ by hand."
        exit 1
    fi
    WORK=$(mktemp -d)
    trap 'rm -rf "$WORK"' EXIT
    echo "  downloading $CMAKE_URL"
    curl -fsSL -o "$WORK/cmake.tar.gz" "$CMAKE_URL"
    ACTUAL=$(shasum -a 256 "$WORK/cmake.tar.gz" | cut -d' ' -f1)
    if [ "$ACTUAL" != "$CMAKE_SHA256" ]; then
        echo "ERROR: SHA-256 mismatch (expected $CMAKE_SHA256, got $ACTUAL)"
        exit 1
    fi
    tar -xzf "$WORK/cmake.tar.gz" -C "$WORK"
    mv "$WORK/cmake-$CMAKE_VERSION-macos-universal/CMake.app" "$CMAKE_APP"
    echo "  installed: $("$CMAKE_BIN/cmake" --version | head -1)"
fi

echo "--- Xcode (checked only)"
if xcodebuild -version >/dev/null 2>&1; then
    echo "  $(xcodebuild -version | head -1): OK"
else
    TODO="$TODO
  - Install Xcode from the App Store, then: sudo xcode-select -s /Applications/Xcode.app && sudo xcodebuild -license accept"
fi

echo
echo "=== Next steps"
[ -n "$TODO" ] && echo "$TODO"
echo "  - Let the Jenkins agent see CMake (an agent started over SSH does not read your shell profile):"
echo "      Jenkins: Manage Jenkins > Nodes > (this node) > Configure > Environment variables: PATH+CMAKE = $CMAKE_BIN"
echo "  - Then run the build again (Preflight checks everything)."
