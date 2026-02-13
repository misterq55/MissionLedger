#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
수입 레코드를 import_field.bat에 추가하는 스크립트
"""

import re
from datetime import datetime

# 수입 데이터
income_records = [
    {
        'category': '수입',
        'item': '남서울 교회 송금',
        'desc': '남서울 교회 송금',
        'amount': 4000000,
        'date': '2024-12-19',
        'currency': 'PHP',
        'original': 162206,
        'rate': 24.66
    },
    {
        'category': '수입',
        'item': '남서울 교회 송금',
        'desc': '남서울 교회 송금',
        'amount': 4500000,
        'date': '2025-01-01',
        'currency': 'PHP',
        'original': 177500,
        'rate': 25.36
    },
    {
        'category': '수입',
        'item': '헌금',
        'desc': '헌금',
        'amount': 512000,
        'date': '2025-01-06',
        'currency': 'PHP',
        'original': 20000,
        'rate': 25.6
    },
    {
        'category': '수입',
        'item': '예비비',
        'desc': '예비비',
        'amount': 102400,
        'date': '2024-12-24',
        'currency': 'PHP',
        'original': 4000,
        'rate': 25.6
    },
    {
        'category': '수입',
        'item': '예비비',
        'desc': '예비비',
        'amount': 203688,
        'date': '2025-01-10',
        'currency': 'PHP',
        'original': 8200,
        'rate': 24.84
    }
]

def create_income_record(data, record_num):
    """수입 레코드 생성"""
    return f'''echo [{record_num}/{{total}}] Importing...
x64\\Release\\MissionLedgerCLI.exe add --file data.ml --type income --category "{data['category']}" --item "{data['item']}" --desc "{data['desc']}" --amount {data['amount']} --date "{data['date']}" --currency {data['currency']} --original {data['original']} --rate {data['rate']}
if errorlevel 1 (
    echo Error importing record {record_num}
    pause
    exit /b 1
)'''

def parse_bat_file(filepath):
    """배치 파일을 읽어서 레코드별로 파싱"""
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # 헤더 부분 (첫 5줄)
    lines = content.split('\n')
    header = lines[:5]

    # 레코드 추출
    records = []
    current_record = []
    in_record = False

    for line in lines[5:]:
        if line.strip().startswith('echo ['):
            if current_record:
                records.append('\n'.join(current_record))
            current_record = [line]
            in_record = True
        elif in_record:
            current_record.append(line)
            if line.strip() == ')':
                in_record = False

    # 마지막 레코드 추가
    if current_record:
        records.append('\n'.join(current_record))

    return header, records

def extract_date_from_record(record):
    """레코드에서 날짜 추출"""
    match = re.search(r'--date\s+"(\d{4}-\d{2}-\d{2})"', record)
    if match:
        return datetime.strptime(match.group(1), '%Y-%m-%d')
    return datetime.max

def sort_records_by_date(records):
    """레코드를 날짜순으로 정렬"""
    return sorted(records, key=extract_date_from_record)

def update_record_numbers(records):
    """레코드 번호를 다시 매김"""
    total = len(records)
    updated_records = []

    for i, record in enumerate(records, 1):
        # echo [N/M] 또는 echo [N/{total}] 패턴 업데이트
        updated = re.sub(
            r'echo \[\d+/(\d+|\{total\})\]',
            f'echo [{i}/{total}]',
            record
        )
        # Error importing record N 패턴 업데이트
        updated = re.sub(
            r'echo Error importing record \d+',
            f'echo Error importing record {i}',
            updated
        )
        updated_records.append(updated)

    return updated_records

def write_sorted_bat(filepath, header, records):
    """정렬된 배치 파일 작성"""
    with open(filepath, 'w', encoding='utf-8') as f:
        # 헤더 작성
        f.write('\n'.join(header))
        f.write('\n\n')

        # 레코드 작성
        for record in records:
            f.write(record)
            f.write('\n\n')

        # 푸터 작성
        f.write('echo.\n')
        f.write(f'echo Import completed: {len(records)} records\n')
        f.write('pause\n')

def main():
    input_file = 'import_field.bat'

    print(f'Reading {input_file}...')
    header, existing_records = parse_bat_file(input_file)
    print(f'Found {len(existing_records)} existing records')

    # 수입 레코드 생성
    print(f'\nAdding {len(income_records)} income records...')
    new_records = []
    for i, data in enumerate(income_records, 1):
        record = create_income_record(data, i)
        new_records.append(record)
        print(f'  - {data["date"]}: {data["item"]} ({data["amount"]:,}원)')

    # 모든 레코드 합치기
    all_records = existing_records + new_records
    print(f'\nTotal records: {len(all_records)}')

    print('Sorting by date...')
    sorted_records = sort_records_by_date(all_records)

    print('Updating record numbers...')
    updated_records = update_record_numbers(sorted_records)

    print(f'Writing to {input_file}...')
    write_sorted_bat(input_file, header, updated_records)

    # 날짜별 요약 출력
    print('\n=== Date Summary ===')
    date_counts = {}
    for record in sorted_records:
        date = extract_date_from_record(record)
        if date != datetime.max:
            date_str = date.strftime('%Y-%m-%d')
            date_counts[date_str] = date_counts.get(date_str, 0) + 1

    for date_str in sorted(date_counts.keys()):
        print(f'{date_str}: {date_counts[date_str]} records')

    print(f'\nDone! Updated {input_file} with {len(new_records)} new income records')
    print(f'Total records: {len(all_records)}')

if __name__ == '__main__':
    main()
