# Architecture Documentation

This document describes the architectural design and patterns used in the MissionLedger project.

## MVC Pattern Implementation

The codebase follows a strict MVC architecture with centralized component management:

- **Model**: `FMLModel` class manages transaction data using `std::map<int, std::shared_ptr<FMLTransaction>>`
- **View**: `wxMLMainFrame` class implementing `IMLView` and `IMLModelObserver` interfaces
- **Controller**: `FMLController` class implementing `IMLController` for transaction operations
- **Observer Pattern**: Model notifies View of data changes via `IMLModelObserver` interface
- **MVC Holder**: `FMLMVCHolder` singleton pattern for global MVC component access

## Design Approach: Model First Strategy

This project adopts a **Model First** design approach for MVC implementation. This strategy prioritizes establishing a solid data foundation before building controller logic and UI components.

### Rationale for Model First

**Why Model First?**
- **Data-Centric Application**: Financial transaction management is fundamentally about data integrity and business rules
- **Clear Domain Model**: Transaction entities and their relationships are well-defined from requirements
- **Stable Foundation**: Data structures change less frequently than UI/UX requirements
- **Testability**: Business logic can be validated independently without UI dependencies
- **Domain Complexity**: Financial calculations, validations, and aggregations require careful design

**Alternative Approaches Considered:**
1. **View First**: Better for UI-heavy apps with unclear requirements - not suitable for this project
2. **Controller First (Use Case Driven)**: Good for feature-centric design - viable but Model First provides better data integrity

### Design Principles

- **Separation of Concerns**: Strict MVC boundaries - View never accesses Model directly
- **Dependency Direction**: View → Controller → Model (never reversed)
- **Data Flow**: Model notifies via Observer, View requests via Controller
- **Testing Strategy**: Unit test Model logic → Integration test Controller → UI test View

## Core Data Structure

The main entity is `FMLTransaction` representing financial transactions with:
- Transaction type (Income/Expense) via `E_MLTransactionType` enum
- Categories, items, descriptions, amounts
- Timestamps using `std::chrono::system_clock::time_point`
- Receipt number tracking

## Key Interfaces

- `IMLModel`: Data management interface with Add/Remove/Save/Load operations
- `IMLController`: Business logic interface for transaction handling
- `IMLView`: UI interface for transaction display and user input
- `IMLStorageProvider`: Storage abstraction interface using Strategy Pattern for pluggable persistence
- `IMLModelObserver`: Observer pattern for data change notifications
- `FMLMVCHolder`: Singleton holder for centralized MVC component access

## MVC Holder Pattern

The `FMLMVCHolder` singleton provides centralized access to MVC components throughout the application.

### Design Benefits
- **Initialization Flexibility**: MVC components can be created in any order
- **Global Access**: Any wxWidgets window/dialog can access components via MVCHolder
- **Decoupling**: Component lifecycles are independent

### Trade-offs
- Runtime dependency (components must be registered before use)
- Implicit coupling (dependency not visible in constructor)
- Acceptable for single-instance desktop applications

### Initialization Pattern

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

### Usage in Components

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

## Storage Provider Pattern

The project uses Strategy Pattern for data persistence, allowing flexible storage backend selection.

### Design Principles
- **Abstraction**: `IMLStorageProvider` interface decouples Model from storage implementation
- **Pluggability**: Storage backend can be changed at runtime without modifying Model code
- **Testability**: Mock storage providers enable unit testing without file I/O
- **Extensibility**: New storage types can be added without touching existing code

### Interface Overview

```cpp
class IMLStorageProvider {
    virtual bool Initialize(const std::string& filePath) = 0;
    virtual bool SaveTransaction(const FMLTransactionData& data) = 0;
    virtual bool SaveAllTransactions(const std::vector<FMLTransactionData>& transactions) = 0;
    virtual bool LoadAllTransactions(std::vector<FMLTransactionData>& outTransactions) = 0;
    virtual bool DeleteTransaction(int transactionId) = 0;
    virtual bool UpdateTransaction(const FMLTransactionData& data) = 0;
    virtual int GetLastTransactionId() = 0;
    virtual E_MLStorageType GetStorageType() const = 0;
    virtual bool IsReady() const = 0;
};
```

### Implementations
- **SQLite** (✅ Implemented): Recommended for production (ACID compliance, efficient querying, data integrity)
- **JSON** (Planned): Good for prototyping and human-readable data
- **XML** (Planned): Compatible with external tools, wxWidgets built-in support

### Usage Example

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

## DTO (Data Transfer Object) Strategy

This project uses a **single DTO pattern** for data exchange between layers to balance simplicity and maintainability.

### Design Decision

**Single DTO Approach**: `FMLTransactionData` serves both as input and output DTO

```cpp
struct FMLTransactionData {
    int TransactionId = -1;       // -1 for input (new transaction), actual ID for output
    E_MLTransactionType Type;
    std::string Category;
    std::string Item;
    std::string Description;
    int64_t Amount;                // Integer for financial precision (no floating-point errors)
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

### DTO vs Entity Separation

**What View/Controller See (DTO)**:
```cpp
FMLTransactionData data;  // Plain struct
data.DateTime = "2025-12-30 15:30:45";  // Formatted string
```

**What Model Uses Internally (Entity)**:
```cpp
FMLTransaction transaction;  // Rich domain object
auto data = transaction.GetData();  // Access underlying data
bool isValid = transaction.IsValid();  // Business logic
```

**Data Access**: Entity exposes data via `GetData()` method, returning const reference to internal `FMLTransactionData`

## Entity-DTO Architecture (Data-Oriented Design)

This project adopts a **Data-Oriented Design** approach where Entity wraps Data structure rather than duplicating fields.

### Design Evolution

**Previous Design** (Field Duplication):
```cpp
class FMLTransaction {
private:
    int Id;                           // 12 individual fields
    E_MLTransactionType Type;
    std::string Category;
    // ... 9 more fields

public:
    int GetId() const;                // 12 individual getters
    E_MLTransactionType GetType() const;
    // ... 10 more getters
};
```

**Current Design** (Data-Oriented):
```cpp
class FMLTransaction {
private:
    FMLTransactionData data_;         // Single data container

public:
    const FMLTransactionData& GetData() const;  // Single getter
    void SetData(const FMLTransactionData& data);

    // Business logic methods
    bool MatchesFilter(const FMLFilterCriteria& criteria) const;
    bool IsValid() const;
};
```

### Design Benefits

1. **Reduced Code Duplication**: 12 getters → 1 getter (90% reduction)
2. **Clear Responsibility Separation**:
   - `FMLTransactionData`: Pure data container (DTO)
   - `FMLTransaction`: Business logic + data wrapper (Entity)
3. **Simplified Model Layer**: Filtering logic reduced from 80 lines → 10 lines
4. **Entity Encapsulation**: Business logic (validation, filtering) lives in Entity, not Model
5. **Storage Independence**: Storage layer uses DTO directly, no Entity dependency

### Business Logic Encapsulation

**What Entity Owns**:
```cpp
bool FMLTransaction::MatchesFilter(const FMLFilterCriteria& criteria) const {
    if (criteria.UseTypeFilter && data_.Type != criteria.TypeFilter)
        return false;

    if (criteria.UseDateRangeFilter)
        if (data_.DateTime < criteria.StartDate || data_.DateTime > criteria.EndDate)
            return false;

    // ... all filter logic
    return true;
}

bool FMLTransaction::IsValid() const {
    if (data_.Amount <= 0) return false;
    if (data_.Category.empty()) return false;
    return true;
}
```

**What Model Does** (Simplified):
```cpp
std::vector<FMLTransactionData> FMLModel::GetFilteredTransactionData(
    const FMLFilterCriteria& criteria) {

    std::vector<FMLTransactionData> result;
    for (const auto& pair : Transactions) {
        if (pair.second->MatchesFilter(criteria))  // Delegated to Entity
            result.push_back(pair.second->GetData());
    }
    return result;
}
```

### Layer Separation

**Storage ↔ DTO ↔ Model(Entity) ↔ DTO ↔ View**

```
┌──────────────┐
│   Storage    │ Uses FMLTransactionData (DTO)
│   (SQLite)   │ - No Entity dependency
└──────┬───────┘ - Direct field access
       │
       ↓ DTO
┌──────────────┐
│    Model     │ Manages FMLTransaction (Entity)
│              │ - Converts DTO → Entity on load
└──────┬───────┘ - Converts Entity → DTO on save
       │
       ↓ DTO
┌──────────────┐
│ View/Control │ Uses FMLTransactionData (DTO)
│              │ - No Entity access
└──────────────┘
```

### Rationale for Data-Oriented Design

**Why Wrap Data Instead of Field Duplication?**
- **C++ Best Practice**: Composition over inheritance/duplication
- **Maintainability**: Adding new fields requires changing only `FMLTransactionData`
- **Performance**: Single memory block (cache-friendly), no getter overhead
- **Simplicity**: Clear data flow, no synchronization issues between Entity fields and DTO

**Why Not Direct Field Access?**
- **Encapsulation**: Business logic methods (MatchesFilter, IsValid) justify Entity existence
- **Future Flexibility**: Can add computed properties, lazy loading, or validation later
- **Domain Model**: Entity represents domain concept, not just data storage

**Alternative Considered**: Entity with individual fields duplicating DTO
- Rejected due to maintenance overhead (12+ fields, 12+ getters)
- Synchronization bugs when adding/removing fields
- No clear benefit over Data-Oriented approach for this use case

## Controller Implementation Pattern

The `FMLController` class implements a **thin controller** approach using the MVCHolder pattern.

### MVCHolder-based Design

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

### Controller Responsibilities

**What Controller Does**:
- Delegate operations to Model
- Provide DTO-based interface to View
- Validate Model availability (null checks)

**What Controller Does NOT Do**:
- Business logic (delegated to Model)
- Data transformation (Model handles Entity ↔ DTO conversion)
- UI updates (View's responsibility via Observer pattern)

## View Architecture Design

The project uses an **is-a inheritance pattern** where wxWidgets frames directly implement MVC interfaces.

### Design Pattern (Is-a)

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

### Implementation Structure

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

### Benefits

1. **Simplicity**: Single class handles both UI and MVC logic
2. **Direct Updates**: Observer events directly manipulate wxWidgets controls
3. **wxWidgets Standard**: Follows conventional wxWidgets patterns
4. **Less Boilerplate**: No additional View wrapper classes needed
5. **MVC Compliance**: Still maintains strict Model-View-Controller boundaries

### Trade-offs

- **Technology Lock-in**: Tightly coupled to wxWidgets (acceptable for desktop-only app)
- **Testing**: Requires wxWidgets instantiation for testing (mitigated by using wxWidgets test framework)
- **Has-a Alternative**: Previously considered composition pattern, but deemed over-engineered for this use case

## Observer Pattern Implementation

The Observer pattern enables automatic UI updates when Model data changes.

### Design
- Model maintains a single `shared_ptr<IMLModelObserver>` (currently supports one observer)
- View implements IMLModelObserver and registers itself with Model via `AddObserver()`
- Model notifies observer when data changes (Add/Remove/Update/Load/Save)

**Current Implementation Note**:
- Single observer design (sufficient for desktop app with one main view)
- Future enhancement: Convert to `std::vector<std::weak_ptr<IMLModelObserver>>` for multiple observers if needed

### Event Types

```cpp
class IMLModelObserver {
    virtual void OnTransactionAdded(const FMLTransactionData& data) = 0;
    virtual void OnTransactionDeleted(const int transactionId) = 0;
    virtual void OnTransactionUpdated(const FMLTransactionData& data) = 0;
    virtual void OnTransactionsCleared() = 0;
    virtual void OnBudgetAdded(const FMLItemBudgetData& budgetData) = 0;
    virtual void OnBudgetDeleted(const int budgetId) = 0;
    virtual void OnBudgetUpdated(const FMLItemBudgetData& budgetData) = 0;
    virtual void OnBudgetCleared() = 0;
    virtual void OnDataLoaded() = 0;
    virtual void OnDataSaved() = 0;
};
```

### Benefits
- Automatic UI synchronization with data changes
- Decouples Model from View (Model doesn't know about View implementation)
- Single observer design simplifies lifecycle management

## Budget System Design

The Budget system follows the same architectural patterns as Transactions for consistency and maintainability.

### Design Philosophy: Transaction-Budget Consistency

**Core Principle**: Budget system mirrors Transaction system architecture for unified learning curve and maintenance.

| Aspect | Transaction | Budget |
|--------|-------------|---------|
| **CRUD DTO** | `FMLTransactionData` | `FMLItemBudgetData` |
| **Entity** | `FMLTransaction` | `FMLItemBudget` |
| **UI Display** | `DisplayTransactions(vector<TransactionData>)` | `DisplayBudgets(vector<ItemBudgetData>)` |
| **Summary** | `FMLTransactionSummary` (total only) | `FMLBudgetSummary` (hierarchical) |

### ItemBudgetData Structure

Budget items use a **hybrid DTO pattern** combining input fields and calculated fields:

```cpp
struct FMLItemBudgetData {
    // Primary Key
    int BudgetId = -1;

    // Input Fields (User-provided)
    E_MLTransactionType Type;      // Income/Expense classification
    std::string Category;           // Budget category (e.g., "항공", "생활")
    std::string Item;               // Budget item (e.g., "항공료 선결제", "숙박비")
    int64_t BudgetAmount = 0;       // Budgeted amount (user input)

    // Calculated Fields (Auto-computed from Transactions)
    int64_t ActualAmount = 0;       // Actual amount from matching transactions
    int64_t Variance = 0;           // Variance (Actual - Budget)
    int TransactionCount = 0;       // Number of matching transactions
};
```

**Design Rationale**:
- **Hybrid Pattern**: Combines input DTO + calculated fields in single structure
- **Consistency**: Mirrors `FMLTransactionData.DateTime` pattern (input: empty, output: formatted)
- **Simplicity**: Single structure avoids separate Input/Display DTOs
- **Clarity**: Calculated fields marked as "ReadOnly" in comments

### Budget-Transaction Integration

**Key Concept**: Actual amounts are auto-calculated by matching transactions to budget items.

**Matching Logic**:
```cpp
// Transactions with matching (Type, Category, Item) contribute to ActualAmount
Budget: Type=Expense, Category="항공", Item="항공료 선결제", BudgetAmount=3,000,000
  ↓ matches
Transaction: Type=Expense, Category="항공", Item="항공료 선결제", Amount=2,800,000
  ↓ result
ActualAmount=2,800,000, Variance=-200,000 (under budget)
```

**GetAllBudgets() Behavior**:
```cpp
// Returns FMLItemBudgetData with calculated fields populated
auto budgets = controller->GetAllBudgets();  // Scans transactions, calculates ActualAmount
```

### Budget Summary Hierarchy

Unlike Transactions (single-level summary), Budgets use a **3-tier hierarchical structure**:

```
FMLBudgetSummary (Overall)
  ├─ TotalBudget, TotalActualExpense, TotalActualIncome, TotalVariance
  └─ Categories: map<string, FMLCategoryBudgetSummary>
       └─ FMLCategoryBudgetSummary (Per Category)
            ├─ TotalBudget, TotalActualExpense, TotalActualIncome
            └─ Items: map<string, FMLItemBudgetSummary>
                 └─ FMLItemBudgetSummary (Per Item)
                      └─ BudgetAmount, ActualAmount, Variance
```

**Example**:
```
Overall Summary:
  TotalBudget=5,000,000, TotalActualExpense=4,900,000, Variance=-100,000

  Category "항공":
    TotalBudget=3,000,000, TotalActualExpense=2,800,000
    Items:
      - "항공료 선결제": Budget=3,000,000, Actual=2,800,000, Variance=-200,000

  Category "생활":
    TotalBudget=2,000,000, TotalActualExpense=2,100,000
    Items:
      - "숙박비": Budget=2,000,000, Actual=2,100,000, Variance=+100,000
```

### UI Display Pattern

**Budget List View**: Displays `FMLItemBudgetData` with all fields (input + calculated)

```cpp
void DisplayBudgets(const std::vector<FMLItemBudgetData>& budgets) {
    for (const auto& budget : budgets) {
        AddRow(budget.Category, budget.Item,
               budget.BudgetAmount,    // User input
               budget.ActualAmount,    // Auto-calculated
               budget.Variance);       // Auto-calculated
    }
}
```

**Summary View**: Displays `FMLBudgetSummary` for hierarchical aggregation

```cpp
void DisplayBudgetSummary(const FMLBudgetSummary& summary) {
    // Show overall totals
    ShowTotal(summary.TotalBudget, summary.TotalActualExpense);

    // Show per-category breakdown
    for (const auto& [category, catSummary] : summary.Categories) {
        ShowCategoryRow(category, catSummary.TotalBudget, catSummary.TotalActualExpense);
    }
}
```

### Consistency vs Domain Characteristics

**Why Transaction and Budget share the same pattern despite different domains?**

- **Transaction**: Displays raw input data (what user entered)
- **Budget**: Displays input + calculated data (budget vs actual comparison)

**Both use the same DTO-based UI pattern because**:
1. **Consistency**: Easier to learn, maintain, and extend
2. **Flexibility**: DTO can contain both input and calculated fields
3. **Simplicity**: Single pattern across all domains
4. **Precedent**: `FMLTransactionData.DateTime` already uses input/output duality

**Trade-off**: Calculated fields in DTO slightly blur "pure input DTO" concept, but the benefit of consistency outweighs this theoretical concern.
