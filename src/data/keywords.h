#ifndef KEYWORDS_H
#define KEYWORDS_H

/* 한국어 호칭 상수 */
#define KW_HUSBAND        "남편"
#define KW_WIFE           "아내"

#define KW_FATHER         "아버지"
#define KW_MOTHER         "어머니"
#define KW_SON            "아들"
#define KW_DAUGHTER       "딸"

#define KW_GRANDFATHER    "할아버지"
#define KW_GRANDMOTHER    "할머니"
#define KW_GRANDSON       "손자"
#define KW_GRANDDAUGHTER  "손녀"

#define KW_OLDER_BRO      "형"       /* M → M 연상 */
#define KW_YOUNGER_BRO    "남동생"
#define KW_OLDER_SIS      "누나"     /* M → F 연상 */
#define KW_YOUNGER_SIS    "여동생"
#define KW_OPPA           "오빠"     /* F → M 연상 */
#define KW_UNNIE          "언니"     /* F → F 연상 */

#define KW_BIG_UNCLE      "큰아버지" /* 부계, 아버지보다 연상 */
#define KW_UNCLE          "삼촌"     /* 부계, 아버지보다 연하 */
#define KW_EXT_UNCLE      "외삼촌"   /* 모계 */
#define KW_AUNT_PAT       "고모"     /* 부계 */
#define KW_AUNT_MAT       "이모"     /* 모계 */
#define KW_NEPHEW         "조카"
#define KW_NIECE          "조카딸"

#define KW_COUSIN_M_OLD_M "사촌형"        /* M → M 연상 */
#define KW_COUSIN_M_YNG   "사촌동생"      /* → M 연하 (공통) */
#define KW_COUSIN_F_OLD_M "사촌누나"      /* M → F 연상 */
#define KW_COUSIN_F_OLD_F "사촌언니"      /* F → F 연상 */
#define KW_COUSIN_M_OLD_F "사촌오빠"      /* F → M 연상 */
#define KW_COUSIN_F_YNG   "사촌여동생"    /* → F 연하 (공통) */

#define KW_SELF           "본인"
#define KW_UNKNOWN        "미상"

/*
 * 기본 매핑 테이블
 * [촌수 0-4][본인성별 M=0,F=1][상대성별 M=0,F=1][세대방향 UP=0,SAME=1,DOWN=2]
 *
 * SAME 세대에서 연상/연하 구분이 필요한 경우(2촌 형제, 4촌 사촌)는 NULL을 저장하고
 * relation.c 에서 나이 비교 후 KW_* 상수를 직접 반환한다.
 */
extern const char *relation_table[5][2][2][3];

#endif /* KEYWORDS_H */
