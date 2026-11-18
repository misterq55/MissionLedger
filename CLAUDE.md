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
cmake --build --preset windows-debug      # Debug build
cmake --build --preset windows-release    # Release build
build_installer.bat                       # configure + Release build + Inno Setup installer
```

- Output: `build/windows/bin/<Debug|Release>/MissionLedger.exe`, `MissionLedgerCLI.exe` (+ `.pdb`)
- Clean build: delete `build/` and generate again. The first build also compiles wxWidgets and PDF-Writer (about 1–2 minutes)
- CMake, not MSBuild, is called directly, so the Visual Studio edition path does not matter. In Git Bash pass `--parallel` instead of `/m` (MSYS path conversion mangles `/` switches; use `MSYS_NO_PATHCONV=1` if a `/` switch is needed)
- CI build number: `cmake --preset windows -DML_BUILD_NUMBER=N` (4th field of the exe version)

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
- **Platform**: Windows x64 (macOS planned, [docs/CI_PLAN.md](docs/CI_PLAN.md))

### Code Structure

```
MissionLedger/                   # Repository root
├── CMakeLists.txt               # Root build: VERSION, third_party, common settings
├── CMakePresets.json            # windows configure preset, windows-debug/-release build presets
├── GenerateProjectFiles.bat     # Generates build/windows/MissionLedger.sln
├── third_party/                 # Third-party libraries (shared)
│   ├── sqlite/                  # SQLite amalgamation
│   ├── pdfwriter/               # PDF-Writer library (add_subdirectory)
│   └── wxWidgets/               # wxWidgets 3.2.8 source subset (add_subdirectory)
├── MissionLedgerCore/           # Core static library (pure C++, no wxWidgets); CMakeLists.txt
│   └── src/
│       ├── MLDefine.h          # Core enums and data structures
│       ├── interface/          # Abstract interfaces (IMLModel, IMLView, ...)
│       └── module/
│           ├── common/holder/  # FMLMVCHolder singleton
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

**Completed Features** (as of 2026-11-18):
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
  - `build_installer.bat` → `installer\output\MissionLedger-<version>-Setup.exe`
- ✅ Versioning (v1.0.0) — [docs/VERSIONING.md](docs/VERSIONING.md)
  - Root `VERSION` file is the single source of truth (CMake → `.rc`; `.iss`, bat read it directly)
  - `CHANGELOG.md` (Keep a Changelog), annotated tags `vX.Y.Z`
- ✅ Core folder separation: Core sources in `MissionLedgerCore/src/` (one project = one folder)
- ✅ CMake build (2026-11-18): generated IDE projects, vendored wxWidgets, `.sln`/`.vcxproj`/`Version.props` removed. Verified against the former MSBuild build (identical CLI output and PDFs except the document ID, same version info, GUI checked, installer builds)

**Design Decision**: Real-time Budget Tracking
- ⏸️ Phase 3 (Budget-Transaction Integration) postponed
- Budget ActualAmount calculation deferred to settlement report generation
- Simplified architecture: Budget tab = budget planning only
- Settlement comparison performed during PDF export (not real-time)

**Next Steps** (build infrastructure, in order — details in [docs/CI_PLAN.md](docs/CI_PLAN.md) §0 and §10):
1. ✅ Vendor wxWidgets 3.2.8 in `third_party/wxWidgets/` (2026-11-18)
2. ✅ CMake migration (2026-11-18)
3. 📋 CTest smoke test + Jenkinsfile (Windows, then macOS). Fixture ready: `tests/fixtures/sample.ml` (made-up data; use it on any PC instead of real `.ml` files)
4. 📋 macOS build (`MacOpenFiles`, fonts, bundle, Universal); signing deferred

**Next Steps** (features):
- 📋 Excel/CSV export (transaction list + settlement report)
- 📋 Additional features (multi-currency support, custom report templates, etc.)

**Known leftovers** (low priority): `MLMVCHolderExample.h` includes non-existent headers (not compiled, but listed in the generated IDE project because headers are globbed); `register_ml_extension.reg` writes an HKCU association to the Debug exe that overrides the installer's, and its exe path predates the CMake output folder — consider deleting it. The former MSBuild Release used `/GL` (whole program optimization); the CMake build does not (exe about 0.1 MB larger, speed difference not measured). PDF layout nits seen with `sample.ml`: in the settlement report the last data row overlaps the total row's border; wrapped lines in the transaction list can start with a space; the settlement title comes from the input `.ml` name but the transaction-list title from the output PDF name. PDF CJK coverage: export uses one font for the whole document with no per-glyph fallback (bundled `fonts/NotoSansKR-*.ttf` if present, else Malgun Gothic). Malgun Gothic covers Hangul, Korean Hanja and kana but not Japanese shinjitai kanji (e.g. 気駅広) or Simplified Chinese (e.g. 们这说话); checked against the font's glyph map on 2026-11-18, PDF output not tested. GUI likely fine via Windows font fallback (not tested). Options if needed: bundle Noto Sans CJK, or a per-glyph fallback chain in `MLPDFDocument`

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
- Bump the version only at release time; record user-visible changes under `[미배포]` in `CHANGELOG.md` as you work
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
