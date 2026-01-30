# FMLModel 유닛 테스트 계획

## 개요

FMLModel 클래스의 안정성과 정확성을 보장하기 위한 유닛 테스트 계획입니다. 특히 재무 계산 로직의 정확성과 향후 리팩토링을 위한 안전망 구축을 목표로 합니다.

### 테스트 목적

1. **버그 방지**: 비즈니스 로직 정확성 검증 (특히 재무 계산)
2. **회귀 방지**: 코드 변경 시 기존 기능 보호
3. **복잡도 모니터링**: Model 클래스 비대화 감지 (God Object 경고)
4. **리팩토링 안전망**: 미래에 클래스 분리 시 안전 보장

### 테스트 프레임워크

- **프레임워크**: Google Test
- **플랫폼**: Visual Studio 2022, Windows
- **언어**: C++17[ARCHITECTURE.md](ARCHITECTURE.md)
- **목표 커버리지**: 전체 85%, 비즈니스 로직 100%

---

## 테스트 범위

### 테스트 대상 파일

- **헤더**: `MissionLedger/src/module/mvc/model/MLModel.h`
- **구현**: `MissionLedger/src/module/mvc/model/MLModel.cpp` (463줄)
- **책임**: 7가지 (CRUD, 조회, 비즈니스 로직, 파일 작업, Observer, Storage 연동, DTO 변환)

### 테스트 카테고리

#### 1. CRUD 작업 (~20개 테스트)

**대상 메서드**:
- `AddTransaction()` - 거래 추가
- `UpdateTransaction()` - 거래 수정
- `RemoveTransaction()` - 거래 삭제

**테스트 시나리오**:
- 정상 추가/수정/삭제
- 존재하지 않는 ID 처리
- Observer 알림 검증
- UnsavedChanges 상태 변경 확인

#### 2. 데이터 조회 (~15개 테스트)

**대상 메서드**:
- `GetTransactionData()` - 단일 거래 조회 (DTO)
- `GetAllTransactionData()` - 전체 거래 조회 (DTO)
- `GetFilteredTransactionData()` - 필터링된 거래 조회
- `GetTransaction()` - Entity 기반 조회

**테스트 시나리오**:
- 빈 모델 조회
- 존재하지 않는 ID 조회
- 날짜 범위 필터
- 거래 유형 필터 (Income/Expense)
- 카테고리 필터
- 금액 범위 필터
- 텍스트 검색 (OR 조건)
- 복합 필터 조합

#### 3. 비즈니스 로직 (~20개 테스트) ⭐ 최우선

**대상 메서드**:
- `CalculateTransactionSummary()` - 전체 요약 계산
- `CalculateFilteredTransactionSummary()` - 필터링된 요약 계산
- `GetCategoryTotal()` - 카테고리별 합계
- `GetAllTotal()` - 전체 잔액

**테스트 시나리오**:
- 빈 모델 계산 (0/0/0)
- Income만 있는 경우
- Expense만 있는 경우
- Income/Expense 혼합
- 큰 금액 처리 (INT64_MAX)
- 음수 잔액 처리
- 카테고리별 집계
- 필터 적용 시 계산

#### 4. 파일 작업 (~15개 테스트)

**대상 메서드**:
- `OpenFile()` - 파일 열기
- `SaveFile()` - 파일 저장
- `SaveFileAs()` - 다른 이름으로 저장
- `NewFile()` - 새 파일 생성
- `GetCurrentFilePath()` - 현재 파일 경로
- `HasUnsavedChanges()` - 저장 여부 확인

**테스트 시나리오**:
- 정상 열기/저장
- Storage 초기화 실패
- 저장 실패 (Storage 오류)
- 로드 실패 (Storage 오류)
- TransactionIdIndex 갱신
- 빈 파일 처리
- CurrentFilePath 변경
- UnsavedChanges 플래그 관리

#### 5. Observer 패턴 (~10개 테스트)

**대상 이벤트**:
- `OnTransactionAdded` - 거래 추가 알림
- `OnTransactionUpdated` - 거래 수정 알림
- `OnTransactionRemoved` - 거래 삭제 알림
- `OnTransactionsCleared` - 전체 삭제 알림
- `OnDataLoaded` - 데이터 로드 알림
- `OnDataSaved` - 데이터 저장 알림

**테스트 시나리오**:
- 각 CRUD 작업 후 Observer 호출 확인
- 전달된 DTO 정확성 검증
- Observer 미등록 시 크래시 방지
- 실패한 작업은 알림 없음 확인

#### 6. 엣지 케이스 (~10개 테스트)

**시나리오**:
- 경계값: INT64_MAX, INT64_MIN
- 빈 문자열, 매우 긴 문자열
- 존재하지 않는 ID 반복 처리
- 날짜 필터 경계값 (StartDate = EndDate, StartDate > EndDate)
- 모든 필터 활성화 / 모든 필터 비활성화
- 동일 데이터 중복 추가

---

## Mock 설계

### MockStorageProvider

**역할**: IMLStorageProvider 인터페이스 구현, 메모리 기반 저장소

**주요 기능**:
- 메모리에 거래 저장 (실제 파일 I/O 없음)
- 호출 횟수 추적 (검증용)
- 실패 시뮬레이션 플래그
  - `SimulateInitializeFailure`
  - `SimulateSaveFailure`
  - `SimulateLoadFailure`

**위치**: `tests/model/mocks/MockStorageProvider.h`

### MockModelObserver

**역할**: IMLModelObserver 인터페이스 구현, 이벤트 추적

**주요 기능**:
- 모든 Observer 이벤트 수신
- 호출 횟수 추적
- 전달된 파라미터 저장 (검증용)

**위치**: `tests/model/mocks/MockModelObserver.h`

---

## 테스트 구조

```
MissionLedger/
├── tests/
│   ├── main.cpp                    # Google Test 메인
│   └── model/
│       ├── model_crud_test.cpp         # CRUD 테스트
│       ├── model_query_test.cpp        # 데이터 조회 테스트
│       ├── model_business_test.cpp     # 비즈니스 로직 테스트 ⭐
│       ├── model_filter_test.cpp       # 필터링 테스트
│       ├── model_file_test.cpp         # 파일 작업 테스트
│       ├── model_observer_test.cpp     # Observer 패턴 테스트
│       └── mocks/
│           ├── MockStorageProvider.h   # Mock Storage
│           └── MockModelObserver.h     # Mock Observer
```

---

## 구현 계획

### Phase 1: 핵심 비즈니스 로직 (2-3일) ⭐ 최우선

**목표**: 재무 계산 정확성 보장

1. **Mock 클래스 구현**
   - MockStorageProvider (~250줄)
   - MockModelObserver (~120줄)

2. **비즈니스 로직 테스트** (`model_business_test.cpp`)
   - CalculateTransactionSummary 전체 케이스
   - CalculateFilteredTransactionSummary
   - GetCategoryTotal, GetAllTotal
   - 엣지 케이스: 빈 모델, 큰 금액

3. **필터링 테스트** (`model_filter_test.cpp`)
   - GetFilteredTransactionData 모든 필터 타입
   - 복합 필터 조합
   - 날짜/금액 범위 경계값

**완료 기준**: 비즈니스 로직 100% 커버리지

**예상 토큰**: 21,000-28,000

---

### Phase 2: CRUD 및 Observer (2-3일)

**목표**: 기본 데이터 조작 안정성

4. **CRUD 테스트** (`model_crud_test.cpp`)
   - AddTransaction 전체 케이스
   - UpdateTransaction 성공/실패
   - RemoveTransaction 성공/실패

5. **데이터 조회 테스트** (`model_query_test.cpp`)
   - GetTransactionData, GetAllTransactionData
   - 빈 모델, 존재하지 않는 ID

6. **Observer 테스트** (`model_observer_test.cpp`)
   - 모든 CRUD 작업 후 알림 검증
   - Observer 미등록 시 안전성

**완료 기준**: CRUD 및 Observer 85%+ 커버리지

**예상 토큰**: 19,000-27,000

---

### Phase 3: 파일 작업 (2-3일)

**목표**: 파일 저장/로드 안정성

7. **파일 작업 테스트** (`model_file_test.cpp`)
   - OpenFile 성공/실패 케이스
   - SaveFile 성공/실패 케이스
   - SaveFileAs 경로 변경 검증
   - NewFile 데이터 초기화

8. **상태 관리 테스트**
   - GetCurrentFilePath
   - HasUnsavedChanges (모든 작업 후 상태)

**완료 기준**: 파일 작업 80%+ 커버리지

**예상 토큰**: 12,500-18,500

---

### Phase 4: 엣지 케이스 및 리팩토링 (1-2일)

**목표**: 예외 상황 처리

9. **엣지 케이스 테스트**
   - 경계값: INT64_MAX, 빈 문자열, 매우 긴 문자열
   - 존재하지 않는 ID 처리
   - 날짜 필터 경계값
   - 모든 필터 비활성화

10. **테스트 코드 리팩토링**
    - 중복 코드 제거
    - 헬퍼 함수 추가
    - 가독성 개선

**완료 기준**: 전체 85%+ 커버리지, 모든 엣지 케이스 PASS

**예상 토큰**: 10,000-15,000

---

## 예상 커버리지 목표

| 카테고리 | 목표 커버리지 | 이유 |
|---------|------------|------|
| 핵심 비즈니스 로직 | 100% | 재무 계산 정확성이 가장 중요 |
| CRUD 작업 | 95%+ | 기본 기능 보장 필수 |
| 필터링 로직 | 90%+ | 복잡한 조건 분기 |
| 파일 작업 | 80%+ | Storage 의존성으로 일부 경로 테스트 어려움 |
| Observer 패턴 | 90%+ | MVC 통신의 핵심 |
| **전체** | **85%+** | 실용적인 목표 |

---

## 기대 효과

### 1. 버그 조기 발견
- 재무 계산 오류 사전 차단
- 필터링 버그 경계값 처리 검증
- 메모리 누수 확인

### 2. 리팩토링 안전망
- **Model 클래스 분리**: 미래에 FMLModel을 Repository/Service/FileService로 분리 시 회귀 방지
- **인터페이스 변경**: IMLModel 수정 시 영향도 파악
- **성능 최적화**: 알고리즘 변경 후 동작 보장

### 3. 복잡도 모니터링 (핵심 가치 ⭐)
- **테스트 파일 크기**: Model 복잡도 지표
- **테스트 작성 난이도**: God Object 경고 신호
- **조기 경보 시스템**: 리팩토링 시점 판단 근거

### 4. 문서화 효과
- 테스트 코드가 FMLModel 사용법 문서 역할
- 테스트 케이스가 요구사항 명세 역할

### 5. 개발 속도 향상
- 디버깅 시간 단축
- 코드 변경 자신감 확보

---

## 총 소요 시간 및 토큰

### 시간
- **Phase 1**: 2-3일
- **Phase 2**: 2-3일
- **Phase 3**: 2-3일
- **Phase 4**: 1-2일
- **총 예상**: 7-11일

### 토큰
- **Phase 1**: 21,000-28,000
- **Phase 2**: 19,000-27,000
- **Phase 3**: 12,500-18,500
- **Phase 4**: 10,000-15,000
- **총 예상**: 62,500-88,500 토큰

---

## 테스트 작성 가이드라인

### AAA 패턴
```cpp
TEST_F(FMLModelBusinessTest, CalculateSummary_Mixed) {
    // Arrange (Given)
    model->AddTransaction(CreateTestIncome(100000, "급여"));
    model->AddTransaction(CreateTestExpense(30000, "식비"));

    // Act (When)
    FMLTransactionSummary summary = model->CalculateTransactionSummary();

    // Assert (Then)
    EXPECT_EQ(100000, summary.TotalIncome);
    EXPECT_EQ(30000, summary.TotalExpense);
    EXPECT_EQ(70000, summary.Balance);
    EXPECT_EQ(2, summary.TransactionCount);
}
```

### 테스트 명명 규칙
```
TEST(클래스명Test, 메서드명_시나리오_예상결과)
```

**예시**:
- `TEST(FMLModelTest, AddTransaction_Success)`
- `TEST(FMLModelTest, UpdateTransaction_NonExistingId_ReturnsFalse)`
- `TEST(FMLModelTest, FilterByDateRange_ValidRange_ReturnsMatchingTransactions)`

---

## 향후 확장 계획

### Controller 테스트
- FMLController 단위 테스트
- MockModel 사용한 격리 테스트

### 통합 테스트
- Model + Storage 실제 통합 (SQLite 파일 사용)
- Model + Controller + View E2E 테스트

### 성능 테스트
- 10,000개 거래 로드 시간 측정
- 필터링 성능 벤치마크
- 메모리 사용량 프로파일링

---

## 참고 자료

- **전체 상세 계획**: `C:\Users\Q\AppData\Local\Temp\claude\C--Work-Projects-Others-MissionLedger\tasks\a10e98e.output`
- **Google Test 문서**: https://google.github.io/googletest/
- **프로젝트 아키텍처**: `docs/ARCHITECTURE.md`
- **코딩 컨벤션**: `docs/CONVENTIONS.md`
