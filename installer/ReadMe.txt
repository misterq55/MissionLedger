미션레저 (MissionLedger)
선교·프로젝트의 예산과 거래 내역을 관리하고, 결산 보고서와 거래 내역서를 PDF로 만드는 프로그램입니다.
버전은 Windows는 설정 > 앱 > 설치된 앱의 미션레저 항목, macOS는 앱의 "정보 가져오기"에서 확인할 수 있습니다.


1. 필요 환경
   - Windows 10 (버전 1809) 이상, 64비트
   - macOS 11 (Big Sur) 이상, Apple Silicon 또는 Intel


2. 설치
   Windows
   - 설치 파일(MissionLedger-<버전>-Setup.exe)을 실행합니다. 관리자 권한을 묻습니다.
   - "명령줄에서 mledger 사용 (PATH에 추가)"는 명령줄 도구를 쓸 때만 필요합니다 (기본 켜짐, 꺼도 프로그램 사용에는 지장 없음).
   - 서명하지 않은 프로그램이라 "Windows의 PC 보호" 창이 나오면 "추가 정보" > "실행"을 누릅니다.

   macOS
   - .dmg 파일을 열고 MissionLedger를 Applications(응용 프로그램) 폴더로 끌어 놓습니다.
   - 처음 실행할 때 "확인되지 않은 개발자" 경고가 나오면:
     시스템 설정 > 개인정보 보호 및 보안 > 아래쪽의 "확인 없이 열기"를 누르고 다시 실행합니다. (한 번만 하면 됩니다)

   새 버전은 같은 방법으로 설치하면 이전 버전을 대신합니다. 데이터 파일(.ml)은 그대로입니다.


3. 제거
   - Windows: 설정 > 앱 > 설치된 앱 > 미션레저 > 제거
   - macOS: Applications 폴더의 MissionLedger를 휴지통으로 옮깁니다.
     명령줄 도구용 링크를 만들었다면 터미널에서 지웁니다: sudo rm /usr/local/bin/mledger
   만든 데이터 파일(.ml)은 지워지지 않습니다.


4. 데이터와 백업
   - .ml 파일 하나가 장부 하나입니다. 어디에 두어도 되고, 파일을 더블클릭하면 프로그램으로 열립니다.
   - 백업은 .ml 파일을 복사해 두면 됩니다 (USB, 클라우드 드라이브 등).
   - 명령줄 도구로 파일을 고치면 고치기 직전 상태가 같은 폴더에 <파일이름>.ml.bak으로 남습니다.


5. 명령줄 도구 (mledger)
   터미널이나 AI 도구(예: Claude Code)에서 장부를 다루는 명령줄 도구가 함께 설치됩니다.
   사용법은 같은 폴더의 CLI.md를 보세요. (macOS: .dmg 안의 CLI.md, 또는 앱 안 Contents/Resources/CLI.md)


6. 문의
   misterq55@gmail.com


7. 라이선스
   - 이 프로그램: LICENSE.txt (MIT License)
   - 포함된 라이브러리와 글꼴: THIRD_PARTY_NOTICES.txt
