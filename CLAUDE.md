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

- **wxWidgets 3.2.8**: GUI framework (requires WXWIN environment variable)
- **Visual Studio 2022**: Windows 10 SDK
- **Platform**: Windows (Win32/x64)

### wxWidgets Setup

1. **Installation**: wxWidgets should be installed at `C:\wxWidgets-3.2.8`
2. **Building**: Use Visual Studio 2022 to build wxWidgets libraries
   - Open `C:\wxWidgets-3.2.8\build\msw\wx_vc17.sln` in Visual Studio 2022
   - Build for Release x64 configuration
   - Libraries will be generated in `C:\wxWidgets-3.2.8\lib\vc_x64_lib`
3. **Environment**: WXWIN environment variable must be set to `C:\wxWidgets-3.2.8`

## Architecture

### MVC Pattern Implementation

The codebase follows a strict MVC architecture with centralized component management:

- **Model**: `CMLModel` class manages transaction data using `std::map<int, std::shared_ptr<CMLTransaction>>`
- **View**: Interface defined by `IMLView` (currently minimal)
- **Controller**: Interface defined by `IMLController` for transaction operations
- **Observer Pattern**: `MLModelObserver` provides event notifications for data changes
- **MVC Holder**: `CMLMVCHolder` singleton pattern for global MVC component access

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
- `CMLMVCHolder`: Singleton holder for centralized MVC component access

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
        ├── common/             # Common utilities and patterns
        │   ├── holder/         # MVC Holder singleton pattern
        │   │   ├── CMLMVCHolder.*      # MVC component holder
        │   │   └── MLMVCHolderExample.h # Usage examples
        │   └── observer/       # Observer pattern implementation
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

### Current Implementation Status

- **Model**: `MLModel` class implemented with basic CRUD operations
- **Controller**: `IMLController` interface defined, implementation pending
- **View**: `IMLView` interface defined, implementation pending
- **Observer**: `MLModelObserver` well-designed with specific transaction events
- **MVC Holder**: `CMLMVCHolder` singleton implemented for centralized component access

### MVC Holder Usage

The `CMLMVCHolder` singleton provides centralized access to MVC components throughout the application:

#### Initialization (in main.cpp):
```cpp
#include "module/common/holder/MLMVCHolder.h"

class MyApp : public wxApp {
    virtual bool OnInit() override {
        // Create MVC components
        auto model = std::make_shared<CMLModel>();
        auto view = std::make_shared<CMLView>();
        auto controller = std::make_shared<CMLController>();

        // Register with holder
        auto& holder = CMLMVCHolder::GetInstance();
        holder.SetModel(model);
        holder.SetView(view);
        holder.SetController(controller);

        // Connect observer
        model->AddObserver(view);

        return true;
    }

    virtual int OnExit() override {
        CMLMVCHolder::DestroyInstance();
        return wxApp::OnExit();
    }
};
```

#### Usage in wxWidgets Windows/Dialogs:
```cpp
// In menu handlers
void OnMenuSave(wxCommandEvent& event) {
    auto model = CMLMVCHolder::GetInstance().GetModel();
    if (model) model->Save();
}

// In dialog OK handlers
void AddTransactionDialog::OnOK(wxCommandEvent& event) {
    auto controller = CMLMVCHolder::GetInstance().GetController();
    if (controller) controller->AddTransaction(transactionData);
}
```

### Future Development Considerations

#### Transaction Filtering System
When implementing transaction filtering (by date, category, type, amount, etc.):

1. **Filtering Location**: Implement filtering logic in Model layer (`MLModel`) for proper MVC separation
2. **Data Transfer**: Use DTO/ViewModel pattern for View data to avoid MVC violations
   - Create `SMLTransactionViewData` struct for UI-specific data representation
   - Controller converts `CMLTransaction` to `SMLTransactionViewData`
3. **Performance Strategies**:
   - Start with simple approach: full data filtering on demand
   - Consider caching for frequently used filters
   - Use wxWidgets Virtual List for large datasets
   - Implement Iterator pattern for memory-efficient data access
4. **Observer Integration**:
   - Existing Observer pattern provides granular transaction events
   - Avoid passing Model objects directly to maintain MVC boundaries
   - Consider ID-based notifications with on-demand data retrieval

## Coding Conventions

This project follows specific naming conventions:

- **Classes**: Prefix with `CML` (e.g., `CMLModel`, `CMLTransaction`)
- **Structs**: Prefix with `SML` (e.g., `SMLTransactionData`)
- **Enums**: Prefix with `E_ML` (e.g., `E_MLTransactionType`)
- **Member variables**: PascalCase convention (e.g., `TransactionId`, `Amount`)
- **Local variables/parameters**: camelCase convention (e.g., `transactionData`, `userId`)

## Communication Language

**Important**: When working with this codebase, Claude Code should communicate in Korean (한국어) as this is the preferred language for this project. All responses, explanations, and documentation should be provided in Korean.