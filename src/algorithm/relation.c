#include <string.h>
#include "relation.h"
#include "dfs.h"
#include "../data/keywords.h"

typedef enum { DIR_UP = 0, DIR_SAME = 1, DIR_DOWN = 2 } Direction;

/* path[i] → path[i+1] 이동이 UP 인지 DOWN 인지 판별 */
static int isUpStep(Person *from, Person *to) {
    return (to == from->parent);
}

static int isSpouseStep(Person *from, Person *to) {
    return (to == from->spouse);
}

/* UP/DOWN 단계 수를 계산해 방향과 촌수를 결정 */
static void countSteps(Person **path, int pathLen,
                       int *upOut, int *downOut) {
    int up = 0, down = 0;
    for (int i = 0; i < pathLen - 1; i++) {
        if (isSpouseStep(path[i], path[i+1])) continue;
        if (isUpStep(path[i], path[i+1])) up++;
        else down++;
    }
    *upOut   = up;
    *downOut = down;
}

const char *getRelation(Person *self, Person *target,
                        Person **path, int pathLen) {
    if (pathLen <= 0) return KW_UNKNOWN;
    if (self == target || pathLen == 1) return KW_SELF;

    int upSteps = 0, downSteps = 0;
    countSteps(path, pathLen, &upSteps, &downSteps);

    int chonsu  = upSteps + downSteps;
    int genDiff = downSteps - upSteps;  /* 양수: target이 아래 세대 */
    int selfG   = (self->gender   == 'M') ? 0 : 1;
    int targetG = (target->gender == 'M') ? 0 : 1;

    /* ── 0촌: 배우자 ── */
    if (chonsu == 0 || (pathLen == 2 && isSpouseStep(path[0], path[1])))
        return (selfG == 0) ? KW_WIFE : KW_HUSBAND;

    /* ── 1촌 ── */
    if (chonsu == 1) {
        if (genDiff < 0) return (targetG == 0) ? KW_FATHER   : KW_MOTHER;
        else             return (targetG == 0) ? KW_SON       : KW_DAUGHTER;
    }

    /* ── 2촌 ── */
    if (chonsu == 2) {
        if (genDiff < 0)
            return (targetG == 0) ? KW_GRANDFATHER : KW_GRANDMOTHER;
        if (genDiff > 0)
            return (targetG == 0) ? KW_GRANDSON    : KW_GRANDDAUGHTER;
        /* SAME: 형제자매 — 나이 비교 */
        int older = (target->age > self->age);
        if (selfG == 0)
            return targetG == 0 ? (older ? KW_OLDER_BRO : KW_YOUNGER_BRO)
                                : (older ? KW_OLDER_SIS  : KW_YOUNGER_SIS);
        else
            return targetG == 0 ? (older ? KW_OPPA       : KW_YOUNGER_BRO)
                                : (older ? KW_UNNIE       : KW_YOUNGER_SIS);
    }

    /* ── 3촌 ── */
    if (chonsu == 3) {
        if (genDiff > 0)
            return (targetG == 0) ? KW_NEPHEW : KW_NIECE;

        if (genDiff < 0) {
            /* 부계(부의 형제자매) / 모계(모의 형제자매) 구분
               path: self → 내 부모(path[1]) → 조부모 → 삼촌/고모 */
            Person *myParent = (pathLen > 1) ? path[1] : NULL;
            int paternal = (myParent && myParent->gender == 'M');

            if (targetG == 0) { /* target 남성 */
                if (paternal) {
                    int olderThanDad = myParent && (target->age > myParent->age);
                    return olderThanDad ? KW_BIG_UNCLE : KW_UNCLE;
                }
                return KW_EXT_UNCLE;
            } else { /* target 여성 */
                return paternal ? KW_AUNT_PAT : KW_AUNT_MAT;
            }
        }
    }

    /* ── 4촌 ── */
    if (chonsu == 4) {
        if (genDiff < 0) return (targetG == 0) ? "증조할아버지" : "증조할머니";
        if (genDiff > 0) return (targetG == 0) ? "증손자"       : "증손녀";
        /* SAME: 사촌 — 나이 비교 */
        int older = (target->age > self->age);
        if (selfG == 0)
            return targetG == 0 ? (older ? KW_COUSIN_M_OLD_M : KW_COUSIN_M_YNG)
                                : (older ? KW_COUSIN_F_OLD_M  : KW_COUSIN_F_YNG);
        else
            return targetG == 0 ? (older ? KW_COUSIN_M_OLD_F : KW_COUSIN_M_YNG)
                                : (older ? KW_COUSIN_F_OLD_F  : KW_COUSIN_F_YNG);
    }

    return KW_UNKNOWN;
}

/* 역방향: target 기준으로 self 의 호칭 — 경로를 뒤집어 재호출 */
const char *getRelationReverse(Person *self, Person *target,
                               Person **path, int pathLen) {
    /* 경로 역순 배열 생성 */
    Person *rev[MAX_PATH_LEN];
    for (int i = 0; i < pathLen; i++)
        rev[i] = path[pathLen - 1 - i];
    return getRelation(target, self, rev, pathLen);
}
