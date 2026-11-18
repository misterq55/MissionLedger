# CI/CD Plan: Jenkins Build Automation (Windows + macOS)

> Created: 2026-11-18
> Status: Planning (not implemented)

## Goals

- Keep a `Jenkinsfile` in the repository and automate builds through a Jenkins pipeline
- Stages: Build → Test → Installer → Deploy
- Target platforms: **Windows x64** and **macOS** (Universal: arm64 + x86_64)
- Build machines: existing Windows/macOS machines that are already Jenkins nodes for other projects

---

## 1. Issues to Fix First

| Issue | Location | Fix |
|---|---|---|
| The trailing `pause` hangs CI builds | `build_installer.bat` | Don't call this script from CI; run each step directly in the Jenkinsfile |
| MSBuild path is hard-coded to the Community edition | `build_installer.bat`, CLAUDE.md | Find it with `vswhere.exe` (no longer needed after the CMake migration) |
| ~~The version is hard-coded in 3 separate places~~ ✅ Done (2026-11-18) | `MLVersion.h`, `MissionLedger.iss` (`MyAppVersion`), `build_installer.bat` (output filename) | The root `VERSION` file is now the single source of truth (see [VERSIONING.md](VERSIONING.md)) |
| `third_party/pdfwriter/build` is gitignored | pdfwriter `.gitignore` | Build it with CMake in CI (folded in via `add_subdirectory` after the CMake migration) |
| No sample `.ml` file is committed for smoke tests | All `.ml` files in the root are untracked | Add `tests/fixtures/sample.ml` with non-personal data |
| The build system works only with Visual Studio | `.sln` / `.vcxproj` | **Migrate to CMake** (required before a macOS build is possible) |

### Single-Source Versioning (✅ Implemented 2026-11-18)

The policy and the full flow are documented in [VERSIONING.md](VERSIONING.md). What CI relies on:

- **Source of truth**: the root `VERSION` file (`MAJOR.MINOR.PATCH`, one line). CI reads it with `Get-Content VERSION`
- **Build number**: pass `msbuild ... /p:MLBuildNumber=%BUILD_NUMBER%` (read by `Version.props`, becomes the 4th part of the exe file version)
- **Installer**: `MissionLedger.iss` reads `VERSION` itself; `ISCC /DMyAppVersion=x.y.z` still overrides it. ISCC fails if the packaged exe's `ProductVersion` differs from it
- On tag builds, check that the Git tag (`v1.2.3`) matches `VERSION`, and fail the build if they differ (still to do, in the Jenkinsfile)
- After the CMake migration: `file(READ VERSION ...)` → `project(MissionLedger VERSION ...)`, and generate `CFBundleShortVersionString` / `CFBundleVersion` in `Info.plist` with `configure_file`

---

## 2. Dependencies (Deps)

"Deps" is short for **dependencies**: the external libraries that must be ready before the application can be built.

| Library | How it is provided | CI work |
|---|---|---|
| wxWidgets 3.2.8 | Pre-installed on the node | Specify the path |
| SQLite | Amalgamation source is in the repository | None (compiled with the app) |
| PDF-Writer | Only the source is in the repository | Build with CMake (becomes part of the same build after the migration) |

---

## 3. CMake Migration (Required for the macOS Build)

Maintaining both `.vcxproj` files and an Xcode project would mean editing two places every time a file is added. Use a single CMake build for both platforms instead.

```
CMakeLists.txt                  # Root: common settings, find_package(wxWidgets)
├── MissionLedgerCore/          # Static library (SQLite, Model, Storage)
├── MissionLedger/              # GUI app (Win: WIN32 exe / Mac: MACOSX_BUNDLE .app)
├── MissionLedgerCLI/           # CLI
└── third_party/pdfwriter       # Included with add_subdirectory
```

The same build commands work on both platforms:
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

- Keep the existing `.sln` alongside CMake until the migration is stable
- After the migration, check on Windows that the results match the current MSBuild output (run the exe, generate PDFs with the CLI)

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

## 5. Draft Jenkinsfile (Current MSBuild Setup, Windows Only)

> After the CMake migration, replace the `Deps` and `Build` stages with CMake commands and add a parallel macOS stage.

```groovy
pipeline {
    agent { label 'windows && vs2022' }

    options {
        timestamps()
        timeout(time: 60, unit: 'MINUTES')
        disableConcurrentBuilds()
        buildDiscarder(logRotator(numToKeepStr: '20', artifactNumToKeepStr: '5'))
    }

    environment {
        WXWIN = 'C:\\wxWidgets-3.2.8'
        ISCC  = 'C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe'
    }

    stages {
        stage('Version') {
            steps {
                script {
                    env.MSBUILD = powershell(returnStdout: true, script: '''
                        & "${env:ProgramFiles(x86)}\\Microsoft Visual Studio\\Installer\\vswhere.exe" `
                          -latest -products * -requires Microsoft.Component.MSBuild `
                          -find "MSBuild\\**\\Bin\\MSBuild.exe" | Select-Object -First 1
                    ''').trim()

                    env.APP_VERSION = readFile('VERSION').trim()

                    currentBuild.displayName = "#${BUILD_NUMBER} v${APP_VERSION}"

                    if (buildingTag() && env.TAG_NAME != "v${env.APP_VERSION}") {
                        error "Tag (${env.TAG_NAME}) does not match VERSION (v${env.APP_VERSION})"
                    }
                }
            }
        }

        stage('Deps: PDF-Writer') {
            steps {
                bat '''
                    if not exist third_party\\pdfwriter\\build mkdir third_party\\pdfwriter\\build
                    cd third_party\\pdfwriter\\build
                    if not exist CMakeCache.txt cmake .. -G "Visual Studio 17 2022" -A x64 || exit /b 1
                    cmake --build . --config Release -- /m /v:minimal || exit /b 1
                '''
            }
        }

        stage('Build') {
            steps {
                bat '"%MSBUILD%" MissionLedger.sln /p:Configuration=Release /p:Platform=x64 /p:MLBuildNumber=%BUILD_NUMBER% /m /v:minimal /nologo'
            }
        }

        stage('Smoke Test') {
            steps {
                bat '''
                    if not exist out mkdir out
                    x64\\Release\\MissionLedgerCLI.exe export-settlement tests\\fixtures\\sample.ml out\\settlement.pdf || exit /b 1
                    x64\\Release\\MissionLedgerCLI.exe export-transactions tests\\fixtures\\sample.ml out\\transactions.pdf || exit /b 1
                    if not exist out\\settlement.pdf exit /b 1
                '''
            }
        }

        stage('Package') {
            when { anyOf { branch 'main'; buildingTag() } }
            steps {
                bat '"%ISCC%" /Qp /DMyAppVersion=%APP_VERSION% installer\\MissionLedger.iss'
            }
        }

        stage('Release') {
            when { buildingTag() }
            steps {
                input message: "Deploy v${APP_VERSION} to GitHub Releases?", ok: 'Deploy'
                withCredentials([string(credentialsId: 'github-token', variable: 'GH_TOKEN')]) {
                    bat 'gh release create %TAG_NAME% installer\\output\\MissionLedger-%APP_VERSION%-Setup.exe --title "MissionLedger %APP_VERSION%" --generate-notes'
                }
            }
        }
    }

    post {
        success {
            archiveArtifacts artifacts: 'installer/output/*.exe, x64/Release/*.exe, x64/Release/*.pdb, out/*.pdf',
                             allowEmptyArchive: true, fingerprint: true
        }
    }
}
```

### Multi-Platform Structure (After the CMake Migration)

```groovy
stage('Build & Package') {
    parallel {
        stage('Windows') {
            agent { label 'windows && vs2022 && wx-3.2' }
            steps { /* cmake build → smoke test → ISCC → (signtool) → stash */ }
        }
        stage('macOS') {
            agent { label 'macos && xcode && wx-3.2' }
            steps { /* cmake build → smoke test → .app → codesign → notarize → dmg → stash */ }
        }
    }
}
stage('Release') {
    when { buildingTag() }
    // Unstash both platforms' artifacts and upload them to a single GitHub Release
}
```

---

## 6. macOS Build

### Code Readiness

- `MLPDFExporter.cpp` (font paths) and `main_cli.cpp` (UTF-8 argument conversion) already have `_WIN32` / `__APPLE__` branches
- wxWidgets, SQLite, and PDF-Writer are all cross-platform libraries
- The main blocker is the build system (→ CMake migration)

### Additional macOS Work

| Item | Details |
|---|---|
| wxWidgets | Build it yourself and install it to a fixed path instead of using Homebrew (Universal static build; protects against `brew upgrade` from other projects) |
| App bundle | Write `Info.plist`, convert `app.ico` to `.icns`, register `.ml` with `CFBundleDocumentTypes` |
| Opening files | macOS passes double-clicked files through `wxApp::MacOpenFiles()`, not argv, so it must be overridden |
| Fonts | Malgun Gothic is not available. Use Apple SD Gothic Neo or bundle Noto Sans KR |
| Installer | `.dmg` (`create-dmg` or `hdiutil`) |
| Signing / notarization | Requires an Apple Developer account. Sign with `codesign` and notarize with `notarytool` (Gatekeeper blocks unsigned apps) |

---

## 7. Verifying macOS Builds (Checking from Windows)

A macOS app cannot be run directly on Windows: it is a Mach-O binary, and Apple's license allows macOS VMs only on Apple hardware.
→ **The macOS node runs the checks automatically, and people review the Jenkins results from a browser on Windows.**

| Check | Command on macOS | How to review on Windows |
|---|---|---|
| Architecture | `lipo -archs .../MacOS/MissionLedger` | Log (`arm64 x86_64`) |
| CLI behavior | `MissionLedgerCLI export-settlement sample.ml out.pdf` | **Download the PDF artifact and compare it side by side with the Windows build's PDF** |
| Missing dependencies | Use `otool -L` to look for external paths such as `/opt/homebrew` | Fail the build if any remain |
| Signature | `codesign --verify --deep --strict` | Log |
| Notarization / Gatekeeper | `spctl -a -vv MissionLedger.app` | Log (`accepted`) |
| GUI launch | Launch the app, then take a screenshot with `screencapture` | Screenshot PNG artifact |

- GUI screenshots require the agent to run in a **logged-in user session** with screen recording permission (an agent started as a LaunchDaemon cannot open windows)
- For a final manual check before a release, connect to the Mac from Windows with VNC (Screen Sharing)
- Extra safeguard: a Linux (wxGTK) build in WSL catches Windows-only code early

---

## 8. Notes for Shared Nodes

The nodes are shared with other projects, so keep the following in mind.

### Pin the Toolchain

| Item | Approach |
|---|---|
| wxWidgets | Don't rely on global environment variables; set the path explicitly in the Jenkinsfile `environment` (`C:\wxWidgets-3.2.8`, `/opt/wx/3.2.8`) |
| Xcode | Pin the version with `DEVELOPER_DIR=/Applications/Xcode_XX.app/Contents/Developer` |
| Visual Studio | Limit the range with `vswhere -version [17.0,18.0)` and specify the CMake generator explicitly |

### Labels

Use labels to show which tools are installed on each node.
```groovy
agent { label 'windows && vs2022 && wx-3.2' }
agent { label 'macos && xcode && wx-3.2' }
```

### macOS Code-Signing Keychain

- Don't put the certificate in the login keychain (builds from other projects could access it)
- Use a temporary keychain for each build: `security create-keychain` → `import` → `codesign` → `delete-keychain`
- Store the `.p12` and the notarization API key in Jenkins Credentials (Secret file / Secret text)

### Resources

- Executors are shared, so cap `/m` and `-j` if needed
- Serialize shared resources such as the signing stage with Lockable Resources
- Keep workspaces for incremental builds (don't use `cleanWs()`), but manage disk space with `buildDiscarder` and periodic cleanup. Do clean builds only for tag builds

---

## 9. Additional Suggestions

- **Code signing (Windows)**: unsigned installers trigger SmartScreen warnings. Sign the exe and the Setup with `signtool sign /fd sha256 /tr <timestamp-url>` and store the certificate in Credentials
- **Keep PDBs**: archive the `.pdb` files for each release to analyze crash dumps
- **Parallel Debug build**: compile the Debug configuration in PR builds too, to catch errors that occur only in Debug
- **Update checks**: a future in-app update check can use the GitHub Releases API directly

---

## 10. Roadmap

1. [ ] CMake migration: port sources, include paths, preprocessor definitions, and link settings from the 3 vcxproj files (Core, GUI, CLI), then compare results on Windows
2. [x] Single-source versioning: `VERSION` → MSBuild (`Version.props`), `.iss`, `build_installer.bat` (2026-11-18). CMake / `Info.plist` remain for steps 1 and 5
3. [ ] Add `tests/fixtures/sample.ml` for smoke tests
4. [ ] Write the Windows Jenkinsfile (CMake-based)
5. [ ] Get a local macOS build working (`MacOpenFiles`, fonts, bundle)
6. [ ] Add the parallel macOS stage (including automated checks)
7. [ ] Signing and notarization (Windows signtool, macOS codesign/notarytool), `.dmg` creation
8. [ ] Tag-based deployment to GitHub Releases
