#pragma once

// MissionLedger 버전 정보
// 설치 프로그램 및 리소스 스크립트에서 사용

#define ML_VERSION_MAJOR    1
#define ML_VERSION_MINOR    0
#define ML_VERSION_PATCH    0
#define ML_VERSION_BUILD    0

// 문자열 매크로 헬퍼
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// 버전 문자열 (예: "1.0.0")
#define ML_VERSION_STRING \
    TOSTRING(ML_VERSION_MAJOR) "." \
    TOSTRING(ML_VERSION_MINOR) "." \
    TOSTRING(ML_VERSION_PATCH)

// 전체 버전 문자열 (빌드 번호 포함, 예: "1.0.0.0")
#define ML_VERSION_FULL_STRING \
    TOSTRING(ML_VERSION_MAJOR) "." \
    TOSTRING(ML_VERSION_MINOR) "." \
    TOSTRING(ML_VERSION_PATCH) "." \
    TOSTRING(ML_VERSION_BUILD)

// 애플리케이션 정보
#define ML_APP_NAME         "MissionLedger"
#define ML_APP_NAME_KR      "미션레저"
#define ML_APP_DESCRIPTION  "Mission and Project Budget Management"
#define ML_APP_DESCRIPTION_KR "선교 및 프로젝트 예산 관리 프로그램"

// 회사/조직 정보
#define ML_COMPANY_NAME     "MissionLedger Project"
#define ML_COPYRIGHT        "Copyright (C) 2025 MissionLedger Project"

// 파일 정보
#define ML_FILE_DESCRIPTION ML_APP_DESCRIPTION_KR
#define ML_INTERNAL_NAME    "MissionLedger"
#define ML_ORIGINAL_FILENAME "MissionLedger.exe"
#define ML_PRODUCT_NAME     ML_APP_NAME_KR
