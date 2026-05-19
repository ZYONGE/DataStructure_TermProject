#include <stddef.h>
#include "keywords.h"

/*
 * relation_table[촌수][본인 M=0/F=1][상대 M=0/F=1][방향 UP=0/SAME=1/DOWN=2]
 *
 * NULL : relation.c 에서 나이 비교 등 추가 로직으로 처리
 */
const char *relation_table[5][2][2][3] = {
    /* ── 0촌 (배우자) ─────────────────────────────────────── */
    {
        { { NULL, KW_WIFE,    NULL }, { NULL, KW_HUSBAND, NULL } }, /* M self */
        { { NULL, KW_HUSBAND, NULL }, { NULL, KW_WIFE,    NULL } }, /* F self */
    },

    /* ── 1촌 (부모 / 자녀) ────────────────────────────────── */
    {
        { { KW_FATHER, NULL, KW_SON      }, { KW_MOTHER, NULL, KW_DAUGHTER } }, /* M self */
        { { KW_FATHER, NULL, KW_SON      }, { KW_MOTHER, NULL, KW_DAUGHTER } }, /* F self */
    },

    /* ── 2촌 (조부모 / 형제자매 / 손자녀) ────────────────── */
    /* SAME(1): NULL → relation.c 에서 나이 비교 후 결정       */
    {
        { { KW_GRANDFATHER, NULL, KW_GRANDSON     }, /* M self, M target */
          { KW_GRANDMOTHER, NULL, KW_GRANDDAUGHTER } }, /* M self, F target */
        { { KW_GRANDFATHER, NULL, KW_GRANDSON     }, /* F self, M target */
          { KW_GRANDMOTHER, NULL, KW_GRANDDAUGHTER } }, /* F self, F target */
    },

    /* ── 3촌 (큰아버지/삼촌/고모/외삼촌/이모 / 조카) ──── */
    /* UP(0): NULL → relation.c 에서 부계/모계 + 나이 비교 후 결정 */
    {
        { { NULL, NULL, KW_NEPHEW }, { NULL, NULL, KW_NIECE } }, /* M self */
        { { NULL, NULL, KW_NEPHEW }, { NULL, NULL, KW_NIECE } }, /* F self */
    },

    /* ── 4촌 (사촌) ───────────────────────────────────────── */
    /* SAME(1): NULL → relation.c 에서 나이 비교 후 결정       */
    {
        { { NULL, NULL, NULL }, { NULL, NULL, NULL } }, /* M self */
        { { NULL, NULL, NULL }, { NULL, NULL, NULL } }, /* F self */
    },
};
