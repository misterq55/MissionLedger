@echo off
chcp 65001 > nul
set CLI=C:\Work\Others\MissionLedger\x64\Release\MissionLedgerCLI.exe
set FILE=data.ml

echo === 수입 데이터 입력 시작 ===

REM 회비
echo [회비] 입력 중...
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "박세영" --date 2024-11-14
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "서문규" --date 2024-11-14
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "진예주" --date 2024-11-15
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "이정수" --date 2024-11-21
%CLI% add --file %FILE% --type income --amount 1000000 --category "회비" --item "김영미" --date 2024-11-21
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "최주영" --date 2024-11-21
%CLI% add --file %FILE% --type income --amount 300000 --category "회비" --item "장세린" --date 2024-11-22
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "서찬민" --date 2024-11-23
%CLI% add --file %FILE% --type income --amount 300000 --category "회비" --item "장세린" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 1000000 --category "회비" --item "하종규" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 1000000 --category "회비" --item "박은경" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "신인재" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "이하연" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "조수아" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "박예은" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "박희원" --date 2024-11-26
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "박경린" --date 2024-11-26
%CLI% add --file %FILE% --type income --amount 1000000 --category "회비" --item "김대헌" --date 2024-11-27
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "김승혜" --date 2024-11-28
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "윤서현" --date 2024-11-28
%CLI% add --file %FILE% --type income --amount 500000 --category "회비" --item "김하은" --date 2024-11-28
%CLI% add --file %FILE% --type income --amount 200000 --category "회비" --item "장세린" --date 2024-11-29
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "김유니" --date 2024-12-05
%CLI% add --file %FILE% --type income --amount 400000 --category "회비" --item "박세철" --date 2024-12-12
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "이재혁" --date 2024-12-26

REM 후원
echo [후원] 입력 중...
%CLI% add --file %FILE% --type income --amount 200000 --category "후원" --item "당회 지원" --date 2024-12-01
%CLI% add --file %FILE% --type income --amount 800000 --category "후원" --item "목적 헌금" --date 2024-11-26
%CLI% add --file %FILE% --type income --amount 900000 --category "후원" --item "목적 헌금" --date 2024-12-03
%CLI% add --file %FILE% --type income --amount 840000 --category "후원" --item "목적 헌금" --date 2024-12-10
%CLI% add --file %FILE% --type income --amount 1680000 --category "후원" --item "목적 헌금" --date 2024-12-24
%CLI% add --file %FILE% --type income --amount 2000000 --category "후원" --item "재정 지원"

REM 기타
echo [기타] 입력 중...
%CLI% add --file %FILE% --type income --amount 1042 --category "기타" --item "결산이자" --date 2024-12-14

echo === 입력 완료 ===
echo 결과 확인:
%CLI% list %FILE%

pause
