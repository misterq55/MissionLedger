#pragma once

// MissionLedger 버전 정보
// 리소스 스크립트(MissionLedger.rc)에서 사용
//
// 버전 숫자는 이 파일에 직접 적지 않습니다.
// 저장소 루트의 VERSION 파일을 Version.props가 읽어 전처리기 정의로 전달합니다.
// 버전을 바꾸려면 VERSION 파일만 수정하세요. (docs/VERSIONING.md 참고)

#if !defined(ML_VERSION_MAJOR) || !defined(ML_VERSION_MINOR) || !defined(ML_VERSION_PATCH)
    #if defined(__INTELLISENSE__)
        // 편집기(IntelliSense)용 자리표시자 - 실제 빌드에는 사용되지 않음
        #define ML_VERSION_MAJOR    0
        #define ML_VERSION_MINOR    0
        #define ML_VERSION_PATCH    0
    #else
        #error "ML_VERSION_* is not defined. Import Version.props in the project (reads the VERSION file)."
    #endif
#endif

// 빌드 번호: 기본 0, CI에서 /p:MLBuildNumber=N 으로 주입
#ifndef ML_VERSION_BUILD
    #define ML_VERSION_BUILD    0
#endif

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
