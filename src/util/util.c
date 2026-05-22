#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

/* ══════════════════════════════════════════════════════════════════
 *  Windows 구현
 * ══════════════════════════════════════════════════════════════════ */
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600   /* Vista+: CONSOLE_FONT_INFOEX */
#endif
#include <windows.h>
#include <wchar.h>
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
char readKey(void)     { return (char)_getch(); }

int getTermCols(void) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}
int getTermRows(void) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

void setupTerminal(void) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    SetConsoleTitleA("\xEC\xB4\x8C\xEC\x88\x98 \xEA\xB3\x84\xEC\xB0\xB8\xEA\xB8\xB0");

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    /* 한글 폰트 설정: 굴림체는 CJK 문자를 2칸 너비로 올바르게 렌더링 */
    CONSOLE_FONT_INFOEX cfi;
    memset(&cfi, 0, sizeof(cfi));
    cfi.cbSize      = sizeof(cfi);
    cfi.dwFontSize.Y = 18;
    cfi.FontFamily  = FF_DONTCARE;
    cfi.FontWeight  = FW_NORMAL;
    wcsncpy(cfi.FaceName, L"굴림체", LF_FACESIZE - 1); /* 굴림체 */
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);

    /* VTP 활성화: ANSI 이스케이프 시퀀스 지원 */
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode))
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    /* 버퍼를 창보다 먼저 크게 잡아야 창 크기 조정이 성공한다 */
    COORD bufSize = { 80, 200 };
    SetConsoleScreenBufferSize(hOut, bufSize);

    /* UI 최소 크기: 80열 × 32행 */
    SMALL_RECT winRect = { 0, 0, 79, 31 };
    SetConsoleWindowInfo(hOut, TRUE, &winRect);
}
void restoreTerminal(void) { showCursor(); resetColor(); }
void flushInput(void)      { FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)); }

/* ══════════════════════════════════════════════════════════════════
 *  macOS / POSIX 구현 (ANSI escape codes)
 * ══════════════════════════════════════════════════════════════════ */
#else
#include <locale.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>

/* 시그널 핸들러: Ctrl+C 등에서 커서/색상 복원 후 종료 */
static void sigHandler(int sig) {
    (void)sig;
    printf("\033[?25h\033[0m\n");   /* 커서 표시, 색상 리셋 */
    fflush(stdout);
    exit(0);
}

void setupTerminal(void) {
    setlocale(LC_ALL, "");          /* UTF-8 한국어 출력 활성화 */
    signal(SIGINT,  sigHandler);
    signal(SIGTERM, sigHandler);
}

void restoreTerminal(void) {
    showCursor();
    resetColor();
    fflush(stdout);
}

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

void hideCursor(void) { printf("\033[?25l"); fflush(stdout); }
void showCursor(void) { printf("\033[?25h"); fflush(stdout); }

static const int ansi_fg[] = {37, 31, 32, 33, 34, 35, 36, 97};

void setColor(int color) {
    int idx = (color >= 0 && color < 8) ? color : 0;
    printf("\033[%dm", ansi_fg[idx]);
    fflush(stdout);
}
void resetColor(void) { printf("\033[0m"); fflush(stdout); }
void clearScreen(void) { printf("\033[2J\033[H"); fflush(stdout); }

/* 단일 키 읽기: raw/noecho 모드에서 한 바이트 읽은 뒤 복원.
   ESC 시퀀스(방향키 등)의 나머지 바이트는 드레인한다.
   stdin 이 TTY 가 아닌 경우(파이프/파일)에도 안전하게 동작한다. */
char readKey(void) {
    /* 비-TTY stdin (파이프/파일): raw 모드 불필요.
       메뉴 키 '1'\n 에서 '1' 을 읽고, 뒤따르는 '\n' 은 삼켜서
       이후 fgets 가 빈 줄을 읽지 않도록 한다. */
    if (!isatty(STDIN_FILENO)) {
        int c = getchar();
        if (c == EOF) return '\0';
        if (c == '\n') return '\n';          /* Enter 자체 */
        int nxt = getchar();                 /* 뒤따르는 '\n' 소비 */
        if (nxt != '\n' && nxt != EOF)
            ungetc(nxt, stdin);             /* '\n' 이 아니면 되돌리기 */
        return (char)c;
    }

    struct termios old, raw;
    tcgetattr(STDIN_FILENO, &old);
    raw = old;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    unsigned char ch = 0;
    ssize_t n = read(STDIN_FILENO, &ch, 1);
    if (n <= 0) ch = 0;   /* EOF 또는 오류 */

    /* ESC 시퀀스 잔여 바이트 소비 (방향키, F-키 등) */
    if (ch == 0x1B) {
        fd_set fds;
        struct timeval tv = {0, 50000}; /* 50ms 대기 */
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        while (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
            unsigned char tmp;
            if (read(STDIN_FILENO, &tmp, 1) != 1) break;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            tv.tv_usec = 10000;
        }
        ch = 0; /* ESC 시퀀스 → 0 반환, default 케이스에서 무시 */
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return (char)ch;
}

int getTermCols(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) return w.ws_col;
    return 80;
}
int getTermRows(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) return w.ws_row;
    return 24;
}

/* stdin 잔여 데이터 드레인 (fgets 전 호출) */
void flushInput(void) {
    tcflush(STDIN_FILENO, TCIFLUSH);
}

#endif /* _WIN32 */

/* ══════════════════════════════════════════════════════════════════
 *  공통 구현
 * ══════════════════════════════════════════════════════════════════ */

void clearArea(int x, int y, int w) {
    gotoxy(x, y);
    for (int i = 0; i < w; i++) putchar(' ');
    fflush(stdout);
}

/* UTF-8 문자열의 터미널 표시 너비 계산
   ASCII: 1열 / 2바이트: 1열 / 3바이트(한글 포함): 2열 / 4바이트: 2열 */
int koreanWidth(const char *str) {
    int width = 0;
    const unsigned char *s = (const unsigned char *)str;
    while (*s) {
        if      (*s < 0x80)            { width++;  s++;  }
        else if ((*s & 0xE0) == 0xC0)  { width++;  s += 2; }
        else if ((*s & 0xF0) == 0xE0)  { width += 2; s += 3; }
        else if ((*s & 0xF8) == 0xF0)  { width += 2; s += 4; }
        else                           { s++; }
    }
    return width;
}
