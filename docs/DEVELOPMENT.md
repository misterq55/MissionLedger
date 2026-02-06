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
   - ✅ Differential update pattern (Single Source of Truth)
   - ✅ Auto-update category filter on data changes
   - ✅ Apply/Clear filter buttons

8. **Differential Update Implementation**
   - ✅ Single Source of Truth pattern (read from UI, not cached state)
   - ✅ `GetCurrentListIds()` helper method
   - ✅ Incremental UI updates (add/remove only changed items)
   - ✅ Eliminated state synchronization bugs
   - ✅ Scroll position preservation during filtering

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

### Phase 4: Data Export & Distribution ⏳ IN PROGRESS

13. **Transaction Summary Display** ✅ COMPLETED (2026-01-30)
    - ✅ List footer panel with summary totals
    - ✅ FMLTransactionSummary data structure
    - ✅ Real-time calculation (Income/Expense/Balance)
    - ✅ Filter-aware summary (respects active filter)
    - ✅ Color-coded display (Income: green, Expense: red, Balance: blue/red)
    - ✅ Currency formatting with thousand separators
    - ✅ Observer integration (all data change events trigger summary update)
    - ✅ Helper methods (createSummaryPanel, updateSummaryPanel, displaySummary, formatAmountWithComma, buildCurrentFilterCriteria)

14. **Excel/CSV Export** ⏳ PENDING
    - ⏳ CSV export with UTF-8 BOM (Korean support)
    - ⏳ Export current view (respects active filter)
    - ⏳ File menu integration
    - ⏳ Column headers and formatting

15. **.ml File Registration** ⏳ PENDING
    - ⏳ Installer creation (Inno Setup / NSIS)
    - ⏳ Windows registry setup for file association
    - ⏳ Double-click .ml file → launch MissionLedger
    - ⏳ Icon association

16. **PDF Export** 📋 PLANNED
    - 📋 Library: wxPdfDocument (wxWidgets integration, Korean font support)
    - 📋 Reference layout: 결산안.pdf
    - 📋 Estimated time: 2-3 hours (basic implementation)
    - 📋 Features: Transaction table, summary, Korean fonts

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

The filtering system uses a **differential update pattern** with **Single Source of Truth** principle:

#### Architecture
1. **Filtering Location**: Model layer (`FMLModel::GetFilteredTransactionData()`)
2. **Filter Criteria**: `FMLFilterCriteria` struct in `MLDefine.h`
   - Date range filter (start/end date)
   - Transaction type filter (Income/Expense/All)
   - Category filter (specific category or all)
   - Amount range filter (optional, not currently used)
3. **Data Flow**: View → Controller → Model (strict MVC boundaries)

#### Differential Update Implementation

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

#### Key Benefits
- ✅ No state synchronization bugs (UI is the single source of truth)
- ✅ Incremental updates preserve scroll position
- ✅ Minimal UI operations (only changed items)
- ✅ Automatic category list updates on data changes

#### Observer Integration
- Filter-aware Observer handlers check `filterActive` flag
- When filter active: reapply filter on data changes
- When filter inactive: direct UI manipulation for performance

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
