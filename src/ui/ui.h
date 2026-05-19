#ifndef UI_H
#define UI_H

/* ── 화면 레이아웃 상수 ──────────────────────────────────────────
 *
 *  열(x): 0 ~ UI_WIDTH-1
 *  행(y): 0 ~ UI_HEIGHT-1
 *
 *  Row 0        : 상단 테두리 ╔═══╗
 *  Row 1        : 조부모 세대 헤더
 *  Row 2        : 조부모 덱 뷰
 *  Row 3        : 구분선 ╠═══╣
 *  Row 4        : 부모 세대 헤더
 *  Row 5        : 부모 덱 뷰
 *  Row 6        : 구분선 ╠═══╣
 *  Row 7        : 본인 세대 헤더
 *  Row 8        : 본인 덱 뷰
 *  Row 9        : 구분선 ╠══╦══╣
 *  Row 10       : MENU 헤더 / MAIN 헤더
 *  Row 11~27    : MENU 내용 / MAIN 내용
 *  Row 28       : 하단 테두리 ╚══╩══╝
 */

#define UI_WIDTH      80
#define UI_HEIGHT     29

/* 덱 뷰 행 (내용 출력 행) */
#define ROW_GRAND_DEQUE   2
#define ROW_PARENT_DEQUE  5
#define ROW_SELF_DEQUE    8

/* 하단 영역 */
#define ROW_BOTTOM_START  10
#define ROW_BOTTOM_END    27

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
