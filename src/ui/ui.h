#ifndef UI_H
#define UI_H

/* ── 화면 레이아웃 상수 ──────────────────────────────────────────
 *
 *  UI_TOP 만큼 아래에서 시작하므로 터미널 위쪽 여백이 생긴다.
 *
 *  Row UI_TOP+0  : 상단 테두리 ╔═══╗
 *  Row UI_TOP+1  : 조부모 세대 헤더
 *  Row UI_TOP+2  : 조부모 덱 뷰
 *  Row UI_TOP+3  : 구분선 ╠═══╣
 *  Row UI_TOP+4  : 부모 세대 헤더
 *  Row UI_TOP+5  : 부모 덱 뷰
 *  Row UI_TOP+6  : 구분선 ╠═══╣
 *  Row UI_TOP+7  : 본인 세대 헤더
 *  Row UI_TOP+8  : 본인 덱 뷰
 *  Row UI_TOP+9  : 구분선 ╠══╦══╣
 *  Row UI_TOP+10 : MENU 헤더 / MAIN 헤더
 *  Row UI_TOP+11~27 : MENU 내용 / MAIN 내용
 *  Row UI_TOP+28 : 하단 테두리 ╚══╩══╝
 */

#define UI_TOP        2           /* 상단 여백 (행 수) */
#define UI_WIDTH      80
#define UI_HEIGHT     (29 + UI_TOP)

/* 덱 뷰 행 (내용 출력 행) */
#define ROW_GRAND_DEQUE   (UI_TOP + 2)
#define ROW_PARENT_DEQUE  (UI_TOP + 5)
#define ROW_SELF_DEQUE    (UI_TOP + 8)

/* 하단 영역 */
#define ROW_BOTTOM_START  (UI_TOP + 10)
#define ROW_BOTTOM_END    (UI_TOP + 27)

#define COL_MENU_START    1
#define COL_MENU_END      15
#define COL_DIVIDER       16
#define COL_MAIN_START    17

/* 메인 영역 내용 시작 좌표 */
#define MAIN_X  (COL_MAIN_START + 1)
#define MAIN_Y  (ROW_BOTTOM_START + 1)
#define MAIN_W  (UI_WIDTH - COL_MAIN_START - 2)

void initUI(void);           /* 화면 초기화 + 테두리 1회 출력 */
void drawFrame(void);        /* 테두리 전체 재출력 */
void updateMenuCursor(int selected); /* 선택 항목 하이라이트 */

/* MAIN 영역 출력 */
void clearMainArea(void);
void printMain(int row, const char *fmt, ...); /* row: 0-base (MAIN 영역 내) */

#endif /* UI_H */
