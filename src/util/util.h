#ifndef UTIL_H
#define UTIL_H

/* 콘솔 색상 상수 */
#define C_DEFAULT  0
#define C_RED      1
#define C_GREEN    2
#define C_YELLOW   3
#define C_BLUE     4
#define C_MAGENTA  5
#define C_CYAN     6
#define C_WHITE    7

void gotoxy(int x, int y);
void hideCursor(void);
void showCursor(void);
void clearArea(int x, int y, int w);
int  koreanWidth(const char *str);
void setColor(int color);
void resetColor(void);
void clearScreen(void);
char readKey(void);

#endif /* UTIL_H */
