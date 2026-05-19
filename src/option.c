#include <stdio.h>
#include "option.h"
#include "ui/ui.h"
#include "ui/deque_view.h"
#include "util/util.h"

static FamilySide g_familySide = FAMILY_SIDE_NONE;

FamilySide selectFamilySide(void) {
    clearMainArea();
    printMain(0, "=== 가문 선택 ===");
    printMain(1, "어느 가문의 가족 구성도를 입력하시겠습니까?");
    printMain(2, "  1. 친가 (아버지 쪽)");
    printMain(3, "  2. 외가 (어머니 쪽)");
    printMain(5, "선택 (1 또는 2):");

    while (1) {
        gotoxy(MAIN_X, MAIN_Y + 6);
        setColor(C_GREEN);
        printf("> ");
        resetColor();
        showCursor();
        fflush(stdout);

        char ch = readKey();
        hideCursor();

        if (ch == '1') {
            g_familySide = FAMILY_SIDE_PATERNAL;
            printMain(7, "친가 모드로 시작합니다.");
            break;
        }
        if (ch == '2') {
            g_familySide = FAMILY_SIDE_MATERNAL;
            printMain(7, "외가 모드로 시작합니다.");
            break;
        }
        printMain(6, "1 또는 2를 눌러주세요.");
    }

    return g_familySide;
}

FamilySide getFamilySide(void) {
    return g_familySide;
}

const char *getFamilySideName(void) {
    switch (g_familySide) {
    case FAMILY_SIDE_PATERNAL: return "친가";
    case FAMILY_SIDE_MATERNAL: return "외가";
    default:                   return "미선택";
    }
}
