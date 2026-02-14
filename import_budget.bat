@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

echo ========================================
echo Budget Data Import
echo ========================================

REM 수입 항목
echo [1/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type income --category "수입" --item "교회 예산" --amount 17500000
if errorlevel 1 (
    echo Error importing budget 1
    pause
    exit /b 1
)

echo [2/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type income --category "수입" --item "회비" --amount 18000000
if errorlevel 1 (
    echo Error importing budget 2
    pause
    exit /b 1
)

echo [3/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type income --category "수입" --item "후원" --amount 4000000
if errorlevel 1 (
    echo Error importing budget 3
    pause
    exit /b 1
)

REM 지출 항목 - 항공
echo [4/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "항공" --item "항공료 선결제" --amount 3000000
if errorlevel 1 (
    echo Error importing budget 4
    pause
    exit /b 1
)

echo [5/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "항공" --item "항공료 후결제" --amount 20000000
if errorlevel 1 (
    echo Error importing budget 5
    pause
    exit /b 1
)

echo [6/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "항공" --item "여행자 보험" --amount 300000
if errorlevel 1 (
    echo Error importing budget 6
    pause
    exit /b 1
)

REM 지출 항목 - 생활
echo [7/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "생활" --item "숙박비" --amount 4000000
if errorlevel 1 (
    echo Error importing budget 7
    pause
    exit /b 1
)

echo [8/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "생활" --item "식비" --amount 2250000
if errorlevel 1 (
    echo Error importing budget 8
    pause
    exit /b 1
)

echo [9/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "생활" --item "차 렌탈" --amount 1250000
if errorlevel 1 (
    echo Error importing budget 9
    pause
    exit /b 1
)

echo [10/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "생활" --item "출/입국일 식비" --amount 550000
if errorlevel 1 (
    echo Error importing budget 10
    pause
    exit /b 1
)

echo [11/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "생활" --item "간식비" --amount 500000
if errorlevel 1 (
    echo Error importing budget 11
    pause
    exit /b 1
)

REM 지출 항목 - 사역 준비
echo [12/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "사역 준비" --item "사역팀 모임비" --amount 250000
if errorlevel 1 (
    echo Error importing budget 12
    pause
    exit /b 1
)

echo [13/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "사역 준비" --item "전체 모임비" --amount 1100000
if errorlevel 1 (
    echo Error importing budget 13
    pause
    exit /b 1
)

echo [14/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "사역 준비" --item "사역팀 로밍" --amount 100000
if errorlevel 1 (
    echo Error importing budget 14
    pause
    exit /b 1
)

echo [15/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "사역 준비" --item "사역지 로컬 대여" --amount 500000
if errorlevel 1 (
    echo Error importing budget 15
    pause
    exit /b 1
)

echo [16/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "사역 준비" --item "인쇄물" --amount 700000
if errorlevel 1 (
    echo Error importing budget 16
    pause
    exit /b 1
)

echo [17/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "사역 준비" --item "단체티" --amount 500000
if errorlevel 1 (
    echo Error importing budget 17
    pause
    exit /b 1
)

REM 지출 항목 - 팀사역
echo [18/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "팀사역" --item "사역팀" --amount 65200
if errorlevel 1 (
    echo Error importing budget 18
    pause
    exit /b 1
)

echo [19/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "팀사역" --item "어린이 팀" --amount 1500000
if errorlevel 1 (
    echo Error importing budget 19
    pause
    exit /b 1
)

echo [20/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "팀사역" --item "의료" --amount 1500000
if errorlevel 1 (
    echo Error importing budget 20
    pause
    exit /b 1
)

echo [21/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "팀사역" --item "공연팀" --amount 100000
if errorlevel 1 (
    echo Error importing budget 21
    pause
    exit /b 1
)

echo [22/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "팀사역" --item "찬양" --amount 167980
if errorlevel 1 (
    echo Error importing budget 22
    pause
    exit /b 1
)

echo [23/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "팀사역" --item "교회보수" --amount 152400
if errorlevel 1 (
    echo Error importing budget 23
    pause
    exit /b 1
)

echo [24/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "팀사역" --item "구제" --amount 400000
if errorlevel 1 (
    echo Error importing budget 24
    pause
    exit /b 1
)

echo [25/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "팀사역" --item "교회보수 현지" --amount 1000000
if errorlevel 1 (
    echo Error importing budget 25
    pause
    exit /b 1
)

REM 지출 항목 - 기타
echo [26/26] Importing budget...
x64\Release\MissionLedgerCLI.exe budget add --file data.ml --type expense --category "기타" --item "예비비" --amount 3000000
if errorlevel 1 (
    echo Error importing budget 26
    pause
    exit /b 1
)

echo.
echo Import completed: 26 budget items
echo   - 수입: 3 items (39,500,000원)
echo   - 지출: 23 items (39,500,000원)
pause
