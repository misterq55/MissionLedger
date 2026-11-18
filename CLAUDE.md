# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MissionLedger is a C++ desktop application built with wxWidgets for financial transaction management. The project uses Visual Studio 2022 (v143 toolset) and implements a classic MVC (Model-View-Controller) architecture with an observer pattern for data notifications.

**Architecture**: Model First MVC with Observer pattern, Storage Provider (Strategy Pattern), MVC Holder (Singleton), and DTO-based data exchange.

## Quick Reference

### Build Commands

#### Visual Studio Developer Command Prompt

```bash
# Debug build
msbuild MissionLedger.sln /p:Configuration=Debug /p:Platform=x64

# Release build
msbuild MissionLedger.sln /p:Configuration=Release /p:Platform=x64

# Clean
msbuild MissionLedger.sln /t:Clean
```

#### Git Bash (MSYS2 Environment)

Git Bash requires special handling due to automatic path conversion. Use `MSYS_NO_PATHCONV=1` to prevent `/p:`, `/v:` switches from being converted to Windows paths.

```bash
# Debug build (recommended)
MSYS_NO_PATHCONV=1 "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" MissionLedger.sln /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo

# Release build
MSYS_NO_PATHCONV=1 "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" MissionLedger.sln /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo

# Clean
MSYS_NO_PATHCONV=1 "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" MissionLedger.sln /t:Clean /v:minimal /nologo
```

**Note**: Adjust the MSBuild path if using Professional or Enterprise edition:
- Community: `C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe`
- Professional: `C:/Program Files/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe`
- Enterprise: `C:/Program Files/Microsoft Visual Studio/2022/Enterprise/MSBuild/Current/Bin/MSBuild.exe`

### Dependencies

- **wxWidgets 3.2.6**: GUI framework, prebuilt static libraries outside the repository at `C:\wxWidgets-3.2.6` (`WXWIN` environment variable required; GUI project only). Planned: vendor 3.2.8 in `third_party/wxWidgets/` with the CMake migration ([docs/CI_PLAN.md](docs/CI_PLAN.md))
- **Visual Studio 2022**: Windows 10 SDK, v143 toolset
- **SQLite**: Embedded via amalgamation (third_party/sqlite/)
- **PDF-Writer**: PDF generation library (third_party/pdfwriter/)
- **Platform**: Windows x64

### Code Structure

```
MissionLedger/                   # Solution root
├── third_party/                 # Third-party libraries (shared)
│   ├── sqlite/                  # SQLite amalgamation
│   └── pdfwriter/               # PDF-Writer library (CMake-based)
├── MissionLedgerCore/           # Core static library (pure C++, no wxWidgets)
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
├── MissionLedger/               # GUI application (wxWidgets)
│   ├── main.cpp                 # Application entry point
│   ├── MissionLedger.rc         # Icon and version resources
│   └── src/
│       ├── MLVersion.h         # Version macros (values come from /VERSION)
│       └── module/mvc/view/    # wxMLMainFrame, MLBudgetDialog
└── MissionLedgerCLI/            # Command-line interface
    ├── main_cli.cpp
    └── src/module/mvc/view/    # MLCLIView
```

- **One project = one folder**: each project compiles only the sources under its own folder. Put Core code (model, controller, storage, export, interfaces) in `MissionLedgerCore/src/`, never in the GUI or CLI folders
- Include paths are rooted at each project's `src/` (e.g. `#include "module/mvc/model/MLModel.h"`); GUI and CLI also add `MissionLedgerCore/src`

### First-Time Setup (Other Computers)

When cloning this repository on a new computer, you must generate PDF-Writer build files using CMake:

```bash
# Navigate to PDF-Writer directory
cd third_party/pdfwriter
mkdir -p build
cd build

# Generate Visual Studio project files
cmake .. -G "Visual Studio 17 2022" -A x64

# Build Debug and Release libraries
cmake --build . --config Debug
cmake --build . --config Release
```

After this, you can build the entire solution normally with MSBuild.

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
  - Root `VERSION` file is the single source of truth (`Version.props` → `.rc`, `.iss`, bat)
  - `CHANGELOG.md` (Keep a Changelog), annotated tags `vX.Y.Z`
- ✅ Core folder separation: Core sources in `MissionLedgerCore/src/` (one project = one folder)

**Design Decision**: Real-time Budget Tracking
- ⏸️ Phase 3 (Budget-Transaction Integration) postponed
- Budget ActualAmount calculation deferred to settlement report generation
- Simplified architecture: Budget tab = budget planning only
- Settlement comparison performed during PDF export (not real-time)

**Next Steps** (build infrastructure, in order — details in [docs/CI_PLAN.md](docs/CI_PLAN.md) §0 and §10):
1. 📋 Vendor wxWidgets 3.2.8 in `third_party/wxWidgets/` (official release archive, trimmed)
2. 📋 CMake migration: generated IDE projects (Unreal GenerateProjectFiles style), per-folder automatic source collection, `add_subdirectory` for PDF-Writer/wxWidgets; then delete `.sln`/`.vcxproj`/`Version.props`
3. 📋 CTest smoke test + Jenkinsfile (Windows, then macOS). Fixture ready: `tests/fixtures/sample.ml` (made-up data; use it on any PC instead of real `.ml` files)
4. 📋 macOS build (`MacOpenFiles`, fonts, bundle, Universal); signing deferred

**Next Steps** (features):
- 📋 Excel/CSV export (transaction list + settlement report)
- 📋 Additional features (multi-currency support, custom report templates, etc.)

**Known leftovers** (low priority): `MLMVCHolderExample.h` includes non-existent headers (not built); `MissionLedger.vcxproj.filters` has stale Core entries (goes away with CMake); `register_ml_extension.reg` writes an HKCU association to the Debug exe that overrides the installer's — consider deleting it. PDF layout nits seen with `sample.ml`: in the settlement report the last data row overlaps the total row's border; wrapped lines in the transaction list can start with a space; the settlement title comes from the input `.ml` name but the transaction-list title from the output PDF name

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

### Git
- **Never commit or push without an explicit request in that turn** ("커밋해 주세요", "push 해 주세요"). A written rule such as the session wrap-up, an earlier approval, or "save the docs" is not permission. Finish the edits, report them, and ask
- When asked to commit: split by concern (code vs docs) and write Korean messages with `Feat:` / `Fix:` / `Refactor:` / `Docs:` / `Chore:` prefixes

### Before changing files
- For any multi-file change, first explain which files change and how, then wait for the go-ahead. If the user asks "설명만 먼저", do not edit anything
- A question ("~인가요?", "어떤가요?") is a question, not approval. Answer it and ask before acting

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
- Build commands (msbuild configurations)
- MVC architecture patterns and boundaries (View → Controller → Model)
- Storage provider implementation (SQLite with DI pattern)
- UTF-8 encoding requirements (`FromUTF8()` for display, `ToUTF8().data()` for storage)
- Observer pattern integration
- Current implementation status and roadmap
- Communication language (Korean)
- Versioning rule (`VERSION` file is the single source of truth)
- Working rules (never commit/push without an explicit request; explain before multi-file edits)
- Links to detailed documentation
