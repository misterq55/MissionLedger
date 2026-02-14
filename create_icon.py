#!/usr/bin/env python3
"""
MissionLedger 아이콘 생성 스크립트
간단한 "ML" 로고와 책/장부 모티프를 사용한 아이콘 생성
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_icon():
    """멀티 사이즈 .ico 파일 생성 (32x32, 48x48, 256x256)"""

    sizes = [256, 48, 32]  # 큰 것부터 생성
    images = []

    for size in sizes:
        # 새 이미지 생성 (흰색 배경)
        img = Image.new('RGBA', (size, size), (255, 255, 255, 255))
        draw = ImageDraw.Draw(img)

        # 색상 정의 (심플한 빨간색)
        text_color = (220, 20, 60, 255)  # 진한 빨간색 (Crimson)

        # "ML" 텍스트 (폰트 크기 조절 - 더 크게)
        try:
            # 시스템 폰트 사용 시도 (굵은 폰트)
            font_size = int(size * 0.55)  # 55% 크기 (더 크게)
            try:
                # Windows - 굵은 폰트
                font = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", font_size)
            except:
                try:
                    # 대체 폰트
                    font = ImageFont.truetype("arial.ttf", font_size)
                except:
                    # 기본 폰트
                    font = ImageFont.load_default()
        except:
            font = ImageFont.load_default()

        text = "ML"

        # 텍스트 위치 계산 (완전 중앙)
        bbox = draw.textbbox((0, 0), text, font=font)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
        text_x = (size - text_width) // 2
        text_y = (size - text_height) // 2 - bbox[1]  # baseline 조정

        # 메인 텍스트 (빨간색, 그림자 없음)
        draw.text(
            (text_x, text_y),
            text,
            fill=text_color,
            font=font
        )

        images.append(img)

    # .ico 파일로 저장 (멀티 사이즈)
    output_path = "MissionLedger/resources/app.ico"
    images[0].save(
        output_path,
        format='ICO',
        sizes=[(img.width, img.height) for img in images],
        append_images=images[1:]
    )

    print(f"[OK] Icon created: {output_path}")
    print(f"   - Sizes: {', '.join([f'{s}x{s}' for s in sizes])}")
    print(f"   - File size: {os.path.getsize(output_path):,} bytes")

    # 미리보기용 PNG도 생성 (256x256)
    preview_path = "MissionLedger/resources/app_preview.png"
    images[0].save(preview_path, format='PNG')
    print(f"[OK] Preview created: {preview_path}")

if __name__ == "__main__":
    os.chdir("C:/Work/Others/MissionLedger")
    create_icon()
