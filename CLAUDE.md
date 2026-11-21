# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MissionLedger is a C++ desktop application built with wxWidgets for financial transaction management. The project uses Visual Studio 2022 (v143 toolset) and implements a classic MVC (Model-View-Controller) architecture with an observer pattern for data notifications.

**Architecture**: Model First MVC with Observer pattern, Storage Provider (Strategy Pattern), MVC Holder (Singleton), and DTO-based data exchange.

## Quick Reference

### Build Commands (CMake)

The build is defined by `CMakeLists.txt` files; the Visual Studio solution is **generated** into `build/windows/` and never committed (like Unreal's GenerateProjectFiles). Run from the repository root:

```bash
cmake --preset windows                    # generate build/windows/MissionLedger.sln (= GenerateProjectFiles.bat)
cmake --preset macos                      # macOS (Xcode), build/macos: MissionLedger.app + CLI (Universal, macOS 11.0+)
cmake --build --preset windows-debug      # Debug build
cmake --build --preset windows-release    # Release build
ctest --preset windows-release            # CLI smoke tests on tests/fixtures/sample.ml (or windows-debug)
cmake --build --preset windows-installer  # Inno Setup installer from the Release build (Release only)
cmake --workflow --preset windows-release # all of it: configure → Release build → smoke test → installer
cmake --workflow --preset macos-release   # same on macOS; the installer is an unsigned .dmg (installer/output/)
build_installer.bat                       # local double-click wrapper for the workflow above (ends with pause; not for CI)
release.bat 1.1.0                         # release: VERSION + CHANGELOG + commit + tag v1.1.0 (macOS: ./release.sh); then push and run Jenkins
```

- Output: `build/windows/bin/<Debug|Release>/MissionLedger.exe`, `MissionLedgerCLI.exe` (+ `.pdb`)
- Clean build: delete `build/` and generate again. The first build also compiles wxWidgets and PDF-Writer (about 1–2 minutes)
- CMake, not MSBuild, is called directly, so the Visual Studio edition path does not matter. In Git Bash pass `--parallel` instead of `/m` (MSYS path conversion mangles `/` switches; use `MSYS_NO_PATHCONV=1` if a `/` switch is needed)
- CI build number: `cmake --preset windows -DML_BUILD_NUMBER=N` (4th field of the exe version)
- CI (Jenkins) calls the presets one per stage (configure / build / test / installer) so each result shows separately; the workflow preset and `build_installer.bat` are for people
- Jenkins: one Pipeline job, Script Path `jenkins/Build.Jenkinsfile`, parameters `PLATFORM` (both/windows/macos), `BUILD` (`dev` default: Release build uploaded to Drive `dev/<n>-<commit>/`, newest 5 kept / `shipping`: the release, needs the pushed tag `v<VERSION>` and `PLATFORM=both`, clean build, `v<VERSION>/` + `latest/` / `test`: no upload / `debug`), `CLEAN_BUILD`, `DEPLOY_DRY_RUN`. Node check on any PC: `cmake -DPLATFORM=windows -P jenkins/scripts/preflight.cmake`
- Smoke tests check exit codes, the totals in `tests/fixtures/README.md`, that both PDFs are created (not PDF contents) and that opening a missing file fails without creating it; on macOS the same five also run on the x86_64 half (Rosetta). Output goes to `build/windows/tests/out/<Config>/`
- The `installer` target finds `ISCC.exe` in the default Inno Setup 6 folder; elsewhere configure with `-DML_ISCC=<path>`

### Working in the IDE

- **Generated solution**: run `GenerateProjectFiles.bat` once, open `build/windows/MissionLedger.sln`, edit/build/F5 as usual (startup project is MissionLedger)
- **Or Visual Studio "Open Folder"** on the repository root: VS reads `CMakePresets.json` directly, and Solution Explorer shows the real folders
- **Add a source file**: create it in the project's folder (in "Open Folder" mode Solution Explorer can do this). It is picked up on the next build. **Never** use "Add New Item" or property pages in the generated solution: those changes live only in `build/` and are lost on regeneration
- **Change include paths, defines, link options**: edit the project's `CMakeLists.txt`
- Every `.cpp`/`.h` under a project's `src/` is part of the build: don't keep backup or scratch files there
- Regenerate only after deleting `build/`, a fresh clone, or a preset change; otherwise the build re-runs CMake by itself

### Dependencies

All libraries come from the repository; nothing is installed or configured on the machine (no `WXWIN`).

- **Tools**: CMake 3.25+, Visual Studio 2022 (C++ workload, Windows 10 SDK, v143), Inno Setup 6.7+ (installer only)
- **wxWidgets 3.2.8**: GUI framework, vendored source in `third_party/wxWidgets/` (see its `VERSION.txt`), built as static `base` + `core` via `add_subdirectory` (GUI project only)
- **SQLite**: Embedded via amalgamation (third_party/sqlite/), compiled into Core
- **PDF-Writer**: PDF generation library (third_party/pdfwriter/), `add_subdirectory` (OpenSSL off)
- **Noto Sans KR 2.004** (OFL): PDF export font, `third_party/fonts/NotoSansKR/` (see its `VERSION.txt`); the `fonts` target copies it next to the executables, the installer to `{app}\fonts\`
- **Platform**: Windows x64 (released); macOS Universal (arm64 + x86_64, 11.0+) built and smoke-tested by Jenkins, GUI not checked by hand yet, not released ([docs/CI_PLAN.md](docs/CI_PLAN.md) §6)

### Code Structure

```
MissionLedger/                   # Repository root
├── CMakeLists.txt               # Root build: VERSION, third_party, common settings
├── CMakePresets.json            # windows configure; build/test presets; windows-installer; windows-release workflow
├── GenerateProjectFiles.bat     # Generates build/windows/MissionLedger.sln
├── third_party/                 # Third-party libraries (shared)
│   ├── sqlite/                  # SQLite amalgamation
│   ├── pdfwriter/               # PDF-Writer library (add_subdirectory)
│   └── wxWidgets/               # wxWidgets 3.2.8 source subset (add_subdirectory)
├── tests/                       # CTest smoke tests (CMakeLists.txt, cli_smoke.cmake) + fixtures/sample.ml
├── installer/                   # Inno Setup script (.iss), package.cmake (installer target)
├── jenkins/                     # Build.Jenkinsfile (Jenkins Pipeline job, Script Path), scripts/preflight.cmake (node check), scripts/release_check.cmake + deploy.cmake (Google Drive upload), scripts/node_check_* (node inventory), scripts/node_setup_* (install missing tools, by hand)
├── MissionLedgerCore/           # Core static library (pure C++, no wxWidgets); CMakeLists.txt
│   └── src/
│       ├── MLDefine.h          # Core enums and data structures
│       ├── interface/          # Abstract interfaces (IMLModel, IMLView, ...)
│       └── module/
│           ├── common/holder/  # FMLMVCHolder singleton
│           ├── common/format/  # FMLFormat: amount/decimal/console-column text, locale-independent (GUI, CLI, PDF all use it)
│           ├── common/currency/ # FMLCurrency: foreign amount x rate -> KRW (rounded), used by GUI and CLI
│           ├── storage/        # FMLSQLiteStorage implementation
│           ├── export/         # PDF export functionality
│           └── mvc/
│               ├── model/      # FMLModel, FMLTransaction, FMLBudget
│               └── controller/ # FMLController
├── MissionLedger/               # GUI application (wxWidgets); CMakeLists.txt
│   ├── main.cpp                 # Application entry point
│   ├── MissionLedger.rc         # Icon and version resources
│   └── src/
│       ├── MLVersion.h         # Version macros (values come from /VERSION)
│       └── module/mvc/view/    # wxMLMainFrame, MLBudgetDialog
└── MissionLedgerCLI/            # Command-line interface; CMakeLists.txt
    ├── main_cli.cpp
    └── src/module/mvc/view/    # MLCLIView
```

- **One project = one folder**: each project compiles the sources under its own folder (`file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` in its `CMakeLists.txt`). Put Core code (model, controller, storage, export, interfaces) in `MissionLedgerCore/src/`, never in the GUI or CLI folders
- Include paths are rooted at each project's `src/` (e.g. `#include "module/mvc/model/MLModel.h"`); GUI and CLI get `MissionLedgerCore/src` through the Core target

### First-Time Setup (Other Computers)

Clone, then run `GenerateProjectFiles.bat` (or `cmake --preset windows`). No other setup: wxWidgets and PDF-Writer are built from `third_party/` on the first build.

## Current Implementation Status

**Current release**: v1.0.0 (2026-11-18, Windows only) — see [CHANGELOG.md](CHANGELOG.md)

**Completed Features** (as of 2026-11-21):
- ✅ Full CRUD operations with Observer pattern
- ✅ SQLite storage provider with DI pattern
- ✅ File menu (New/Open/Save/SaveAs) with keyboard shortcuts
- ✅ Transaction filtering (date range, type, category) with Observer pattern integration
- ✅ UTF-8 encoding support for Korean text
- ✅ MVC architecture with strict boundaries (View → Controller → Model)
- ✅ List footer with income/expense/balance totals (real-time summary with filter support)
- ✅ Exchange rate support (foreign currency transactions with auto-conversion)
- ✅ Entity-DTO architecture refactoring (Data-Oriented Design, -136 lines net reduction)
- ✅ Budget system Phase 1-2: Complete budget management
  - Budget data structure with Type field and hierarchical Category/Item
  - SQLite budgets table with AUTOINCREMENT
  - Model/Controller CRUD methods with Observer pattern
  - 2-panel UI layout (Category list + Item detail list)
  - Inline input fields with Add/Update/Delete operations
  - Budget summary panel (budget amount only, no real-time calculation)
  - Complete save/load/delete functionality
  - CLI support (budget list/add/update/delete)
  - Transaction.BudgetId and ActualAmount fields reserved for future use
- ✅ PDF export (PDF-Writer, Malgun Gothic embedded) — user-facing details in [CHANGELOG.md](CHANGELOG.md)
  - Settlement report (`Ctrl+E`, CLI `export-settlement <in.ml> <out.pdf>`): budget vs actual per category, built from `FMLSettlmentData`
  - Transaction list (`Ctrl+T`, CLI `export-transactions <in.ml> [out.pdf]`): Category → Item → Transaction with merged cells, wrapping, inline exchange rates, subtotals
  - Structure: `FMLPDFDocument` → `FMLPDFRow` → cell; `DrawRow()` advances Y and splits merged cells across page breaks
- ✅ Windows Installer (Inno Setup) — [docs/INSTALLER.md](docs/INSTALLER.md)
  - `.ml` file association, Start menu/desktop shortcuts, uninstall cleanup
  - Bundles `vc_redist.x64.exe` (installed only when missing/outdated); admin install only
  - `cmake --build --preset windows-installer` (or `build_installer.bat`) → `installer\output\MissionLedger-<version>-Setup.exe`
- ✅ Versioning (v1.0.0) — [docs/VERSIONING.md](docs/VERSIONING.md)
  - Root `VERSION` file is the single source of truth (CMake → `.rc`; `.iss`, bat read it directly)
  - `CHANGELOG.md` (Keep a Changelog), annotated tags `vX.Y.Z`
- ✅ Core folder separation: Core sources in `MissionLedgerCore/src/` (one project = one folder)
- ✅ CMake build (2026-11-18): generated IDE projects, vendored wxWidgets, `.sln`/`.vcxproj`/`Version.props` removed. Verified against the former MSBuild build (identical CLI output and PDFs except the document ID, same version info, GUI checked, installer builds)
- ✅ Jenkins pipeline (2026-11-19): `jenkins/Build.Jenkinsfile` (matrix WindowsMini/MacMini, macOS = node check only), `jenkins/scripts/` (preflight, node_check, node_setup). First successful build on Jenkins: WindowsMini built, tested and packaged, MacMini passed the node check (about 10 minutes, first build)
- ✅ Formatting fixes (2026-11-20, found through the macOS build): amounts, decimals and console columns from one locale-independent Core class `FMLFormat`; foreign amount → KRW from `FMLCurrency::ConvertToKRW` (rounded, was truncated); GUI edit form keeps the exchange rate's precision; receipt-number sort no longer crashes on Korean or very long numbers. CLI/PDF output for `sample.ml` unchanged (PDFs byte-identical except the document ID); GUI checked by hand. Items only recorded: TODO.md "포맷/표시 관련 남은 항목"
- ✅ CTest smoke tests, `installer` target and `windows-release` workflow preset (2026-11-18): 5 CLI tests (`sample.ml` totals and PDFs, missing file); the workflow stops before packaging when a test fails; verified from a clean `build/` (about 2.5 minutes)
- ✅ macOS app and packaging (2026-11-21, roadmap 7 steps ③–⑤): `MissionLedger.app` (Universal arm64 + x86_64, macOS 11.0+, `.ml` document type, `MacOpenFiles`), unsigned `.dmg` (`macos-installer` preset), `lipo`/`otool` checks, smoke tests on both halves; GUI itself not run yet (see Next Steps 5)
- ✅ Release and deployment (2026-11-21): `release.bat`/`release.sh` (version number is the only input), Jenkins uploads a tagged commit's installers to Google Drive `MissionLedger/v<VERSION>/` + `latest/`; tested with a dry run (Jenkins #15), first real release pending

**Design Decision**: Real-time Budget Tracking
- ⏸️ Phase 3 (Budget-Transaction Integration) postponed
- Budget ActualAmount calculation deferred to settlement report generation
- Simplified architecture: Budget tab = budget planning only
- Settlement comparison performed during PDF export (not real-time)

**Next Steps** (build infrastructure, in order — details in [docs/CI_PLAN.md](docs/CI_PLAN.md) §0 and §10):
1. ✅ Vendor wxWidgets 3.2.8 in `third_party/wxWidgets/` (2026-11-18)
2. ✅ CMake migration (2026-11-18)
3. ✅ CTest smoke test, test presets, `installer` target, workflow preset (2026-11-18)
4. ✅ Jenkins pipeline (`jenkins/Build.Jenkinsfile`, [docs/CI_PLAN.md](docs/CI_PLAN.md) §4–§5); first successful build 2026-11-19
5. 📋 macOS build (`MacOpenFiles`, fonts, bundle, Universal); signing deferred. Developed through CI: edit on Windows, push to `main`, run Jenkins with `PLATFORM=macos`, fix from the log; GUI checked by hand at the end. Steps ①–⑤ in [docs/CI_PLAN.md](docs/CI_PLAN.md) §6 "How the macOS work is done". No advance approval for commits/pushes (decided 2026-11-20): the per-turn rule applies. Claude can start Jenkins builds and read logs via the REST API when the PC has `JENKINS_URL`/`JENKINS_USER`/`JENKINS_TOKEN` user environment variables (setup in §6; starting a build needs approval). ✅ Step ① done 2026-11-20 (macOS presets, `ML_BUILD_GUI=OFF`, Jenkinsfile builds macOS Core + CLI; Jenkins #6 green on both platforms, Mac 5/5 smoke tests). ✅ Step ② done 2026-11-20 (Noto Sans KR bundled in `third_party/fonts/NotoSansKR/` for both platforms, `getExecutablePath()` fixed on macOS; Jenkins #7 green, Mac PDFs with regular and bold). ✅ Step ③ built 2026-11-21 (`MissionLedger.app`: `Info.plist`, `.icns`, `MacOpenFiles()`, fonts in `Contents/Resources/fonts/`, plus a basic unsigned `.dmg` moved up from roadmap 9; Jenkins #9 green on both). The app has not been run yet: the GUI is checked by hand at the end of roadmap 7 on a Jenkins build (decided 2026-11-21). ✅ Steps ④–⑤ done 2026-11-21 (Universal arm64 + x86_64, smoke tests also on x86_64 through Rosetta, `lipo`/`otool` checks before the `.dmg`; Jenkins #12–#13 green on both). **Next: the GUI check by hand on the MacMini with a `dev` build installed from Google Drive** (decided 2026-11-21; dev builds on Drive since Jenkins #16, step 7). GUI automation ([docs/CI_PLAN.md](docs/CI_PLAN.md) §7 "GUI check automation"): ② `--self-test` deferred (needs app code, little gain with one developer checking by hand), ① screenshot and ③ `open sample.ml` optional (Jenkinsfile only). Already known: Windows and macOS PDFs are byte-identical except `/ID` (Jenkins #15) (the last open item of roadmap 7; checklist: install, "Open Anyway", window/Korean text, `.ml` double-click with the app closed and running, menus/⌘ shortcuts, CRUD, budget, filter, save/reopen, PDF export)
6. 🔄 Deployment to Google Drive (written 2026-11-21): `release.bat`/`release.sh` (`scripts/release.cmake`) makes VERSION + CHANGELOG + commit + tag from one version number; a Jenkins build of the tagged commit with `BUILD=shipping` (`PLATFORM=both`; since 2026-11-21 the tag alone no longer releases) does a clean build and uploads both installers and `릴리스노트.txt` to `gdrive:MissionLedger/v<VERSION>/` and `latest/` (share `latest/` once; its link stays). rclone config = Jenkins secret file `missionledger-rclone-conf` (same `rclone.conf` as `tenetris-rclone-conf`, kept separate on purpose). Tested locally with a fake rclone; Jenkins #14 (untagged: Deploy skipped) and #15 (`DEPLOY_DRY_RUN`: secret file, remote `gdrive`, both installers, release notes) passed 2026-11-21. Still to do: the first real release (`release.bat 1.1.0 push` → Jenkins `BUILD=shipping`), after the Mac GUI check; then share `MissionLedger/latest/` once. Next version is 1.1.0 (macOS added = MINOR; add a `### 추가` macOS entry to CHANGELOG before releasing). Details: [docs/CI_PLAN.md](docs/CI_PLAN.md) §0, §4–§5, [docs/VERSIONING.md](docs/VERSIONING.md) §3
7. ✅ **Build kinds** (decided, written and checked on Jenkins #16–#21 2026-11-21; [docs/CI_PLAN.md](docs/CI_PLAN.md) §4, roadmap 11): Jenkins parameter `BUILD` = `dev` (default) / `shipping` / `test` / `debug` replaced `CONFIGURATION` and `DEPLOY`, like Development/Shipping in an Unreal pipeline (one parameter: Debug has no installer, and "Release" config vs "release" output read alike). `dev`: any commit → `gdrive:MissionLedger/dev/<BUILD_NUMBER>-<commit>/`, newest 5 kept, single platform allowed (upload still from WindowsMini), upload failure = UNSTABLE. `shipping`: the release path, only when chosen; Preflight fails before building without the pushed tag `v<VERSION>` on this commit or without `PLATFORM=both`; always clean. A tag alone no longer makes a release. dev and shipping are the same Release build (dev shows the previous version number, told apart by folder name; dev is incremental unless `CLEAN_BUILD`); one install identity (installs replace each other); no app code changes. Jenkins #16 (`dev`, both → `dev/16-408628c/`), #17 (`dev`, macos → `.dmg` only), #18 (`shipping` untagged → fails in Preflight), #19 (`shipping` dry run), #20 (`test`), #21 (`debug`) all as expected (roadmap 11). Promotion (ship the checked dev build without rebuilding) noted as a later option. **Next: the Mac GUI check with `dev/17-408628c/MissionLedger-1.0.0.dmg` from Drive** (step 5) → first real release 1.1.0

**Next Steps** (features):
- 📋 CLI full review (`MLCLIView`): check every command and option, error handling and exit codes, the two help texts (argument mode vs interactive mode list different commands), and interactive input parsing. See TODO.md
- 📋 Excel/CSV export (transaction list + settlement report)
- 📋 Additional features (multi-currency support, custom report templates, etc.)

**Known leftovers** (low priority): `MLMVCHolderExample.h` includes non-existent headers (not compiled, but listed in the generated IDE project because headers are globbed); `register_ml_extension.reg` writes an HKCU association to the Debug exe that overrides the installer's, and its exe path predates the CMake output folder — consider deleting it. The former MSBuild Release used `/GL` (whole program optimization); the CMake build does not (exe about 0.1 MB larger, speed difference not measured). PDF layout nits seen with `sample.ml`: in the settlement report the last data row overlaps the total row's border; wrapped lines in the transaction list can start with a space; the settlement title comes from the input `.ml` name but the transaction-list title from the output PDF name. PDF CJK coverage: export uses one font for the whole document with no per-glyph fallback: the bundled Noto Sans KR (`third_party/fonts/NotoSansKR/`, copied to `fonts/` next to the executable; since 2026-11-20), else the system font (Malgun Gothic / Apple SD Gothic Neo). Noto Sans KR draws Hangul, Korean Hanja, kana, Japanese shinjitai (気駅広) and Traditional Chinese but not Simplified Chinese (们这说话 → boxes); checked in an exported PDF with these sample characters on 2026-11-20. Accepted for now (no Simplified Chinese data yet). (Malgun Gothic, the former Windows font, lacked 気駅広 too.) GUI likely fine via system font fallback (not tested). Options if needed later: swap in Noto Sans CJK KR (full pan-CJK, about 16 MB per weight) in the same folder, or a per-glyph fallback chain in `MLPDFDocument`

**Session wrap-up**: before starting a new task or switching to another PC, update this status section, `CHANGELOG.md` `[미배포]`, and the roadmap in `docs/CI_PLAN.md`, then **ask the user whether to commit and push** (see "Working Rules"). Claude's local memory does not travel between PCs; anything another session needs must be in the repository

## Coding Conventions

### Naming Rules
- **Classes**: `FMLModel`, `FMLTransaction` (FML prefix, PascalCase)
- **Interfaces**: `IMLModel`, `IMLView` (IML prefix, PascalCase)
- **Enums**: `E_MLTransactionType` (E_ML prefix, PascalCase)
- **Structs**: `FMLTransactionData`, `FMLFilterCriteria` (FML prefix, PascalCase)
- **Public functions**: `AddTransaction()`, `GetData()` (PascalCase)
- **Private functions**: `rebuildCategoryCache()`, `notifyObservers()` (camelCase)
- **Variables/parameters**: `transactionData`, `userId` (camelCase)
- **Member variables**: `TransactionId`, `Amount` (PascalCase)

### Key Patterns

**UTF-8 Handling**:
```cpp
// Display (Korean text in UI)
label->SetLabel(wxString::FromUTF8("거래 내역"));

// Storage (saving to SQLite)
std::string text = textCtrl->GetValue().ToUTF8().data();
```

**MVC Access**:
```cpp
// View accesses Controller (NEVER Model directly)
auto controller = FMLMVCHolder::GetInstance().GetController();
controller->AddTransaction(data);

// Controller accesses Model
auto model = FMLMVCHolder::GetInstance().GetModel();
model->AddTransaction(data);
```

**Observer Pattern**:
```cpp
// Model notifies observers on data changes
void FMLModel::AddTransaction(const FMLTransactionData& data) {
    // Add transaction logic...
    notifyObservers([&](auto observer) {
        observer->OnTransactionAdded(data);
    });
}
```

**Const Correctness**:
```cpp
// Use const for local variables that won't change
const bool isValid = ValidateInput(data);
const int count = items.size();

if (isValid) {
    // Clear that these values are immutable
}
```

### Versioning
- The root `VERSION` file (`MAJOR.MINOR.PATCH`) is the **only** place the version is written. Never hard-code version numbers in `MLVersion.h`, `.iss`, or scripts
- Bump the version only at release time, and only with `release.bat` / `release.sh` (it writes VERSION, CHANGELOG, commit and tag together); record user-visible changes under `[미배포]` in `CHANGELOG.md` as you work
- Policy and release procedure: [docs/VERSIONING.md](docs/VERSIONING.md)

## Architecture Guidelines

### MVC Boundaries (STRICT)
- **View** → **Controller** → **Model** (one direction only)
- View uses DTO (`FMLTransactionData`) for all data exchange
- Model uses Entity (`FMLTransaction`) internally
- Observer pattern for Model → View notifications (indirect, interface-based)

### Design Principles
- **Separation of Concerns**: View never accesses Model directly
- **Dependency Direction**: View → Controller → Model (never reversed)
- **Single Source of Truth**: UI state read from controls, not cached (prevents sync bugs)
- **Update Strategy**: Incremental updates when filter inactive, full reload when filter active (balances performance and correctness)

## Working Rules (for Claude Code)

### Approval required for every change (STRICT)
Any action that changes the repository, the working tree, or anything outside it needs the user's explicit approval **in that turn**. Explain what will be done (commands / files / how), then wait. Also enforced by `ask` rules in `.claude/settings.json`
- **Git**: `commit`, `push`, `fetch`, `pull`, `merge`, `rebase`, `reset`, `checkout`/`switch`, `stash`, `tag`, branch create/delete — all need approval. Read-only commands (`status`, `log`, `diff`, `show`, `branch -l`, `ls-remote`) are fine
- **Files**: creating, editing, moving, or deleting any file (source, docs, config, scripts), including one-line fixes
- **Build / tools**: builds, CMake generation, installs, or scripts that write files
- **Not approval**: a question ("~인가요?", "설마 ~안 했나?"), a written rule (e.g. session wrap-up), an earlier approval, or approval of a different action
- Reading files and read-only investigation need no approval. If the user asks "설명만 먼저", only explain
- When asked to commit: split by concern (code vs docs) and write Korean messages with `Feat:` / `Fix:` / `Refactor:` / `Docs:` / `Build:` / `Chore:` prefixes (`Build:` = build system, CI, dependencies, packaging; see docs/CONVENTIONS.md)

### Documents
- When rewriting a document, don't silently drop content that is still valid (open decisions, future tasks). Report what was removed or moved
- Keep verified facts and guesses apart: say when something is inferred rather than checked in the code or git history
- When a past decision is unclear, check git history before assuming it was deliberate (much of the earlier work was generated with Claude and may not reflect a conscious choice)

### Project context
- Non-commercial project shared with a small group (mission team). Commercial-grade steps (clean-PC installer testing, paid code signing) are low priority
- The maintainer works mainly with Unreal Engine: explain build and tooling concepts with Unreal analogies where useful (e.g. `CMakeLists.txt` ≈ `.Build.cs`, CMake project generation ≈ GenerateProjectFiles, generated IDE files are never committed, never add files through the IDE)
- Prefer IDE-independent setups: the source tree is the truth and IDE projects are generated
- Build nodes are shared with other projects: they should need only general tools; project-specific dependencies come from the repository ([docs/CI_PLAN.md](docs/CI_PLAN.md) §0)

## Communication Language

**Important**: When working with this codebase, Claude Code should communicate in **Korean (한국어)** as this is the preferred language for this project. All responses, explanations, and documentation should be provided in Korean.

## Additional Documentation

For detailed information, refer to:
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)**: MVC pattern, design decisions, Storage Provider, DTO strategy, Observer pattern
- **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)**: Development roadmap, implementation patterns, filtering system, GUI guidelines
- **[docs/CONVENTIONS.md](docs/CONVENTIONS.md)**: Detailed naming rules, code style, wxWidgets conventions, MVC compliance
- **[docs/VERSIONING.md](docs/VERSIONING.md)**: Version policy (SemVer, `.ml` compatibility), `VERSION` file flow, release procedure
- **[CHANGELOG.md](CHANGELOG.md)**: User-visible changes per release

## Compact Instructions

When compacting, always preserve:
- Build commands (CMake presets, generated IDE projects, never add files through the IDE)
- MVC architecture patterns and boundaries (View → Controller → Model)
- Storage provider implementation (SQLite with DI pattern)
- UTF-8 encoding requirements (`FromUTF8()` for display, `ToUTF8().data()` for storage)
- Observer pattern integration
- Current implementation status and roadmap
- Communication language (Korean)
- Versioning rule (`VERSION` file is the single source of truth)
- Working rules (never commit/push without an explicit request; explain before multi-file edits)
- Links to detailed documentation
