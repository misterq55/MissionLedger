# CI/CD Plan: Jenkins Build Automation (Windows + macOS)

> Created: 2026-11-18 · Decisions recorded: 2026-11-18
> Status: Planning (CI not implemented). Prerequisites partly done: single-source versioning, Core folder separation

## Goals

- Keep a `Jenkinsfile` in the repository and automate builds through a Jenkins pipeline
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
| Scripts | Build, test and packaging logic lives in CMake (`CMakePresets.json` + workflow presets), so the same command works on both platforms | `build_installer.bat` is replaced by `cmake --workflow --preset release`. Only steps that exist on a single platform (signing, notarization) get short platform scripts (PowerShell / sh) |
| C++ runtime (Windows) | Keep the dynamic runtime (`/MD`) and **bundle `vc_redist.x64.exe`** in the installer (already implemented, see [INSTALLER.md](INSTALLER.md)) | Optional later: switch to `/MT` with `CMAKE_MSVC_RUNTIME_LIBRARY` (applies to vendored wxWidgets and PDF-Writer too) and drop the bundled redist |
| macOS signing | Not now. Build and test unsigned first | Distributing to others without Gatekeeper warnings needs the Apple Developer Program ($99/year). Check Apple's nonprofit fee waiver first. A few known users can use System Settings → Privacy & Security → "Open Anyway" |
| Versioning | ✅ Done. Root `VERSION` file is the single source of truth | See §1 and [VERSIONING.md](VERSIONING.md) |

---

## 1. Issues to Fix First

| Issue | Location | Fix |
|---|---|---|
| The trailing `pause` hangs CI builds | `build_installer.bat` | Don't call this script from CI. After the CMake migration the script is replaced by a workflow preset |
| MSBuild path is hard-coded to the Community edition | `build_installer.bat`, CLAUDE.md | Goes away with the CMake migration (`cmake --build`) |
| ~~The version is hard-coded in 3 separate places~~ ✅ Done (2026-11-18) | `MLVersion.h`, `MissionLedger.iss` (`MyAppVersion`), `build_installer.bat` (output filename) | The root `VERSION` file is now the single source of truth (see [VERSIONING.md](VERSIONING.md)) |
| ~~Core sources live in the GUI folder~~ ✅ Done (2026-11-18, `e94b226`) | `MissionLedger/src/` | Moved to `MissionLedgerCore/src/` (one project = one folder) |
| wxWidgets is outside the repository and its build is undocumented | `WXWIN` differs per PC (`C:\wxWidgets-3.2.6` on one, `C:\wxWidgets-3.2.8` on another) | ✅ Source vendored (2026-11-18): wxWidgets 3.2.8 in `third_party/wxWidgets/` (see §2). The build still uses `WXWIN` until the CMake migration |
| `third_party/pdfwriter/build` is gitignored, but `MissionLedger.sln` references the vcxproj files generated there | `.sln`, pdfwriter `.gitignore` | A fresh clone must run CMake for PDF-Writer before the `.sln` opens. Goes away with `add_subdirectory` |
| ~~No sample `.ml` file is committed for smoke tests~~ ✅ Done (2026-11-18) | All `.ml` files in the root are untracked | `tests/fixtures/sample.ml` with made-up data (see [tests/fixtures/README.md](../tests/fixtures/README.md)) |
| The build system works only with Visual Studio | `.sln` / `.vcxproj` | **Migrate to CMake** (required before a macOS build is possible) |

### Single-Source Versioning (✅ Implemented 2026-11-18)

The policy and the full flow are documented in [VERSIONING.md](VERSIONING.md). What CI relies on:

- **Source of truth**: the root `VERSION` file (`MAJOR.MINOR.PATCH`, one line). CI reads it with `readFile('VERSION')`
- **Build number**: currently `msbuild ... /p:MLBuildNumber=%BUILD_NUMBER%` (read by `Version.props`). After the CMake migration: `-DML_BUILD_NUMBER=%BUILD_NUMBER%`
- **Installer**: `MissionLedger.iss` reads `VERSION` itself; `ISCC /DMyAppVersion=x.y.z` still overrides it. ISCC fails if the packaged exe's `ProductVersion` differs from it
- On tag builds, check that the Git tag (`v1.2.3`) matches `VERSION`, and fail the build if they differ (in the Jenkinsfile)
- After the CMake migration: `file(READ VERSION ...)` → `project(MissionLedger VERSION ...)` → `.rc` defines, and `CFBundleShortVersionString` / `CFBundleVersion` in `Info.plist` via `configure_file`. `Version.props` is then removed

---

## 2. Dependencies (Deps)

"Deps" is short for **dependencies**: the external libraries that must be ready before the application can be built.

| Library | Now | After the CMake migration | CI work |
|---|---|---|---|
| SQLite | Amalgamation source in `third_party/sqlite/`, compiled inside the Core project | `sqlite3.c` added to the Core target | None |
| PDF-Writer | Source in `third_party/pdfwriter/`; CMake-generated vcxproj files added to `MissionLedger.sln` | `add_subdirectory(third_party/pdfwriter)` | None |
| wxWidgets | Prebuilt outside the repository, found via `WXWIN` (version differs per PC). The 3.2.8 source is already in `third_party/wxWidgets/` but nothing uses it yet | **Source in `third_party/wxWidgets/` (3.2.8)**, `add_subdirectory` | None |

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

## 3. CMake Migration (Required for the macOS Build)

Maintaining both `.vcxproj` files and an Xcode project would mean editing two places every time a file is added. Use a single CMake build for both platforms instead.

```
CMakeLists.txt                  # Root: reads VERSION, common settings, add_subdirectory for each part
CMakePresets.json               # Configure/build/test/package presets and workflow presets (Win/Mac)
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
cmake --workflow --preset release        # configure → build → smoke test → installer, same on Win/Mac
```

| Task | Now | After |
|---|---|---|
| Edit, build, debug | Visual Studio | Unchanged (generated `.sln`, or VS "Open Folder") |
| Add a source file | Solution Explorer | Create it in the folder; never through the IDE (it would go into the generated project only, possibly under `build/`) |
| Include paths, defines, link options | Property pages | `CMakeLists.txt` |
| Debugger arguments / working dir | Property pages | `VS_DEBUGGER_*` target properties or `launch.vs.json` |
| Build output | `x64\Release\` | `build/<preset>/...` (update `.iss` and other paths accordingly) |
| New PC | Build wxWidgets, set `WXWIN`, run CMake for PDF-Writer | Clone and build |

### Migration steps

1. Write the CMake build next to the existing `.sln` (wxWidgets 3.2.8 is already vendored; use the verified options in §2)
2. Check on Windows that the results match the current MSBuild output: run the GUI, generate both PDFs with the CLI, compare exe version info, build the installer
3. **Then delete the old build files right away** (`.sln`, `.vcxproj`, `.filters`, `Version.props`) so files don't have to be registered in two places. Keep the transition short
4. Add `build/` to `.gitignore`

---

## 4. Pipeline Structure

```
Checkout → Version → Build → Smoke Test → Package → (Sign) → Archive → Release
                     └── Windows / macOS in parallel ──┘
```

| Trigger | Scope |
|---|---|
| PR / feature branch | Build + Smoke Test |
| `main` branch | + Package + Archive (installers for internal testing) |
| `v*` tag | + approval (`input`), then deploy to GitHub Releases |

- Use a Multibranch Pipeline with tag discovery enabled (needed for `buildingTag()`)
- Trigger: a GitHub webhook is recommended; use `pollSCM('H/15 * * * *')` if Jenkins can't be reached from outside
- Deploy target: GitHub Releases (`gh release create`, with the token stored in Jenkins Credentials)

---

## 5. Draft Jenkinsfile (CMake-Based)

> Written for after the CMake migration. Preset names are placeholders until `CMakePresets.json` exists. No wxWidgets path or `WXWIN` is needed: it is built from `third_party/`.

```groovy
pipeline {
    agent none

    options {
        timestamps()
        timeout(time: 90, unit: 'MINUTES')      // first build on a node also compiles wxWidgets
        disableConcurrentBuilds()
        buildDiscarder(logRotator(numToKeepStr: '20', artifactNumToKeepStr: '5'))
    }

    stages {
        stage('Version') {
            agent any
            steps {
                script {
                    env.APP_VERSION = readFile('VERSION').trim()
                    currentBuild.displayName = "#${BUILD_NUMBER} v${APP_VERSION}"
                    if (buildingTag() && env.TAG_NAME != "v${env.APP_VERSION}") {
                        error "Tag (${env.TAG_NAME}) does not match VERSION (v${env.APP_VERSION})"
                    }
                }
            }
        }

        stage('Build & Package') {
            parallel {
                stage('Windows') {
                    agent { label 'windows && vs2022' }
                    environment { ISCC = 'C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe' }
                    steps {
                        bat 'cmake --preset windows -DML_BUILD_NUMBER=%BUILD_NUMBER%'
                        bat 'cmake --build --preset windows-release'
                        bat 'ctest --preset windows-release'          // CLI smoke test: sample.ml → 2 PDFs
                        script {
                            if (env.BRANCH_NAME == 'main' || buildingTag()) {
                                bat 'cmake --build --preset windows-release --target installer'   // runs ISCC
                            }
                        }
                        stash name: 'win', includes: 'installer/output/*.exe', allowEmpty: true
                    }
                    post { success { archiveArtifacts artifacts: 'installer/output/*.exe, build/**/Release/*.pdb, build/**/smoke/*.pdf', allowEmptyArchive: true, fingerprint: true } }
                }
                stage('macOS') {
                    agent { label 'macos && xcode' }
                    steps {
                        sh 'cmake --preset macos -DML_BUILD_NUMBER=$BUILD_NUMBER'
                        sh 'cmake --build --preset macos-release'
                        sh 'ctest --preset macos-release'
                        sh 'lipo -archs build/macos/MissionLedger/Release/MissionLedger.app/Contents/MacOS/MissionLedger'
                        // later: codesign → notarize → dmg (§6)
                        stash name: 'mac', includes: 'build/macos/**/*.dmg', allowEmpty: true
                    }
                }
            }
        }

        stage('Release') {
            when { buildingTag() }
            agent { label 'windows' }
            steps {
                input message: "Deploy v${APP_VERSION} to GitHub Releases?", ok: 'Deploy'
                unstash 'win'
                unstash 'mac'
                withCredentials([string(credentialsId: 'github-token', variable: 'GH_TOKEN')]) {
                    // release notes = this version's section of CHANGELOG.md, extracted to out\release-notes.md
                    bat 'gh release create %TAG_NAME% installer\\output\\MissionLedger-%APP_VERSION%-Setup.exe --title "MissionLedger %APP_VERSION%" --notes-file out\\release-notes.md'
                }
            }
        }
    }
}
```

- Until the CMake migration is done, there is no Jenkinsfile. The previous MSBuild-only draft (which needed `WXWIN` and a separate PDF-Writer stage) is in the history of this file

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
| CMake | A stable release (3.25+ for workflow presets). Avoid release candidates (the dev PC currently has 4.0.0-rc2) | Same |
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
- **Parallel Debug build**: compile the Debug configuration in PR builds too, to catch errors that occur only in Debug
- **Update checks**: a future in-app update check can use the GitHub Releases API directly

---

## 10. Roadmap

1. [x] Single-source versioning: `VERSION` → MSBuild (`Version.props`), `.iss`, `build_installer.bat` (2026-11-18)
2. [x] Core folder separation: Core sources in `MissionLedgerCore/src/` (2026-11-18, `e94b226`)
3. [x] Vendor wxWidgets 3.2.8 in `third_party/wxWidgets/` (release archive, trimmed; stand-alone CMake build verified) (2026-11-18)
4. [ ] CMake migration: root `CMakeLists.txt`, `CMakePresets.json`, one `CMakeLists.txt` per project with automatic source collection, `add_subdirectory` for PDF-Writer and wxWidgets, `VERSION` → `project()` and `.rc`, installer target. Compare results on Windows, then delete the `.sln`/`.vcxproj`/`Version.props` and add `build/` to `.gitignore`
5. [ ] CTest smoke test (CLI → 2 PDFs). Fixture done: `tests/fixtures/sample.ml` (2026-11-18)
6. [ ] Write the Jenkinsfile (Windows first)
7. [ ] Get a local macOS build working (`MacOpenFiles`, fonts, bundle, Universal)
8. [ ] Add the parallel macOS stage (including automated checks)
9. [ ] `.dmg` creation; signing and notarization when distribution requires it (Windows signtool, macOS codesign/notarytool)
10. [ ] Tag-based deployment to GitHub Releases
