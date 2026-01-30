# 리스트 정렬 구현 방식 조사 보고서

이 문서는 다른 데스크톱 애플리케이션들의 리스트 컬럼 헤더 클릭 정렬 기능 구현 방식을 조사한 결과입니다.

**조사 일자**: 2026-01-30
**조사 목적**: MissionLedger의 거래 리스트 정렬 기능 구현을 위한 업계 표준 패턴 분석

---

## 목차

1. [조사 대상 프로그램](#조사-대상-프로그램)
2. [Qt Framework - QTableView](#1-qt-framework---qtableview)
3. [wxWidgets - wxListCtrl](#2-wxwidgets---wxlistctrl)
4. [GnuCash](#3-gnucash)
5. [Windows Explorer](#4-windows-explorer)
6. [Microsoft Excel](#5-microsoft-excel)
7. [업계 표준 패턴 정리](#업계-표준-패턴-정리)
8. [세션 상태 관리](#세션-상태-관리)
9. [결론 및 권장사항](#결론-및-권장사항)

---

## 조사 대상 프로그램

| 프로그램 | 유형 | UI 프레임워크 | 선정 이유 |
|---------|------|-------------|----------|
| **Qt TableView** | GUI Framework | Qt 6 | 업계 표준 MVC 프레임워크 |
| **wxWidgets ListCtrl** | GUI Framework | wxWidgets 3.2 | 우리 프로젝트와 동일 |
| **GnuCash** | 재무 관리 | GTK+ | 오픈소스 재무 앱 |
| **Windows Explorer** | 파일 관리 | Win32 | 데스크톱 UI 표준 |
| **Microsoft Excel** | 스프레드시트 | Office | 데이터 정렬의 표준 |

---

## 1. Qt Framework - QTableView ⭐

### 아키텍처: Proxy Model 패턴

Qt는 Model과 View 사이에 **Proxy Model**을 두어 정렬을 처리하는 독특한 아키텍처를 채택했습니다.

```cpp
// Model (데이터 원본) - 정렬 로직 없음
QAbstractTableModel* sourceModel = new MyDataModel();

// Proxy Model (중간 레이어) - 정렬 담당
QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel();
proxyModel->setSourceModel(sourceModel);

// View (표시)
QTableView* tableView = new QTableView();
tableView->setModel(proxyModel);
tableView->setSortingEnabled(true);  // 헤더 클릭 자동 처리
```

### 동작 원리

1. **View가 컬럼 헤더 클릭 감지**
2. **Proxy Model이 정렬 수행** (`sort()` 메서드 자동 호출)
3. **인덱스 매핑 자동 관리** (`mapToSource()`, `mapFromSource()`)
4. **View는 Proxy의 정렬된 데이터 표시**

### 특징

- ✅ **원본 데이터 불변**: sourceModel은 입력 순서 유지
- ✅ **투명한 인덱스 변환**: Proxy가 자동으로 View 인덱스 ↔ Model 인덱스 매핑
- ✅ **필터 + 정렬 통합**: `QSortFilterProxyModel`이 둘 다 처리
- ✅ **재사용성**: 하나의 Model에 여러 View/Proxy 연결 가능
- ⚠️ **복잡도**: Proxy 레이어 추가로 구조 복잡

### MVC 관점

- **Model**: 데이터만 관리 (정렬 로직 없음)
- **Proxy**: 정렬/필터링 전담 (View와 Model 사이)
- **View**: 표시만 담당

### 참조

- [Qt Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html)
- [QTableView Class Reference](https://doc.qt.io/qt-6/qtableview.html)
- [QSortFilterProxyModel Forum Discussion](https://forum.qt.io/topic/99812/qtableview-qabstracttablemodel-qsortfilterproxymodel-sorting)

---

## 2. wxWidgets - wxListCtrl

### 아키텍처: View 레이어 정렬

wxWidgets는 **View에서 직접 정렬**을 처리하는 전통적인 방식을 사용합니다.

```cpp
// 이벤트 핸들러
void MyFrame::OnColumnClick(wxListEvent& event) {
    int column = event.GetColumn();

    // 정렬 방향 토글
    if (sortColumn == column) {
        sortAscending = !sortAscending;
    } else {
        sortColumn = column;
        sortAscending = true;
    }

    // 정렬 수행
    listCtrl->SortItems(MyCompareFunction, column);

    // 정렬 아이콘 표시
    listCtrl->ShowSortIndicator(column, sortAscending);
}

// 비교 함수 (정적 또는 전역)
int wxCALLBACK MyCompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    // item1, item2는 SetItemData()로 설정한 클라이언트 데이터
    // 예: Transaction ID

    MyFrame* frame = (MyFrame*)sortData;
    int column = frame->sortColumn;

    // 컬럼별 비교 로직
    switch(column) {
        case 0: // 유형
            return CompareType(item1, item2);
        case 1: // 카테고리
            return CompareCategory(item1, item2);
        // ...
    }
}
```

### 핵심 메커니즘

**1. SetItemData로 ID 저장**:
```cpp
long index = listCtrl->InsertItem(0, "수입");
listCtrl->SetItemData(index, transactionId);  // ID 연결
```

**2. SortItems로 정렬**:
- 리스트 항목의 **표시 순서만 변경**
- 원본 데이터는 변경하지 않음
- `SetItemData()`로 저장한 ID는 그대로 유지

**3. 정렬 후 데이터 접근**:
```cpp
long index = listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
int transactionId = listCtrl->GetItemData(index);
// transactionId로 원본 데이터 조회
```

### 특징

- ✅ **View에서 표시 순서만 정렬**: 원본 Model 데이터 불변
- ✅ **비교 함수로 유연한 정렬**: 컬럼별 커스텀 로직 구현 가능
- ✅ **간단한 API**: `SortItems()` 한 줄로 정렬
- ❌ **수동 구현 필요**: Qt처럼 자동화되지 않음 (이벤트 핸들러, 비교 함수 직접 작성)
- ❌ **프레임 참조 필요**: 비교 함수가 정적이라 프레임 포인터 전달 필요

### MVC 관점

- **Model**: 데이터 관리 (ID 순서 유지)
- **Controller**: 이벤트 핸들러에서 정렬 요청
- **View**: `SortItems()`로 표시 순서 변경

### 참조

- [wxListCtrl Column Sorting Forum Discussion](https://forums.wxwidgets.org/viewtopic.php?t=2174)
- [wxListCtrl Sorting Challenge Thread](https://forums.wxwidgets.org/viewtopic.php?f=1&t=9063)
- [wxListCtrl Class Reference](https://docs.wxwidgets.org/latest/classwx_list_ctrl.html)

---

## 3. GnuCash

### 아키텍처: View 레이어 정렬 (GTK+ 기반)

GnuCash는 오픈소스 재무 관리 소프트웨어로, GTK+ TreeView를 사용합니다.

```cpp
// GTK+ TreeView 정렬 설정
GtkTreeView* treeView = gtk_tree_view_new();
GtkTreeModel* model = create_transaction_model();

// 컬럼에 정렬 ID 설정
gtk_tree_view_column_set_sort_column_id(column, COLUMN_CATEGORY);

// TreeSortable 인터페이스로 비교 함수 등록
gtk_tree_sortable_set_sort_func(
    GTK_TREE_SORTABLE(model),
    COLUMN_CATEGORY,
    compare_category_func,
    NULL, NULL
);

// 자동으로 헤더 클릭 시 정렬
```

### 실제 구현 예시

GnuCash의 거래 목록 (Register View):
- **다단계 정렬**: Type → Name 순으로 정렬
- **날짜 기본 정렬**: 최신 거래가 상단
- **정렬 상태 유지**: 세션 간 정렬 기준 저장

### 특징

- ✅ **View 레벨 정렬**: 거래 데이터(Model)는 입력 순서 유지
- ✅ **GTK+ 프레임워크 활용**: TreeView/ListStore의 내장 정렬 기능 사용
- ✅ **다단계 정렬 지원**: Shift+클릭으로 2차 정렬
- ✅ **성능 최적화**: Virtual Scrolling으로 대량 데이터 처리

### 데이터 무결성

GnuCash는 재무 앱이므로:
- ✅ **입력 순서(Transaction ID) 보존** 필수
- ✅ **감사 추적(Audit Trail)** 위해 원본 순서 변경 금지
- ✅ **정렬은 "보는 방식"일 뿐** - 저장 시 입력 순서 유지

### 참조

- [GnuCash GitHub Repository](https://github.com/Gnucash/gnucash)
- [GnuCash Sources Documentation](https://wiki.gnucash.org/wiki/index.php?title=GnuCash_Sources)

---

## 4. Windows Explorer

### 동작 방식

**기본 정렬**:
1. 컬럼 헤더 클릭 → **오름차순** 정렬
2. 같은 컬럼 다시 클릭 → **내림차순** 정렬
3. 다른 컬럼 클릭 → 새 컬럼 기준 오름차순

**다단계 정렬**:
- **Shift + 클릭**: 2차 정렬 기준 추가
- 예: "수정한 날짜" 클릭 → Shift+"이름" 클릭
  - 결과: 날짜 → 이름 순으로 정렬

### 정렬 상태 표시

- **화살표 아이콘**: ▲ (오름차순) / ▼ (내림차순)
- **컬럼 강조**: 정렬된 컬럼은 하이라이트

### 정렬 상태 유지

- ✅ **폴더별 정렬 상태 저장**: 폴더마다 다른 정렬 기준 유지
- ✅ **세션 간 유지**: 탐색기 닫았다 열어도 정렬 유지
- ✅ **뷰 모드별 저장**: Details, List, Tiles 등 뷰마다 별도 저장

### 특징

- ✅ **View 레벨 정렬**: 파일 시스템(Model)은 변경 안됨
- ✅ **사용자 경험 최적화**: 직관적인 UI (화살표, 하이라이트)
- ✅ **영구 저장**: 사용자 설정으로 저장
- ✅ **다단계 정렬**: Power User 기능 제공

### 참조

- [Windows File Explorer Sorting Guide](https://www.digitalcitizen.life/transform-windows-explorer-filtering-options/)
- [File Explorer Column Sorting Forum](https://www.elevenforum.com/t/file-explorer-sorting-question.20208/)

---

## 5. Microsoft Excel

### 두 가지 정렬 모드

Excel은 용도에 따라 **두 가지 정렬 방식**을 제공합니다.

#### A. 데이터 정렬 (Sort)

```
홈 → 정렬 및 필터 → 사용자 지정 정렬
```

- ✅ **실제 셀 순서 변경** (Model 변경)
- ✅ **영구적**: 저장 시 정렬된 상태로 저장
- ✅ **Undo 가능**: 실행 취소로 원래 순서 복원

**사용 시나리오**:
- 데이터베이스 가져오기 후 정리
- 보고서 출력 전 정렬
- 데이터 구조 자체를 재구성

#### B. 필터 보기 (Filter View)

```
데이터 → 필터 → 컬럼 드롭다운 → 정렬
```

- ✅ **표시만 변경** (View 정렬)
- ✅ **원본 보존**: 필터 해제 시 원래 순서로
- ✅ **임시적**: 저장해도 원본 순서 유지

**사용 시나리오**:
- 데이터 분석 중 임시 정렬
- 특정 기준으로 데이터 탐색
- 원본 순서 보존 필요 시

### 특징

- ✅ **유연성**: 용도에 따라 Model 정렬 vs View 정렬 선택
- ✅ **명확한 구분**: UI에서 두 기능이 분리되어 있음
- ✅ **Excel 표**: 표 기능 사용 시 자동 정렬 UI 제공

### MVC 관점

| 기능 | Model 변경 | View 변경 | 영구성 |
|------|-----------|----------|-------|
| **데이터 정렬** | ✅ 변경 | ✅ 변경 | 영구 |
| **필터 보기** | ❌ 불변 | ✅ 변경 | 임시 |

### 참조

- [Excel Sort Data Documentation](https://support.microsoft.com/en-us/office/sort-data-in-a-range-or-table-in-excel-62d0b95d-2a90-4610-a6ae-2e545c4a4654)
- [Excel Multi-Level Sorting Guide](https://trumpexcel.com/multiple-level-sorting-excel/)

---

## 업계 표준 패턴 정리

### 📊 데스크톱 앱 정렬 구현 위치

| 프로그램 | 정렬 위치 | 원본 데이터 변경 | 아키텍처 패턴 | 비고 |
|---------|----------|----------------|-------------|------|
| **Qt (QTableView)** | Proxy Model | ❌ 불변 | Proxy Pattern | 중간 레이어 |
| **wxWidgets** | View | ❌ 불변 | View 정렬 | SortItems() |
| **GnuCash** | View | ❌ 불변 | View 정렬 | GTK+ TreeView |
| **Windows Explorer** | View | ❌ 불변 | View 정렬 | 폴더별 저장 |
| **Excel (필터)** | View | ❌ 불변 | View 정렬 | 임시 정렬 |
| **Excel (정렬)** | Model | ✅ 변경 | Model 정렬 | 영구 정렬 |

### 🎯 재무 앱의 공통 패턴

**GnuCash, Excel 필터, Windows Explorer 등 대부분의 데스크톱 앱**:

1. ✅ **View 레이어에서 정렬** (표시 순서만 변경)
2. ✅ **원본 데이터 불변** (입력 순서/ID 순서 유지)
3. ✅ **세션 내 정렬 상태 유지** (메모리)
4. ✅ **필터 + 정렬 조합 지원**
5. ✅ **정렬 아이콘 표시** (▲/▼)

### 왜 View 레이어 정렬인가?

#### 재무 앱의 특수성

1. **감사 추적(Audit Trail) 필수**
   - 거래 데이터의 입력 순서는 법적 의미가 있음
   - Transaction ID 순서는 보존되어야 함

2. **정렬은 "보는 방식"**
   - 정렬은 사용자의 분석 도구일 뿐
   - 비즈니스 데이터 자체가 아님

3. **정렬 해제 시 원본 복원**
   - 사용자가 정렬을 해제하면 입력 순서로 돌아가야 함
   - Model이 정렬되면 원본 순서를 잃음

#### 기술적 이유

1. **MVC 경계 명확**
   - Model: 데이터만 관리
   - View: 표시 방식 관리 (정렬 포함)

2. **유연성**
   - 정렬 기준 추가/변경이 쉬움
   - Model 코드 변경 불필요

3. **성능**
   - 정렬은 표시할 데이터에만 수행
   - 필터 후 정렬 (대상 최소화)

---

## 세션 상태 관리

### 정렬 상태 저장 방식

| 방식 | 저장 위치 | 유지 범위 | 복원 시점 | 예시 |
|------|----------|----------|-----------|------|
| **메모리** | 멤버 변수 | 세션 내 | 앱 재시작 시 초기화 | 대부분의 앱 |
| **설정 파일** | .ini/.config | 영구 | 앱 시작 시 | Windows Explorer |
| **DB/파일** | 앱 데이터 | 영구 | 데이터 로드 시 | Visual Studio 레이아웃 |

### 일반적 구현 패턴

#### 세션 내 유지 (권장)

```cpp
// View 클래스에 정렬 상태 저장
class wxMLMainFrame : public wxFrame {
private:
    int currentSortColumn = -1;      // -1 = 정렬 안함
    bool currentSortAscending = true;
};

void wxMLMainFrame::OnColumnClick(wxListEvent& event) {
    int column = event.GetColumn();

    // 토글 로직
    if (currentSortColumn == column) {
        currentSortAscending = !currentSortAscending;
    } else {
        currentSortColumn = column;
        currentSortAscending = true;
    }

    // 정렬 수행
    ApplySorting();
}
```

#### 영구 저장 (선택)

```cpp
// 앱 종료 시 저장
void wxMLMainFrame::SaveSortState() {
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("/sort/column", currentSortColumn);
    config->Write("/sort/ascending", currentSortAscending);
}

// 앱 시작 시 복원
void wxMLMainFrame::LoadSortState() {
    wxConfigBase* config = wxConfigBase::Get();
    currentSortColumn = config->Read("/sort/column", -1);
    currentSortAscending = config->Read("/sort/ascending", true);

    if (currentSortColumn != -1) {
        ApplySorting();
    }
}
```

### 저장 범위 선택 기준

| 저장 여부 | 장점 | 단점 | 적합한 경우 |
|----------|------|------|-----------|
| **저장 안함** | 간단 | 매번 기본 정렬 | 대부분의 앱 |
| **저장함** | 사용자 편의 | 복잡도 증가 | 파워 유저용 앱 |

**재무 앱의 일반적 선택**: **저장 안함** (세션 내만 유지)

**이유**:
- 데이터 파일마다 다른 정렬 기준이 필요할 수 있음
- 기본 정렬(날짜 내림차순)이 대부분의 경우 유용
- 과도한 설정 저장은 사용자 혼란 초래

### 참조

- [Desktop Session Management](https://docs.oracle.com/cd/E19253-01/819-0917/gosgetstarted-74/index.html)
- [Frontend State Persistence](https://engineering.pipefy.com/2023/03/24/7-ways-to-persist-state-in-frontend/)
- [ASP.NET Session Management](https://learn.microsoft.com/en-us/aspnet/core/fundamentals/app-state?view=aspnetcore-10.0)

---

## 결론 및 권장사항

### ✅ 업계 표준 (데스크톱 재무 앱)

**방식**: **View 레이어에서 정렬** (wxWidgets `SortItems()` 사용)

### 권장 이유

#### 1. 업계 표준 준수
- ✅ **Qt 제외 대부분의 데스크톱 앱이 이 방식 사용**
- ✅ **wxWidgets 프레임워크의 권장 방식**
- ✅ **GnuCash, Windows Explorer 등 성숙한 앱들의 선택**

#### 2. 재무 앱의 요구사항 충족
- ✅ **입력 순서 보존 필수** (감사 추적)
- ✅ **정렬은 분석 도구** (비즈니스 데이터 아님)
- ✅ **정렬 해제 시 원본 복원 가능**

#### 3. MVC 아키텍처 준수
- ✅ **Model: 데이터만 관리** (정렬 로직 없음)
- ✅ **View: 표시 방식 관리** (정렬 포함)
- ✅ **경계 명확**: 역할 분리 명확

#### 4. 구현 용이성
- ✅ **기존 코드 변경 최소**
- ✅ **wxWidgets API 활용** (SortItems, ShowSortIndicator)
- ✅ **유지보수 용이**: 정렬 로직이 View에 집중

### 🚀 MissionLedger 구현 권장사항

#### 기본 설계

```cpp
class wxMLMainFrame : public wxFrame {
private:
    // 정렬 상태
    int currentSortColumn = -1;      // -1 = 정렬 안함 (ID 순)
    bool currentSortAscending = true;

    // 이벤트 핸들러
    void OnColumnHeaderClick(wxListEvent& event);

    // 헬퍼 메서드
    void ApplySorting();
    static int wxCALLBACK CompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);
};
```

#### 구현 단계

1. **이벤트 바인딩**
   ```cpp
   listCtrl->Bind(wxEVT_LIST_COL_CLICK, &wxMLMainFrame::OnColumnHeaderClick, this);
   ```

2. **컬럼 클릭 핸들러**
   - 정렬 방향 토글 (오름차순 ↔ 내림차순)
   - 세 번째 클릭 시 정렬 해제 (ID 순으로 복원)

3. **비교 함수 구현**
   - 컬럼별 비교 로직 (한글, 숫자, 날짜)
   - SetItemData()로 저장한 Transaction ID 사용

4. **정렬 아이콘 표시**
   - `ShowSortIndicator(column, ascending)`

5. **필터와 통합**
   - 필터된 결과 내에서 정렬
   - `applyCurrentFilter()` 수정

#### 기본 동작

- **기본 정렬**: 날짜 내림차순 (최신순)
- **토글**: 오름차순 → 내림차순 → 정렬 해제
- **상태 유지**: 세션 내만 (영구 저장 안함)
- **모든 컬럼 정렬 가능**

### 🔍 Qt의 Proxy Model 패턴은?

#### 장점
- ✅ **이상적인 설계**: Model과 View 완전 분리
- ✅ **자동화**: 정렬/필터링 로직 프레임워크가 제공
- ✅ **재사용성**: 여러 View에 같은 Model 사용 가능

#### 단점
- ❌ **wxWidgets에는 없음**: 직접 구현 필요
- ❌ **높은 복잡도**: Proxy 레이어 구현 + 인덱스 매핑
- ❌ **과도한 설계**: 단일 View 앱에는 불필요

#### 결론
- 💡 **현재는 View 정렬로 구현**
- 💡 **추후 대규모 리팩토링 시 고려 가능** (다중 View 추가 시)

---

## 부록: 컬럼별 정렬 구현 예시

### MissionLedger 리스트 컬럼

```
0. 유형 (수입/지출)
1. 카테고리
2. 항목
3. 금액
4. 영수증번호
5. 날짜
```

### 비교 함수 구현 예시

```cpp
int wxCALLBACK CompareTransactions(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
    auto* frame = reinterpret_cast<wxMLMainFrame*>(sortData);
    int column = frame->currentSortColumn;
    bool ascending = frame->currentSortAscending;

    // Transaction ID로 데이터 조회
    auto controller = FMLMVCHolder::GetInstance().GetController();
    auto data1 = controller->GetTransactionData(item1);
    auto data2 = controller->GetTransactionData(item2);

    int result = 0;

    switch(column) {
        case 0: // 유형
            result = static_cast<int>(data1.Type) - static_cast<int>(data2.Type);
            break;

        case 1: // 카테고리 (한글)
            result = data1.Category.compare(data2.Category);
            break;

        case 2: // 항목 (한글)
            result = data1.Item.compare(data2.Item);
            break;

        case 3: // 금액 (숫자)
            result = (data1.Amount < data2.Amount) ? -1 : (data1.Amount > data2.Amount) ? 1 : 0;
            break;

        case 4: // 영수증번호
            result = data1.ReceiptNumber.compare(data2.ReceiptNumber);
            break;

        case 5: // 날짜
            result = data1.DateTime.compare(data2.DateTime);
            break;
    }

    return ascending ? result : -result;
}
```

---

## 참고 자료

### 공식 문서
- [Qt Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html)
- [wxWidgets wxListCtrl Documentation](https://docs.wxwidgets.org/latest/classwx_list_ctrl.html)
- [Microsoft Excel Sort Documentation](https://support.microsoft.com/en-us/office/sort-data-in-a-range-or-table-in-excel-62d0b95d-2a90-4610-a6ae-2e545c4a4654)

### 오픈소스 프로젝트
- [GnuCash GitHub Repository](https://github.com/Gnucash/gnucash)

### 커뮤니티 토론
- [wxWidgets Forum: Sorting in wxListCtrl](https://forums.wxwidgets.org/viewtopic.php?t=2174)
- [Qt Forum: QSortFilterProxyModel Sorting](https://forum.qt.io/topic/99812/qtableview-qabstracttablemodel-qsortfilterproxymodel-sorting)

### 아키텍처 가이드
- [MVVM Application Architecture](https://www.ramotion.com/blog/what-is-mvvm/)
- [MVC Architecture Best Practices](https://elvinbaghele.medium.com/deep-dive-into-model-view-controller-mvc-best-practices-and-case-studies-c758e13ec4cf)

---

**문서 버전**: 1.0
**최종 수정**: 2026-01-30
**작성자**: Claude Sonnet 4.5
