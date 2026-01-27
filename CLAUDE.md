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
- **View**: `wxMLMainFrame` class implementing `IMLView` and `IMLModelObserver` interfaces
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
- `IMLView`: UI interface for transaction display and user input
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

As of 2026-01-27, the project has:
- ✅ Basic Model structure (`FMLModel`, `FMLTransaction`)
- ✅ All interface definitions (Model, View, Controller, Storage, Observer)
- ✅ MVC Holder infrastructure
- ✅ Controller implementation (`FMLController` using MVCHolder pattern)
- ✅ DTO-based data access (Model provides `GetTransactionData` returning `FMLTransactionData`)
- ✅ Enhanced Model interface (CRUD operations, DTO conversion, business logic, filtering)
- ✅ **View implementation** (`wxMLMainFrame` in separate files implementing `IMLView` + `IMLModelObserver`)
- ✅ **Is-a architecture** (wxMLMainFrame directly inherits IMLView and IMLModelObserver)
- ✅ **Full CRUD functionality** (Add/Update/Delete with Observer pattern)
- ✅ **Observer pattern fully implemented** (Model → Observer → View UI updates)
- ✅ **UTF-8 encoding support** (wxString::FromUTF8 for Korean text, /utf-8 compiler flag, ToUTF8() for data storage)
- ✅ Clean UI (ID column removed, user-friendly display)
- ✅ **Enhanced input controls** (wxDatePickerCtrl for date selection, wxTextValidator for numeric amount input)
- ✅ **SQLite Storage Provider** (`FMLSQLiteStorage` with DI pattern)
- ✅ **File menu** (New/Open/Save/SaveAs with keyboard shortcuts)
- ✅ **Command line file opening** (supports .ml file association)
- ✅ **Transaction Filtering** (Date range, transaction type, category with differential update)
- ✅ **MVC Architecture Refinement** (View → Controller → Model boundaries strictly enforced)
- ⏳ Excel/CSV Export
- ⏳ Installer (file association registration)

#### Development Roadmap

The project follows a phased approach for implementation, prioritizing core functionality before advanced features.

**Phase 1: Core Program Functionality** (Current Priority)

1. **View Architecture Improvement** ✅ COMPLETED (2026-01-04)
   - ✅ wxMLMainFrame directly implements IMLView and IMLModelObserver (is-a pattern)
   - ✅ Separated wxMLMainFrame to MLMainFrame.h/cpp files
   - ✅ Removed FMLGuiView intermediate layer (simplified architecture)
   - ✅ Applied is-a inheritance pattern for wxWidgets integration

2. **Observer Pattern Implementation** ✅ COMPLETED (2026-01-04)
   - ✅ Added observer management to FMLModel (AddObserver/RemoveObserver)
   - ✅ Connected wxMLMainFrame as Observer (model->AddObserver(frame))
   - ✅ Implemented Observer event handlers (OnTransactionAdded, OnTransactionRemoved, etc.)
   - ✅ Observer events trigger automatic UI updates

3. **UTF-8 Encoding Support** ✅ COMPLETED (2026-01-04)
   - ✅ Added /utf-8 compiler flag to project settings
   - ✅ All Korean strings use wxString::FromUTF8() wrapper
   - ✅ Proper display of Korean text in UI

4. **Complete CRUD Operations** ✅ COMPLETED (2026-01-26)
   - ✅ Transaction Add (implemented with Observer pattern)
   - ✅ Transaction Update (list item selection → inline edit → update button)
   - ✅ Transaction Delete (delete button with confirmation dialog)
   - ✅ List selection events and input field population

**Phase 2: File Persistence (.ml File Support)** ✅ COMPLETED (2026-01-26)

5. **Storage Provider Implementation** ✅ COMPLETED
   - ✅ SQLite implementation (`FMLSQLiteStorage`)
   - ✅ Integrated with FMLModel via dependency injection
   - ✅ Save/Load operations with IMLStorageProvider

6. **File Dialog Integration** ✅ COMPLETED
   - ✅ File → New (Ctrl+N)
   - ✅ File → Open (.ml files) (Ctrl+O)
   - ✅ File → Save (Ctrl+S)
   - ✅ File → Save As (Ctrl+Shift+S)
   - ✅ Unsaved changes confirmation on close
   - ✅ Command line argument for file association

**Phase 3: Data Management Features** ✅ COMPLETED (2026-01-27)

7. **Transaction Filtering** ✅ COMPLETED
   - ✅ Filter UI panel (date range, transaction type, category)
   - ✅ `FMLFilterCriteria` data structure
   - ✅ Model layer filtering (`GetFilteredTransactionData`)
   - ✅ Controller layer delegation
   - ✅ Differential update pattern (Single Source of Truth)
   - ✅ Auto-update category filter on data changes
   - ✅ Apply/Clear filter buttons

8. **Differential Update Implementation** ✅ COMPLETED
   - ✅ Single Source of Truth pattern (read from UI, not cached state)
   - ✅ `GetCurrentListIds()` helper method
   - ✅ Incremental UI updates (add/remove only changed items)
   - ✅ Eliminated state synchronization bugs
   - ✅ Scroll position preservation during filtering

9. **MVC Architecture Refinement** ✅ COMPLETED
   - ✅ Removed direct Model references from View
   - ✅ Added file operation methods to Controller interface
   - ✅ All View operations route through Controller
   - ✅ Strict MVC boundary enforcement

10. **UTF-8 Encoding Fix** ✅ COMPLETED
    - ✅ Fixed Korean text storage issue (CP949 → UTF-8)
    - ✅ Applied `wxString::ToUTF8().data()` for all string conversions
    - ✅ Proper SQLite UTF-8 encoding

**Phase 4: Data Export & Distribution** ⏳ IN PROGRESS

11. **Excel/CSV Export** ⏳ PENDING
    - ⏳ CSV export with UTF-8 BOM (Korean support)
    - ⏳ Export current view (respects active filter)
    - ⏳ File menu integration
    - ⏳ Column headers and formatting

12. **.ml File Registration** ⏳ PENDING
    - ⏳ Installer creation (Inno Setup / NSIS)
    - ⏳ Windows registry setup for file association
    - ⏳ Double-click .ml file → launch MissionLedger
    - ⏳ Icon association

**Deferred Features:**
- Multiple document interface (MDI) support
- Report generation (charts, summaries)
- Multi-language support
- Undo/Redo functionality

**Current Focus**: Phase 4 - Excel/CSV Export (essential for practical use)

## Code Organization

```
MissionLedger/
├── main.cpp                     # Application entry point (MissionLedger class)
└── src/
    ├── MLDefine.h              # Core enums and data structures
    ├── interface/              # Abstract interfaces
    │   ├── IMLController.h     # Controller interface
    │   ├── IMLModel.h          # Model interface
    │   ├── IMLModelObserver.h  # Observer interface for Model events
    │   ├── IMLStorageProvider.h # Storage provider interface
    │   └── IMLView.h           # View interface
    └── module/
        ├── common/             # Common utilities and patterns
        │   └── holder/         # MVC Holder singleton pattern
        │       ├── FMLMVCHolder.*      # MVC component holder
        │       └── MLMVCHolderExample.h # Usage examples
        ├── storage/            # Storage implementations
        │   └── MLSQLiteStorage.*   # SQLite storage provider
        ├── third_party/        # Third-party libraries
        │   └── sqlite/         # SQLite amalgamation (sqlite3.c, sqlite3.h)
        └── mvc/
            ├── model/          # Model implementation
            │   ├── FMLModel.*  # Main model class
            │   └── transaction/# Transaction entity
            │       └── FMLTransaction.*
            ├── controller/     # Controller implementation
            │   └── FMLController.*  # Main controller class
            └── view/           # View implementation
                └── MLMainFrame.*    # Main window (wxFrame + IMLView + IMLModelObserver)
```

## Development Notes

- **Include paths**: Source files use `$(ProjectDir)src` as additional include directory
- **UTF-8 Encoding**: Project uses `/utf-8` compiler flag for proper Korean text handling
- **Korean Strings**: All Korean text uses `wxString::FromUTF8("한글")` wrapper for display
- **Observer Pattern**: Model automatically notifies View of data changes via IMLModelObserver
- **MVC Holder**: Centralized singleton access to Model, View, and Controller components

### Current Implementation Status

- **Model**: `FMLModel` class implemented with full CRUD operations, file operations, DTO conversion, and business logic
- **Controller**: `FMLController` class implemented using MVCHolder pattern for Model access
- **View**: `wxMLMainFrame` class implemented (MLMainFrame.h/cpp) with IMLView, IMLModelObserver, and file menu
- **Storage Provider**: `FMLSQLiteStorage` implemented with SQLite amalgamation, injected via DI pattern
- **Observer**: `IMLModelObserver` interface fully implemented with automatic UI updates
- **MVC Holder**: `FMLMVCHolder` singleton implemented for centralized component access
- **DTO**: `FMLTransactionData` struct serves as single DTO for both input and output
- **UI**: Full CRUD with file menu (New/Open/Save/SaveAs), keyboard shortcuts, unsaved changes detection

### MVC Holder Usage

The `FMLMVCHolder` singleton provides centralized access to MVC components throughout the application:

#### Initialization (in main.cpp):
```cpp
#include "module/common/holder/MLMVCHolder.h"
#include "module/mvc/view/MLMainFrame.h"
#include "module/storage/MLSQLiteStorage.h"

class MissionLedger : public wxApp {
    virtual bool OnInit() override {
        // Create MVC components
        auto& mvcHolder = FMLMVCHolder::GetInstance();
        auto model = std::make_shared<FMLModel>();
        auto controller = std::make_shared<FMLController>();

        // Storage Provider injection (DI pattern)
        auto storageProvider = std::make_shared<FMLSQLiteStorage>();
        model->SetStorageProvider(storageProvider);

        // Create main frame (wxWidgets manages memory)
        wxMLMainFrame* frame = new wxMLMainFrame();

        // Register with holder
        mvcHolder.SetModel(model);
        mvcHolder.SetController(controller);
        mvcHolder.SetView(std::shared_ptr<IMLView>(frame, [](IMLView*){})); // No-op deleter

        // Connect observer (frame implements IMLModelObserver)
        model->AddObserver(std::shared_ptr<IMLModelObserver>(frame, [](IMLModelObserver*){}));

        frame->Show();

        // Open file from command line argument (for file association)
        if (argc > 1) {
            wxString filePath = argv[1];
            if (wxFileExists(filePath) && filePath.EndsWith(".ml")) {
                model->OpenFile(filePath.ToStdString());
            }
        }

        return true;
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

### DTO (Data Transfer Object) Strategy

This project uses a **single DTO pattern** for data exchange between layers to balance simplicity and maintainability.

#### Design Decision

**Single DTO Approach**: `FMLTransactionData` serves both as input and output DTO
```cpp
struct FMLTransactionData {
    int TransactionId = -1;       // -1 for input (new transaction), actual ID for output
    E_MLTransactionType Type;
    std::string Category;
    std::string Item;
    std::string Description;
    double Amount;
    std::string ReceiptNumber;
    std::string DateTime;          // Empty for input, formatted string for output
};
```

**Rationale**:
- **Simplicity**: Single structure reduces code duplication and maintenance overhead
- **Pragmatic**: For simple CRUD applications, field overlap is typically 80%+
- **Clear Boundaries**: View/Controller use DTO, Model uses Entity internally
- **Flexibility**: Fields can be optional based on context (input vs output)

**Alternative Considered**: Separate Input/Output DTOs
- Would require duplicating 7+ fields across structures
- Overhead not justified for current application complexity
- Can refactor later if security or validation requirements grow

#### DTO vs Entity Separation

**What View/Controller See (DTO)**:
```cpp
FMLTransactionData data;  // Plain struct
data.DateTime = "2025-12-30 15:30:45";  // Formatted string
```

**What Model Uses Internally (Entity)**:
```cpp
FMLTransaction transaction;  // Rich domain object
std::chrono::system_clock::time_point dateTime;  // Native type
std::string GetDateTimeString() const;  // Business logic
```

**Conversion**: Model provides `convertToTransactionData()` private method to transform Entity → DTO

### Controller Implementation Pattern

The `FMLController` class implements a **thin controller** approach using the MVCHolder pattern.

#### MVCHolder-based Design

Controller does not hold a direct reference to Model. Instead, it accesses Model through `FMLMVCHolder`:

```cpp
class FMLController : public IMLController {
public:
    FMLController() = default;  // No dependencies injected

    void AddTransaction(const FMLTransactionData& data) override {
        auto model = FMLMVCHolder::GetInstance().GetModel();  // Get on demand
        if (model) model->AddTransaction(data);
    }
};
```

**Benefits**:
- **Initialization Flexibility**: MVC components can be created in any order
- **Global Access**: Any wxWidgets window/dialog can access Controller via MVCHolder
- **Decoupling**: Controller lifecycle is independent of Model lifecycle

**Trade-offs**:
- Runtime dependency (Model must be registered before use)
- Implicit coupling (dependency not visible in constructor)
- Acceptable for single-instance desktop applications

#### Controller Responsibilities

**What Controller Does**:
- Delegate operations to Model
- Provide DTO-based interface to View
- Validate Model availability (null checks)

**What Controller Does NOT Do**:
- Business logic (delegated to Model)
- Data transformation (Model handles Entity ↔ DTO conversion)
- UI updates (View's responsibility via Observer pattern)

### View Architecture Design

The project uses an **is-a inheritance pattern** where wxWidgets frames directly implement MVC interfaces.

#### Design Pattern (Is-a)

```
┌─────────────────────────────────────────┐
│         wxMLMainFrame                   │
│  (wxFrame + IMLView + IMLModelObserver) │
│                                         │
│  - wxWidgets UI (buttons, layout)      │
│  - MVC logic (Observer handlers)       │
│  - Direct Controller/Model access      │
└────────┬────────────────────────────────┘
         │ communicates with
         ↓
┌─────────────────┐
│  FMLController  │ → FMLModel
└─────────────────┘
```

**Rationale for Is-a Pattern:**
- **Simplicity**: No intermediate layer between UI and MVC logic
- **wxWidgets Convention**: Standard pattern for wxWidgets applications
- **Direct Event Handling**: Observer events directly update UI
- **Single Responsibility**: One class manages both UI and View logic for this desktop-only application

#### Implementation Structure

```cpp
// MLMainFrame.h
class wxMLMainFrame : public wxFrame, public IMLView, public IMLModelObserver {
public:
    wxMLMainFrame();

    // IMLView interface
    void AddTransaction(const FMLTransactionData& data) override;
    void DisplayTransaction(const FMLTransactionData& data) override;
    void DisplayTransactions() override;

    // IMLModelObserver interface
    void OnTransactionAdded(const FMLTransactionData& data) override;
    void OnTransactionRemoved(int transactionId) override;
    void OnTransactionUpdated(const FMLTransactionData& data) override;
    void OnTransactionsCleared() override;
    void OnDataLoaded() override;
    void OnDataSaved() override;

private:
    // UI Controls
    wxListCtrl* listCtrl;
    wxTextCtrl* categoryText;
    // ...

    // Event Handlers
    void OnAddTransaction(wxCommandEvent& event);
    void RefreshTransactionList();
    void ClearInputFields();
};
```

#### Benefits

1. **Simplicity**: Single class handles both UI and MVC logic
2. **Direct Updates**: Observer events directly manipulate wxWidgets controls
3. **wxWidgets Standard**: Follows conventional wxWidgets patterns
4. **Less Boilerplate**: No additional View wrapper classes needed
5. **MVC Compliance**: Still maintains strict Model-View-Controller boundaries

#### Trade-offs

- **Technology Lock-in**: Tightly coupled to wxWidgets (acceptable for desktop-only app)
- **Testing**: Requires wxWidgets instantiation for testing (mitigated by using wxWidgets test framework)
- **Has-a Alternative**: Previously considered composition pattern, but deemed over-engineered for this use case

### Implemented Design Patterns

#### Transaction Filtering System (✅ Implemented)

The filtering system uses a **differential update pattern** with **Single Source of Truth** principle:

**Architecture**:
1. **Filtering Location**: Model layer (`FMLModel::GetFilteredTransactionData()`)
2. **Filter Criteria**: `FMLFilterCriteria` struct in `MLDefine.h`
   - Date range filter (start/end date)
   - Transaction type filter (Income/Expense/All)
   - Category filter (specific category or all)
   - Amount range filter (optional, not currently used)
3. **Data Flow**: View → Controller → Model (strict MVC boundaries)

**Differential Update Implementation**:
```cpp
// Single Source of Truth: Read from UI, not cached state
std::set<int> GetCurrentListIds() {
    std::set<int> ids;
    for (long i = 0; i < listCtrl->GetItemCount(); i++) {
        ids.insert(listCtrl->GetItemData(i));
    }
    return ids;
}

void ApplyCurrentFilter() {
    std::set<int> previousIds = GetCurrentListIds();  // From UI
    std::set<int> currentIds = /* filtered results */;

    // Calculate diff and update only changed items
    RemoveItems(previousIds - currentIds);
    AddItems(currentIds - previousIds);
}
```

**Key Benefits**:
- ✅ No state synchronization bugs (UI is the single source of truth)
- ✅ Incremental updates preserve scroll position
- ✅ Minimal UI operations (only changed items)
- ✅ Automatic category list updates on data changes

**Observer Integration**:
- Filter-aware Observer handlers check `filterActive` flag
- When filter active: reapply filter on data changes
- When filter inactive: direct UI manipulation for performance

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

### Future Development Considerations

#### Potential Enhancements

**Filtering System**:
- Consider unified filtering approach (all operations through filter, even "show all")
- Amount range filter UI (currently defined in criteria but not exposed)
- Filter presets/favorites
- Filter history

**Performance Optimization**:
- For datasets >10,000 items: consider wxListCtrl virtual mode
- Database-level filtering (push filter to SQL query)
- Lazy loading with pagination

**Export Features**:
- Excel export with formatting and formulas
- PDF report generation
- Chart visualization

## Coding Conventions

This project follows specific naming conventions:

- **Classes**: Prefix with `FML` (e.g., `FMLModel`, `FMLTransaction`)
- **Structs**: Prefix with `FML` (e.g., `FMLTransactionData`)
- **Enums**: Prefix with `E_ML` (e.g., `E_MLTransactionType`)
- **Interfaces**: Prefix with `IML` (e.g., `IMLModel`, `IMLController`, `IMLView`)
- **Member variables**: PascalCase convention (e.g., `TransactionId`, `Amount`)
- **Public functions**: PascalCase convention (e.g., `AddTransaction()`, `GetTransactionData()`)
- **Private functions**: camelCase convention (e.g., `convertToTransactionData()`, `calculateTotal()`)
- **Local variables/parameters**: camelCase convention (e.g., `transactionData`, `userId`)

## Communication Language

**Important**: When working with this codebase, Claude Code should communicate in Korean (한국어) as this is the preferred language for this project. All responses, explanations, and documentation should be provided in Korean.

## Compact Instructions

When compacting, always preserve:
- Build commands (msbuild configurations)
- MVC architecture patterns and boundaries
- Storage provider implementation details (SQLite)
- File menu operations (New/Open/Save/SaveAs)
- Filtering system architecture (differential update, Single Source of Truth)
- Observer pattern integration
- UTF-8 encoding requirements (ToUTF8() for storage)
- Current implementation status and roadmap