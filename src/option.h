#ifndef OPTION_H
#define OPTION_H

typedef enum {
    FAMILY_SIDE_NONE     = 0,
    FAMILY_SIDE_PATERNAL = 1,   /* 친가 (아버지 쪽) */
    FAMILY_SIDE_MATERNAL = 2,   /* 외가 (어머니 쪽) */
} FamilySide;

/*
 * 프로그램 시작 시 사용자에게 친가/외가를 선택받는다.
 * 선택 결과(FAMILY_SIDE_PATERNAL or FAMILY_SIDE_MATERNAL)를 반환한다.
 */
FamilySide selectFamilySide(void);

/*
 * 현재 선택된 가문을 반환한다.
 */
FamilySide getFamilySide(void);

/*
 * 선택된 가문 이름 문자열을 반환한다. ("친가" / "외가")
 */
const char *getFamilySideName(void);

#endif /* OPTION_H */
