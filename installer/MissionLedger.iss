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

; 포장할 exe가 있는 폴더 (기본값: CMake windows 프리셋의 Release 출력).
; 다른 폴더를 쓰려면 ISCC /DExeDir="..." 로 넘깁니다.
#ifndef ExeDir
  #define ExeDir AddBackslash(SourcePath) + "..\build\windows\bin\Release"
#endif

; 안전장치: 포장할 exe의 버전이 VERSION과 다르면 중단합니다.
; (VERSION을 고치고 Release 빌드를 하지 않은 채 설치 파일을 만드는 실수 방지)
#define MyAppExePath AddBackslash(ExeDir) + MyAppExeName
#if !FileExists(MyAppExePath)
  #error MissionLedger.exe 가 없습니다. Release 빌드를 먼저 실행하세요. (ExeDir 확인)
#endif
; 명령줄 도구 mledger.exe (MissionLedgerCLI 프로젝트, docs/CLI_DESIGN.md §2)
#define MyCLIExeName "mledger.exe"
#define MyCLIExePath AddBackslash(ExeDir) + MyCLIExeName
#if !FileExists(MyCLIExePath)
  #error mledger.exe 가 없습니다. Release 빌드를 먼저 실행하세요. (ExeDir 확인)
#endif
#define ExeVersion GetStringFileInfo(MyAppExePath, "ProductVersion")
#if ExeVersion != MyAppVersion
  #error MissionLedger.exe 버전이 VERSION 파일과 다릅니다. Release 빌드를 다시 실행하세요.
#endif

; ===== Visual C++ 런타임 =====
; MissionLedger.exe는 동적 런타임(/MD)으로 빌드되어 MSVCP140.dll 등이 필요합니다.
; Visual Studio에 포함된 vc_redist.x64.exe를 설치 파일에 넣고, 런타임이 없거나 오래된 PC에서만 설치합니다.
; 다른 경로를 쓰려면 ISCC /DVCRedistPath="...\vc_redist.x64.exe" 로 넘깁니다.
#ifndef VCRedistPath
  #define VSRoot "C:\Program Files\Microsoft Visual Studio\2022\"
  #define VCRedistSub "\VC\Redist\MSVC\v143\vc_redist.x64.exe"
  #if FileExists(VSRoot + "Community" + VCRedistSub)
    #define VCRedistPath VSRoot + "Community" + VCRedistSub
  #elif FileExists(VSRoot + "Professional" + VCRedistSub)
    #define VCRedistPath VSRoot + "Professional" + VCRedistSub
  #elif FileExists(VSRoot + "Enterprise" + VCRedistSub)
    #define VCRedistPath VSRoot + "Enterprise" + VCRedistSub
  #elif FileExists("C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools" + VCRedistSub)
    #define VCRedistPath "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools" + VCRedistSub
  #else
    #error vc_redist.x64.exe를 찾을 수 없습니다. Visual Studio 2022의 C++ 워크로드를 설치하거나 /DVCRedistPath로 경로를 지정하세요.
  #endif
#endif
#if !FileExists(VCRedistPath)
  #error VCRedistPath에 지정한 vc_redist.x64.exe가 없습니다.
#endif
; 포함할 런타임의 버전 (예: 14.44.35211) - 설치된 런타임이 이보다 낮으면 설치
#define VCRedistMajor
#define VCRedistMinor
#define VCRedistBuild
#define VCRedistRev
#expr GetVersionComponents(VCRedistPath, VCRedistMajor, VCRedistMinor, VCRedistBuild, VCRedistRev)
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
; PATH 변경(작업 "addtopath") 후 새로 여는 터미널에 바로 반영되도록 알림
ChangesEnvironment=yes

; 압축 설정
Compression=lzma2/max
SolidCompression=yes

; Windows 버전 요구사항
MinVersion=10.0.17763
; Visual C++ 런타임 설치에 관리자 권한이 필요하므로 "현재 사용자만 설치"는 허용하지 않습니다.
PrivilegesRequired=admin

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
; 기본으로 켜짐 (VS Code, Git과 같은 방식). 시스템 PATH에 {app}을 추가하고, 제거하면 그 항목만 뺌
Name: "addtopath"; Description: "명령줄에서 mledger 사용 (PATH에 추가)(&P)"; GroupDescription: "명령줄 도구:"

[Files]
; 실행 파일 및 라이브러리
Source: "{#MyAppExePath}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyCLIExePath}"; DestDir: "{app}"; Flags: ignoreversion
; PDF 내보내기 폰트 (Noto Sans KR, OFL 라이선스 파일 포함). 실행 파일 옆 fonts\ 에서 찾음
; 폰트가 없으면 설치 프로그램 빌드가 실패하도록 skipifsourcedoesntexist는 쓰지 않음
Source: "..\third_party\fonts\NotoSansKR\*"; DestDir: "{app}\fonts"; Flags: ignoreversion
; 문서
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion isreadme
Source: "..\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
; Visual C++ 런타임 설치 파일 - 필요한 PC에서만 임시 폴더에 풀고 설치 후 삭제
Source: "{#VCRedistPath}"; DestDir: "{tmp}"; DestName: "vc_redist.x64.exe"; Flags: deleteafterinstall; Check: VCRedistNeedsInstall

[Icons]
; 시작 메뉴 바로가기
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
; 바탕화면 바로가기 (선택 사항)
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; .ml 파일 확장자 연결 (Windows 10/11 호환)
; HKA: 관리자 설치이므로 HKLM\Software\Classes에 기록 (사용자 설치였다면 HKCU)

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
; Visual C++ 런타임 설치 (필요한 경우에만, 화면 표시 없음)
; 종료 코드 3010(재부팅 필요)도 정상으로 처리됩니다.
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "Visual C++ 런타임을 설치하는 중..."; Flags: waituntilterminated; Check: VCRedistNeedsInstall
; 설치 완료 후 실행 옵션
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; 제거 시 삭제할 파일/폴더
Type: filesandordirs; Name: "{app}"

[Code]
// Visual C++ 2015-2022 런타임(x64)이 없거나, 포함한 버전보다 낮으면 True
function VCRedistNeedsInstall(): Boolean;
var
  Installed, Major, Minor, Bld: Cardinal;
  Key: String;
begin
  Key := 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64';
  Result := True;
  if RegQueryDWordValue(HKLM64, Key, 'Installed', Installed) and (Installed = 1) and
     RegQueryDWordValue(HKLM64, Key, 'Major', Major) and
     RegQueryDWordValue(HKLM64, Key, 'Minor', Minor) and
     RegQueryDWordValue(HKLM64, Key, 'Bld', Bld) then
  begin
    if Major <> {#VCRedistMajor} then
      Result := Major < {#VCRedistMajor}
    else if Minor <> {#VCRedistMinor} then
      Result := Minor < {#VCRedistMinor}
    else
      Result := Bld < {#VCRedistBuild};
  end;
end;

// ===== PATH (작업 "addtopath") =====
// 시스템 PATH (관리자 설치). 항목 비교는 대소문자 무시, 다른 항목은 건드리지 않음
const
  EnvironmentKey = 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment';

// ';' + Paths + ';' 안에서 ';' + Dir + ';'의 위치 (없으면 0)
function PathEntryPos(Paths, Dir: String): Integer;
begin
  Result := Pos(';' + Uppercase(Dir) + ';', ';' + Uppercase(Paths) + ';');
end;

procedure AddToPath(Dir: String);
var
  Paths: String;
begin
  if not RegQueryStringValue(HKLM, EnvironmentKey, 'Path', Paths) then
    Paths := '';
  if PathEntryPos(Paths, Dir) > 0 then
    exit;
  if (Paths <> '') and (Copy(Paths, Length(Paths), 1) <> ';') then
    Paths := Paths + ';';
  // REG_EXPAND_SZ 유지 (다른 항목의 %SystemRoot% 등)
  RegWriteExpandStringValue(HKLM, EnvironmentKey, 'Path', Paths + Dir);
end;

procedure RemoveFromPath(Dir: String);
var
  Paths, Wrapped: String;
  P: Integer;
begin
  if not RegQueryStringValue(HKLM, EnvironmentKey, 'Path', Paths) then
    exit;
  P := PathEntryPos(Paths, Dir);
  if P = 0 then
    exit;
  // ';A;Dir;B;' -> ';A;B;' (';Dir' 삭제), 앞뒤에 붙인 ';'를 떼면 원래 모양
  Wrapped := ';' + Paths + ';';
  Delete(Wrapped, P, Length(Dir) + 1);
  RegWriteExpandStringValue(HKLM, EnvironmentKey, 'Path', Copy(Wrapped, 2, Length(Wrapped) - 2));
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // 다시 설치하면서 체크를 끄면 이전에 넣은 항목을 뺌
    if WizardIsTaskSelected('addtopath') then
      AddToPath(ExpandConstant('{app}'))
    else
      RemoveFromPath(ExpandConstant('{app}'));
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
    RemoveFromPath(ExpandConstant('{app}'));
end;

// 설치 전 확인 사항
function InitializeSetup(): Boolean;
begin
  Result := True;
end;
