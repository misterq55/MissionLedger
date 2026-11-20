# CI/CD Plan: Jenkins Build Automation (Windows + macOS)

> Created: 2026-11-18 · Decisions recorded: 2026-11-18
> Status: Planning (CI not implemented). Prerequisites partly done: single-source versioning, Core folder separation

## Goals

- Keep the Jenkinsfile in the repository (`jenkins/Build.Jenkinsfile`) and automate builds through a Jenkins pipeline
- Stages: Build → Test → Installer → Deploy
- Target platforms: **Windows x64** and **macOS** (Universal: arm64 + x86_64)
- Build machines: existing Windows/macOS machines that are already Jenkins nodes for other projects
- **Keep per-node setup to a minimum.** A node needs only general-purpose tools (compiler, CMake, Git, installer tool). Everything specific to this project, including wxWidgets, comes from the repository

---

## 0. Decisions

| Topic | Decision | Notes |
|---|---|---|
| Build system | **CMake** generates the IDE projects (Visual Studio `.sln`, Xcode) from `CMakeLists.txt`, like Unreal's GenerateProjectFiles | Generated projects live in `build/` and are never committed. Never add files or change settings through the IDE; edit the source folders and `CMakeLists.txt` |
| Source collection | Each project collects the sources under its own folder automatically (`file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`) | Rule: one project = one folder. ✅ Folder separation done (`e94b226`): Core sources are in `MissionLedgerCore/src/` |
| wxWidgets | **Vendor wxWidgets 3.2.8** in `third_party/wxWidgets/` and build it with `add_subdirectory`, the same way as PDF-Writer | Static libraries, only `base` and `core` (the app uses nothing else). Nothing is installed on nodes and no `WXWIN` is needed. See §2 |
| Scripts | Build, test and packaging logic lives in CMake (`CMakePresets.json` + workflow presets), so the same command works on both platforms | ✅ `cmake --workflow --preset windows-release` (macOS: `macos-release` later). `build_installer.bat` is now a thin local wrapper for it. Only steps that exist on a single platform (signing, notarization) get short platform scripts (PowerShell / sh) |
| C++ runtime (Windows) | Keep the dynamic runtime (`/MD`) and **bundle `vc_redist.x64.exe`** in the installer (already implemented, see [INSTALLER.md](INSTALLER.md)) | Optional later: switch to `/MT` with `CMAKE_MSVC_RUNTIME_LIBRARY` (applies to vendored wxWidgets and PDF-Writer too) and drop the bundled redist |
| macOS signing | Not now. Build and test unsigned first | Distributing to others without Gatekeeper warnings needs the Apple Developer Program ($99/year). Check Apple's nonprofit fee waiver first. A few known users can use System Settings → Privacy & Security → "Open Anyway" |
| Versioning | ✅ Done. Root `VERSION` file is the single source of truth | See §1 and [VERSIONING.md](VERSIONING.md) |
| Jenkins job | ✅ One **Pipeline** job ("Pipeline script from SCM", `main`), started by hand with parameters. No Multibranch, no webhook | Jenkinsfiles live in `jenkins/` (`jenkins/Build.Jenkinsfile`). Nodes: `WindowsMini`, `MacMini`. See §4–§5 |
| PDF font | ✅ **Bundle Noto Sans KR** (Regular + Bold, OFL) in `third_party/fonts/NotoSansKR/` and use it on both platforms (decided and done 2026-11-20, roadmap 7 step ②) | Same PDF output on Windows and macOS (expected; Windows PDFs change from Malgun Gothic to Noto Sans KR). CMake copies the fonts next to the executable (`fonts/`; macOS app: `Contents/Resources/fonts/`); the installer and the macOS bundle take them from `third_party/fonts`. CJK coverage stays as now (Hangul, Korean Hanja; not Japanese shinjitai or Simplified Chinese), not needed yet. If it is needed later, swap in Noto Sans CJK KR (full pan-CJK set, about 16 MB per weight) in the same folder and change the file names in the code |
| Deploy target | ✅ **Google Drive** (decided 2026-11-21, roadmap 10) | rclone on the deploy node (WindowsMini); `rclone.conf` (remote `gdrive`, OAuth user token of a personal account) is the Jenkins secret file `missionledger-rclone-conf`, never in the repository. Layout `MissionLedger/v<VERSION>/` (kept) + `MissionLedger/latest/` (replaced on each release; share that folder once, its link stays the same). No share links are made automatically |

---

## 1. Issues to Fix First

| Issue | Location | Fix |
|---|---|---|
| ~~The trailing `pause` hangs CI builds~~ ✅ Done (2026-11-18) | `build_installer.bat` | CI never calls it: it uses the presets (`installer` target, test/workflow presets). The bat is a local wrapper for `cmake --workflow --preset windows-release` |
| ~~MSBuild path is hard-coded to the Community edition~~ ✅ Done (2026-11-18) | `build_installer.bat`, CLAUDE.md | Gone with the CMake migration (`cmake --build`) |
| ~~The version is hard-coded in 3 separate places~~ ✅ Done (2026-11-18) | `MLVersion.h`, `MissionLedger.iss` (`MyAppVersion`), `build_installer.bat` (output filename) | The root `VERSION` file is now the single source of truth (see [VERSIONING.md](VERSIONING.md)) |
| ~~Core sources live in the GUI folder~~ ✅ Done (2026-11-18, `e94b226`) | `MissionLedger/src/` | Moved to `MissionLedgerCore/src/` (one project = one folder) |
| ~~wxWidgets is outside the repository and its build is undocumented~~ ✅ Done (2026-11-18) | `WXWIN` differed per PC (`C:\wxWidgets-3.2.6` on one, `C:\wxWidgets-3.2.8` on another) | wxWidgets 3.2.8 vendored in `third_party/wxWidgets/` and built by CMake (see §2). `WXWIN` is no longer used |
| ~~`third_party/pdfwriter/build` is gitignored, but `MissionLedger.sln` references the vcxproj files generated there~~ ✅ Done (2026-11-18) | `.sln`, pdfwriter `.gitignore` | `add_subdirectory(third_party/pdfwriter)`; no separate PDF-Writer step |
| ~~No sample `.ml` file is committed for smoke tests~~ ✅ Done (2026-11-18) | All `.ml` files in the root are untracked | `tests/fixtures/sample.ml` with made-up data (see [tests/fixtures/README.md](../tests/fixtures/README.md)) |
| ~~The build system works only with Visual Studio~~ ✅ Done (2026-11-18) | `.sln` / `.vcxproj` (removed) | Migrated to CMake (§3) |

### Single-Source Versioning (✅ Implemented 2026-11-18)

The policy and the full flow are documented in [VERSIONING.md](VERSIONING.md). What CI relies on:

- **Source of truth**: the root `VERSION` file (`MAJOR.MINOR.PATCH`, one line). CI reads it with `readFile('VERSION')`
- **Build number**: `cmake --preset windows -DML_BUILD_NUMBER=%BUILD_NUMBER%` (4th field of the exe version)
- **Installer**: `MissionLedger.iss` reads `VERSION` itself; `ISCC /DMyAppVersion=x.y.z` still overrides it. ISCC fails if the packaged exe's `ProductVersion` differs from it
- Releases (roadmap 10, done): `release.bat` / `release.sh` (`scripts/release.cmake`) writes `VERSION` and CHANGELOG, commits and tags `v<VERSION>` in one step, so the version number is the only input. A build is a release when the remote tag `v<VERSION>` points to the commit being built (`jenkins/scripts/release_check.cmake`, `git ls-remote`, since the job is not Multibranch and does not fetch tags); another `v*` tag on the commit fails the build
- CMake (done): `file(READ VERSION ...)` → `project(MissionLedger VERSION ...)` → `.rc` defines (`Version.props` removed). Planned for macOS: `CFBundleShortVersionString` / `CFBundleVersion` in `Info.plist` via `configure_file`

---

## 2. Dependencies (Deps)

"Deps" is short for **dependencies**: the external libraries that must be ready before the application can be built.

| Library | Before (MSBuild) | Now (CMake, 2026-11-18) | CI work |
|---|---|---|---|
| SQLite | Amalgamation source in `third_party/sqlite/`, compiled inside the Core project | `sqlite3.c` in the Core target | None |
| PDF-Writer | Source in `third_party/pdfwriter/`; CMake-generated vcxproj files added to `MissionLedger.sln` | `add_subdirectory(third_party/pdfwriter)`, `PDFHUMMUS_NO_OPENSSL=ON` | None |
| wxWidgets | Prebuilt outside the repository, found via `WXWIN` (version differed per PC) | **Source in `third_party/wxWidgets/` (3.2.8)**, `add_subdirectory` | None |

All three come from the repository, so a fresh clone builds with no extra setup.

### Vendoring wxWidgets (✅ Source vendored 2026-11-18)

- **Source**: the official release archive `wxWidgets-3.2.8.tar.bz2` (SHA-1 checked against the release notes), **not** a `git clone`. The Git repository keeps the bundled libraries (png, jpeg, zlib, expat, ...) as submodules, so a plain clone is missing them
- **Committed** (5,027 files, about 86 MB, copied unchanged): `include/`, `src/`, `3rdparty/`, `art/`, `build/cmake/`, `build/bakefiles/version.bkl`, `CMakeLists.txt`, `README.md`, licence files in `docs/`
  - `art/`: stock icons that `src/common/artstd.cpp` and `arttango.cpp` `#include`. wxcore does not compile without it
  - `build/bakefiles/version.bkl`: the root `CMakeLists.txt` always reads it (shared-library version numbers; unused for static builds, but configure fails without the file)
- **Left out**: the rest of `docs/`, `samples/`, `tests/`, `demos/`, `utils/`, `interface/`, `locale/`, `misc/`, `distrib/`, `lib/`, the rest of `build/`, autoconf/configure files
- **Nested `.gitignore` files removed** (`3rdparty/catch`, `3rdparty/nanosvg`, `3rdparty/pcre`, `src/tiff`). They are the upstream projects' own rules; `src/tiff/.gitignore` hid `libtiff/tif_config.h` and `tiffconf.h`, which libtiff needs (wx ships them pre-generated). The root `.gitignore` was also cleaned up so its generic VS rules (`Win32/`, `ARM/`, `*.opt`) no longer match source folders
- Details, checksums and the upgrade procedure: `third_party/wxWidgets/VERSION.txt`
- **Verified** with a stand-alone CMake project outside the repository (Windows, VS 2022, Release): configure, build of `wxbase` + `wxcore` + builtin zlib/png/jpeg/tiff/regex, a test app using the same wx headers as MissionLedger linked and ran. Clean build took about 25 s on a 20-thread PC
- Build options (as verified):
  ```cmake
  set(wxBUILD_SHARED OFF CACHE BOOL "" FORCE)        # static libraries
  set(wxBUILD_MONOLITHIC OFF CACHE BOOL "" FORCE)
  set(wxBUILD_INSTALL OFF CACHE BOOL "" FORCE)       # install needs files that are left out
  set(wxBUILD_LOCALES OFF CACHE STRING "" FORCE)     # locale/ is left out
  # modules the app does not use (it uses only base and core).
  # XRC must stay OFF (wxrc lives in utils/, which is left out).
  # HTML OFF also needs WXHTML_HELP OFF (include/wx/chkconf.h).
  foreach(opt XRC HTML WXHTML_HELP AUI STC WEBVIEW PROPGRID RIBBON RICHTEXT MEDIACTRL OPENGL)
      set(wxUSE_${opt} OFF CACHE BOOL "" FORCE)
  endforeach()
  add_subdirectory(third_party/wxWidgets EXCLUDE_FROM_ALL)
  target_link_libraries(MissionLedger PRIVATE wx::core wx::base)
  ```
  - Candidates to turn off later: `wxUSE_LIBTIFF` / `wxUSE_LIBJPEG` (the app loads no TIFF/JPEG images; check before turning off)
- **Keep `/utf-8`** on our own targets (the current vcxproj files have it). Without it, MSVC on Korean Windows reads BOM-less UTF-8 sources as CP949 (warning C4819) and Korean string literals can break
- The first build compiles wxWidgets too and takes longer. Later builds are incremental as long as the build folder is kept (§8)
- Upgrading within 3.2.x is API/ABI compatible (bug fixes only): replace the folder with the same subset and rebuild (see `VERSION.txt`). Don't move to 3.3.x as part of this work (new series with larger changes)

---

## 3. CMake Migration (✅ Done 2026-11-18, Windows)

Maintaining both `.vcxproj` files and an Xcode project would mean editing two places every time a file is added. Use a single CMake build for both platforms instead.

```
CMakeLists.txt                  # Root: reads VERSION, common settings, add_subdirectory for each part
CMakePresets.json               # windows configure; windows-debug/-release build + test; windows-installer; windows-release workflow. Later: macos
GenerateProjectFiles.bat        # = cmake --preset windows (generates build/windows/MissionLedger.sln)
├── MissionLedgerCore/          # Static library: src/** (model, controller, storage, export) + SQLite
├── MissionLedger/              # GUI app: src/** + main.cpp + .rc (Win: WIN32 exe / Mac: MACOSX_BUNDLE .app)
├── MissionLedgerCLI/           # CLI: src/** + main_cli.cpp
└── third_party/
    ├── sqlite/
    ├── pdfwriter/              # add_subdirectory
    └── wxWidgets/              # add_subdirectory (3.2.8, vendored)
```

### Source collection

```cmake
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h)
add_library(MissionLedgerCore STATIC ${CORE_SOURCES} ${PROJECT_SOURCE_DIR}/third_party/sqlite/sqlite3.c)
target_include_directories(MissionLedgerCore PUBLIC src)       # GUI/CLI inherit Core's include path
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${CORE_SOURCES})   # IDE filters = folder tree
```

- Adding a file = create it in the project's folder, then build. CMake notices the new file and regenerates the IDE project
- Every file under a project's folder is compiled, so don't keep backup or scratch `.cpp` files there
- Include paths stay rooted at each project's `src/`, so `#include` lines don't change

### Daily workflow

```bash
cmake --preset windows                   # generate build/windows/MissionLedger.sln (once, or when presets change)
cmake --build --preset windows-release   # or open the .sln and press F5 as before
ctest --preset windows-release           # CLI smoke tests (tests/CMakeLists.txt)
cmake --build --preset windows-installer # installer (Release only)
cmake --workflow --preset windows-release # configure → Release build → smoke test → installer (= build_installer.bat)
```

| Task | Before (MSBuild) | Now (CMake) |
|---|---|---|
| Edit, build, debug | Visual Studio | Unchanged (generated `.sln`, or VS "Open Folder") |
| Add a source file | Solution Explorer | Create it in the folder; never through the IDE (it would go into the generated project only, possibly under `build/`) |
| Include paths, defines, link options | Property pages | `CMakeLists.txt` |
| Debugger arguments / working dir | Property pages | `VS_DEBUGGER_*` target properties or `launch.vs.json` |
| Build output | `x64\Release\` | `build/windows/bin/<Config>/` (`.iss` default `ExeDir`, `build_installer.bat`, `tests/fixtures/README.md` updated) |
| New PC | Build wxWidgets, set `WXWIN`, run CMake for PDF-Writer | Clone, run `GenerateProjectFiles.bat`, build |

### Migration steps (all done 2026-11-18)

1. ✅ CMake build written next to the `.sln`, using the verified wxWidgets options in §2. Settings ported from the vcxproj files: include paths, `/utf-8`, Unicode, SubSystem, Release PDB (`/Zi /DEBUG /OPT:REF /OPT:ICF /Gy /Oi`). Added: C++17 (the vcxproj files had no standard set, so MSVC used C++14). Not ported: `/GL` (whole program optimization)
2. ✅ Compared with the MSBuild output on Windows: CLI `list` / `budget list` identical, both PDFs byte-identical except the document ID, same exe version info, GUI checked by hand, installer builds. Found and fixed on the way: Korean ProductName/FileDescription in the exe properties were garbled in both builds (`#pragma code_page` came after the include in `.rc`)
3. ✅ Deleted `.sln`, `.vcxproj`, `.filters`, `Version.props`; `build_installer.bat` uses the presets; `.iss` defaults to the CMake output. Verified again from a clean state (no `build/`, no `x64/`, no PDF-Writer pre-build): `build_installer.bat` alone configures, builds and packages in about 2 minutes
4. ✅ `/build/` added to `.gitignore`

---

## 4. Pipeline Structure

```
Platforms (matrix, both run to the end) ──────────────────────────────────────────┐
  WindowsMini: Preflight → Clean? → Configure → Build → Test → Package* → Archive │→ Deploy** (on WindowsMini)
  MacMini:     Preflight → Clean? → Configure → Build → Test → Package* → Archive │
* Release only. Windows: Inno Setup .exe; macOS: unsigned .dmg (since roadmap 7 step ③, §6)
** Only for a release (tag v<VERSION> on this commit) with PLATFORM=both, Release, DEPLOY on; after both platforms succeeded
```

| Parameter | Values (default first) | Effect |
|---|---|---|
| `PLATFORM` | `both`, `windows`, `macos` | A platform that is not selected is skipped without waiting for its node |
| `CONFIGURATION` | `Release`, `Debug` | Preset `<platform>-release` / `-debug`. Debug skips Package and archives only the smoke test PDFs (a Debug exe needs the debug runtime, so it is not shipped) |
| `CLEAN_BUILD` | off | Deletes `build/<platform>/` first. Normal builds keep the workspace so wxWidgets is not rebuilt |
| `DEPLOY` | on | Upload when the commit is a release (tag `v<VERSION>`); an untagged commit never uploads, so on is safe as the default. A release also forces a clean build. Needs `PLATFORM=both` and `Release` (otherwise the build is UNSTABLE with a message and nothing is uploaded). Untick to rebuild a tagged commit without uploading |
| `DEPLOY_DRY_RUN` | off | Runs the Deploy stage on any commit without writing: rclone, the secret file, the remote, the existing folders and the release notes are checked, uploads run with `rclone --dry-run`. Needs `PLATFORM=both`, `Release` |

- If one platform fails the whole build is FAILURE, but the other platform still finishes (matrix `failFast` off), so both problems show up in one run
- Started by hand ("Build with Parameters"). Polling or a webhook can be added later (`triggers { pollSCM(...) }`)
- Failure notification is a stub (`notifyResult()` in the Jenkinsfile prints a line); replace its body with mail or a messenger webhook when needed

---

## 5. Jenkinsfile (`jenkins/Build.Jenkinsfile`)

Written 2026-11-18; the commands were first run on Windows in a fresh clone with a 108-character workspace path (all steps passed). **First successful Jenkins build on 2026-11-19** (`PLATFORM=both`, `Release`): WindowsMini built, tested and packaged, MacMini passed the node check. It took about 10 minutes, the first build on a node, which also compiles wxWidgets and PDF-Writer and clones the repository (wxWidgets source is about 86 MB). Later builds keep the workspace and should be much shorter (not measured yet).

### Files

| File | Purpose |
|---|---|
| `jenkins/Build.Jenkinsfile` | The pipeline (§4). Node names, the platforms that build and the platforms that package are at the top (`PLATFORMS`, `IMPLEMENTED`, `PACKAGED`) |
| `jenkins/scripts/release_check.cmake` | Is this build a release? Writes `yes`/`no` (Preflight, and again in Deploy) |
| `jenkins/scripts/deploy.cmake` | The upload (§4 `DEPLOY`): both installers + `릴리스노트.txt` (this version's CHANGELOG section, CRLF + BOM for Notepad) to `gdrive:MissionLedger/v<VERSION>/`; refuses if that folder exists; then `rclone sync` to `latest/` only if this is the highest version there (`sync` keeps the folder and its share link). Tested with a fake rclone on a local folder (2026-11-21) |
| `scripts/release.cmake` + `release.bat` / `release.sh` | Release script (docs/VERSIONING.md §3) |
| `jenkins/scripts/preflight.cmake` | Node check, first stage on every platform: CMake 3.25+ (warns on a release candidate), Git, VS 2022 with C++ x64 tools (`vswhere`) and `vc_redist.x64.exe`, Inno Setup, workspace path length (warns above 120 characters; the longest path below the workspace is about 132) / Xcode on macOS. Lists every problem, then fails. Also runs on a PC: `cmake -DPLATFORM=windows -P jenkins/scripts/preflight.cmake` |
| `jenkins/scripts/node_check_windows.ps1`, `node_check_macos.sh` | Read-only inventory for setting up a node by hand: where CMake / VS / vc_redist / Inno Setup / Git (Windows) or CMake / Homebrew / Xcode / Git / Java (macOS) are, how the Jenkins agent runs and as which user, and the PATH entries. Needs no CMake, so it also works when Preflight cannot start. Run from the repository or the node's Jenkins workspace: `powershell -NoProfile -ExecutionPolicy Bypass -File jenkins\scripts\node_check_windows.ps1` / `sh jenkins/scripts/node_check_macos.sh` |
| `jenkins/scripts/node_setup_windows.ps1`, `node_setup_macos.sh` | **Run by hand, once per node** (the pipeline only checks, it never installs: nodes are shared, installs need admin rights, and the agent would not see a new PATH in the same run). Installs only what is missing, at pinned versions with SHA-256 checks: Windows CMake 3.31.12 (MSI, system PATH) and Inno Setup 6.7.4 (all users); macOS CMake 3.31.12 (official universal package in `/Applications/CMake.app`, not Homebrew, which installs the newest untested version). Visual Studio, Git and Xcode are only checked, with instructions. Windows: administrator PowerShell, `powershell -NoProfile -ExecutionPolicy Bypass -File jenkins\scripts\node_setup_windows.ps1`; macOS: `sh jenkins/scripts/node_setup_macos.sh`. Run on both nodes on 2026-11-19 (installed CMake 3.31.12 on both and Inno Setup 6.7.4 on WindowsMini, hashes matched); preflight and the CMake check in the Jenkinsfile print these commands when something is missing |

### Stages per platform (CMake presets, §3)

| Stage | Command | When |
|---|---|---|
| Preflight | Check that `cmake` is on the agent's PATH (clear message with the agent PATH if not), then `cmake -DPLATFORM=<p> -DWORKSPACE=<ws> -P jenkins/scripts/preflight.cmake`; also sets the build name `#N v1.0.0 Release` | Always |
| Clean | delete `build/<p>/` | `CLEAN_BUILD`, or a release that is uploaded |
| Configure | `cmake --preset <p> -DML_BUILD_NUMBER=<BUILD_NUMBER>` | Implemented platforms |
| Build | `cmake --build --preset <p>-<config> --parallel` | 〃 |
| Test | `ctest --preset <p>-<config> --output-junit ctest-junit.xml` → JUnit report `build/<p>/ctest-junit.xml` | 〃 |
| Package | delete `installer/output/`, then `cmake --build --preset <p>-installer`; `stash` the installer when this build uploads | Release, packaged platforms (`PACKAGED`: Windows, macOS) |
| Archive | `installer/output/*`, `build/<p>/bin/Release/*.pdb` (Release, packaged platforms); `build/<p>/tests/out/<config>/*.pdf` | Implemented platforms |

### Jenkins setup (once)

1. Plugins: Pipeline, Git, Timestamper, JUnit
2. Nodes `WindowsMini` and `MacMini` online (the node name is used as the label). Tools per §8. Per node: `node_check_*` (what is installed where) → `node_setup_*` (install what is missing) → let the agent see CMake → run the build (Preflight checks the rest). The agent's PATH can differ from a terminal's, so add it under Manage Jenkins → Nodes → node → Configure → Environment variables: WindowsMini `PATH+CMAKE` = `C:\Program Files\CMake\bin` (or reboot: the agent is a Windows service and keeps the PATH it started with), MacMini `PATH+CMAKE` = `/Applications/CMake.app/Contents/bin` (the agent is started over SSH and does not read the shell profile). Checked on 2026-11-19: WindowsMini has VS Community 2022 17.14 with C++ tools and Git, but no CMake and no Inno Setup; MacMini has Xcode 16.4 (arm64), Git and Homebrew, but no CMake
3. New Item → **Pipeline** → Definition "Pipeline script from SCM" → SCM Git, `https://github.com/misterq55/MissionLedger.git` (public, no credentials) → Branch `*/main` → Script Path `jenkins/Build.Jenkinsfile`
4. The first run uses the default parameters (`PLATFORM=both`, so `MacMini` must be online); afterwards "Build with Parameters" shows the choices

### Notes

- First run (build #2, 2026-11-19): pipeline structure worked on both nodes (checkout, matrix, failure notification stub); both failed in Preflight because `cmake` was not on the agents' PATH. Fixed by `node_setup_*` and `PATH+CMAKE` in the node settings; the next run succeeded. The agent's real environment is shown under Manage Jenkins → Nodes → node → System Information
- Timeouts: 60 minutes per platform, **including the wait for its node** (Declarative cannot time the wait alone). If a node is offline, run with `PLATFORM` set to the other platform
- Archive retention: last 20 builds, artifacts of the last 5 (an installer is about 28 MB)
- Korean CLI output can look garbled in the Jenkins console (code page); the smoke test reads the output as UTF-8, so results are not affected
- The former draft (Multibranch, tag trigger, GitHub Releases upload with `gh release create` and release notes from the CHANGELOG section) is in the history of this file. Its remaining ideas moved to roadmap 8 and 10

---

## 6. macOS Build

### Code Readiness

- `MLPDFDocument.cpp` (font search paths) and `main_cli.cpp` (UTF-8 argument conversion) already have `_WIN32` / `__APPLE__` branches
- wxWidgets, SQLite, and PDF-Writer are all cross-platform libraries, and all three are built from the repository
- The main blocker is the build system (→ CMake migration)

### Additional macOS Work

| Item | Details |
|---|---|
| wxWidgets | Built from `third_party/wxWidgets/` like on Windows. `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` makes wxWidgets and the app Universal in one build. No Homebrew |
| App bundle | ✅ (step ③) `MissionLedger/resources/Info.plist.in`, `app.icns` made from the PNGs in `app.ico`, `.ml` registered with `CFBundleDocumentTypes` / `UTExportedTypeDeclarations` |
| Opening files | ✅ (step ③, not run yet) macOS passes double-clicked files through `wxApp::MacOpenFiles()` (an Apple Event to the running app), not argv. `main.cpp` overrides it; argv, `MacOpenFiles` and File → Open all end in `wxMLMainFrame::openFileAtPath()` |
| File paths | Check that the Korean path handling in `main.cpp` works on macOS (UTF-8 paths) |
| Fonts | ✅ Malgun Gothic is not available on macOS: Noto Sans KR is bundled for both platforms (step ②) |
| Installer | ✅ (step ③, moved up from roadmap 9) basic `.dmg` with `hdiutil` only (app + `Applications` link): `installer/package_macos.cmake`, `macos-installer` preset, built by the Jenkins Package stage. Styling (background, window layout) left for roadmap 9 |
| Signing / notarization | Deferred (§0). Requires the Apple Developer Program. Sign with `codesign` (hardened runtime) and notarize with `notarytool`. Unsigned builds work on the build Mac and, after "Open Anyway", on other Macs |

### How the macOS work is done (decided 2026-11-19)

Working directly on the Mac is inconvenient, so the build is developed through CI:

- Edit on Windows → push to `main` (no separate branch) → run Jenkins with `PLATFORM=macos` → read the log → fix. Every few changes also run `PLATFORM=windows` (or `both`) to make sure Windows still builds
- Mac-only CMake goes inside `if(APPLE)`, so the Windows build is not affected
- Limit: the MacMini agent is started over SSH and cannot open windows, so the GUI (app launch, Korean text on screen, double-click opening of `.ml`) cannot be checked by Jenkins. Check it by hand once at the end (Screen Sharing / VNC, or at the Mac), with a checklist
- Facts checked: the vendored wxWidgets includes the macOS sources (`src/osx`, `include/wx/osx`); MacMini is arm64 with Xcode 16.4 and CMake 3.31.12 (`/Applications/CMake.app`); the first build on the Mac is expected to take 10 minutes or more (wxWidgets)
- Approval (decided 2026-11-20): no advance approval added; the per-turn rule in CLAUDE.md stays. Each try needs someone to run Jenkins and read the log anyway, so approving a push costs only one extra message
- Jenkins from Claude Code (set up 2026-11-20, per PC): Claude can start a build and read its log through the Jenkins REST API (`POST {JENKINS_URL}/job/MissionLedger/buildWithParameters?PLATFORM=macos`, then `.../<n>/api/json` and `.../<n>/consoleText`), authenticated with a Jenkins API token. Setup on each PC: create a token per PC in Jenkins (user → Security → API Token), store it as the user environment variables `JENKINS_URL` (no trailing `/`), `JENKINS_USER`, `JENKINS_TOKEN`, never in the repository or in chat. Starting a build needs approval like a push (e.g. "push and run the build"); reading status and logs does not. The controller is served over plain http, so the token travels unencrypted: consider https before using it from outside the home network

| Step | Work | Checked by |
|---|---|---|
| ① | `macos` configure/build/test presets (Xcode generator: multi-config like Visual Studio, so `bin/<Config>/` and the `-debug`/`-release` presets match Windows; Ninja would be an extra tool on the node), `ML_BUILD_GUI` option (the `macos` preset sets it OFF, so wxWidgets is not even configured), build **Core + CLI only**. The Jenkinsfile builds macOS from here (`IMPLEMENTED`); Package and the PDB archive stay Windows only (`PACKAGED`). ✅ **Done 2026-11-20** (Jenkins #6, `PLATFORM=both`: Mac build and 5/5 smoke tests, Windows unchanged; incremental Mac build about 2 minutes). Found on the way: #4 the repository root was on Core's include path, and on the case-insensitive macOS file system the root `VERSION` file was picked up for the C++ header `<version>` (fixed: include `third_party/sqlite` instead of the root; never put the root on an include path); #5 CLI/PDF thousands separators came from `std::locale("")`, and the agent runs with the C locale, so two tests failed (fixed: locale-independent `FMLFormat` in Core, which also led to the formatting fixes in CHANGELOG `[미배포]`) | Compile, smoke tests (`ctest --preset macos-release`) |
| ② | ✅ **Done 2026-11-20** (Jenkins #7, both platforms green; the Mac PDFs embed `NotoSansKR-Regular` and `NotoSansKR-Bold`). Fonts: `notofonts/noto-cjk` tag `Sans2.004`, `Sans/SubsetOTF/KR` Regular + Bold `.otf` (CFF; the official repository has static Korean weights only as OTF, Google Fonts only a variable font), `LICENSE` (OFL), `VERSION.txt` with URLs and SHA-256. A CMake `fonts` target copies them to `bin/<Config>/fonts/` on every build; the installer installs them to `{app}\fonts\` (installer about 28 → 34 MB); the smoke tests check that both faces are embedded (verified to fail when `fonts/` is missing). Coverage checked in a PDF with sample characters: Hangul, Korean Hanja, kana, Japanese shinjitai (気駅広) and Traditional Chinese are drawn, Simplified Chinese (们这说话) is not. Plan as written before the work: PDF Korean font (§0 "PDF font"): Malgun Gothic does not exist on macOS, so bundle Noto Sans KR in `third_party/fonts/NotoSansKR/` (with `OFL.txt`) for both platforms. Work: (a) `getExecutablePath()` in `MLPDFDocument.cpp` reads `/proc/self/exe` on non-Windows, which does not exist on macOS, so the bundled `fonts/` is never found there: use `_NSGetExecutablePath` on `__APPLE__`; (b) also search `../Resources/fonts/` for the app bundle; (c) CMake copies the fonts into the build output; (d) enable the commented-out font line in `installer/MissionLedger.iss` and point it to `third_party/fonts`; (e) CHANGELOG `[미배포]`: Windows PDFs now use Noto Sans KR. Check the downloaded font's glyph map first. Observed in the Mac PDF of Jenkins #6 (before this step): without a bundled font, export already falls back to `/System/Library/Fonts/AppleSDGothicNeo.ttc`; PDF-Writer embeds it as a CFF subset (`FontFile3`), Korean text and layout match Windows. But there is **no bold**: on macOS the bold lookup returns the same `.ttc` (its first face, Regular), so headings and totals are not bold. Bundling Noto Sans KR Bold fixes this | PDF artifacts, opened on Windows |
| ③ | ✅ **Built 2026-11-21** (Jenkins #9, both platforms green; running the app is not checked yet). `macos` preset: `ML_BUILD_GUI` ON, `CMAKE_OSX_DEPLOYMENT_TARGET` 11.0 (without it the minimum would be the build Mac's SDK, 15.5). `MACOSX_BUNDLE` with `Info.plist.in` (bundle ID `io.github.misterq55.MissionLedger`, `.ml` type `io.github.misterq55.missionledger.ml`: keep both once released), `app.icns`, fonts in `Contents/Resources/fonts/`, ad-hoc signature (`CODE_SIGN_IDENTITY "-"`). `MacOpenFiles()` in `main.cpp`. Basic `.dmg` (see the Installer row), about 12 MB, arm64 only until ④. Found on the way: Jenkins #8 failed at configure because the vendored wxWidgets lacked `wx-config.in` / `wx-config-inplace.in` (read only by non-MSVC builds); both added from the 3.2.8 archive. Mac Mini builds everything, wxWidgets included, in about a minute. GUI check by hand: deferred to the end of roadmap 7 (decided 2026-11-21), on a build from Jenkins | Build succeeds; running it is checked by hand at the end |
| ④ | ✅ **Done 2026-11-21** (Jenkins #12, both green). `macos` preset: `CMAKE_OSX_ARCHITECTURES` `arm64;x86_64`; wxWidgets, PDF-Writer (libpng etc.) and our code build for both without changes. The smoke tests also run on the x86_64 half (`cli_*_x86_64`, `arch -x86_64` through Rosetta 2, which the MacMini has; skipped without it): 10/10. `.dmg` about 16.6 MB (arm64 only: 12 MB). #11 failed in Package: `if(IN_LIST)` in a `cmake -P` script needs `cmake_minimum_required` (policy CMP0057) | `lipo -archs` in the Jenkins log (§7) |
| ⑤ | ✅ **Done 2026-11-21** (Jenkins #12) except the GUI check by hand. `PLATFORM=both` builds the full macOS app since step ③. §7 checks in `installer/package_macos.cmake`, before the `.dmg` is made: `lipo -archs` must list every `CMAKE_OSX_ARCHITECTURES` entry, and `otool -L` must show only `/usr/lib` and `/System` libraries (fails on e.g. `/opt/homebrew`). `otool -arch all -L` since Jenkins #13 (plain `otool -L` prints only the build Mac's architecture, arm64, of a Universal file; #12 checked only that half). (`macos` was already added to `IMPLEMENTED` in step ①, because Jenkins skips the build of a platform that is not in it) | Both platforms green |

---

## 7. Verifying macOS Builds (Checking from Windows)

A macOS app cannot be run directly on Windows: it is a Mach-O binary, and Apple's license allows macOS VMs only on Apple hardware.
→ **The macOS node runs the checks automatically, and people review the Jenkins results from a browser on Windows.**

| Check | Command on macOS | How to review on Windows |
|---|---|---|
| Architecture | `lipo -archs .../MacOS/MissionLedger` | Log (`arm64 x86_64`) |
| CLI behavior | `MissionLedgerCLI export-settlement sample.ml out.pdf` | **Download the PDF artifact and compare it side by side with the Windows build's PDF** |
| Missing dependencies | Use `otool -L` to look for external paths such as `/opt/homebrew` | Fail the build if any remain |
| Signature (once signing is set up) | `codesign --verify --deep --strict` | Log |
| Notarization / Gatekeeper (once signing is set up) | `spctl -a -vv MissionLedger.app` | Log (`accepted`) |
| GUI launch | Launch the app, then take a screenshot with `screencapture` | Screenshot PNG artifact |

- GUI screenshots require the agent to run in a **logged-in user session** with screen recording permission (an agent started as a LaunchDaemon cannot open windows)
- For a final manual check before a release, connect to the Mac from Windows with VNC (Screen Sharing)
- Extra safeguard: a Linux (wxGTK) build in WSL catches Windows-only code early

---

## 8. Notes for Shared Nodes

The nodes are shared with other projects, so keep the following in mind.

### What a Node Needs

Only general-purpose tools. Nothing specific to MissionLedger is installed on a node.

| | Windows | macOS |
|---|---|---|
| Compiler | Visual Studio 2022 **Build Tools** (no IDE needed): "Desktop development with C++" workload (v143, Windows 10/11 SDK). Also provides `vc_redist.x64.exe` for the installer | Xcode + Command Line Tools |
| CMake | A stable release (3.25+ for workflow presets); `node_setup_*` installs 3.31.12, the line the build was checked with (4.x likely works: the vendored libraries' `cmake_minimum_required` settings allow it, but no build was run). Avoid release candidates (the dev PC has 3.31.0-rc2) | Same |
| Git | ✅ | ✅ |
| Packaging | Inno Setup 6.7+ | `hdiutil` (built in) or `create-dmg` |
| Jenkins agent | Java runtime | Java runtime, running in a logged-in session (§7) |

Not needed: wxWidgets, `WXWIN`, any library path.

### Pin the Toolchain

| Item | Approach |
|---|---|
| wxWidgets / PDF-Writer / SQLite | Pinned by the repository (`third_party/`). Nothing to pin on the node |
| Xcode | Pin the version with `DEVELOPER_DIR=/Applications/Xcode_XX.app/Contents/Developer` |
| Visual Studio | Specify the CMake generator (`Visual Studio 17 2022`) in the preset; limit with `vswhere -version [17.0,18.0)` if several versions are installed |

### Labels

Use labels only for the general tools.
```groovy
agent { label 'windows && vs2022' }
agent { label 'macos && xcode' }
```

### macOS Code-Signing Keychain (once signing is set up)

- Don't put the certificate in the login keychain (builds from other projects could access it)
- Use a temporary keychain for each build: `security create-keychain` → `import` → `codesign` → `delete-keychain`
- Store the `.p12` and the notarization API key in Jenkins Credentials (Secret file / Secret text)

### Resources

- Executors are shared, so cap `/m` and `-j` if needed
- Serialize shared resources such as the signing stage with Lockable Resources
- Keep workspaces for incremental builds (don't use `cleanWs()`). This matters more now: the first build on a node compiles wxWidgets, later builds reuse it. Manage disk space (a few hundred MB per workspace) with `buildDiscarder` and periodic cleanup. Do clean builds only for tag builds
- Downloading is limited to the Git checkout; there are no other network fetches during the build

---

## 9. Additional Suggestions

- **Code signing (Windows)**: unsigned installers trigger SmartScreen warnings ("More info → Run anyway"). Sign the exe and the Setup with `signtool sign /fd sha256 /tr <timestamp-url>` and store the certificate in Credentials. Optional for non-commercial distribution
- **Keep PDBs**: archive the `.pdb` files for each release to analyze crash dumps
- **Debug build**: ✅ `CONFIGURATION=Debug` parameter, to catch errors that occur only in Debug
- **Update checks**: a future in-app update check can use the download location's API (GitHub Releases or Google Drive)

---

## 10. Roadmap

1. [x] Single-source versioning: `VERSION` → MSBuild (`Version.props`, replaced by CMake in step 4), `.iss`, `build_installer.bat` (2026-11-18)
2. [x] Core folder separation: Core sources in `MissionLedgerCore/src/` (2026-11-18, `e94b226`)
3. [x] Vendor wxWidgets 3.2.8 in `third_party/wxWidgets/` (release archive, trimmed; stand-alone CMake build verified) (2026-11-18)
4. [x] CMake migration: root `CMakeLists.txt`, `CMakePresets.json`, `GenerateProjectFiles.bat`, one `CMakeLists.txt` per project with automatic source collection, `add_subdirectory` for PDF-Writer and wxWidgets, `VERSION` → `project()` and `.rc`. Compared on Windows, then deleted the `.sln`/`.vcxproj`/`Version.props` and added `build/` to `.gitignore` (2026-11-18, see §3)
5. [x] CTest smoke tests (`list`/`budget list` totals, 2 PDFs from `tests/fixtures/sample.ml`), test presets, `installer` target (ISCC, Release only) and the `windows-release` workflow preset; `build_installer.bat` became a wrapper for the workflow. Checked that a wrong expected value fails the test and stops the workflow before packaging, and that `installer` refuses Debug (2026-11-18)
6. [x] `jenkins/Build.Jenkinsfile` (matrix WindowsMini/MacMini, parameters, macOS cell = preflight only) and `jenkins/scripts/preflight.cmake` written (2026-11-18). Checked on Windows: preflight passes on the dev PC and lists 3 problems on a simulated bare node; all Windows stages run in a fresh clone (Release + installer, Debug, JUnit file with 5 tests, exe version `1.0.0.<BUILD_NUMBER>`). Nodes prepared with `node_check_*` / `node_setup_*` and `PATH+CMAKE`; first successful Jenkins build 2026-11-19 (about 10 minutes, first build on the nodes)
7. [ ] macOS build (`macos` presets, Core + CLI, fonts, bundle and `MacOpenFiles`, Universal), developed through Jenkins with `PLATFORM=macos`: steps ①–⑤ and the way of working in §6 "How the macOS work is done"
8. [x] (done within roadmap 7, steps ③–⑤, 2026-11-21) Turn on the macOS cell fully: the checks of §7 (`lipo -archs`, `otool -L`) and the GUI in the build. (`macos` is in `IMPLEMENTED` since roadmap 7 step ①, building Core + CLI)
9. [ ] `.dmg` styling (the basic unsigned `.dmg` exists since roadmap 7 step ③); signing and notarization when distribution requires it (Windows signtool, macOS codesign/notarytool)
10. [ ] Deployment (written 2026-11-21 and tested locally with a fake rclone; still to do: a Jenkins run with DEPLOY_DRY_RUN, then the first real upload with the next release): Google Drive with rclone, `DEPLOY` stage, release script, tag check, `latest/` (§0, §4, §5). The `DEPLOY` default stays on: the tag decides, not the checkbox (decided 2026-11-21)
