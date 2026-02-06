# Coding Conventions

This document defines the coding standards and naming conventions for the MissionLedger project.

## Naming Conventions

### Classes
- Prefix with `FML`
- Use PascalCase
- Examples: `FMLModel`, `FMLTransaction`, `FMLController`

### Structs
- Prefix with `FML`
- Use PascalCase
- Examples: `FMLTransactionData`, `FMLFilterCriteria`

### Enums
- Prefix with `E_ML`
- Use PascalCase
- Examples: `E_MLTransactionType`, `E_MLStorageType`

### Interfaces
- Prefix with `IML`
- Use PascalCase
- Examples: `IMLModel`, `IMLController`, `IMLView`, `IMLStorageProvider`

### Member Variables
- Use PascalCase
- No prefix required
- Examples: `TransactionId`, `Amount`, `Category`

### Public Functions
- Use PascalCase
- Verb-first naming for actions
- Examples: `AddTransaction()`, `GetTransactionData()`, `SaveFile()`

### Private Functions
- Use camelCase
- Verb-first naming for actions
- Examples: `convertToTransactionData()`, `calculateTotal()`, `notifyObservers()`

### Local Variables and Parameters
- Use camelCase
- Descriptive names preferred
- Examples: `transactionData`, `userId`, `filePath`

## File Naming

- Header files: Use `.h` extension
- Implementation files: Use `.cpp` extension
- Match the class name (without prefix for main classes)
  - `FMLModel` → `FMLModel.h` / `FMLModel.cpp`
  - `wxMLMainFrame` → `MLMainFrame.h` / `MLMainFrame.cpp`

## Code Organization

### Include Order
1. Corresponding header file (for .cpp files)
2. C++ standard library headers
3. Third-party library headers (wxWidgets, SQLite)
4. Project headers

Example:
```cpp
#include "FMLModel.h"              // Corresponding header

#include <vector>                  // Standard library
#include <memory>
#include <map>

#include <wx/wx.h>                 // Third-party

#include "interface/IMLModel.h"    // Project headers
#include "MLDefine.h"
```

### Header Guards
Use `#pragma once` for header guards (modern, simpler than traditional include guards)

```cpp
#pragma once

class FMLModel {
    // ...
};
```

## Documentation

### File Headers
Include brief description at the top of each file:

```cpp
// FMLModel.h
// Model class for managing transaction data in MissionLedger
```

### Function Comments
- Use C++ style comments (`//`) for brief descriptions
- Use multi-line comments (`/* */`) for detailed explanations
- Document public API thoroughly
- Private functions may have minimal comments if self-explanatory

Example:
```cpp
// Adds a new transaction to the model
// Returns true if successful, false otherwise
bool AddTransaction(const FMLTransactionData& data);

private:
    // Rebuilds category cache from all transactions
    void rebuildCategoryCache() const;
```

## Code Style

### Indentation
- Use tabs for indentation (Visual Studio default)
- Consistent with Visual Studio project settings

### Braces
- Opening brace on same line for functions, classes, control structures
- Always use braces for control structures, even single-line statements

```cpp
void MyFunction() {
    if (condition) {
        DoSomething();
    }
}
```

### Spacing
- Space after keywords: `if (condition)`, `for (int i = 0; ...)`
- Space around operators: `a + b`, `x = 5`
- No space before function parentheses: `MyFunction()`

### Line Length
- Prefer lines under 120 characters
- Break long lines at logical points

## Smart Pointer Usage

### Ownership Guidelines
- Use `std::shared_ptr` for shared ownership (Model's transaction map)
- Use `std::weak_ptr` for observers (prevent circular references)
- Use `std::unique_ptr` for exclusive ownership
- Avoid raw pointers for ownership (except wxWidgets managed objects)

Example:
```cpp
class FMLModel {
    std::map<int, std::shared_ptr<FMLTransaction>> transactions;  // Shared ownership
    std::vector<std::weak_ptr<IMLModelObserver>> observers;       // Weak references
};
```

## Error Handling

### Return Values
- Use `bool` for success/failure operations
- Return false on error, true on success
- Consider exceptions for exceptional conditions (rare in current codebase)

### Validation
- Validate input parameters at function entry
- Check for null pointers before dereferencing
- Validate data before persisting to storage

Example:
```cpp
bool AddTransaction(const FMLTransactionData& data) {
    if (data.Amount <= 0) {
        return false;  // Invalid amount
    }

    // Proceed with operation
    return true;
}
```

## wxWidgets Specific Conventions

### Memory Management
- wxWidgets manages memory for window objects (wxFrame, wxDialog, etc.)
- Use `new` for creating windows, DO NOT use smart pointers
- Parent windows delete their children automatically

### String Handling
- **Display**: Use `wxString::FromUTF8("텍스트")` for Korean text in UI
- **Storage**: Use `wxString::ToUTF8().data()` for saving to SQLite
- **Conversion**: `std::string` ↔ `wxString` conversion via `ToStdString()` / `FromUTF8()`

Example:
```cpp
// Display in UI
label->SetLabel(wxString::FromUTF8("거래 내역"));

// Store in database
std::string category = categoryText->GetValue().ToUTF8().data();

// Conversion
wxString wx = wxString::FromUTF8(stdString.c_str());
std::string std = wxStr.ToUTF8().data();
```

### Event Handling
- Use event tables or `Bind()` for event connections
- Event handler functions should be private
- Prefix event handlers with `On`: `OnButtonClick()`, `OnMenuSave()`

### Naming Convention Exceptions

wxWidgets has specific naming conventions that differ from the project's general rules:

#### Event Handlers (Private Functions Exception)
- Event handlers are **private functions** but use **PascalCase** (not camelCase)
- Must use `On` prefix followed by event description
- This follows wxWidgets framework conventions

```cpp
private:
    // ✅ Event handlers - PascalCase with On prefix (wxWidgets convention)
    void OnButtonClick(wxCommandEvent& event);
    void OnMenuSave(wxCommandEvent& event);
    void OnListItemSelected(wxListEvent& event);

    // ✅ Other private functions - camelCase (project convention)
    void updateButtonStates();
    void clearInputFields();
```

#### UI Control Member Variables (camelCase Exception)
- wxWidgets UI control pointers use **camelCase** (not PascalCase)
- This follows wxWidgets community conventions and improves readability
- General (non-wxWidgets) member variables still use **PascalCase**

```cpp
private:
    // ✅ wxWidgets UI controls - camelCase (wxWidgets convention)
    wxTextCtrl* categoryText;
    wxListCtrl* listCtrl;
    wxButton* addButton;
    wxRadioButton* incomeRadio;

    // ✅ General member variables - PascalCase (project convention)
    int SelectedTransactionId = -1;
    bool FilterActive = false;
    std::string CurrentFilePath;
```

**Rationale:**
- **Event Handlers**: wxWidgets documentation and examples consistently use `OnXxx` pattern
- **UI Controls**: Shorter camelCase names (`listCtrl`) are more readable than PascalCase (`ListCtrl`)
- **Consistency**: Following framework conventions reduces confusion and matches official examples

## MVC Architecture Compliance

### Dependency Rules
- **View** may access Controller (never Model directly)
- **Controller** may access Model (never View directly)
- **Model** notifies View via Observer pattern (indirect, interface-based)

### Data Access
- View uses DTO (`FMLTransactionData`) for all data exchange
- Model uses Entity (`FMLTransaction`) internally
- Controller performs DTO ↔ Entity conversion (delegated to Model)

### Examples

**❌ WRONG - View accessing Model directly:**
```cpp
void wxMLMainFrame::OnSave(wxCommandEvent& event) {
    auto model = FMLMVCHolder::GetInstance().GetModel();
    model->Save();  // Direct Model access from View
}
```

**✅ CORRECT - View accessing through Controller:**
```cpp
void wxMLMainFrame::OnSave(wxCommandEvent& event) {
    auto controller = FMLMVCHolder::GetInstance().GetController();
    controller->SaveFile();  // Controller delegates to Model
}
```

## Version Control

### Commit Messages
- Use Korean for commit messages (project preference)
- Format: `<Type>: <Description>`
- Types: `Feat`, `Fix`, `Refactor`, `Docs`, `Style`, `Test`

Examples:
```
Feat: 거래 필터링 기능 구현
Fix: UTF-8 인코딩 문제 수정
Refactor: Observer 패턴 적용
```

### Branch Naming
- Use descriptive branch names
- Format: `feature/<feature-name>`, `fix/<bug-name>`
- Use English for branch names (international convention)

Examples:
```
feature/transaction-filtering
fix/utf8-encoding-issue
refactor/mvc-architecture
```
