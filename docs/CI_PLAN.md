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
| Deploy target | **Google Drive** under consideration instead of GitHub Releases (roadmap 10) | Likely rclone on the node with credentials in Jenkins. A personal Google account needs an OAuth user token (service accounts have no My Drive storage); a Workspace shared drive can use a service account |

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
- Deploy builds (roadmap 10): check that the release tag (`v1.2.3`) matches `VERSION` and fail if not. The job is not Multibranch, so it does not see tags by itself; check with git in the Deploy stage
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
  WindowsMini: Preflight → Clean? → Configure → Build → Test → Package* → Archive │→ Deploy (stub)
  MacMini:     Preflight → "not implemented" (until roadmap 7–8)                  │
* Release only
```

| Parameter | Values (default first) | Effect |
|---|---|---|
| `PLATFORM` | `both`, `windows`, `macos` | A platform that is not selected is skipped without waiting for its node |
| `CONFIGURATION` | `Release`, `Debug` | Preset `<platform>-release` / `-debug`. Debug skips Package and archives only the smoke test PDFs (a Debug exe needs the debug runtime, so it is not shipped) |
| `CLEAN_BUILD` | off | Deletes `build/<platform>/` first. Normal builds keep the workspace so wxWidgets is not rebuilt |
| `DEPLOY` | on | **Stub**: prints a message only. Roadmap 10 implements it (then: default off, force Release + clean build) |

- If one platform fails the whole build is FAILURE, but the other platform still finishes (matrix `failFast` off), so both problems show up in one run
- Started by hand ("Build with Parameters"). Polling or a webhook can be added later (`triggers { pollSCM(...) }`)
- Failure notification is a stub (`notifyResult()` in the Jenkinsfile prints a line); replace its body with mail or a messenger webhook when needed

---

## 5. Jenkinsfile (`jenkins/Build.Jenkinsfile`)

Written 2026-11-18. The commands were run on Windows in a fresh clone with a 108-character workspace path (all steps passed, see §10 step 6); the first run on the real Jenkins is still to be done.

### Files

| File | Purpose |
|---|---|
| `jenkins/Build.Jenkinsfile` | The pipeline (§4). Node names and the list of implemented platforms are at the top (`PLATFORMS`, `IMPLEMENTED`) |
| `jenkins/scripts/preflight.cmake` | Node check, first stage on every platform: CMake 3.25+ (warns on a release candidate), Git, VS 2022 with C++ x64 tools (`vswhere`) and `vc_redist.x64.exe`, Inno Setup, workspace path length (warns above 120 characters; the longest path below the workspace is about 132) / Xcode on macOS. Lists every problem, then fails. Also runs on a PC: `cmake -DPLATFORM=windows -P jenkins/scripts/preflight.cmake` |
| `jenkins/scripts/node_check_windows.ps1`, `node_check_macos.sh` | Read-only inventory for setting up a node by hand: where CMake / VS / vc_redist / Inno Setup / Git (Windows) or CMake / Homebrew / Xcode / Git / Java (macOS) are, how the Jenkins agent runs and as which user, and the PATH entries. Needs no CMake, so it also works when Preflight cannot start. Run from the repository or the node's Jenkins workspace: `powershell -NoProfile -ExecutionPolicy Bypass -File jenkins\scripts\node_check_windows.ps1` / `sh jenkins/scripts/node_check_macos.sh` |
| `jenkins/scripts/node_setup_windows.ps1`, `node_setup_macos.sh` | **Run by hand, once per node** (the pipeline only checks, it never installs: nodes are shared, installs need admin rights, and the agent would not see a new PATH in the same run). Installs only what is missing, at pinned versions with SHA-256 checks: Windows CMake 3.31.12 (MSI, system PATH) and Inno Setup 6.7.4 (all users); macOS CMake 3.31.12 (official universal package in `/Applications/CMake.app`, not Homebrew, which installs the newest untested version). Visual Studio, Git and Xcode are only checked, with instructions. Windows: administrator PowerShell, `powershell -NoProfile -ExecutionPolicy Bypass -File jenkins\scripts\node_setup_windows.ps1`; macOS: `sh jenkins/scripts/node_setup_macos.sh`. Not run end to end yet (parse/syntax checked only); preflight and the CMake check in the Jenkinsfile print these commands when something is missing |

### Stages per platform (CMake presets, §3)

| Stage | Command | When |
|---|---|---|
| Preflight | Check that `cmake` is on the agent's PATH (clear message with the agent PATH if not), then `cmake -DPLATFORM=<p> -DWORKSPACE=<ws> -P jenkins/scripts/preflight.cmake`; also sets the build name `#N v1.0.0 Release` | Always |
| Clean | delete `build/<p>/` | `CLEAN_BUILD` |
| Configure | `cmake --preset <p> -DML_BUILD_NUMBER=<BUILD_NUMBER>` | Implemented platforms |
| Build | `cmake --build --preset <p>-<config> --parallel` | 〃 |
| Test | `ctest --preset <p>-<config> --output-junit ctest-junit.xml` → JUnit report `build/<p>/ctest-junit.xml` | 〃 |
| Package | delete `installer/output/`, then `cmake --build --preset <p>-installer` | Release |
| Archive | `installer/output/*`, `build/<p>/bin/Release/*.pdb` (Release); `build/<p>/tests/out/<config>/*.pdf` | 〃 |

### Jenkins setup (once)

1. Plugins: Pipeline, Git, Timestamper, JUnit
2. Nodes `WindowsMini` and `MacMini` online (the node name is used as the label). Tools per §8. Per node: `node_check_*` (what is installed where) → `node_setup_*` (install what is missing) → let the agent see CMake → run the build (Preflight checks the rest). The agent's PATH can differ from a terminal's, so add it under Manage Jenkins → Nodes → node → Configure → Environment variables: WindowsMini `PATH+CMAKE` = `C:\Program Files\CMake\bin` (or reboot: the agent is a Windows service and keeps the PATH it started with), MacMini `PATH+CMAKE` = `/Applications/CMake.app/Contents/bin` (the agent is started over SSH and does not read the shell profile). Checked on 2026-11-19: WindowsMini has VS Community 2022 17.14 with C++ tools and Git, but no CMake and no Inno Setup; MacMini has Xcode 16.4 (arm64), Git and Homebrew, but no CMake
3. New Item → **Pipeline** → Definition "Pipeline script from SCM" → SCM Git, `https://github.com/misterq55/MissionLedger.git` (public, no credentials) → Branch `*/main` → Script Path `jenkins/Build.Jenkinsfile`
4. The first run uses the default parameters (`PLATFORM=both`, so `MacMini` must be online); afterwards "Build with Parameters" shows the choices

### Notes

- First run (build #2, 2026-11-19): pipeline structure worked on both nodes (checkout, matrix, failure notification stub); both failed in Preflight because `cmake` was not on the agents' PATH. The agent's real environment is shown under Manage Jenkins → Nodes → node → System Information
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
| App bundle | Write `Info.plist`, convert `app.ico` to `.icns`, register `.ml` with `CFBundleDocumentTypes` / `UTExportedTypeDeclarations` |
| Opening files | macOS passes double-clicked files through `wxApp::MacOpenFiles()`, not argv, so it must be overridden |
| File paths | Check that the Korean path handling in `main.cpp` works on macOS (UTF-8 paths) |
| Fonts | Malgun Gothic is not available. Use Apple SD Gothic Neo or bundle Noto Sans KR |
| Installer | `.dmg` (`create-dmg` or `hdiutil`), as a CMake target |
| Signing / notarization | Deferred (§0). Requires the Apple Developer Program. Sign with `codesign` (hardened runtime) and notarize with `notarytool`. Unsigned builds work on the build Mac and, after "Open Anyway", on other Macs |

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
6. [ ] `jenkins/Build.Jenkinsfile` (matrix WindowsMini/MacMini, parameters, macOS cell = preflight only) and `jenkins/scripts/preflight.cmake` written (2026-11-18). Checked on Windows: preflight passes on the dev PC and lists 3 problems on a simulated bare node; all Windows stages run in a fresh clone (Release + installer, Debug, JUnit file with 5 tests, exe version `1.0.0.<BUILD_NUMBER>`). **Remaining: first run on the real Jenkins**
7. [ ] Get a local macOS build working (`MacOpenFiles`, fonts, bundle, Universal), `macos` presets
8. [ ] Turn on the macOS cell: add `macos` to `IMPLEMENTED` in the Jenkinsfile, plus the checks of §7 (`lipo -archs`, `otool -L`)
9. [ ] `.dmg` creation; signing and notarization when distribution requires it (Windows signtool, macOS codesign/notarytool)
10. [ ] Deployment: implement the `DEPLOY` stage (Google Drive under consideration, §0): force Release + clean build, check `VERSION` against the release tag, upload the installers; release notes from this version's CHANGELOG section. Then set the `DEPLOY` default to off
