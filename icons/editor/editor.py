import re  # Добавьте эту строку в начало файла
import pygame
import os
from pygame.locals import *
import tkinter as tk
from tkinter import filedialog
import threading
import queue

# Инициализация Pygame
pygame.init()

# Константы
DEFAULT_ICON_SIZE = 32
PIXEL_SIZE = 15
GRID_COLOR = (200, 200, 200)
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
BUTTON_COLOR = (220, 220, 220)
WINDOW_WIDTH = 800
WINDOW_HEIGHT = 600
PREVIEW_AREA_HEIGHT = 120  # Высота области предпросмотра
BG_ALPHA_CHANGE = 10  # Шаг изменения прозрачности
MOVE_SPEED = 5       # Скорость перемещения

# Создание окна
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
pygame.display.set_caption("Расширенный редактор иконок")

class IconEditor:
    def __init__(self):
        self.icon_size = DEFAULT_ICON_SIZE
        self.pixels = [[0 for _ in range(DEFAULT_ICON_SIZE)] for _ in range(DEFAULT_ICON_SIZE)]
        self.icon_name = "my_icon"
        self.input_active = False
        self.bg_image = None
        self.bg_alpha = 128
        self.preview_scale = 1
        self.preview_hover = False
        self.drawing = False
        self.file_dialog_active = False
        self.image_queue = queue.Queue()

        # Атрибуты для перемещения
        self.bg_offset_x = 0
        self.bg_offset_y = 0
        self.icon_pos_x = 220  # Начальная позиция X (левый край сетки)
        self.icon_pos_y = 50   #
        self.icon_grid_x = 0  # Позиция иконки в сетке по X
        self.icon_grid_y = 0  # Позиция иконки в сетке по Y
        self.dragging = None

        # UI элементы
        self.font = pygame.font.SysFont(None, 24)
        self.buttons = [
            {"rect": pygame.Rect(10, 350, 180, 40), "text": "Загрузить иконку", "action": self.load_header_file},
            {"rect": pygame.Rect(10, 50, 180, 40), "text": "Очистить", "action": self.clear_grid},
            {"rect": pygame.Rect(10, 100, 180, 40), "text": "Сохранить", "action": self.save_to_header},
            {"rect": pygame.Rect(10, 150, 180, 40), "text": "Инвертировать", "action": self.invert_colors},
            {"rect": pygame.Rect(10, 200, 180, 40), "text": "Загрузить фон", "action": self.load_background},
            {"rect": pygame.Rect(10, 250, 180, 40), "text": "Удалить фон", "action": self.remove_background},
            {"rect": pygame.Rect(10, 300, 180, 40), "text": f"Размер: {DEFAULT_ICON_SIZE}x{DEFAULT_ICON_SIZE}", "action": self.change_size},
            {"rect": pygame.Rect(10, 400, 180, 40), "text": "Сбросить смещения", "action": self.reset_offsets},
            {"rect": pygame.Rect(10, 450, 180, 40), "text": "Прозрачность +", "action": lambda: self.change_alpha(BG_ALPHA_CHANGE)},
            {"rect": pygame.Rect(10, 500, 180, 40), "text": "Прозрачность -", "action": lambda: self.change_alpha(-BG_ALPHA_CHANGE)}
        ]
        
        self.preview_rect = pygame.Rect(10, WINDOW_HEIGHT - 150, 180, 140)

    def reset_offsets(self):
        """Сброс позиции иконки и фона"""
        self.icon_pos_x = 220
        self.icon_pos_y = 50
        self.bg_offset_x = 0
        self.bg_offset_y = 0
        self.icon_grid_x = 0
        self.icon_grid_y = 0


    def change_alpha(self, delta):
        """Изменение прозрачности фона"""
        if self.bg_image:
            self.bg_alpha = max(0, min(255, self.bg_alpha + delta))

    def change_size(self):
        if self.icon_size == 32:
            self.icon_size = 16
        else:
            self.icon_size = 32
        self.pixels = [[0 for _ in range(self.icon_size)] for _ in range(self.icon_size)]
        self.buttons[6]["text"] = f"Размер: {self.icon_size}x{self.icon_size}"
        
        if self.bg_image:
            original_image = pygame.image.load("temp_bg.png")
            self.bg_image = pygame.transform.scale(
                original_image, 
                (self.icon_size * PIXEL_SIZE, self.icon_size * PIXEL_SIZE))
            os.remove("temp_bg.png")
    
    # def clamp_offsets(self):
    #     """Ограничивает смещения иконки строго в пределах сетки"""
    #     max_offset = self.icon_size * PIXEL_SIZE - 1
    #     self.icon_offset_x = max(0, min(max_offset, self.icon_offset_x))
    #     self.icon_offset_y = max(0, min(max_offset, self.icon_offset_y))
    #     # Округляем до целых пикселей
    #     self.icon_offset_x = (self.icon_offset_x // PIXEL_SIZE) * PIXEL_SIZE
    #     self.icon_offset_y = (self.icon_offset_y // PIXEL_SIZE) * PIXEL_SIZE
        
    def load_background(self):
        def run_file_dialog():
            root = tk.Tk()
            root.withdraw()
            root.attributes('-topmost', True)
            
            file_path = filedialog.askopenfilename(
                title="Выберите фоновое изображение",
                filetypes=[("PNG с прозрачностью", "*.png"), ("Изображения", "*.bmp;*.jpg;*.jpeg")],
                initialdir=os.getcwd()
            )
            root.destroy()
            self.image_queue.put(("background", file_path))
            self.file_dialog_active = False
        
        if not self.file_dialog_active:
            self.file_dialog_active = True
            threading.Thread(target=run_file_dialog, daemon=True).start()


    def remove_background(self):
        self.bg_image = None
        if os.path.exists("temp_bg.png"):
            os.remove("temp_bg.png")

    def clear_grid(self):
        self.pixels = [[0 for _ in range(self.icon_size)] for _ in range(self.icon_size)]

    def invert_colors(self):
        for y in range(self.icon_size):
            for x in range(self.icon_size):
                self.pixels[y][x] = 1 - self.pixels[y][x]

    def save_to_header(self):
        filename = f"{self.icon_name}.h"
        with open(filename, "w") as f:
            f.write(f"// Иконка {self.icon_name} ({self.icon_size}x{self.icon_size})\n")
            f.write("#pragma once\n#include <avr/pgmspace.h>\n\n")
            f.write(f"const uint8_t PROGMEM {self.icon_name}[] = {{\n")
            
            for y in range(self.icon_size):
                row_bytes = []
                for x in range(0, self.icon_size, 8):
                    byte = 0
                    for bit in range(8):
                        if x + bit < self.icon_size and self.pixels[y][x + bit]:
                            byte |= 1 << (7 - bit)
                    row_bytes.append(f"0x{byte:02X}")
                f.write("  " + ", ".join(row_bytes) + ("," if y < self.icon_size - 1 else "") + "\n")
            
            f.write("};\n")
        print(f"Иконка сохранена в {filename}")

    def draw_interface(self):
        screen.fill(WHITE)
        
        # Поле ввода имени
        pygame.draw.rect(screen, (240, 240, 240) if self.input_active else WHITE, (10, 5, 300, 30))
        pygame.draw.rect(screen, BLACK, (10, 5, 300, 30), 2)
        name_surface = self.font.render(f"Имя: {self.icon_name}", True, BLACK)
        screen.blit(name_surface, (15, 10))

        # Кнопки управления
        for button in self.buttons:
            pygame.draw.rect(screen, BUTTON_COLOR, button["rect"])
            text_surface = self.font.render(button["text"], True, BLACK)
            text_rect = text_surface.get_rect(center=button["rect"].center)
            screen.blit(text_surface, text_rect)

        # Основная сетка для рисования
        grid_rect = pygame.Rect(220, 50, self.icon_size * PIXEL_SIZE, self.icon_size * PIXEL_SIZE)
        
        # Фоновая картинка с учетом смещения
        if self.bg_image:
            bg_rect = grid_rect.move(self.bg_offset_x, self.bg_offset_y)
            bg_copy = self.bg_image.copy()
            bg_copy.set_alpha(self.bg_alpha)
            screen.blit(bg_copy, bg_rect.topleft)
        
        # Сетка
        for x in range(self.icon_size + 1):
            pygame.draw.line(screen, GRID_COLOR, 
                           (grid_rect.left + x * PIXEL_SIZE, grid_rect.top),
                           (grid_rect.left + x * PIXEL_SIZE, grid_rect.bottom))
            
        for y in range(self.icon_size + 1):
            pygame.draw.line(screen, GRID_COLOR,
                           (grid_rect.left, grid_rect.top + y * PIXEL_SIZE),
                           (grid_rect.right, grid_rect.top + y * PIXEL_SIZE))

        # Пиксели иконки с учетом ее позиции в сетке
        for y in range(self.icon_size):
            for x in range(self.icon_size):
                if self.pixels[y][x]:
                    draw_x = grid_rect.left + (x + self.icon_grid_x) * PIXEL_SIZE + 1
                    draw_y = grid_rect.top + (y + self.icon_grid_y) * PIXEL_SIZE + 1
                    pygame.draw.rect(screen, BLACK, (draw_x, draw_y, PIXEL_SIZE - 1, PIXEL_SIZE - 1))                
        # # Рисуем иконку в новых координатах
        # for y in range(self.icon_size):
        #     for x in range(self.icon_size):
        #         if self.pixels[y][x]:
        #             pygame.draw.rect(screen, BLACK,
        #                 (self.icon_pos_x + x * PIXEL_SIZE + 1,
        #                 self.icon_pos_y + y * PIXEL_SIZE + 1,
        #                 PIXEL_SIZE - 1, PIXEL_SIZE - 1))
                    
        # Подсветка при перемещении иконки
        if self.dragging == 'icon':
            s = pygame.Surface((self.icon_size * PIXEL_SIZE, self.icon_size * PIXEL_SIZE), pygame.SRCALPHA)
            s.fill((255, 255, 0, 64))  # Полупрозрачный желтый
            screen.blit(s, (
                grid_rect.left + self.icon_grid_x * PIXEL_SIZE,
                grid_rect.top + self.icon_grid_y * PIXEL_SIZE
            ))
        # # Рисуем полупрозрачный прямоугольник вокруг иконки
            # s = pygame.Surface((self.icon_size * PIXEL_SIZE, self.icon_size * PIXEL_SIZE), pygame.SRCALPHA)
            # s.fill((255, 255, 0, 64))
            # screen.blit(s, (self.icon_pos_x, self.icon_pos_y))

            
        # Область предпросмотра (всегда 1:1)
        pygame.draw.rect(screen, BUTTON_COLOR, self.preview_rect)
        pygame.draw.rect(screen, BLACK, self.preview_rect, 2)
        
        # Рассчитываем размер предпросмотра (1:1)
        preview_size = min(self.preview_rect.width - 20, self.preview_rect.height - 20)
        preview_size = min(preview_size, self.icon_size)  # Не больше размера иконки
        
        # Центрируем предпросмотр
        preview_x = self.preview_rect.x + (self.preview_rect.width - preview_size) // 2
        preview_y = self.preview_rect.y + (self.preview_rect.height - preview_size) // 2
        
        # Рисуем белую подложку
        pygame.draw.rect(screen, WHITE, (preview_x, preview_y, preview_size, preview_size))
        
        # Рассчитываем размер одного пикселя (1:1)
        pixel_size = preview_size // self.icon_size
        if pixel_size < 1:
            pixel_size = 1
        
        # Рисуем пиксели в предпросмотре
        for y in range(self.icon_size):
            for x in range(self.icon_size):
                if self.pixels[y][x]:
                    pygame.draw.rect(
                        screen, BLACK,
                        (preview_x + x * pixel_size,
                         preview_y + y * pixel_size,
                         pixel_size, pixel_size))
        
        # Подписи для управления
        font_small = pygame.font.SysFont(None, 20)
        controls_text = [
            "Управление:",
            "Shift+ЛКМ - двигать иконку",
            "Ctrl+ЛКМ - двигать фон"
        ]
        
        for i, text in enumerate(controls_text):
            text_surface = font_small.render(text, True, BLACK)
            screen.blit(text_surface, (self.preview_rect.x, self.preview_rect.y - 20 - i*20))

    def load_header_file(self):
        def run_file_dialog():
            root = tk.Tk()
            root.withdraw()
            root.attributes('-topmost', True)
            
            file_path = filedialog.askopenfilename(
                title="Выберите заголовочный файл иконки",
                filetypes=[("Заголовочные файлы", "*.h"), ("Все файлы", "*.*")],
                initialdir=os.getcwd()
            )
            root.destroy()
            self.image_queue.put(("header", file_path))
            self.file_dialog_active = False
        
        if not self.file_dialog_active:
            self.file_dialog_active = True
            threading.Thread(target=run_file_dialog, daemon=True).start()

    def parse_header_file(self, file_path):
        try:
            with open(file_path, 'r') as f:
                content = f.read()
                
                # Извлекаем имя иконки
                name_match = re.search(r'const uint8_t PROGMEM (\w+)\[\]', content)
                if name_match:
                    self.icon_name = name_match.group(1)
                
                # Извлекаем данные иконки
                data_matches = re.findall(r'0x([0-9A-Fa-f]{2})', content)
                if not data_matches:
                    raise ValueError("Не найдены данные иконки в файле")
                    
                # Преобразуем hex в бинарные данные
                byte_values = [int(b, 16) for b in data_matches]
                
                # Определяем размер иконки (предполагаем квадратную)
                size = int(len(byte_values) ** 0.5)
                if size * size != len(byte_values):
                    size = int((len(byte_values) * 8) ** 0.5)
                    
                self.icon_size = size
                self.pixels = [[0 for _ in range(size)] for _ in range(size)]
                
                # Преобразуем байты в пиксели
                for y in range(size):
                    for x in range(0, size, 8):
                        byte = byte_values[(y * size + x) // 8]
                        for bit in range(8):
                            if x + bit < size:
                                self.pixels[y][x + bit] = (byte >> (7 - bit)) & 1
                
                print(f"Иконка загружена из {os.path.basename(file_path)}")
                return True

        except Exception as e:
            print(f"Ошибка загрузки заголовочного файла: {e}")
            return False

            
    def handle_events(self):
        try:
            item = self.image_queue.get_nowait()
            if isinstance(item, tuple) and len(item) == 2:
                file_type, file_path = item
                if file_path:
                    if file_type == "header":
                        self.parse_header_file(file_path)
                    elif file_type == "background":
                        try:
                            if file_path.lower().endswith('.png'):
                                original_image = pygame.image.load(file_path).convert_alpha()
                            else:
                                original_image = pygame.image.load(file_path).convert()
                            
                            self.bg_image = pygame.Surface((self.icon_size * PIXEL_SIZE, self.icon_size * PIXEL_SIZE), pygame.SRCALPHA)
                            scaled_image = pygame.transform.scale(
                                original_image, 
                                (self.icon_size * PIXEL_SIZE, self.icon_size * PIXEL_SIZE))
                            self.bg_image.blit(scaled_image, (0, 0))
                            print(f"Фоновое изображение загружено: {os.path.basename(file_path)}")
                        except Exception as e:
                            print(f"Ошибка загрузки изображения: {e}")
            else:
                # Обработка случая, если в очереди не tuple
                print(f"Неожиданный элемент в очереди: {item}")
        except queue.Empty:
            pass

        for event in pygame.event.get():
            if event.type == QUIT:
                if os.path.exists("temp_bg.png"):
                    os.remove("temp_bg.png")
                return False
        
            # elif event.type == MOUSEBUTTONDOWN:
            #     keys = pygame.key.get_mods()
            #     if keys & pygame.KMOD_SHIFT:
            #         # Проверяем, кликнули ли по иконке
            #         mouse_x, mouse_y = event.pos
            #         icon_rect = pygame.Rect(
            #             self.icon_pos_x,
            #             self.icon_pos_y,
            #             self.icon_size * PIXEL_SIZE,
            #             self.icon_size * PIXEL_SIZE
            #         )
            #         if icon_rect.collidepoint(mouse_x, mouse_y):
            #             self.dragging = 'icon'
            #             # Запоминаем смещение курсора относительно иконки
            #             self.drag_offset_x = mouse_x - self.icon_pos_x
            #             self.drag_offset_y = mouse_y - self.icon_pos_y
                        
            elif event.type == MOUSEBUTTONDOWN:
                keys = pygame.key.get_mods()
                if keys & pygame.KMOD_SHIFT:
                    self.dragging = 'icon'
                    self.drag_start_x = event.pos[0]
                    self.drag_start_y = event.pos[1]
                    # Запоминаем начальную позицию в пикселях
                    self.drag_icon_start_x = self.icon_grid_x
                    self.drag_icon_start_y = self.icon_grid_y
                    
                elif keys & pygame.KMOD_CTRL:
                    self.dragging = 'bg'
                else:
                    for button in self.buttons:
                        if button["rect"].collidepoint(event.pos) and button["action"]:
                            button["action"]()
                    
                    if 10 <= event.pos[0] <= 310 and 5 <= event.pos[1] <= 35:
                        self.input_active = True
                    else:
                        self.input_active = False
                    
                    grid_rect = pygame.Rect(220, 50, self.icon_size * PIXEL_SIZE, self.icon_size * PIXEL_SIZE)
                    if grid_rect.collidepoint(event.pos):
                        self.drawing = True
                        x = (event.pos[0] - grid_rect.left) // PIXEL_SIZE
                        y = (event.pos[1] - grid_rect.top) // PIXEL_SIZE
                        if 0 <= x < self.icon_size and 0 <= y < self.icon_size:
                            if event.button == 1:
                                self.pixels[y][x] = 1
                            elif event.button == 3:
                                self.pixels[y][x] = 0
            
            elif event.type == MOUSEBUTTONUP:
                self.drawing = False
                self.dragging = None

            elif event.type == MOUSEMOTION:
                if self.dragging == 'icon' and event.buttons[0]:
                    # Вычисляем смещение в пикселях сетки
                    delta_x = (event.pos[0] - self.drag_start_x) // PIXEL_SIZE
                    delta_y = (event.pos[1] - self.drag_start_y) // PIXEL_SIZE
                    
                    # Новая позиция иконки
                    new_x = self.drag_icon_start_x + delta_x
                    new_y = self.drag_icon_start_y + delta_y

                    # Ограничиваем позицию, чтобы иконка не выходила за сетку
                    self.icon_grid_x = new_x #max(-self.icon_size + 1, min(self.icon_size - 1, new_x))
                    self.icon_grid_y = new_y #max(-self.icon_size + 1, min(self.icon_size - 1, new_y))                   
                    
                    # # Ограничиваем перемещение в пределах окна
                    # self.icon_pos_x = max(220, min(
                    #     WINDOW_WIDTH - self.icon_size * PIXEL_SIZE - 10,
                    #     self.icon_pos_x
                    # ))
                    # self.icon_pos_y = max(50, min(
                    #     WINDOW_HEIGHT - self.icon_size * PIXEL_SIZE - 10,
                    #     self.icon_pos_y
                    # ))
                
                elif self.dragging == 'bg' and event.buttons[0]:
                    self.bg_offset_x += event.rel[0]
                    self.bg_offset_y += event.rel[1]
            
                
                #self.preview_hover = self.preview_rect.collidepoint(event.pos)
                
                if self.drawing:
                    grid_rect = pygame.Rect(220, 50, self.icon_size * PIXEL_SIZE, self.icon_size * PIXEL_SIZE)
                    if grid_rect.collidepoint(event.pos):
                        x = (event.pos[0] - grid_rect.left) // PIXEL_SIZE
                        y = (event.pos[1] - grid_rect.top) // PIXEL_SIZE
                        if 0 <= x < self.icon_size and 0 <= y < self.icon_size:
                            buttons = pygame.mouse.get_pressed()
                            if buttons[0]:
                                self.pixels[y][x] = 1
                            elif buttons[2]:
                                self.pixels[y][x] = 0
            
            elif event.type == KEYDOWN and self.input_active:
                if event.key == K_RETURN:
                    self.input_active = False
                elif event.key == K_BACKSPACE:
                    self.icon_name = self.icon_name[:-1]
                elif event.unicode.isalnum() or event.unicode in ('_', '-'):
                    self.icon_name += event.unicode
        
        return True
    
# Основной цикл
editor = IconEditor()
clock = pygame.time.Clock()
running = True


while running:
    running = editor.handle_events()
    editor.draw_interface()
    pygame.display.flip()
    clock.tick(60)

pygame.quit()