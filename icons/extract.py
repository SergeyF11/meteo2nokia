import os
import cv2
import numpy as np
from PIL import Image
import argparse

# Путь к файлу с иконками
ICON_SHEET_PATH = "Weather-Icons.png"

# Размер сетки (5 столбцов, 3 строки)
GRID_COLS = 5
GRID_ROWS = 3

# Размер иконок (ширина и высота)
ICON_SIZE = (52, 52)  # Исходный размер иконок

# Желаемый размер иконок после удаления пустых строк и столбцов
OUT_SIZE = (32, 32)  # Измените на нужный размер

# Папка для сохранения вырезанных иконок
OUTPUT_DIR = "icons"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Инвертировать цвета (True/False)
INVERT_COLORS = True  # Измените на False, если инверсия не нужна

EMPTY_TRESHOLD = 10

def remove_empty_rows_and_columns(image):
    """Удаляет пустые строки и столбцы только с краев изображения."""
    # Преобразуем изображение в массив numpy
    img_array = np.array(image)
    
    # Находим строки и столбцы, которые не полностью пустые (яркость выше порога)
    if INVERT_COLORS:
        brightness_threshold = 255-EMPTY_TRESHOLD  # Порог яркости для определения пустых пикселей
        non_empty_rows = np.where(np.min(img_array, axis=1) < brightness_threshold)[0]
        non_empty_cols = np.where(np.min(img_array, axis=0) < brightness_threshold)[0]
    else:
        brightness_threshold = EMPTY_TRESHOLD  # Порог яркости для определения пустых пикселей
        non_empty_rows = np.where(np.max(img_array, axis=1) > brightness_threshold)[0]
        non_empty_cols = np.where(np.max(img_array, axis=0) > brightness_threshold)[0]
    
    # Если есть непустые строки и столбцы, обрезаем изображение
    if len(non_empty_rows) > 0 and len(non_empty_cols) > 0:
        # Находим границы для обрезки (первый и последний непустые строки и столбцы)
        top = non_empty_rows[0]
        bottom = non_empty_rows[-1] + 1
        left = non_empty_cols[0]
        right = non_empty_cols[-1] + 1
        
        # Обрезаем изображение до найденных границ
        cropped_image = img_array[top:bottom, left:right]
        return Image.fromarray(cropped_image)
    else:
        return image  # Если все строки и столбцы пустые, возвращаем исходное изображение

def resize_with_aspect_ratio(image, target_size):
    """Изменяет размер изображения с сохранением пропорций и добавляет padding."""
    original_width, original_height = image.size
    target_width, target_height = target_size

    # Вычисляем соотношение сторон
    ratio = min(target_width / original_width, target_height / original_height)
    new_width = int(original_width * ratio)
    new_height = int(original_height * ratio)

    # Изменяем размер с сохранением пропорций
    resized_image = image.resize((new_width, new_height), Image.LANCZOS)

    # Создаем новое изображение с целевым размером и черным фоном
    new_image = Image.new("L", target_size, 255)  # Черный фон для градаций серого

    # Вычисляем координаты для центрирования
    x_offset = (target_width - new_width) // 2
    y_offset = (target_height - new_height) // 2

    # Вставляем измененное изображение в центр нового изображения
    new_image.paste(resized_image, (x_offset, y_offset))

    return new_image

def extract_icons_from_grid(image_path, grid_cols, grid_rows, icon_size, output_dir):
    """Вырезает иконки из изображения, разделенного на сетку."""
    image = Image.open(image_path).convert("L")  # Открываем изображение в градациях серого
    width, height = image.size

    # Вычисляем размер каждой ячейки сетки
    cell_width = width // grid_cols
    cell_height = height // grid_rows

    icon_paths = []
    for row in range(grid_rows):
        for col in range(grid_cols):
            # Вычисляем координаты для вырезания иконки
            x = col * cell_width
            y = row * cell_height
            w = cell_width
            h = cell_height

            # Вырезаем иконку
            icon = image.crop((x, y, x + w, y + h))

            # Удаляем пустые строки и столбцы только с краев
            cleaned_icon = remove_empty_rows_and_columns(icon)

            # Изменяем размер с сохранением пропорций и добавляем padding
            resized_icon = resize_with_aspect_ratio(cleaned_icon, OUT_SIZE)

            # Сохраняем иконку
            icon_name = f"icon_{row}_{col}.png"
            icon_path = os.path.join(output_dir, icon_name)
            resized_icon.save(icon_path)
            icon_paths.append((icon_name, icon_path))

    return icon_paths

def extract_single_icon(image_path, row, col, grid_cols, grid_rows, icon_size, output_dir):
    """Вырезает одну иконку из изображения по координатам в сетке."""
    image = Image.open(image_path).convert("L")  # Открываем изображение в градациях серого
    width, height = image.size

    # Вычисляем размер каждой ячейки сетки
    cell_width = width // grid_cols
    cell_height = height // grid_rows

    # Вычисляем координаты для вырезания иконки
    x = col * cell_width
    y = row * cell_height
    w = cell_width
    h = cell_height

    # Вырезаем иконку
    icon = image.crop((x, y, x + w, y + h))

    # Удаляем пустые строки и столбцы только с краев
    cleaned_icon = remove_empty_rows_and_columns(icon)

    # Изменяем размер с сохранением пропорций и добавляем padding
    resized_icon = resize_with_aspect_ratio(cleaned_icon, OUT_SIZE)

    # Сохраняем иконку
    icon_name = f"icon_{row}_{col}.png"
    icon_path = os.path.join(output_dir, icon_name)
    resized_icon.save(icon_path)

    return icon_path

def convert_image_to_bitmap(image_path, invert):
    """Конвертирует изображение в битовый массив для Adafruit_GFX."""
    image = Image.open(image_path).convert("1")  # Конвертируем в черно-белый формат
    width, height = image.size

    # Вычисляем количество байт на строку (8 пикселей = 1 байт)
    bytes_per_row = (width + 7) // 8
    bitmap = []

    for y in range(height):
        for byte in range(bytes_per_row):
            value = 0
            for bit in range(8):
                x = byte * 8 + bit
                if x < width:
                    pixel = image.getpixel((x, y))
                    if invert:
                        pixel = 1 - pixel  # Инвертируем цвет
                    value |= (pixel & 0x01) << (7 - bit)  # Упаковываем биты в байт
            bitmap.append(f"0x{value:02X}")  # Преобразуем в hex

    return bitmap, width, height

def generate_header_file(icon_data, output_file):
    """Генерирует заголовочный файл для Arduino."""
    with open(output_file, "w") as f:
        f.write("#pragma once\n\n")
        f.write("#include <Arduino.h>\n\n")
        f.write("namespace Icons {\n")

        # Добавляем информацию о размере иконок
        f.write(f"const uint8_t width = {OUT_SIZE[0]};\n")
        f.write(f"const uint8_t height = {OUT_SIZE[1]};\n\n")

        for icon_name, (bitmap, width, height) in icon_data.items():
            bytes_per_row = (width + 7) // 8
            f.write(f"const uint8_t {icon_name}[{height * bytes_per_row}] PROGMEM = {{\n")
            for i in range(0, len(bitmap), bytes_per_row):
                f.write("    " + ", ".join(bitmap[i:i + bytes_per_row]) + ",\n")
            f.write("};\n\n")

        f.write("}\n")
    print(f"Заголовочный файл {output_file} создан.")

def main():
    parser = argparse.ArgumentParser(description="Конвертация иконок для Arduino.")
    parser.add_argument("--all", action="store_true", help="Конвертировать все иконки из сетки")
    parser.add_argument("--single", type=str, help="Конвертировать одну иконку из файла")
    parser.add_argument("--grid", type=int, nargs=2, metavar=("ROW", "COL"), help="Конвертировать одну иконку по координатам в сетке (ROW COL)")
    parser.add_argument("--output", "-o", type=str, default="weather_icons.h", help="Имя выходного файла для Arduino")
    args = parser.parse_args()

    if args.all:
        # Конвертируем все иконки из сетки
        icon_paths = extract_icons_from_grid(ICON_SHEET_PATH, GRID_COLS, GRID_ROWS, ICON_SIZE, OUTPUT_DIR)
        icon_data = {}
        for icon_name, icon_path in icon_paths:
            bitmap, width, height = convert_image_to_bitmap(icon_path, INVERT_COLORS)
            icon_data[icon_name.replace(".png", "")] = (bitmap, width, height)
            print(f"Иконка {icon_name} конвертирована (размер: {width}x{height}).")
        generate_header_file(icon_data, args.output)

    elif args.single:
        # Конвертируем одну иконку из файла
        icon_path = args.single
        bitmap, width, height = convert_image_to_bitmap(icon_path, INVERT_COLORS)
        icon_name = os.path.basename(icon_path).replace(".png", "")
        icon_data = {icon_name: (bitmap, width, height)}
        print(f"Иконка {icon_name} конвертирована (размер: {width}x{height}).")
        generate_header_file(icon_data, args.output)

    elif args.grid:
        # Конвертируем одну иконку по координатам в сетке
        row, col = args.grid
        icon_path = extract_single_icon(ICON_SHEET_PATH, row, col, GRID_COLS, GRID_ROWS, ICON_SIZE, OUTPUT_DIR)
        bitmap, width, height = convert_image_to_bitmap(icon_path, INVERT_COLORS)
        icon_name = os.path.basename(icon_path).replace(".png", "")
        icon_data = {icon_name: (bitmap, width, height)}
        print(f"Иконка {icon_name} конвертирована (размер: {width}x{height}).")
        generate_header_file(icon_data, args.output)

    else:
        print("Не указан режим работы. Используйте --all, --single или --grid.")

if __name__ == "__main__":
    main()