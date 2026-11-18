; Inno Setup Script for MissionLedger
; 선교 및 프로젝트 예산 관리 프로그램

#define MyAppName "미션레저"
#define MyAppEnglishName "MissionLedger"
#define MyAppPublisher "MissionLedger Project"
#define MyAppURL "https://github.com/misterq55/MissionLedger"
#define MyAppExeName "MissionLedger.exe"

; ===== 버전 =====
; 저장소 루트의 VERSION 파일에서 읽습니다. (docs/VERSIONING.md 참고)
; CI 등에서 ISCC /DMyAppVersion=x.y.z 로 넘기면 그 값을 우선 사용합니다.
#ifndef MyAppVersion
  #define VersionFile AddBackslash(SourcePath) + "..\VERSION"
  #if !FileExists(VersionFile)
    #error VERSION 파일을 찾을 수 없습니다.
  #endif
  #define VersionHandle FileOpen(VersionFile)
  #define MyAppVersion Trim(FileRead(VersionHandle))
  #expr FileClose(VersionHandle)
#endif
#if MyAppVersion == ""
  #error VERSION 파일이 비어 있습니다.
#endif

; 안전장치: 포장할 exe의 버전이 VERSION과 다르면 중단합니다.
; (VERSION을 고치고 Release 빌드를 하지 않은 채 설치 파일을 만드는 실수 방지)
#define MyAppExePath AddBackslash(SourcePath) + "..\x64\Release\" + MyAppExeName
#if !FileExists(MyAppExePath)
  #error x64\Release\MissionLedger.exe 가 없습니다. Release 빌드를 먼저 실행하세요.
#endif
#define ExeVersion GetStringFileInfo(MyAppExePath, "ProductVersion")
#if ExeVersion != MyAppVersion
  #error MissionLedger.exe 버전이 VERSION 파일과 다릅니다. Release 빌드를 다시 실행하세요.
#endif
#define MyAppAssocName "MissionLedger Document"
#define MyAppAssocExt ".ml"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

[Setup]
; 앱 기본 정보
AppId={{E5F8C9A2-3B4D-4E6F-8A9C-1D2E3F4A5B6C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

; 설치 디렉토리
DefaultDirName={autopf}\{#MyAppEnglishName}
DefaultGroupName={#MyAppName}

; 출력 설정
OutputDir=output
OutputBaseFilename=MissionLedger-{#MyAppVersion}-Setup
SetupIconFile=..\MissionLedger\resources\app.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName={#MyAppName}

; 파일 연결 변경 시 설치/제거 후 탐색기에 알림 (아이콘 갱신)
ChangesAssociations=yes

; 압축 설정
Compression=lzma2/max
SolidCompression=yes

; Windows 버전 요구사항
MinVersion=10.0.17763
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog

; UI 설정
WizardStyle=modern
DisableWelcomePage=no
LicenseFile=..\LICENSE.txt

; 아키텍처 설정 (64비트 전용)
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "korean"; MessagesFile: "compiler:Languages\Korean.isl"

[Tasks]
Name: "desktopicon"; Description: "바탕화면 바로가기 만들기(&D)"; GroupDescription: "추가 아이콘:"; Flags: unchecked

[Files]
; 실행 파일 및 라이브러리
Source: "..\x64\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
; 배포 폰트 (PDF 내보내기용) - 폰트 파일이 있을 경우에만 포함
; Note: Noto Sans KR 폰트를 MissionLedger\fonts\ 디렉토리에 다운로드하면 자동 포함됩니다
; Source: "..\MissionLedger\fonts\*.ttf"; DestDir: "{app}\fonts"; Flags: ignoreversion skipifsourcedoesntexist
; 문서
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion isreadme
Source: "..\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
; 시작 메뉴 바로가기
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
; 바탕화면 바로가기 (선택 사항)
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; .ml 파일 확장자 연결 (Windows 10/11 호환)
; HKA: 관리자 설치 시 HKLM, 사용자 설치 시 HKCU의 Software\Classes에 기록

; 1. ProgID 등록
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

; 2. .ml 확장자 → ProgID 연결
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocKey}"; Flags: uninsdeletevalue

; 3. OpenWithProgids ("연결 프로그램" 목록에 표시)
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue

; 4. 애플리케이션 등록 (Open With 메뉴용)
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}"; ValueType: string; ValueName: "FriendlyAppName"; ValueData: "{#MyAppName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: "{#MyAppAssocExt}"; ValueData: ""
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Run]
; 설치 완료 후 실행 옵션
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; 제거 시 삭제할 파일/폴더
Type: filesandordirs; Name: "{app}"

[Code]
// 설치 전 확인 사항
function InitializeSetup(): Boolean;
begin
  Result := True;
end;
