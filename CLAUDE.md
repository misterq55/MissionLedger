# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MissionLedger is a C++ desktop application built with wxWidgets for financial transaction management. The project uses Visual Studio 2022 (v143 toolset) and implements a classic MVC (Model-View-Controller) architecture with an observer pattern for data notifications.

## Build Commands

The project uses Visual Studio MSBuild system:

- **Build (Debug x64)**: `msbuild MissionLedger.sln /p:Configuration=Debug /p:Platform=x64`
- **Build (Release x64)**: `msbuild MissionLedger.sln /p:Configuration=Release /p:Platform=x64`
- **Clean**: `msbuild MissionLedger.sln /t:Clean`

The main project file is located at `MissionLedger/MissionLedger.vcxproj`.

## Dependencies

- **wxWidgets**: GUI framework (requires WXWIN environment variable)
- **Visual Studio 2022**: Windows 10 SDK
- **Platform**: Windows (Win32/x64)

## Architecture

### MVC Pattern Implementation

The codebase follows a strict MVC architecture:

- **Model**: `CMLModel` class manages transaction data using `std::map<int, std::shared_ptr<CMLTransaction>>`
- **View**: Interface defined by `IMLView` (currently minimal)
- **Controller**: Interface defined by `IMLController` for transaction operations
- **Observer Pattern**: `MLModelObserver` provides event notifications for data changes

### Core Data Structure

The main entity is `CMLTransaction` representing financial transactions with:
- Transaction type (Income/Expense) via `E_MLTransactionType` enum
- Categories, items, descriptions, amounts
- Timestamps using `std::chrono::system_clock::time_point`
- Receipt number tracking

### Key Interfaces

- `IMLModel`: Data management interface with Add/Remove/Save/Load operations
- `IMLController`: Business logic interface for transaction handling
- `IMLView`: UI interface (placeholder for future implementation)
- `MLModelObserver`: Observer pattern for data change notifications

## Code Organization

```
MissionLedger/
├── main.cpp                     # wxWidgets application entry point
└── src/
    ├── MLDefine.h              # Core enums and data structures
    ├── interface/              # Abstract interfaces
    │   ├── IMLController.h     # Controller interface
    │   ├── IMLModel.h          # Model interface
    │   └── IMLView.h           # View interface
    └── module/
        ├── common/observer/    # Observer pattern implementation
        └── mvc/
            ├── model/          # Model implementation
            │   ├── CMLModel.*  # Main model class
            │   └── transaction/# Transaction entity
            ├── controller/     # (Placeholder for future implementation)
            └── view/           # (Placeholder for future implementation)
```

## Development Notes

- Include paths: Source files use `$(ProjectDir)src` as additional include directory
- Korean language comments are present throughout the codebase
- The project includes placeholder folders for controller and view implementations
- Transaction management is the core functionality with stub implementations

## Coding Conventions

This project follows specific naming conventions:

- **Classes**: Prefix with `CML` (e.g., `CMLModel`, `CMLTransaction`)
- **Structs**: Prefix with `SML` (e.g., `SMLTransactionData`)
- **Enums**: Prefix with `E_ML` (e.g., `E_MLTransactionType`)
- **Member variables**: PascalCase convention (e.g., `TransactionId`, `Amount`)
- **Local variables/parameters**: camelCase convention (e.g., `transactionData`, `userId`)

## Communication Language

**Important**: When working with this codebase, Claude Code should communicate in Korean (한국어) as this is the preferred language for this project. All responses, explanations, and documentation should be provided in Korean.