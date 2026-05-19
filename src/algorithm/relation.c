#include <string.h>
#include "relation.h"
#include "dfs.h"
#include "../data/keywords.h"

static int isUpStep(Person *from, Person *to) {
    return (to == from->parent);
}

static int isSpouseStep(Person *from, Person *to) {
    return (to == from->spouse);
}

/*
 * 경로의 UP/DOWN 단계 수를 계산한다.
 * DFS가 형제 포인터를 사용하지 않으므로 모든 이동은
 * 부모(up) / 배우자(skip) / 자식(down) 중 하나이다.
 */
static void countSteps(Person **path, int pathLen,
                       int *upOut, int *downOut) {
    int up = 0, down = 0;
    for (int i = 0; i < pathLen - 1; i++) {
        Person *from = path[i];
        Person *to   = path[i+1];
        if (isSpouseStep(from, to)) continue;
        if (isUpStep(from, to))     up++;
        else                        down++;
    }
    *upOut   = up;
    *downOut = down;
}

/*
 * self 기준으로 경로가 외가(어머니 쪽)인지 판별한다.
 * self->parent가 남성이면 그 배우자(어머니)가 경로에 포함되면 외가.
 * self->parent가 여성이면 이미 어머니가 직접 부모이므로 외가.
 */
static int isMaternalPath(Person *self, Person **path, int pathLen) {
    if (!self || !self->parent) return 0;

    Person *motherSide;
    if (self->parent->gender == 'M') {
        motherSide = self->parent->spouse;   /* 어머니 */
    } else {
        motherSide = self->parent;           /* 직접 어머니가 부모 */
    }
    if (!motherSide) return 0;

    for (int i = 1; i < pathLen; i++)
        if (path[i] == motherSide) return 1;
    return 0;
}

int computeChonsu(Person **path, int pathLen) {
    if (pathLen <= 1) return 0;
    if (pathLen == 2 && path[0]->spouse == path[1]) return 0;
    int up = 0, down = 0;
    countSteps(path, pathLen, &up, &down);
    return up + down;
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
    int maternal = isMaternalPath(self, path, pathLen);

    /* ── 0촌: 배우자 ── */
    if (chonsu == 0 || (pathLen == 2 && isSpouseStep(path[0], path[1])))
        return (selfG == 0) ? KW_WIFE : KW_HUSBAND;

    /* ── 1촌 ── */
    if (chonsu == 1) {
        if (genDiff < 0) return (targetG == 0) ? KW_FATHER : KW_MOTHER;
        else             return (targetG == 0) ? KW_SON    : KW_DAUGHTER;
    }

    /* ── 2촌 ── */
    if (chonsu == 2) {
        if (genDiff < 0) {
            if (maternal)
                return (targetG == 0) ? "외할아버지" : "외할머니";
            return (targetG == 0) ? KW_GRANDFATHER : KW_GRANDMOTHER;
        }
        if (genDiff > 0)
            return (targetG == 0) ? KW_GRANDSON : KW_GRANDDAUGHTER;
        /* genDiff == 0: 형제자매 — 나이 비교 */
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
        if (genDiff == 3)
            return (targetG == 0) ? "증손자" : "증손녀";
        if (genDiff == 1)
            return (targetG == 0) ? KW_NEPHEW : KW_NIECE;
        if (genDiff == -3)
            return (targetG == 0) ? "증조할아버지" : "증조할머니";
        if (genDiff == -1) {
            if (targetG == 0) {
                if (!maternal) {
                    /* 부계: path[1]이 내 부모 */
                    Person *myParent = (pathLen > 1) ? path[1] : NULL;
                    int olderThanDad = myParent && (target->age > myParent->age);
                    return olderThanDad ? KW_BIG_UNCLE : KW_UNCLE;
                }
                return KW_EXT_UNCLE;  /* 외삼촌 */
            } else {
                return maternal ? KW_AUNT_MAT : KW_AUNT_PAT;
            }
        }
    }

    /* ── 4촌 ── */
    if (chonsu == 4) {
        if (genDiff == 4)
            return (targetG == 0) ? "고손자" : "고손녀";
        if (genDiff == 2)
            return (targetG == 0) ? "종손" : "종손녀";
        if (genDiff == 0) {
            int older = (target->age > self->age);
            if (selfG == 0)
                return targetG == 0 ? (older ? KW_COUSIN_M_OLD_M : KW_COUSIN_M_YNG)
                                    : (older ? KW_COUSIN_F_OLD_M  : KW_COUSIN_F_YNG);
            else
                return targetG == 0 ? (older ? KW_COUSIN_M_OLD_F : KW_COUSIN_M_YNG)
                                    : (older ? KW_COUSIN_F_OLD_F  : KW_COUSIN_F_YNG);
        }
        if (genDiff == -2) {
            if (targetG == 0) {
                Person *grandpa = (pathLen > 2) ? path[2] : NULL;
                if (grandpa && target->age > grandpa->age)
                    return "큰할아버지";
                return "작은할아버지";
            } else {
                return "대고모";
            }
        }
        if (genDiff == -4)
            return (targetG == 0) ? "고조할아버지" : "고조할머니";
    }

    return KW_UNKNOWN;
}

const char *getRelationReverse(Person *self, Person *target,
                               Person **path, int pathLen) {
    Person *rev[MAX_PATH_LEN];
    for (int i = 0; i < pathLen; i++)
        rev[i] = path[pathLen - 1 - i];
    return getRelation(target, self, rev, pathLen);
}

const char *getRelationMessage(const char *relation) {
    if (!relation) return NULL;
    if (strcmp(relation, "아버지") == 0)         return "아버지 이제야 깨달아요";
    if (strcmp(relation, "어머니") == 0)         return "어머니께 효도합시다.";
    if (strcmp(relation, "아들") == 0)           return "대한의 건아이어라";
    if (strcmp(relation, "딸") == 0)             return "딸은 보물이죠.";
    if (strcmp(relation, "할아버지") == 0 ||
        strcmp(relation, "외할아버지") == 0)     return "조부님의 지혜.";
    if (strcmp(relation, "할머니") == 0 ||
        strcmp(relation, "외할머니") == 0)       return "할머니 손맛이 그리운 날이 있습니다";
    if (strcmp(relation, "증조할아버지") == 0 ||
        strcmp(relation, "증조할머니") == 0)     return "3대가 함께하는 행복";
    if (strcmp(relation, "형") == 0)             return "이스터에그: 저는 형이 2명이 있습니다.";
    if (strcmp(relation, "누나") == 0)           return "누나!!!";
    if (strcmp(relation, "오빠") == 0)           return "오빠가 있어 든든합니다.";
    if (strcmp(relation, "언니") == 0)           return "언니와 함께라면.";
    if (strcmp(relation, "남동생") == 0 ||
        strcmp(relation, "여동생") == 0)         return "동생을 괴롭히지 맙시다.";
    if (strcmp(relation, "손자") == 0 ||
        strcmp(relation, "손녀") == 0)           return "손주가 조부모의 기쁨이라.";
    if (strcmp(relation, "큰아버지") == 0)       return "사실 요즘 백부라는 말을 누가 쓰겠어요?";
    if (strcmp(relation, "삼촌") == 0)           return "조카 사랑은 삼촌이 한다.";
    if (strcmp(relation, "고모") == 0)           return "고모고모 열매";
    if (strcmp(relation, "외삼촌") == 0)         return "외삼촌은 왜 외백부가 아닐까요?";
    if (strcmp(relation, "이모") == 0)           return "고모보다는 이모가 더 친숙한 어감이 있습니다.";
    if (strcmp(relation, "조카") == 0 ||
        strcmp(relation, "조카딸") == 0)         return "조카 사랑은 삼촌이 합니다.";
    if (strstr(relation, "사촌") != NULL)        return "사촌이 땅을 사면? 배가 아프다고요!";
    if (strcmp(relation, "남편") == 0)           return "남편을 사랑할 것.";
    if (strcmp(relation, "아내") == 0)           return "아내를 사랑할 것.";
    return NULL;
}
