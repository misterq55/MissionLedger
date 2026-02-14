# MissionLedger - 배포 폰트

이 디렉토리에는 PDF 내보내기 기능에 사용되는 폰트 파일이 포함됩니다.

## 폰트 다운로드

**Noto Sans KR** (한글 지원)
- 라이선스: SIL Open Font License 1.1 (재배포 자유)
- 다운로드: https://fonts.google.com/noto/specimen/Noto+Sans+KR

### 다운로드 방법

1. 위 Google Fonts 링크 접속
2. "Download family" 버튼 클릭
3. 압축 해제 후 다음 파일을 이 디렉토리에 복사:
   - `NotoSansKR-Regular.ttf` (일반 폰트)
   - `NotoSansKR-Bold.ttf` (굵은 폰트)

### 파일 구조

```
MissionLedger/fonts/
├── README.md (이 파일)
├── NotoSansKR-Regular.ttf (다운로드 필요)
└── NotoSansKR-Bold.ttf (다운로드 필요)
```

## 폰트 사용 우선순위

MissionLedger는 다음 순서로 폰트를 검색합니다:

1. **배포 폰트** (실행 파일 근처 `fonts/` 디렉토리)
   - Windows/macOS/Linux 모두 동일하게 동작
   - 설치 프로그램에 포함 가능

2. **시스템 폰트** (플랫폼별)
   - Windows: `%WINDIR%\Fonts\malgun.ttf` (Malgun Gothic)
   - macOS: `/Library/Fonts/AppleSDGothicNeo.ttc`
   - Linux: `/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc`

3. **Fallback 폰트**
   - Arial (영문 전용, 한글 깨질 수 있음)

## 라이선스

**Noto Sans KR** - SIL Open Font License 1.1

Copyright 2012 Google Inc. All Rights Reserved.

This Font Software is licensed under the SIL Open Font License, Version 1.1.

This license is copied below, and is also available with a FAQ at:
http://scripts.sil.org/OFL

### SIL Open Font License (요약)

- ✅ 상업적 사용 가능
- ✅ 재배포 가능 (수정 여부 무관)
- ✅ 폰트 임베딩 가능 (PDF, 애플리케이션 등)
- ❌ 폰트 자체를 단독 판매 불가

전체 라이선스 내용: https://scripts.sil.org/OFL

## 참고

- Malgun Gothic은 Microsoft 소유로 재배포 불가 (시스템 폰트로만 사용)
- 배포 폰트 없이도 실행 가능 (시스템 폰트 fallback)
- Windows 설치 프로그램은 Noto Sans KR을 자동으로 포함
