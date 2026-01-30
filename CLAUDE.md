# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MissionLedger is a C++ desktop application built with wxWidgets for financial transaction management. The project uses Visual Studio 2022 (v143 toolset) and implements a classic MVC (Model-View-Controller) architecture with an observer pattern for data notifications.

**Architecture**: Model First MVC with Observer pattern, Storage Provider (Strategy Pattern), MVC Holder (Singleton), and DTO-based data exchange.

## Quick Reference

### Build Commands

```bash
# Debug build
msbuild MissionLedger.sln /p:Configuration=Debug /p:Platform=x64

# Release build
msbuild MissionLedger.sln /p:Configuration=Release /p:Platform=x64

# Clean
msbuild MissionLedger.sln /t:Clean
```

### Dependencies

- **wxWidgets 3.2.8**: GUI framework at `C:\wxWidgets-3.2.8` (WXWIN environment variable required)
- **Visual Studio 2022**: Windows 10 SDK, v143 toolset
- **SQLite**: Embedded via amalgamation (src/module/third_party/sqlite/)
- **Platform**: Windows x64

### Code Structure

```
MissionLedger/
├── main.cpp                     # Application entry point
└── src/
    ├── MLDefine.h              # Core enums and data structures
    ├── interface/              # Abstract interfaces (IMLModel, IMLView, IMLController, etc.)
    └── module/
        ├── common/holder/      # FMLMVCHolder singleton
        ├── storage/            # FMLSQLiteStorage implementation
        ├── third_party/sqlite/ # SQLite amalgamation
        └── mvc/
            ├── model/          # FMLModel, FMLTransaction
            ├── controller/     # FMLController
            └── view/           # wxMLMainFrame (MLMainFrame.h/cpp)
```

## Current Implementation Status

**Completed Features** (as of 2026-01-29):
- ✅ Full CRUD operations with Observer pattern
- ✅ SQLite storage provider with DI pattern
- ✅ File menu (New/Open/Save/SaveAs) with keyboard shortcuts
- ✅ Transaction filtering (date range, type, category) with differential updates
- ✅ UTF-8 encoding support for Korean text
- ✅ MVC architecture with strict boundaries (View → Controller → Model)

**Completed**:
- ✅ List footer with income/expense/balance totals (real-time summary with filter support)

**Current Focus**: Phase 4 - Data Export & Distribution
- ⏳ Excel/CSV export
- ⏳ Installer with .ml file association

## Coding Conventions

### Naming Rules
- **Classes**: `FMLModel`, `FMLTransaction` (FML prefix, PascalCase)
- **Interfaces**: `IMLModel`, `IMLView` (IML prefix, PascalCase)
- **Enums**: `E_MLTransactionType` (E_ML prefix, PascalCase)
- **Structs**: `FMLTransactionData`, `FMLFilterCriteria` (FML prefix, PascalCase)
- **Public functions**: `AddTransaction()`, `GetData()` (PascalCase)
- **Private functions**: `convertToDTO()`, `notifyObservers()` (camelCase)
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
- **Differential Updates**: Only modify changed items (preserves scroll position, better UX)

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
