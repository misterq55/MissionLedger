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

As of the latest commit (2026-01-02), the project has:
- ✅ Basic Model structure (`FMLModel`, `FMLTransaction`)
- ✅ All interface definitions (Model, GuiView, Controller, Storage, Observer)
- ✅ MVC Holder infrastructure
- ✅ Controller implementation (`FMLController` using MVCHolder pattern)
- ✅ DTO-based data access (Model provides `GetTransactionData` returning `FMLTransactionData`)
- ✅ Enhanced Model interface (CRUD operations, DTO conversion, business logic)
- ✅ UI implementation (`wxMLMainFrame` with input panel + list control in main.cpp)
- ✅ Transaction Add functionality (working with MVCHolder)
- ✅ View implementation (`FMLGuiView` implementing `IMLGuiView` + `IMLModelObserver`)
- ✅ Observer pattern integration (Model has AddObserver, View connected as Observer)
- ✅ Interface reorganization (IMLView → IMLGuiView, IMLModelObserver moved to interface/)
- ⚠️ Observer event handlers (stubs implemented, UI update logic pending)
- ⏳ Controller-based data access (View still accesses Model directly in some places)
- ⏳ Storage Provider (interface defined, implementations pending)

#### Development Roadmap

The project follows a phased approach for implementation, prioritizing core functionality before advanced features.

**Phase 1: Core Program Functionality** (Current Priority)

1. **View Architecture Improvement** ✅ COMPLETED (2026-01-02)
   - ✅ Separated wxMLMainFrame (wxWidgets UI) from FMLGuiView (MVC logic)
   - ✅ Applied has-a composition pattern (CLAUDE.md "View Architecture Design")
   - ✅ Implemented IMLGuiView and IMLModelObserver interfaces

2. **Observer Pattern Implementation** ⚠️ IN PROGRESS
   - ✅ Added observer management to FMLModel (AddObserver/RemoveObserver)
   - ✅ Connected View as Observer in main.cpp (model->AddObserver(view))
   - ⏳ Implement Observer event handlers in FMLGuiView (UI update logic)
   - ⏳ Emit events from Model on data changes

3. **Controller-based Data Access** ⏳ PENDING
   - Add GetAllTransactionData() to IMLController
   - Modify View to access data through Controller (not Model directly)
   - Maintain strict MVC boundaries (View → Controller → Model)

4. **Complete CRUD Operations** ⏳ IN PROGRESS
   - ✅ Transaction Add (implemented in main.cpp)
   - ⏳ Transaction Update (list item click → edit mode)
   - ⏳ Transaction Delete (delete button/menu)
   - ⏳ List selection events and detail view

**Phase 2: File Persistence (.ml File Support)**

5. **Storage Provider Implementation**
   - Choose implementation: SQLite (recommended), JSON, or XML
   - Integrate with FMLModel via dependency injection
   - Implement Save/Load operations with IMLStorageProvider

6. **File Dialog Integration**
   - File → Open (.ml files)
   - File → Save / Save As
   - Recent files menu
   - Auto-save on exit (optional)

**Phase 3: File Association (Optional)**

7. **.ml File Registration**
   - Windows registry setup for file association
   - Double-click .ml file → launch MissionLedger
   - Icon association

**Deferred Features:**
- Multiple document interface (MDI) support
- Advanced filtering with ID-based caching
- Report generation and Excel export
- Multi-language support

**Current Focus**: Phase 1, Steps 1-3 (View separation and Observer pattern)

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
            ├── controller/     # Controller implementation
            │   └── FMLController.*  # Main controller class
            └── view/           # (Placeholder for future implementation)
```

## Development Notes

- Include paths: Source files use `$(ProjectDir)src` as additional include directory
- Korean language comments are present throughout the codebase
- The project includes placeholder folders for controller and view implementations
- Transaction management is the core functionality with stub implementations

### Current Implementation Status

- **Model**: `FMLModel` class implemented with enhanced CRUD operations, DTO conversion, and business logic
- **Controller**: `FMLController` class implemented using MVCHolder pattern for Model access
- **View**: `IMLView` interface defined, implementation pending
- **Storage Provider**: `IMLStorageProvider` interface defined, concrete implementations pending (SQLite/JSON/XML)
- **Observer**: `IMLModelObserver` interface well-designed with specific transaction events
- **MVC Holder**: `FMLMVCHolder` singleton implemented for centralized component access
- **DTO**: `FMLTransactionData` struct serves as single DTO for both input and output

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

The project adopts a **composition-based View architecture** that separates wxWidgets UI from MVC logic.

#### Design Pattern

```
┌─────────────────┐
│  wxDialog/Frame │ ← Pure UI (buttons, text boxes, layout)
│  (wxWidgets)    │
└────────┬────────┘
         │ has-a (member)
         ↓
┌─────────────────┐
│    FMLView      │ ← MVC logic (implements IMLView + IMLModelObserver)
│  (Custom Class) │
└────────┬────────┘
         │ communicates with
         ↓
┌─────────────────┐
│  FMLController  │ → FMLModel
└─────────────────┘
```

#### Separation of Concerns

**wxDialog/wxFrame Responsibilities**:
- Widget creation and layout
- Event binding (button clicks, text input)
- Direct UI manipulation (enable/disable, show/hide)
- Forward user actions to attached View

**FMLView Responsibilities**:
- Implement `IMLView` interface (UI-agnostic output methods)
- Implement `IMLModelObserver` (receive Model change notifications)
- Communicate with Controller (send user actions, request data)
- Hold reference to wxWidget for UI updates

#### Example Structure

```cpp
class FMLTransactionView : public IMLView, public IMLModelObserver {
public:
    void AttachWindow(wxDialog* dialog) { Dialog = dialog; }

    // IMLView implementation
    void RefreshTransactionList() override;
    void ShowMessage(const std::string& msg, bool isError) override;

    // Observer implementation
    void OnTransactionAdded(std::shared_ptr<FMLTransaction> transaction) override;

    // Event handlers called by wxDialog
    void OnUserAddTransaction(const FMLTransactionData& data);

private:
    wxDialog* Dialog = nullptr;
    void updateDialogUI();
};

class wxTransactionDialog : public wxDialog {
public:
    wxTransactionDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, "거래") {
        TransactionView = std::make_shared<FMLTransactionView>();
        TransactionView->AttachWindow(this);
        CreateControls();
    }

private:
    std::shared_ptr<FMLTransactionView> TransactionView;

    void CreateControls() {
        addButton = new wxButton(this, wxID_ANY, "추가");
        addButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            FMLTransactionData data = CollectDataFromUI();
            TransactionView->OnUserAddTransaction(data);  // Forward to View
        });
    }
};
```

#### Benefits

1. **Technology Independence**: `IMLView` can be implemented with CLI, GUI, or Web UI
2. **Testability**: View logic can be tested without wxWidgets instantiation
3. **Reusability**: Same View can work with different UI frameworks
4. **MVC Compliance**: wxWidgets code remains unaware of Model/Controller
5. **Clear Boundaries**: UI code vs business logic separation

### Future Development Considerations

#### Transaction Filtering System
When implementing transaction filtering (by date, category, type, amount, etc.):

1. **Filtering Location**: Implement filtering logic in Model layer (`MLModel`) for proper MVC separation
2. **Data Transfer**: Use existing `FMLTransactionData` DTO for View data
   - Model provides `GetAllTransactionData()` for filtered results
   - Controller delegates filtering requests to Model
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
- **Interfaces**: Prefix with `IML` (e.g., `IMLModel`, `IMLController`, `IMLView`)
- **Member variables**: PascalCase convention (e.g., `TransactionId`, `Amount`)
- **Public functions**: PascalCase convention (e.g., `AddTransaction()`, `GetTransactionData()`)
- **Private functions**: camelCase convention (e.g., `convertToTransactionData()`, `calculateTotal()`)
- **Local variables/parameters**: camelCase convention (e.g., `transactionData`, `userId`)

## Communication Language

**Important**: When working with this codebase, Claude Code should communicate in Korean (한국어) as this is the preferred language for this project. All responses, explanations, and documentation should be provided in Korean.