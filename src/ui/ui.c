#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "ui.h"
#include "../util/util.h"

/* ── 박스 그리기 문자 (UTF-8) ────────────────────────────────────── */
#define BX_TL  "╔"
#define BX_TR  "╗"
#define BX_BL  "╚"
#define BX_BR  "╝"
#define BX_H   "═"
#define BX_V   "║"
#define BX_ML  "╠"
#define BX_MR  "╣"
#define BX_TT  "╦"
#define BX_BT  "╩"

static void hline(int len) {
    for (int i = 0; i < len; i++) printf(BX_H);
}

void drawFrame(void) {
    int W = UI_WIDTH;

    /* 행 0: 상단 */
    gotoxy(0, 0);
    printf(BX_TL); hline(W - 2); printf(BX_TR);

    /* 행 1~2: 조부모 세대 */
    for (int r = 1; r <= 2; r++) {
        gotoxy(0,     r); printf(BX_V);
        gotoxy(W - 1, r); printf(BX_V);
    }
    gotoxy(2, 1); printf("[ 조부모 세대 ]");

    /* 행 3: 구분선 */
    gotoxy(0, 3); printf(BX_ML); hline(W - 2); printf(BX_MR);

    /* 행 4~5: 부모 세대 */
    for (int r = 4; r <= 5; r++) {
        gotoxy(0,     r); printf(BX_V);
        gotoxy(W - 1, r); printf(BX_V);
    }
    gotoxy(2, 4); printf("[ 부모 세대 ]");

    /* 행 6: 구분선 */
    gotoxy(0, 6); printf(BX_ML); hline(W - 2); printf(BX_MR);

    /* 행 7~8: 본인 세대 */
    for (int r = 7; r <= 8; r++) {
        gotoxy(0,     r); printf(BX_V);
        gotoxy(W - 1, r); printf(BX_V);
    }
    gotoxy(2, 7); printf("[ 본인 세대 ]");

    /* 행 9: T-junction 구분선 ╠══╦══╣ */
    gotoxy(0, 9);
    printf(BX_ML);
    for (int c = 1; c < COL_DIVIDER; c++) printf(BX_H);
    printf(BX_TT);
    for (int c = COL_DIVIDER + 1; c < W - 1; c++) printf(BX_H);
    printf(BX_MR);

    /* 행 10~27: 메뉴 + 메인 영역 세로선 */
    for (int r = ROW_BOTTOM_START; r <= ROW_BOTTOM_END; r++) {
        gotoxy(0,           r); printf(BX_V);
        gotoxy(COL_DIVIDER, r); printf(BX_V);
        gotoxy(W - 1,       r); printf(BX_V);
    }

    /* 행 28: 하단 */
    gotoxy(0, 28);
    printf(BX_BL);
    for (int c = 1; c < COL_DIVIDER; c++) printf(BX_H);
    printf(BX_BT);
    for (int c = COL_DIVIDER + 1; c < W - 1; c++) printf(BX_H);
    printf(BX_BR);

    /* 메뉴 헤더 + 항목 */
    gotoxy(COL_MENU_START, ROW_BOTTOM_START);
    printf("[ MENU ]");

    const char *items[] = {
        "1. 추가",
        "2. 수정",
        "3. 삭제",
        "4. 조회",
        "----------",
        "5. 촌수계산",
        "6. 호칭출력",
        "----------",
        "7. 초기화",
        "0. 종료",
    };
    int nItems = (int)(sizeof(items) / sizeof(items[0]));
    for (int i = 0; i < nItems; i++) {
        gotoxy(COL_MENU_START, ROW_BOTTOM_START + 1 + i);
        printf("%s", items[i]);
    }

    /* 메인 영역 헤더 */
    gotoxy(MAIN_X, ROW_BOTTOM_START);
    printf("[ MAIN ]");

    fflush(stdout);
}

void initUI(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    clearScreen();
    hideCursor();
    drawFrame();
}

void updateMenuCursor(int selected) {
    int promptRow = ROW_BOTTOM_START + 11;
    clearArea(COL_MENU_START, promptRow, COL_DIVIDER - COL_MENU_START - 1);
    gotoxy(COL_MENU_START, promptRow);
    setColor(C_YELLOW);
    printf("선택 > %d", selected);
    resetColor();
    fflush(stdout);
}

/* ── MAIN 영역 ────────────────────────────────────────────────── */

void clearMainArea(void) {
    for (int r = ROW_BOTTOM_START; r <= ROW_BOTTOM_END; r++)
        clearArea(MAIN_X, r, MAIN_W);
}

void printMain(int row, const char *fmt, ...) {
    int absRow = MAIN_Y + row;
    if (absRow > ROW_BOTTOM_END) return;
    clearArea(MAIN_X, absRow, MAIN_W);
    gotoxy(MAIN_X, absRow);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
}
