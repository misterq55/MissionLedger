# Development Guide

This document provides practical guidance for implementing features in the MissionLedger project.

## Development Roadmap

The project follows a phased approach for implementation, prioritizing core functionality before advanced features.

### Phase 1: Core Program Functionality ✅ COMPLETED (2026-01-04)

1. **View Architecture Improvement**
   - ✅ wxMLMainFrame directly implements IMLView and IMLModelObserver (is-a pattern)
   - ✅ Separated wxMLMainFrame to MLMainFrame.h/cpp files
   - ✅ Removed FMLGuiView intermediate layer (simplified architecture)
   - ✅ Applied is-a inheritance pattern for wxWidgets integration

2. **Observer Pattern Implementation**
   - ✅ Added observer management to FMLModel (AddObserver/RemoveObserver)
   - ✅ Connected wxMLMainFrame as Observer (model->AddObserver(frame))
   - ✅ Implemented Observer event handlers (OnTransactionAdded, OnTransactionRemoved, etc.)
   - ✅ Observer events trigger automatic UI updates

3. **UTF-8 Encoding Support**
   - ✅ Added /utf-8 compiler flag to project settings
   - ✅ All Korean strings use wxString::FromUTF8() wrapper
   - ✅ Proper display of Korean text in UI

4. **Complete CRUD Operations**
   - ✅ Transaction Add (implemented with Observer pattern)
   - ✅ Transaction Update (list item selection → inline edit → update button)
   - ✅ Transaction Delete (delete button with confirmation dialog)
   - ✅ List selection events and input field population

### Phase 2: File Persistence (.ml File Support) ✅ COMPLETED (2026-01-26)

5. **Storage Provider Implementation**
   - ✅ SQLite implementation (`FMLSQLiteStorage`)
   - ✅ Integrated with FMLModel via dependency injection
   - ✅ Save/Load operations with IMLStorageProvider

6. **File Dialog Integration**
   - ✅ File → New (Ctrl+N)
   - ✅ File → Open (.ml files) (Ctrl+O)
   - ✅ File → Save (Ctrl+S)
   - ✅ File → Save As (Ctrl+Shift+S)
   - ✅ Unsaved changes confirmation on close
   - ✅ Command line argument for file association

### Phase 3: Data Management Features ✅ COMPLETED (2026-01-27)

7. **Transaction Filtering**
   - ✅ Filter UI panel (date range, transaction type, category)
   - ✅ `FMLFilterCriteria` data structure
   - ✅ Model layer filtering (`GetFilteredTransactionData`)
   - ✅ Controller layer delegation
   - ✅ Full reload pattern for filter changes (consistency and correctness)
   - ✅ Auto-update category filter on data changes
   - ✅ Apply/Clear filter buttons

8. **UI Update Strategy**
   - ✅ Incremental updates when filter inactive (performance optimization)
   - ✅ Full reload when filter active (consistency and correctness)
   - ✅ Freeze/Thaw pattern to minimize flickering
   - ✅ Eliminated state synchronization bugs

9. **MVC Architecture Refinement**
   - ✅ Removed direct Model references from View
   - ✅ Added file operation methods to Controller interface
   - ✅ All View operations route through Controller
   - ✅ Strict MVC boundary enforcement

10. **UTF-8 Encoding Fix**
    - ✅ Fixed Korean text storage issue (CP949 → UTF-8)
    - ✅ Applied `wxString::ToUTF8().data()` for all string conversions
    - ✅ Proper SQLite UTF-8 encoding

11. **Exchange Rate Support** ✅ COMPLETED (2026-02-06)
    - ✅ Added exchange rate fields to `FMLTransactionData`
      - `UseExchangeRate`: Toggle for foreign currency transactions
      - `Currency`: Currency code (e.g., USD, JPY, EUR)
      - `OriginalAmount`: Amount in original currency
      - `ExchangeRate`: Conversion rate to KRW
    - ✅ GUI integration with auto-calculation
    - ✅ CLI support for exchange rate parameters
    - ✅ SQLite schema updated with exchange rate columns

12. **Entity-DTO Architecture Refactoring** ✅ COMPLETED (2026-02-06)
    - ✅ Adopted Data-Oriented Design (Entity wraps Data)
    - ✅ Reduced from 12 individual getters → 1 `GetData()` method
    - ✅ Moved business logic to Entity (`MatchesFilter`, `IsValid`)
    - ✅ Simplified Model filtering logic (80 lines → 10 lines)
    - ✅ Converted Storage to DTO-based (removed Entity dependency)
    - ✅ Net code reduction: -136 lines while adding functionality

13. **Budget System Implementation** ✅ COMPLETED (2026-02-12)

    **Phase 1-2: Data Structure & CRUD** ✅ COMPLETED (2026-02-12)
    - ✅ Budget data structure redesign (`FMLBudgetData`)
      - ✅ Added `Type` field (Income/Expense) for Transaction consistency
      - ✅ Hierarchical structure: Category (required) + Item (optional)
      - ✅ `BudgetAmount` for manual input, `ActualAmount` reserved for future use
      - ✅ `BudgetId` with AUTOINCREMENT for unique identification
    - ✅ Budget CRUD operations with Observer pattern
      - ✅ Model: `AddBudget`, `UpdateBudget`, `DeleteBudget`, `GetBudget`, `GetAllBudgets`
      - ✅ Controller: Full delegation of Budget operations
      - ✅ Observer events: `OnBudgetAdded`, `OnBudgetUpdated`, `OnBudgetDeleted`, `OnBudgetCleared`
    - ✅ Storage layer implementation (SQLite)
      - ✅ `budgets` table with AUTOINCREMENT primary key
      - ✅ `SaveBudget`, `LoadAllBudgets`, `DeleteBudget`, `DeleteAllBudgets` methods
      - ✅ Complete save/load/delete cycle working correctly
      - ✅ Added `budget_id` column to `transactions` table (reserved for future use)
    - ✅ UI implementation (Budget tab)
      - ✅ 2-panel layout: Category list (left) + Item detail list (right)
      - ✅ Inline input fields (Type, Category, Item, Amount, Notes)
      - ✅ Add/Update/Delete buttons with proper state management
      - ✅ Budget summary panel (Total Budget only - simplified design)
      - ✅ List selection integration with input fields
      - ✅ Removed real-time actual amount and variance display
    - ✅ CLI implementation
      - ✅ `budget list` - Display all budgets
      - ✅ `budget add` - Add new budget with options
      - ✅ `budget update` - Update existing budget
      - ✅ `budget delete` - Delete budget by ID
      - ✅ Command-line and interactive modes

    **Phase 3: Budget-Transaction Integration** ⏸️ POSTPONED
    - **Design Decision**: Real-time budget tracking postponed
    - Rationale: For mission project settlement, post-trip comparison is more important than real-time monitoring
    - Budget tab simplified to budget planning only (BudgetAmount input)
    - ActualAmount calculation deferred to settlement report generation (Phase 4)
    - `Transaction.BudgetId` and `ActualAmount` fields reserved for future implementation if needed

### Phase 4: Settlement Report & Data Export ✅ MOSTLY COMPLETED

14. **Transaction Summary Display** ✅ COMPLETED (2026-01-30)
    - ✅ List footer panel with summary totals
    - ✅ FMLTransactionSummary data structure
    - ✅ Real-time calculation (Income/Expense/Balance)
    - ✅ Filter-aware summary (respects active filter)
    - ✅ Color-coded display (Income: green, Expense: red, Balance: blue/red)
    - ✅ Currency formatting with thousand separators
    - ✅ Observer integration (all data change events trigger summary update)
    - ✅ Helper methods (createSummaryPanel, updateSummaryPanel, displaySummary, formatAmountWithComma, buildCurrentFilterCriteria)

15. **Settlement Aggregation Logic** ✅ COMPLETED (2026-02-14)
    - ✅ Data structure design (`FMLSettlmentData` with hierarchical budget/actual)
    - ✅ Category-based transaction aggregation
    - ✅ Budget vs Actual comparison calculation
    - ✅ Exchange rate tracking and display
    - ✅ Model layer implementation (ExportSettlementToPDF)
    - ✅ Controller delegation

16. **Excel/CSV Export** 🎯 NEXT
    - ⏳ CSV export with UTF-8 BOM (Korean support)
    - ⏳ Transaction list export
    - ⏳ Budget comparison table export
    - ⏳ Settlement report export
    - ⏳ File menu integration
    - ⏳ Column headers and formatting

17. **PDF Export** ✅ COMPLETED (2026-02-14)

    **Settlement PDF Export** ✅ COMPLETED (2026-02-14)
    - ✅ Library: PDF-Writer (static library integration)
    - ✅ Korean font support (Malgun Gothic TTF embedding)
    - ✅ **5-section layout**: Summary (budget + actual totals/balance), Exchange rates, Income comparison, Expense comparison
    - ✅ Category-level aggregation (income/expense totals only, no item breakdown for clean presentation)
    - ✅ Side-by-side budget vs actual comparison (3-column tables: Category, Budget, Actual)
    - ✅ Auto-generated title and filename from .ml file
    - ✅ Optimized spacing (30pt subtitle gaps) and alignment (10pt right margin for amounts)
    - ✅ Exchange rate information display with average rates
    - ✅ GUI menu: File → Export Settlement PDF (Ctrl+E)
    - ✅ CLI command: export settlement <file.ml> [--out <output.pdf>]

    **Transaction List PDF Export** ✅ COMPLETED (2026-02-13)
    - ✅ Individual transaction listing with hierarchical grouping (Category → Item → Transactions)
    - ✅ Cell merging for same Category and Item
    - ✅ Multi-line text wrapping for Item and Description columns (UTF-8 safe)
    - ✅ **Vertical center alignment** for multi-line text in Item and Description columns
    - ✅ Dynamic row height calculation based on text content
    - ✅ Mid-group page breaks for optimal space utilization
    - ✅ **Exchange rate information** displayed inline with amount (e.g., "1,000,000원 (PHP 40,000 @ 25.0)")
    - ✅ **Optimized column widths**: Category(50) Item(60) Description(170) Amount(135) Date(60) Receipt(50)
    - ✅ Category-wise subtotals with formatted table layout
    - ✅ Auto-suggested filename from .ml file (GUI) or optional output path (CLI)
    - ✅ GUI menu: File → Export Transaction List PDF (Ctrl+T)
    - ✅ CLI command: export transactions <file.ml> [--out <output.pdf>]

18. **.ml File Registration** ⏳ PENDING
    - ⏳ Installer creation (Inno Setup / NSIS)
    - ⏳ Windows registry setup for file association
    - ⏳ Double-click .ml file → launch MissionLedger
    - ⏳ Icon association

### Deferred Features
- Multiple document interface (MDI) support
- Multi-language support
- Undo/Redo functionality

## Future Development Considerations

### PDF Export (Phase 4 - Planned)

**Recommended Library**: wxPdfDocument

**Why wxPdfDocument?**
- ✅ Perfect integration with wxWidgets
- ✅ Korean font support (TrueType)
- ✅ Easy installation and usage
- ✅ MIT License (free for commercial use)
- ✅ Active maintenance

**Installation:**
```bash
vcpkg install wxpdfview
# Or: https://github.com/utelle/wxpdfdoc
```

**Reference Layout**: `결산안.pdf`
- Summary table (income/expense totals, balance)
- Category grouping (생활, 사역 준비, 팀사역, etc.)
- Detail table (date, item, amount, receipt number)
- Exchange rate information (PHP 24.84)

**Implementation Plan:**
1. Install and test wxPdfDocument basic example
2. Setup Korean fonts (NanumGothic or Malgun Gothic)
3. Render transaction table with proper formatting
4. Add summary section (total income, expense, balance)
5. Integrate file save dialog

**Example Code Structure:**
```cpp
wxPdfDocument pdf;
pdf.AddPage();
pdf.AddFont("NanumGothic", "", "NanumGothic.ttf");
pdf.SetFont("NanumGothic", "", 12);

// Header
pdf.Cell(0, 10, wxString::FromUTF8("거래 내역"), 0, 1, wxPDF_ALIGN_CENTER);

// Table
for (auto& transaction : transactions) {
    pdf.Cell(30, 8, transaction.DateTime);
    pdf.Cell(40, 8, transaction.Category);
    pdf.Cell(50, 8, transaction.Item);
    pdf.Cell(40, 8, formatAmount(transaction.Amount));
    pdf.Ln();
}

pdf.SaveAsFile("output.pdf");
```

**Alternative Approaches:**
- **CSV Export** (Very simple, 1 hour) - Users can open in Excel
- **HTML → PDF** (Requires external tool like wkhtmltopdf)

**Priority**: Deferred until CSV export and installer features are completed

**Current Focus**: Phase 4 - Transaction Summary Display (essential for budget tracking)

## Implementation Patterns

### Transaction Filtering System

The filtering system uses a **full reload pattern** for filter changes, with **incremental updates** for non-filter operations:

#### Architecture
1. **Filtering Location**: Model layer (`FMLModel::GetFilteredTransactionData()`)
2. **Filter Criteria**: `FMLFilterCriteria` struct in `MLDefine.h`
   - Date range filter (start/end date)
   - Transaction type filter (Income/Expense/All)
   - Category filter (specific category or all)
   - Text search filter (item, description, receipt number)
3. **Data Flow**: View → Controller → Model (strict MVC boundaries)

#### Update Strategy Implementation

```cpp
void wxMLMainFrame::applyCurrentFilter()
{
    // Full reload pattern (filter active)
    listCtrl->Freeze();                           // Minimize flickering
    listCtrl->DeleteAllItems();                   // Clear all items

    auto transactions = controller->GetFilteredTransactionData(criteria);
    for (const auto& trans : transactions) {
        DisplayTransaction(trans);                // Add filtered items
    }

    if (currentSortColumn != -1) {
        listCtrl->SortItems(CompareTransactions); // Reapply sorting
    }

    listCtrl->Thaw();                             // Resume rendering
}
```

#### Key Benefits
- ✅ Eliminates state synchronization bugs (no differential calculation needed)
- ✅ Prevents visual artifacts (items disappearing one-by-one)
- ✅ Handles file switching correctly (old data properly cleared)
- ✅ Simple and maintainable (no complex diff logic)
- ✅ Freeze/Thaw minimizes flickering

#### Observer Integration
- Filter-aware Observer handlers check `FilterActive` flag
- **When filter active**: Call `applyCurrentFilter()` (full reload)
- **When filter inactive**: Direct UI manipulation (incremental update for performance)
  - `OnTransactionAdded`: `DisplayTransaction(transactionData)`
  - `OnTransactionRemoved`: `removeListItemByTransactionId(transactionId)`
  - `OnTransactionUpdated`: `DisplayTransaction(transactionData)`

### Budget System Implementation Pattern

The Budget system follows Transaction architecture for consistency, with additional calculated fields.

#### Data Structure Design

**FMLItemBudgetData**: Hybrid DTO (input + calculated fields)
```cpp
struct FMLItemBudgetData {
    // Input Fields (User-provided)
    int BudgetId = -1;
    E_MLTransactionType Type;        // Income/Expense
    std::string Category;
    std::string Item;
    int64_t BudgetAmount = 0;

    // Calculated Fields (Auto-computed)
    int64_t ActualAmount = 0;        // From transactions
    int64_t Variance = 0;            // Actual - Budget
    int TransactionCount = 0;        // Number of matching transactions
};
```

#### Budget-Transaction Matching

**Key Concept**: Actual amounts auto-calculated by matching (Type, Category, Item)

```cpp
std::vector<FMLItemBudgetData> FMLModel::GetAllBudgets() const {
    std::vector<FMLItemBudgetData> result;

    for (const auto& [id, budgetEntity] : Budgets) {
        FMLItemBudgetData budget = budgetEntity->GetData();

        // Calculate ActualAmount by scanning transactions
        int64_t actual = 0;
        int count = 0;
        for (const auto& [tid, transaction] : Transactions) {
            const auto& tData = transaction->GetData();
            if (tData.Type == budget.Type &&
                tData.Category == budget.Category &&
                tData.Item == budget.Item) {
                actual += tData.Amount;
                count++;
            }
        }

        budget.ActualAmount = actual;
        budget.Variance = actual - budget.BudgetAmount;
        budget.TransactionCount = count;
        result.push_back(budget);
    }

    return result;
}
```

#### UI Display Pattern

**Budget List**: Displays `FMLItemBudgetData` with all fields
```cpp
void DisplayBudgets(const std::vector<FMLItemBudgetData>& budgets) {
    budgetListCtrl->DeleteAllItems();

    for (const auto& budget : budgets) {
        long idx = budgetListCtrl->InsertItem(itemCount,
                       wxString::FromUTF8(budget.Category.c_str()));
        budgetListCtrl->SetItem(idx, 1, wxString::FromUTF8(budget.Item.c_str()));
        budgetListCtrl->SetItem(idx, 2, formatAmount(budget.BudgetAmount));
        budgetListCtrl->SetItem(idx, 3, formatAmount(budget.ActualAmount));
        budgetListCtrl->SetItem(idx, 4, formatAmount(budget.Variance));
        budgetListCtrl->SetItemData(idx, budget.BudgetId);  // Associate ID
    }
}
```

**Summary Panel**: Displays `FMLBudgetSummary` for hierarchical aggregation
```cpp
void DisplayBudgetSummary(const FMLBudgetSummary& summary) {
    summaryTotalBudget->SetLabel(formatAmount(summary.TotalBudget));
    summaryTotalActual->SetLabel(formatAmount(summary.TotalActualExpense));
    summaryTotalVariance->SetLabel(formatAmount(summary.TotalVariance));
}
```

#### Consistency with Transaction Pattern

| Aspect | Transaction | Budget |
|--------|-------------|---------|
| **DTO** | `FMLTransactionData` | `FMLItemBudgetData` |
| **Calculated Fields** | `DateTime` (empty → formatted) | `ActualAmount` (0 → calculated) |
| **UI Display** | `DisplayTransactions()` | `DisplayBudgets()` |
| **Summary** | `FMLTransactionSummary` | `FMLBudgetSummary` |
| **Observer** | `OnTransactionAdded/Updated/Deleted` | `OnBudgetAdded/Updated/Deleted` |

### GUI Implementation Guidelines

#### wxListCtrl Row-Data Association

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

#### MVC-Compliant View Design
- Views should not store business data directly
- Use `wxListCtrl::SetItemData()` to associate row indices with entity IDs
- Controller manages current display state and filtering
- Views request data from Controller using IDs, never directly from Model

## Development Notes

### UTF-8 Encoding
- **Compiler Flag**: Project uses `/utf-8` compiler flag for proper Korean text handling
- **Display**: All Korean text uses `wxString::FromUTF8("한글")` wrapper
- **Storage**: Use `wxString::ToUTF8().data()` for storing text in SQLite

### Include Paths
Source files use `$(ProjectDir)src` as additional include directory

### Observer Pattern Usage
Model automatically notifies View of data changes via IMLModelObserver

### MVC Holder Usage
Centralized singleton access to Model, View, and Controller components

## Future Development Considerations

### Potential Enhancements

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

## Testing Strategy

- **Unit Tests**: Model logic (business rules, calculations)
- **Integration Tests**: Controller operations (DTO conversion, delegation)
- **UI Tests**: View behavior (wxWidgets test framework)
