#include <stdio.h>
#include <string.h>
#include "util.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>

void gotoxy(int x, int y) {
    COORD pos = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hideCursor(void) {
    CONSOLE_CURSOR_INFO info = {1, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void showCursor(void) {
    CONSOLE_CURSOR_INFO info = {1, TRUE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

static const WORD win_colors[] = {
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    FOREGROUND_RED   | FOREGROUND_INTENSITY,
    FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
    FOREGROUND_RED   | FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
    FOREGROUND_GREEN | FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
    FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
};

void setColor(int color) {
    int idx = (color >= 0 && color < 8) ? color : 0;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), win_colors[idx]);
}

void resetColor(void) { setColor(C_DEFAULT); }

void clearScreen(void) { system("cls"); }

char readKey(void) { return (char)_getch(); }

#else
/* POSIX / macOS - ANSI escape codes */
#include <termios.h>
#include <unistd.h>

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

void hideCursor(void) {
    printf("\033[?25l");
    fflush(stdout);
}

void showCursor(void) {
    printf("\033[?25h");
    fflush(stdout);
}

static const int ansi_colors[] = {37, 31, 32, 33, 34, 35, 36, 97};

void setColor(int color) {
    int idx = (color >= 0 && color < 8) ? color : 0;
    printf("\033[%dm", ansi_colors[idx]);
    fflush(stdout);
}

void resetColor(void) {
    printf("\033[0m");
    fflush(stdout);
}

void clearScreen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

char readKey(void) {
    struct termios old, raw;
    tcgetattr(STDIN_FILENO, &old);
    raw = old;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    char ch = (char)getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return ch;
}

#endif /* _WIN32 */

void clearArea(int x, int y, int w) {
    gotoxy(x, y);
    for (int i = 0; i < w; i++) putchar(' ');
    fflush(stdout);
}

/* UTF-8 기반 한글 포함 문자열 출력 너비 계산
   ASCII: 1열, 2바이트 UTF-8: 1열, 3바이트(한글 등): 2열, 4바이트: 2열 */
int koreanWidth(const char *str) {
    int width = 0;
    const unsigned char *s = (const unsigned char *)str;
    while (*s) {
        if (*s < 0x80) {
            width++;
            s++;
        } else if ((*s & 0xE0) == 0xC0) {
            width++;
            s += 2;
        } else if ((*s & 0xF0) == 0xE0) {
            width += 2;
            s += 3;
        } else if ((*s & 0xF8) == 0xF0) {
            width += 2;
            s += 4;
        } else {
            s++;
        }
    }
    return width;
}
