@echo off
chcp 65001 > nul
echo Starting field.md import...
echo.

echo [1/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "숙박비" --desc "호텔 비용" --amount 2159589 --date "2025-01-06" --receipt "83" --currency PHP --original 86940.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 1
    pause
    exit /b 1
)

echo [2/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "숙박비" --desc "호텔 직원(담당자) 팁" --amount 37260 --date "2025-01-06" --desc "\"1박 호텔 비용 취소에 감사," --currency PHP --original 1500.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 2
    pause
    exit /b 1
)

echo [3/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "숙박비" --desc "호텔 직원 팁" --amount 1242 --date "2025-01-06" --currency PHP --original 50.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 3
    pause
    exit /b 1
)

echo [4/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "숙박비" --desc "각 방 팁" --amount 13662 --date "2025-01-06" --currency PHP --original 550.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 4
    pause
    exit /b 1
)

echo [5/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "숙박비" --desc "호텔 직원 팁" --amount 6210 --date "2025-01-07" --currency PHP --original 250.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 5
    pause
    exit /b 1
)

echo [6/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "숙박비" --desc "각 방 팁" --amount 11178 --date "2025-01-08" --currency PHP --original 450.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 6
    pause
    exit /b 1
)

echo [7/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "숙박비" --desc "호텔 직원(주방장) 팁" --amount 12420 --date "2025-01-08" --desc "\"사역지로 6시 30분에 출발. 6시에 조식준비를 부탁함." --currency PHP --original 500.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 7
    pause
    exit /b 1
)

echo [8/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "숙박비" --desc "각 방 팁" --amount 13662 --date "2025-01-09" --currency PHP --original 550.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 8
    pause
    exit /b 1
)

echo [9/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "호텔 직원 팁" --amount 2484 --date "2025-01-09" --currency PHP --original 100.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 9
    pause
    exit /b 1
)

echo [10/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "각 방 팁" --amount 12420 --date "2025-01-10" --currency PHP --original 500.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 10
    pause
    exit /b 1
)

echo [11/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "망고 한 광주리" --amount 81723 --date "2025-01-06" --receipt "84" --currency PHP --original 3290.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 11
    pause
    exit /b 1
)

echo [12/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "아침식사" --amount 165682 --date "2025-01-06" --receipt "85" --currency PHP --original 6670.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 12
    pause
    exit /b 1
)

echo [13/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "아침식사(물)" --amount 22852 --date "2025-01-06" --receipt "86" --currency PHP --original 920.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 13
    pause
    exit /b 1
)

echo [14/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심(만다린)" --amount 302853 --date "2025-01-06" --receipt "87" --currency PHP --original 12192.18 --rate 24.84
if errorlevel 1 (
    echo Error importing record 14
    pause
    exit /b 1
)

echo [15/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심 식당 팁" --amount 1242 --date "2025-01-06" --currency PHP --original 50.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 15
    pause
    exit /b 1
)

echo [16/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "단기선교팀 물" --amount 4582 --date "2025-01-06" --receipt "88" --currency PHP --original 184.5 --rate 24.84
if errorlevel 1 (
    echo Error importing record 16
    pause
    exit /b 1
)

echo [17/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심(운전기사2명)" --amount 9936 --date "2025-01-06" --currency PHP --original 400.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 17
    pause
    exit /b 1
)

echo [18/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "집회 후 간식(음료)" --amount 32292 --date "2025-01-06" --receipt "89" --currency PHP --original 1300.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 18
    pause
    exit /b 1
)

echo [19/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "집회 후 간식(빵)" --amount 18182 --date "2025-01-06" --receipt "90" --currency PHP --original 732.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 19
    pause
    exit /b 1
)

echo [20/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "저녁식사(페농)" --amount 171580 --date "2025-01-06" --receipt "91" --currency PHP --original 6907.42 --rate 24.84
if errorlevel 1 (
    echo Error importing record 20
    pause
    exit /b 1
)

echo [21/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "저녁식사(봉사자)" --amount 49680 --date "2025-01-06" --currency PHP --original 2000.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 21
    pause
    exit /b 1
)

echo [22/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심(수라)" --amount 310251 --date "2025-01-07" --receipt "92" --currency PHP --original 12490.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 22
    pause
    exit /b 1
)

echo [23/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심(봉사자)" --amount 20940 --date "2025-01-07" --receipt "93" --currency PHP --original 843.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 23
    pause
    exit /b 1
)

echo [24/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심(봉사자)" --amount 11525 --date "2025-01-07" --receipt "94" --currency PHP --original 464.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 24
    pause
    exit /b 1
)

echo [25/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심(운전기사2명)" --amount 7452 --date "2025-01-07" --currency PHP --original 300.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 25
    pause
    exit /b 1
)

echo [26/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "단기선교팀 음료" --amount 112276 --date "2025-01-07" --receipt "95" --currency PHP --original 4520.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 26
    pause
    exit /b 1
)

echo [27/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "저녁식사(버거킹)" --amount 163596 --date "2025-01-07" --receipt "96" --currency PHP --original 6586.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 27
    pause
    exit /b 1
)

echo [28/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "저녁식사(운전기사)" --amount 4968 --date "2025-01-07" --currency PHP --original 200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 28
    pause
    exit /b 1
)

echo [29/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "저녁식사(봉사자)" --amount 49680 --date "2025-01-07" --currency PHP --original 2000.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 29
    pause
    exit /b 1
)

echo [30/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "단기선교팀 물 25병" --amount 8942 --date "2025-01-07" --receipt "97" --currency PHP --original 360.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 30
    pause
    exit /b 1
)

echo [31/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심(수고비 포함)" --amount 248400 --date "2025-01-08" --currency PHP --original 10000.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 31
    pause
    exit /b 1
)

echo [32/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "음료수" --amount 11674 --date "2025-01-08" --receipt "98" --currency PHP --original 470.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 32
    pause
    exit /b 1
)

echo [33/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "저녁식사(푸농), 운전기사 포함" --amount 394340 --date "2025-01-08" --receipt "99" --currency PHP --original 15875.22 --rate 24.84
if errorlevel 1 (
    echo Error importing record 33
    pause
    exit /b 1
)

echo [34/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심(아웃 오프노웨어)" --amount 239576 --date "2025-01-09" --receipt "100" --currency PHP --original 9644.77 --rate 24.84
if errorlevel 1 (
    echo Error importing record 34
    pause
    exit /b 1
)

echo [35/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심 팁" --amount 2484 --date "2025-01-09" --currency PHP --original 100.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 35
    pause
    exit /b 1
)

echo [36/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "단기선교팀 커피 및 음료수" --amount 90293 --date "2025-01-09" --receipt "101" --currency PHP --original 3635.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 36
    pause
    exit /b 1
)

echo [37/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심식사(운전기사)" --amount 4968 --date "2025-01-09" --currency PHP --original 200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 37
    pause
    exit /b 1
)

echo [38/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "저녁(부페52)" --amount 350865 --date "2025-01-09" --receipt "102" --currency PHP --original 14125.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 38
    pause
    exit /b 1
)

echo [39/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "점심식사" --amount 330993 --date "2025-01-10" --receipt "103" --currency PHP --original 13325.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 39
    pause
    exit /b 1
)

echo [40/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "저녁식사" --amount 298080 --date "2025-01-10" --currency PHP --original 12000.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 40
    pause
    exit /b 1
)

echo [41/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "봉고차 점심, 저녁식사" --amount 9936 --date "2025-01-10" --currency PHP --original 400.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 41
    pause
    exit /b 1
)

echo [42/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "식사" --desc "지프니 저녁식사" --amount 4968 --date "2025-01-10" --currency PHP --original 200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 42
    pause
    exit /b 1
)

echo [43/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 렌트비" --amount 86940 --date "2025-01-06" --currency PHP --original 3500.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 43
    pause
    exit /b 1
)

echo [44/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 운전기사 팁" --amount 4968 --date "2025-01-06" --currency PHP --original 200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 44
    pause
    exit /b 1
)

echo [45/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "지프니 렌트비" --amount 62100 --date "2025-01-06" --currency PHP --original 2500.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 45
    pause
    exit /b 1
)

echo [46/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "지프니 운전기사 팁" --amount 4968 --date "2025-01-06" --currency PHP --original 200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 46
    pause
    exit /b 1
)

echo [47/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 렌트비" --amount 4968 --date "2025-01-07" --currency PHP --original 200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 47
    pause
    exit /b 1
)

echo [48/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 운전기사 팁" --amount 4968 --date "2025-01-07" --currency PHP --original 200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 48
    pause
    exit /b 1
)

echo [49/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "지프니 렌트비" --amount 62100 --date "2025-01-07" --currency PHP --original 2500.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 49
    pause
    exit /b 1
)

echo [50/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "지프니 운전기사 팁" --amount 4968 --date "2025-01-07" --currency PHP --original 200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 50
    pause
    exit /b 1
)

echo [51/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 렌트비 3대" --amount 298080 --date "2025-01-08" --currency PHP --original 12000.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 51
    pause
    exit /b 1
)

echo [52/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 운전기사 팁" --amount 14904 --date "2025-01-08" --currency PHP --original 600.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 52
    pause
    exit /b 1
)

echo [53/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "주유비 1호차" --amount 73204 --date "2025-01-08" --receipt "104" --currency PHP --original 2947.06 --rate 24.84
if errorlevel 1 (
    echo Error importing record 53
    pause
    exit /b 1
)

echo [54/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "주유비 2호차" --amount 45771 --date "2025-01-08" --receipt "105" --currency PHP --original 1842.67 --rate 24.84
if errorlevel 1 (
    echo Error importing record 54
    pause
    exit /b 1
)

echo [55/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "주유비 3호차" --amount 49770 --date "2025-01-08" --receipt "106" --currency PHP --original 2003.63 --rate 24.84
if errorlevel 1 (
    echo Error importing record 55
    pause
    exit /b 1
)

echo [56/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 렌트비" --amount 86940 --date "2025-01-09" --currency PHP --original 3500.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 56
    pause
    exit /b 1
)

echo [57/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 운전기사 팁" --amount 7452 --date "2025-01-09" --currency PHP --original 300.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 57
    pause
    exit /b 1
)

echo [58/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 렌트비" --amount 86940 --date "2025-01-10" --currency PHP --original 3500.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 58
    pause
    exit /b 1
)

echo [59/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "지프니 렌트비" --amount 37260 --date "2025-01-10" --currency PHP --original 1500.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 59
    pause
    exit /b 1
)

echo [60/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "봉고차 운전기사 팁" --amount 4968 --date "2025-01-10" --currency PHP --original 200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 60
    pause
    exit /b 1
)

echo [61/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "지프니 운전기사 팁" --amount 2484 --date "2025-01-10" --currency PHP --original 100.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 61
    pause
    exit /b 1
)

echo [62/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "주유비" --amount 90212 --date "2025-01-10" --receipt "107" --currency PHP --original 3631.74 --rate 24.84
if errorlevel 1 (
    echo Error importing record 62
    pause
    exit /b 1
)

echo [63/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "차량" --desc "주유비" --amount 28678 --date "2025-01-10" --receipt "108" --currency PHP --original 1154.51 --rate 24.84
if errorlevel 1 (
    echo Error importing record 63
    pause
    exit /b 1
)

echo [64/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "단기선교팀 요청물품" --amount 159691 --date "2025-01-03" --currency PHP --original 6428.8 --rate 24.84
if errorlevel 1 (
    echo Error importing record 64
    pause
    exit /b 1
)

echo [65/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "선교사님 교회 헌금" --amount 496800 --date "2025-01-06" --currency PHP --original 20000.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 65
    pause
    exit /b 1
)

echo [66/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "집회 건전지" --amount 38800 --date "2025-01-06" --receipt "109" --currency PHP --original 1562.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 66
    pause
    exit /b 1
)

echo [67/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "집회 건전지 추가구매" --amount 12270 --date "2025-01-06" --receipt "110" --currency PHP --original 494.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 67
    pause
    exit /b 1
)

echo [68/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "건전지 구매 교통비" --amount 2980 --date "2025-01-06" --receipt "111" --currency PHP --original 120.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 68
    pause
    exit /b 1
)

echo [69/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "전도지 복사 100매" --amount 9936 --date "2025-01-07" --receipt "112" --currency PHP --original 400.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 69
    pause
    exit /b 1
)

echo [70/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "전도 선물 및 식수, 음료수" --amount 195561 --date "2025-01-07" --receipt "113" --currency PHP --original 7872.85 --rate 24.84
if errorlevel 1 (
    echo Error importing record 70
    pause
    exit /b 1
)

echo [71/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "선교사님 책장 구매 도움 비용" --amount 203688 --date "2025-01-10" --currency PHP --original 8200.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 71
    pause
    exit /b 1
)

echo [72/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "파라다이스 보트 왕복" --amount 31050 --date "2025-01-10" --currency PHP --original 1250.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 72
    pause
    exit /b 1
)

echo [73/73] Importing...
x64\Release\MissionLedgerCLI.exe add --file data.ml --type expense --category "생활" --item "기타" --desc "파라다이스 입장료" --amount 212382 --date "2025-01-10" --receipt "114" --currency PHP --original 8550.0 --rate 24.84
if errorlevel 1 (
    echo Error importing record 73
    pause
    exit /b 1
)

echo.
echo Import completed: 73 records
pause
