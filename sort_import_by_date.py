#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
import_field.bat를 날짜별로 정렬하는 스크립트
"""

import re
from datetime import datetime

def parse_bat_file(filepath):
    """배치 파일을 읽어서 레코드별로 파싱"""
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # 헤더 부분 (첫 5줄)
    lines = content.split('\n')
    header = lines[:5]

    # 레코드 추출 (echo [N/M] 패턴으로 시작)
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
    return datetime.max  # 날짜를 찾을 수 없으면 맨 뒤로

def sort_records_by_date(records):
    """레코드를 날짜순으로 정렬"""
    return sorted(records, key=extract_date_from_record)

def update_record_numbers(records):
    """레코드 번호를 다시 매김"""
    total = len(records)
    updated_records = []

    for i, record in enumerate(records, 1):
        # echo [N/M] 패턴 업데이트
        updated = re.sub(
            r'echo \[\d+/\d+\]',
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
    output_file = 'import_field_sorted.bat'

    print(f'Reading {input_file}...')
    header, records = parse_bat_file(input_file)

    print(f'Found {len(records)} records')
    print('Sorting by date...')
    sorted_records = sort_records_by_date(records)

    print('Updating record numbers...')
    updated_records = update_record_numbers(sorted_records)

    print(f'Writing to {output_file}...')
    write_sorted_bat(output_file, header, updated_records)

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

    print(f'\nDone! Sorted file saved as {output_file}')
    print(f'Total records: {len(records)}')

if __name__ == '__main__':
    main()
