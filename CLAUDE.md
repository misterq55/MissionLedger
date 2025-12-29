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

- **Model**: `FMLModel` class manages transaction data using `std::map<int, std::shared_ptr<FMLTransaction>>`
- **View**: Interface defined by `IMLView` (currently minimal)
- **Controller**: Interface defined by `IMLController` for transaction operations
- **Observer Pattern**: `MLModelObserver` provides event notifications for data changes
- **MVC Holder**: `FMLMVCHolder` singleton pattern for global MVC component access

### Core Data Structure

The main entity is `FMLTransaction` representing financial transactions with:
- Transaction type (Income/Expense) via `E_MLTransactionType` enum
- Categories, items, descriptions, amounts
- Timestamps using `std::chrono::system_clock::time_point`
- Receipt number tracking

### Key Interfaces

- `IMLModel`: Data management interface with Add/Remove/Save/Load operations
- `IMLController`: Business logic interface for transaction handling
- `IMLView`: UI interface (placeholder for future implementation)
- `IMLStorageProvider`: Storage abstraction interface using Strategy Pattern for pluggable persistence
- `MLModelObserver`: Observer pattern for data change notifications
- `FMLMVCHolder`: Singleton holder for centralized MVC component access

## Design Approach

### Model First Strategy

This project adopts a **Model First** design approach for MVC implementation. This strategy prioritizes establishing a solid data foundation before building controller logic and UI components.

#### Rationale for Model First

**Why Model First?**
- **Data-Centric Application**: Financial transaction management is fundamentally about data integrity and business rules
- **Clear Domain Model**: Transaction entities and their relationships are well-defined from requirements
- **Stable Foundation**: Data structures change less frequently than UI/UX requirements
- **Testability**: Business logic can be validated independently without UI dependencies
- **Domain Complexity**: Financial calculations, validations, and aggregations require careful design

**Alternative Approaches Considered:**
1. **View First**: Better for UI-heavy apps with unclear requirements - not suitable for this project
2. **Controller First (Use Case Driven)**: Good for feature-centric design - viable but Model First provides better data integrity

#### Implementation Phases

**Phase 1: Model Layer (Priority)**
1. **Storage Provider Integration**
   - Implement `IMLStorageProvider` concrete classes (SQLite/JSON)
   - Connect storage to `FMLModel` via dependency injection
   - Implement persistence for all CRUD operations

2. **Observer Pattern Integration**
   - Add observer registration/removal to `FMLModel`
   - Emit events on data changes (Add/Remove/Update/Load/Save)
   - Maintain observer list with weak pointers

3. **Model Interface Enhancement**
   - Data retrieval methods (`GetTransaction`, `GetAllTransactions`)
   - Filtering capabilities (by date range, category, type, amount)
   - Aggregation methods (total income/expense, balance, category summaries)
   - Business logic (validation, calculations, tax computations)

**Phase 2: Controller Layer**
1. **FMLController Implementation**
   - Create concrete controller class implementing `IMLController`
   - Define use cases (Add/Edit/Delete transaction, Apply filters, Get reports)
   - Implement DTO conversion (`FMLTransaction` → `FMLTransactionViewData`)
   - Connect to Model layer

2. **Extended Controller Interface**
   - Transaction update operations
   - Filter management (`GetFilteredTransactionIds`)
   - View data preparation methods
   - Command pattern for undo/redo (future consideration)

**Phase 3: View Layer**
1. **Main Window Implementation**
   - wxFrame-based main window with wxListCtrl
   - Menu bar (File, Edit, View, Reports)
   - Toolbar for common operations

2. **View Integration**
   - Implement `IMLView` interface
   - Inherit from `MLModelObserver` for automatic updates
   - Use MVCHolder for accessing Model/Controller
   - ID-based data association with UI controls

3. **Dialog Windows**
   - Add/Edit Transaction dialog
   - Filter configuration dialog
   - Report generation views

#### Design Principles

- **Separation of Concerns**: Strict MVC boundaries - View never accesses Model directly
- **Dependency Direction**: View → Controller → Model (never reversed)
- **Data Flow**: Model notifies via Observer, View requests via Controller
- **Testing Strategy**: Unit test Model logic → Integration test Controller → UI test View

#### Current Status

As of the latest commit, the project has:
- ✅ Basic Model structure (`FMLModel`, `FMLTransaction`)
- ✅ All interface definitions (Model, View, Controller, Storage, Observer)
- ✅ MVC Holder infrastructure
- ⏳ Storage Provider (interface defined, implementations pending)
- ⏳ Observer integration (interface defined, Model connection pending)
- ⏳ Controller implementation (interface defined, concrete class pending)
- ⏳ View implementation (interface defined, UI pending)

**Next Steps**: Complete Phase 1 (Model Layer) before proceeding to Controller and View implementations.

## Code Organization

```
MissionLedger/
├── main.cpp                     # wxWidgets application entry point
└── src/
    ├── MLDefine.h              # Core enums and data structures
    ├── interface/              # Abstract interfaces
    │   ├── IMLController.h     # Controller interface
    │   ├── IMLModel.h          # Model interface
    │   ├── IMLStorageProvider.h # Storage provider interface
    │   └── IMLView.h           # View interface
    └── module/
        ├── common/             # Common utilities and patterns
        │   ├── holder/         # MVC Holder singleton pattern
        │   │   ├── FMLMVCHolder.*      # MVC component holder
        │   │   └── MLMVCHolderExample.h # Usage examples
        │   └── observer/       # Observer pattern implementation
        ├── storage/            # Storage implementations (planned)
        │   ├── MLSQLiteStorage.*   # SQLite storage provider
        │   ├── MLJsonStorage.*     # JSON storage provider
        │   └── MLXmlStorage.*      # XML storage provider
        └── mvc/
            ├── model/          # Model implementation
            │   ├── FMLModel.*  # Main model class
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
- **Storage Provider**: `IMLStorageProvider` interface defined, concrete implementations pending (SQLite/JSON/XML)
- **Observer**: `MLModelObserver` well-designed with specific transaction events
- **MVC Holder**: `FMLMVCHolder` singleton implemented for centralized component access

### MVC Holder Usage

The `FMLMVCHolder` singleton provides centralized access to MVC components throughout the application:

#### Initialization (in main.cpp):
```cpp
#include "module/common/holder/MLMVCHolder.h"

class MyApp : public wxApp {
    virtual bool OnInit() override {
        // Create MVC components
        auto model = std::make_shared<FMLModel>();
        auto view = std::make_shared<FMLView>();
        auto controller = std::make_shared<FMLController>();

        // Register with holder
        auto& holder = FMLMVCHolder::GetInstance();
        holder.SetModel(model);
        holder.SetView(view);
        holder.SetController(controller);

        // Connect observer
        model->AddObserver(view);

        return true;
    }

    virtual int OnExit() override {
        FMLMVCHolder::DestroyInstance();
        return wxApp::OnExit();
    }
};
```

#### Usage in wxWidgets Windows/Dialogs:
```cpp
// In menu handlers
void OnMenuSave(wxCommandEvent& event) {
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model) model->Save();
}

// In dialog OK handlers
void AddTransactionDialog::OnOK(wxCommandEvent& event) {
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) controller->AddTransaction(transactionData);
}
```

### Storage Provider Pattern

The project uses Strategy Pattern for data persistence, allowing flexible storage backend selection:

#### Design Principles
- **Abstraction**: `IMLStorageProvider` interface decouples Model from storage implementation
- **Pluggability**: Storage backend can be changed at runtime without modifying Model code
- **Testability**: Mock storage providers enable unit testing without file I/O
- **Extensibility**: New storage types can be added without touching existing code

#### Interface Overview
```cpp
class IMLStorageProvider {
    virtual bool Initialize(const std::string& filePath) = 0;
    virtual bool SaveTransaction(const FMLTransaction& transaction) = 0;
    virtual bool SaveAllTransactions(const std::vector<std::shared_ptr<FMLTransaction>>& transactions) = 0;
    virtual bool LoadAllTransactions(std::vector<std::shared_ptr<FMLTransaction>>& outTransactions) = 0;
    virtual bool DeleteTransaction(int transactionId) = 0;
    virtual bool UpdateTransaction(const FMLTransaction& transaction) = 0;
    virtual int GetLastTransactionId() = 0;
    virtual E_MLStorageType GetStorageType() const = 0;
    virtual bool IsReady() const = 0;
};
```

#### Planned Implementations
- **SQLite**: Recommended for production (ACID compliance, efficient querying, data integrity)
- **JSON**: Good for prototyping and human-readable data
- **XML**: Compatible with external tools, wxWidgets built-in support

#### Usage Example
```cpp
// Create storage provider
auto storage = std::make_shared<FMLSQLiteStorage>();
storage->Initialize("transactions.db");

// Inject into model
auto model = std::make_shared<FMLModel>();
model->SetStorageProvider(storage);

// Model uses storage transparently
model->Save();  // Delegates to storage->SaveAllTransactions()
model->Load();  // Delegates to storage->LoadAllTransactions()
```

### Future Development Considerations

#### Transaction Filtering System
When implementing transaction filtering (by date, category, type, amount, etc.):

1. **Filtering Location**: Implement filtering logic in Model layer (`MLModel`) for proper MVC separation
2. **Data Transfer**: Use DTO/ViewModel pattern for View data to avoid MVC violations
   - Create `FMLTransactionViewData` struct for UI-specific data representation
   - Controller converts `FMLTransaction` to `FMLTransactionViewData`
3. **Performance Strategies**:
   - Start with simple approach: full data filtering on demand
   - Consider caching for frequently used filters
   - Use wxWidgets Virtual List for large datasets
   - Implement Iterator pattern for memory-efficient data access
4. **Observer Integration**:
   - Existing Observer pattern provides granular transaction events
   - Avoid passing Model objects directly to maintain MVC boundaries
   - Consider ID-based notifications with on-demand data retrieval

#### GUI Implementation Guidelines

**wxListCtrl Row-Data Association:**
```cpp
// Standard approach: Associate transaction ID with list rows
listCtrl->SetItemData(rowIndex, transactionId);

// Retrieve ID on user interaction
void OnListItemSelected(wxListEvent& event) {
    long selectedRow = event.GetIndex();
    int transactionId = listCtrl->GetItemData(selectedRow);
    // Use transactionId to fetch data from Controller/Model
}
```

**MVC-Compliant View Design:**
- Views should not store business data directly
- Use `wxListCtrl::SetItemData()` to associate row indices with entity IDs
- Controller manages current display state and filtering
- Views request data from Controller using IDs, never directly from Model

**Advanced Filtering Implementation:**
For improved performance with large datasets, consider ID-based filtering with differential updates:

```cpp
class TransactionView {
private:
    std::vector<int> m_PrevFilteredIds;  // Cache previous filter results

public:
    void ApplyFilter(const FilterCriteria& criteria) {
        auto controller = FMLMVCHolder::GetInstance().GetController();
        std::vector<int> currentIds = controller->GetFilteredTransactionIds(criteria);

        // Calculate difference and update only changed rows
        auto diff = CalculateDifference(m_PrevFilteredIds, currentIds);
        UpdateUIWithDiff(diff);

        m_PrevFilteredIds = std::move(currentIds);
    }
};
```

This approach minimizes UI updates and preserves user selection/scroll state during filtering operations.

## Coding Conventions

This project follows specific naming conventions:

- **Classes**: Prefix with `FML` (e.g., `FMLModel`, `FMLTransaction`)
- **Structs**: Prefix with `FML` (e.g., `FMLTransactionData`)
- **Enums**: Prefix with `E_ML` (e.g., `E_MLTransactionType`)
- **Member variables**: PascalCase convention (e.g., `TransactionId`, `Amount`)
- **Local variables/parameters**: camelCase convention (e.g., `transactionData`, `userId`)

## Communication Language

**Important**: When working with this codebase, Claude Code should communicate in Korean (한국어) as this is the preferred language for this project. All responses, explanations, and documentation should be provided in Korean.