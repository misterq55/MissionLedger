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

- **wxWidgets 3.2.8**: GUI framework at `C:\wxWidgets-3.2.8` (WXWIN environment variable required)
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
├── MissionLedger/               # Main GUI application
│   ├── main.cpp                 # Application entry point
│   └── src/
│       ├── MLDefine.h          # Core enums and data structures
│       ├── interface/          # Abstract interfaces
│       └── module/
│           ├── common/holder/  # FMLMVCHolder singleton
│           ├── storage/        # FMLSQLiteStorage implementation
│           ├── export/         # PDF export functionality
│           └── mvc/
│               ├── model/      # FMLModel, FMLTransaction
│               ├── controller/ # FMLController
│               └── view/       # wxMLMainFrame
├── MissionLedgerCore/          # Core business logic library
└── MissionLedgerCLI/           # Command-line interface
```

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

**Completed Features** (as of 2026-03-19):
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
- ✅ Settlement PDF Export: Complete implementation with optimized layout
  - FMLSettlmentData structure with hierarchical budget/actual data (Category → Item)
  - PDF-Writer library integration (static library pattern)
  - Korean font support (Malgun Gothic TTF embedding)
  - **5-section layout**: Summary (budget totals + actual totals/balance), Exchange rates, Income comparison, Expense comparison
  - Category-level aggregation (income/expense totals only, no item breakdown)
  - Side-by-side budget vs actual comparison (3-column tables: Category, Budget, Actual)
  - Auto-generated title and filename from .ml file (e.g., "2024 필리핀 단기 선교.ml" → PDF title and default filename)
  - Optimized spacing (30pt subtitle gaps) and alignment (10pt right margin for amounts)
  - GUI menu: File → Export Settlement PDF (Ctrl+E)
  - CLI command: export-settlement <input.ml> <output.pdf>
- ✅ Transaction List PDF Export: Complete implementation with enhancements
  - Individual transaction listing with hierarchical grouping (Category → Item → Transactions)
  - Cell merging for same Category and Item
  - Multi-line text wrapping for Item and Description columns (UTF-8 safe)
  - **Vertical center alignment** for multi-line text in Item and Description columns
  - Dynamic row height calculation based on text content
  - Mid-group page breaks for optimal space utilization
  - **Exchange rate information** displayed inline with amount (e.g., "1,000,000원 (PHP 40,000 @ 25.0)")
  - **Optimized column widths**: Category(50) Item(60) Description(170) Amount(135) Date(60) Receipt(50)
  - Category-wise subtotals with formatted table layout
  - Auto-suggested filename from .ml file (GUI) or optional output path (CLI)
  - GUI menu: File → Export Transaction List PDF (Ctrl+T)
  - CLI command: export-transactions <input.ml> [output.pdf]
- ✅ PDF Export OOP Refactoring
  - `FMLPDFDocument`, `FMLPDFRow` 추상화 계층 도입 (Document → Row → Cell 위임 구조)
  - `FMLPDFDocument::DrawRow()` API 통해 행 그리기 + Y위치 자동 이동
  - 페이지 분기 시 카테고리/항목 병합 셀 세그먼트 단위 분할 처리 (거래 단위 페이지 분기)
  - 금액 셀 우측 여백 수정 (텍스트 끝닿음 방지)
  - 컬럼 헤더와 첫 번째 데이터 셀 간 불필요한 간격 제거

**Design Decision**: Real-time Budget Tracking
- ⏸️ Phase 3 (Budget-Transaction Integration) postponed
- Budget ActualAmount calculation deferred to settlement report generation
- Simplified architecture: Budget tab = budget planning only
- Settlement comparison performed during PDF export (not real-time)

**Next Steps**:
- 📋 Excel/CSV export (transaction list + settlement report)
- 📋 Installer with .ml file association
- 📋 Additional features (multi-currency support, custom report templates, etc.)

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

## Communication Language

**Important**: When working with this codebase, Claude Code should communicate in **Korean (한국어)** as this is the preferred language for this project. All responses, explanations, and documentation should be provided in Korean.

## Additional Documentation

For detailed information, refer to:
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)**: MVC pattern, design decisions, Storage Provider, DTO strategy, Observer pattern
- **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)**: Development roadmap, implementation patterns, filtering system, GUI guidelines
- **[docs/CONVENTIONS.md](docs/CONVENTIONS.md)**: Detailed naming rules, code style, wxWidgets conventions, MVC compliance

## Compact Instructions

When compacting, always preserve:
- Build commands (msbuild configurations)
- MVC architecture patterns and boundaries (View → Controller → Model)
- Storage provider implementation (SQLite with DI pattern)
- UTF-8 encoding requirements (`FromUTF8()` for display, `ToUTF8().data()` for storage)
- Observer pattern integration
- Current implementation status and roadmap
- Communication language (Korean)
- Links to detailed documentation
