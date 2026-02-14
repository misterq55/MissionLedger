; Inno Setup Script for MissionLedger
; 선교 및 프로젝트 예산 관리 프로그램

#define MyAppName "미션레저"
#define MyAppEnglishName "MissionLedger"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "MissionLedger Project"
#define MyAppURL "https://github.com/misterq55/MissionLedger"
#define MyAppExeName "MissionLedger.exe"
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
; .ml 파일 확장자 연결 (Windows 10/11 방식)
; HKEY_CLASSES_ROOT\.ml -> MissionLedgerDocument.ml
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""
; 애플리케이션 등록 (Open With 메뉴용)
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: "{#MyAppAssocExt}"; ValueData: ""

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

// .ml 파일 연결 알림
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // 파일 연결 갱신을 위해 탐색기에 알림
    RegWriteStringValue(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ml', 'Progid', '{#MyAppAssocKey}');
  end;
end;
