#include <string.h>
#include "relation.h"
#include "dfs.h"
#include "../data/keywords.h"

/* ── LCA 추출 ─────────────────────────────────────────────────────────
 *
 * design.md §10 알고리즘:
 *   X = 나 → LCA 올라간 세대 수
 *   Y = LCA → target 내려간 세대 수
 *
 * 판별 규칙 (경험적):
 *   - 연속된 up-step 이 경로 끝까지면 → all-up (조상 직계) → lca_idx = 0
 *   - up-step 이 전혀 없으면           → all-down (자손 직계) → lca_idx = pathLen-1
 *   - 혼합(방계)                       → lca_idx = (마지막 up-step 인덱스) + 1
 *
 * up-step: path[i+1] == path[i]->parent
 */
static int find_lca_idx(Person **path, int pathLen) {
    int last_up = -1;
    for (int i = 0; i < pathLen - 1; i++) {
        if (path[i + 1] == path[i]->parent)
            last_up = i;
        else
            break;
    }
    if (last_up == pathLen - 2) return 0;           /* all-up: 조상 직계 */
    if (last_up == -1)          return pathLen - 1; /* all-down: 자손 직계 */
    return last_up + 1;                             /* 방계: 전환점 */
}

/* ── 촌수 계산 ──────────────────────────────────────────────────────── */

int computeChonsu(Person **path, int pathLen) {
    if (pathLen <= 1) return 0;
    int lca = find_lca_idx(path, pathLen);
    int X   = lca;
    int Y   = pathLen - 1 - lca;
    return X + Y;
}

/* ── 배우자 후처리 테이블 (design.md §8) ──────────────────────────── */

const char *applySpouseTable(const char *h) {
    if (!h) return KW_UNKNOWN;
    if (strcmp(h, KW_FATHER)      == 0) return KW_MOTHER;
    if (strcmp(h, KW_GRANDFATHER) == 0) return KW_GRANDMOTHER;
    if (strcmp(h, KW_BAEKBU)      == 0) return KW_BAENGMO;
    if (strcmp(h, KW_SUKBU)       == 0) return KW_SUNGMO;
    if (strcmp(h, KW_GOMO)        == 0) return KW_GOMOBU;
    return KW_UNKNOWN;
}

const char *applyReverseSpouseTable(const char *h) {
    if (!h) return KW_UNKNOWN;
    /* 백부/숙부/고모의 배우자(백모/숙모/고모부)가 나를 부르는 호칭 */
    if (strcmp(h, KW_BAEKBU) == 0) return KW_SAENGJIL;
    if (strcmp(h, KW_SUKBU)  == 0) return KW_SAENGJIL;
    if (strcmp(h, KW_GOMO)   == 0) return KW_SAENGJIL;
    return KW_UNKNOWN;
}

/* ── 핵심: 경로 패턴 매칭 → 호칭 결정 ───────────────────────────────
 *
 * design.md §1~§4 의 조건을 순서대로 검사한다.
 * 구현 범위: 조부모~본인(3세대), 친가, 4촌 이내.
 */
const char *getRelation(Person *me, Person *target,
                        Person **path, int pathLen) {
    if (pathLen <= 0) return KW_UNKNOWN;
    if (me == target || pathLen == 1) return KW_SELF;

    int lca_idx   = find_lca_idx(path, pathLen);
    int X         = lca_idx;
    int Y         = pathLen - 1 - lca_idx;
    Person **up   = &path[1];  /* up_nodes[0..X-1] */

    int tM = (target->gender == 'M');
    int mM = (me->gender     == 'M');

    /* ── §1-1 상향 직계 (X=0, Y>0) ─────────────────────────────── */
    if (X == 0) {
        if (Y == 1) return tM ? KW_FATHER : KW_MOTHER;
        if (Y == 2) return tM ? KW_GRANDFATHER : KW_GRANDMOTHER;
        return KW_UNKNOWN;
    }

    /* ── §1-2 하향 직계 (X>0, Y=0) — 역방향 계산 시 진입 ────────── */
    if (Y == 0) {
        if (X == 1) return tM ? KW_SON       : KW_DAUGHTER;
        if (X == 2) return tM ? KW_GRANDSON  : KW_GRANDDAUGHTER;
        return KW_UNKNOWN;
    }

    /* ── §3-1 동항렬 2촌 (X=1, Y=1) ────────────────────────────── */
    if (X == 1 && Y == 1) {
        int older = (target->birth_year < me->birth_year);
        if (tM) return older ? KW_HYUNG : KW_JE;
        else    return older ? KW_JA    : KW_MAE;
    }

    /* ── §2-1 부모 항렬 3촌 (X=2, Y=1) — 백부/숙부/고모 ────────── */
    if (X == 2 && Y == 1) {
        if (!tM) return KW_GOMO;
        /* up_nodes[0] = 나의 직계 부모 */
        Person *my_parent = up[0]; /* path[1] */
        return (target->birth_year < my_parent->birth_year) ? KW_BAEKBU : KW_SUKBU;
    }

    /* ── §4-1 자녀 항렬 3촌 역방향 (X=1, Y=2) — 질/질녀 ─────────
     * 직접 쿼리 범위 외이지만 getRelationReverse 내부에서 진입.     */
    if (X == 1 && Y == 2) {
        return tM ? KW_JILNAM : KW_JILNYEO;
    }

    /* ── §3-2 동항렬 4촌 (X=2, Y=2) — 종형제 / 고종형제 ─────────── */
    if (X == 2 && Y == 2) {
        /* LCA 양쪽 바로 아래 노드를 모두 확인:
         * 역방향 경로에서는 dn0 가 아닌 up1 이 고모일 수 있으므로
         * 둘 중 하나라도 F 면 고모 경유(고종형제) 계열로 판정한다. */
        Person *dn0 = path[lca_idx + 1]; /* target 쪽 분기점 */
        Person *up1 = path[lca_idx - 1]; /* 나 쪽 분기점     */
        int through_female = (dn0->gender == 'F' || up1->gender == 'F');
        int older = (target->birth_year < me->birth_year);

        if (!through_female) {
            /* 백부 또는 숙부 경유 → 종형제 계열 */
            if (tM) return older ? KW_JONGHYUNG : KW_JONGJE;
            else    return older ? KW_JONGJA    : KW_JONGMAE;
        } else {
            /* 고모 경유 → 고종형제 계열 */
            if (tM) return older ? KW_GOJONGHYUNG : KW_GOJONGJE;
            else    return older ? KW_GOJONGJA    : KW_GOJONGMAE;
        }
    }

    (void)mM;
    return KW_UNKNOWN;
}

/* ── 역방향 ─────────────────────────────────────────────────────────── */

const char *getRelationReverse(Person *me, Person *target,
                               Person **path, int pathLen) {
    Person *rev[MAX_PATH_LEN];
    for (int i = 0; i < pathLen; i++)
        rev[i] = path[pathLen - 1 - i];
    return getRelation(target, me, rev, pathLen);
}

/* ── 관계 메시지 ─────────────────────────────────────────────────────── */

const char *getRelationMessage(const char *r) {
    if (!r) return NULL;
    if (strcmp(r, KW_FATHER)      == 0) return "아버지 이제야 깨달아요";
    if (strcmp(r, KW_MOTHER)      == 0) return "어머니께 효도합시다.";
    if (strcmp(r, KW_GRANDFATHER) == 0) return "조부. 조부 위로 증조부. 증조부 위에 고조부...";
    if (strcmp(r, KW_GRANDMOTHER) == 0) return "할머니 손맛이 그리운 날이 있다";
    if (strcmp(r, KW_HYUNG)       == 0) return "이스터에그: 저는 형이 2명이 있습니다.";
    if (strcmp(r, KW_JE)          == 0) return "동생을 괴롭히지 맙시다.";
    if (strcmp(r, KW_JA)          == 0) return "누나!!!";
    if (strcmp(r, KW_MAE)         == 0) return "동생을 괴롭히지 맙시다.";
    if (strcmp(r, KW_BAEKBU)      == 0) return "요즘 백부라는 말을 누가 쓰겠어요?";
    if (strcmp(r, KW_SUKBU)       == 0) return "숙부의 숙은 아저씨라는 뜻입니다.";
    if (strcmp(r, KW_GOMO)        == 0) return "시어미 고, 어머니 모";
    if (strcmp(r, KW_BAENGMO)     == 0) return "맏이 백, 어머니 모";
    if (strcmp(r, KW_SUNGMO)      == 0) return "개인적으로 숙모 보다는 작은 엄마가 더 맘에 드는 호칭입니다.";
    if (strcmp(r, KW_GOMOBU)      == 0) return "GO! 모부";
    if (strstr(r, "종") != NULL)        return "사촌은 촌수 인덱스 그 자체가 호칭인 거의 유일한 경우입니다.";
    return NULL;
}
