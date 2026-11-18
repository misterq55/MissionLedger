#!/bin/sh
# Run on a macOS build node:  sh jenkins/scripts/node_check_macos.sh   (see docs/CI_PLAN.md S5, S8)
# ===== MissionLedger build node check: macOS (read-only) =====
echo "--- CMake (needs 3.25+)"; command -v cmake || echo "  not in this shell's PATH"
for f in /opt/homebrew/bin/cmake /usr/local/bin/cmake /Applications/CMake.app/Contents/bin/cmake; do [ -x "$f" ] && echo "  found: $f ($("$f" --version | head -1))"; done
echo "--- Homebrew"; command -v brew || echo "  brew not in PATH"; ls -d /opt/homebrew /usr/local/Homebrew 2>/dev/null
echo "--- Xcode"; xcode-select -p; xcodebuild -version 2>&1 | head -2; ls -d /Applications/Xcode*.app 2>/dev/null
echo "--- Git";  command -v git && git --version
echo "--- Java"; command -v java && java -version 2>&1 | head -1
echo "--- CPU";  uname -m
echo "--- Jenkins agent (how it runs, as which user)"
ps -axo pid,user,command | grep -iE '[a]gent\.jar|[r]emoting' || echo "  no agent process found"
launchctl list 2>/dev/null | grep -i jenkins
ls ~/Library/LaunchAgents /Library/LaunchAgents /Library/LaunchDaemons 2>/dev/null | grep -i jenkins
echo "--- PATH of this shell"; echo "$PATH" | tr ':' '\n'
