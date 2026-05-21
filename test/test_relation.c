/*
 * test_relation.c — 촌수 계산 / 한국어 호칭 검증 (4촌 이내, 친가)
 *
 * 컴파일:
 *   cd test
 *   gcc -std=c11 -I../include -I../src \
 *       test_relation.c \
 *       ../src/family/family_tree.c \
 *       ../src/algorithm/dfs.c \
 *       ../src/algorithm/relation.c \
 *       ../src/data/keywords.c \
 *       -o test_relation && ./test_relation
 */

#include <stdio.h>
#include <string.h>
#include "../src/family/person.h"
#include "../src/family/family_tree.h"
#include "../src/algorithm/dfs.h"
#include "../src/algorithm/relation.h"
#include "../src/data/keywords.h"

/* ── 헬퍼 ─────────────────────────────────────────────────────────── */

static int g_pass = 0, g_fail = 0;

static void check(const char *label,
                  Person *a, Person *b,
                  int expectedChonsu,
                  const char *expectedRel_a2b,
                  const char *expectedRel_b2a) {
    Person *path[MAX_PATH_LEN];
    int len = findPath(a, b, path, MAX_PATH_LEN);

    if (len < 0) {
        printf("[FAIL] %s : 경로 없음\n", label);
        g_fail++;
        return;
    }

    int chonsu      = computeChonsu(path, len);
    const char *r1  = getRelation(a, b, path, len);
    const char *r2  = getRelationReverse(a, b, path, len);

    int ok = 1;
    if (chonsu != expectedChonsu) {
        printf("[FAIL] %s : 촌수 %d (기대 %d)\n", label, chonsu, expectedChonsu);
        ok = 0;
    }
    if (strcmp(r1, expectedRel_a2b) != 0) {
        printf("[FAIL] %s : %s→%s = \"%s\" (기대 \"%s\")\n",
               label, a->name, b->name, r1, expectedRel_a2b);
        ok = 0;
    }
    if (strcmp(r2, expectedRel_b2a) != 0) {
        printf("[FAIL] %s : %s→%s = \"%s\" (기대 \"%s\")\n",
               label, b->name, a->name, r2, expectedRel_b2a);
        ok = 0;
    }
    if (ok) {
        printf("[PASS] %s : %d촌 | %s→%s=\"%s\" | %s→%s=\"%s\"\n",
               label, chonsu,
               a->name, b->name, r1,
               b->name, a->name, r2);
        g_pass++;
    } else {
        g_fail++;
    }
}

/* ── 가족 트리 ─────────────────────────────────────────────────────────
 *
 * 할아버지(M,1945)─할머니(F,1948)
 *   ├── 백부(M,1965)
 *   ├── 아버지(M,1970)─어머니(F,1972)
 *   │     ├── 나(M,2004)
 *   │     └── 여동생(F,2007)
 *   └── 고모(F,1978)
 *
 * 백부(M,1965) 자녀: 종형(M,2001), 종자(F,2006)
 * 고모(F,1978) 자녀: 고종매(F,2006)
 *
 * 배우자:
 *   할아버지↔할머니
 *   아버지↔어머니 (어머니는 blood DFS 범위 밖 — 친가 전용)
 *   백부↔백모(F,1967)
 *   고모↔고모부(M,1976)
 */

static Person *할아버지, *할머니;
static Person *백부, *백모;
static Person *아버지, *어머니;
static Person *고모, *고모부;
static Person *나, *여동생;
static Person *종형, *종자;
static Person *고종매;

static void buildTree(void) {
    할아버지 = createPerson("할아버지", 'M', 1945);
    할머니   = createPerson("할머니",   'F', 1948);
    addSpouse(할아버지, 할머니);

    백부 = createPerson("백부", 'M', 1965);
    백모 = createPerson("백모", 'F', 1967);
    addSpouse(백부, 백모);
    addChild(할아버지, 백부);

    아버지 = createPerson("아버지", 'M', 1970);
    어머니 = createPerson("어머니", 'F', 1972);
    addSpouse(아버지, 어머니);
    addChild(할아버지, 아버지);

    고모   = createPerson("고모",   'F', 1978);
    고모부 = createPerson("고모부", 'M', 1976);
    addSpouse(고모, 고모부);
    addChild(할아버지, 고모);

    나      = createPerson("나",      'M', 2004);
    여동생  = createPerson("여동생",  'F', 2007);
    addChild(아버지, 나);
    addChild(아버지, 여동생);

    종형 = createPerson("종형", 'M', 2001);
    종자 = createPerson("종자", 'F', 2006);
    addChild(백부, 종형);
    addChild(백부, 종자);

    고종매 = createPerson("고종매", 'F', 2006);
    addChild(고모, 고종매);
}

static void freeTree(void) {
    freeAll(할아버지);
    /* 어머니, 고모부, 백모는 addSpouse로만 연결 — freeAll이 경유 */
}

/* ── 테스트 ─────────────────────────────────────────────────────────── */

static void test_본인(void) {
    printf("\n=== 본인 ===\n");
    Person *path[MAX_PATH_LEN];
    int len = findPath(나, 나, path, MAX_PATH_LEN);
    const char *r = getRelation(나, 나, path, len);
    if (strcmp(r, KW_SELF) == 0) { printf("[PASS] 나→나 : %s\n", r); g_pass++; }
    else { printf("[FAIL] 나→나 : \"%s\" (기대 \"%s\")\n", r, KW_SELF); g_fail++; }
}

static void test_1촌(void) {
    printf("\n=== 1촌: 부모 ===\n");
    check("나(M)→아버지(M)",  나,     아버지, 1, KW_FATHER, KW_UNKNOWN);
    check("여동생(F)→아버지", 여동생, 아버지, 1, KW_FATHER, KW_UNKNOWN);
    /* 어머니/할머니는 blood DFS 도달 불가 (spouse 간선 미사용):
     * main.c에서 p1->spouse==p2 사전 체크로 처리 */
}

static void test_2촌_조부모(void) {
    printf("\n=== 2촌: 조부모 ===\n");
    check("나→할아버지",     나,     할아버지, 2, KW_GRANDFATHER, KW_UNKNOWN);
    check("여동생→할아버지", 여동생, 할아버지, 2, KW_GRANDFATHER, KW_UNKNOWN);
    /* 할머니는 blood DFS 도달 불가 (spouse 간선 미사용) */
}

static void test_2촌_형제(void) {
    printf("\n=== 2촌: 형제자매 ===\n");
    /* 나(M,2004) ↔ 여동생(F,2007): 나 연상 */
    check("나(M,2004)→여동생(F,2007)", 나,      여동생, 2, KW_MAE,  KW_HYUNG);
    check("여동생(F,2007)→나(M,2004)", 여동생,  나,     2, KW_HYUNG, KW_MAE);

    /* 종형(M,2001) ↔ 종자(F,2006): 종형 연상 → 종자는 연하 여성 = 매(妹) */
    check("종형(M,2001)→종자(F,2006)", 종형, 종자, 2, KW_MAE,   KW_HYUNG);
    check("종자(F,2006)→종형(M,2001)", 종자, 종형, 2, KW_HYUNG, KW_MAE);
}

static void test_3촌(void) {
    printf("\n=== 3촌: 백부/숙부/고모 ===\n");
    /* 백부(1965) vs 아버지(1970): 백부 연상 → 백부 */
    check("나→백부(1965,연상)", 나, 백부, 3, KW_BAEKBU, KW_JILNAM);
    /* 아버지(1970) 기준 고모(1978) 연하지만 gender=F → 고모 */
    check("나→고모(F)",         나, 고모, 3, KW_GOMO,   KW_JILNAM);
    check("여동생→백부",         여동생, 백부, 3, KW_BAEKBU, KW_JILNYEO);
    check("여동생→고모",         여동생, 고모, 3, KW_GOMO,   KW_JILNYEO);

    /* 숙부 케이스 확인: 아버지(1970)보다 연하인 남성 삼촌이 있어야 함
     * 현재 트리에는 백부(1965 연상)만 있어 숙부 테스트는 아래 별도 트리 불필요.
     * 대신 조건 검증: 백부가 백부(伯父)로 올바르게 나오는지 확인했으므로 생략. */
}

static void test_4촌_종형제(void) {
    printf("\n=== 4촌: 종형제 (백부 경유) ===\n");
    /* 나(M,2004) ↔ 종형(M,2001): 종형 연상 */
    check("나(M,2004)→종형(M,2001)", 나, 종형, 4, KW_JONGHYUNG, KW_JONGJE);
    /* 나(M,2004) ↔ 종자(F,2006): 종자 연하 → 역방향 종자→나 = 나(연상 M) = 종형 */
    check("나(M,2004)→종자(F,2006)", 나, 종자, 4, KW_JONGMAE, KW_JONGHYUNG);
    /* 여동생(F,2007) ↔ 종형(M,2001): 종형 연상 */
    check("여동생(F,2007)→종형(M,2001)", 여동생, 종형, 4, KW_JONGHYUNG, KW_JONGMAE);
    /* 여동생(F,2007) ↔ 종자(F,2006): 종자 연상 */
    check("여동생(F,2007)→종자(F,2006)", 여동생, 종자, 4, KW_JONGJA, KW_JONGMAE);
}

static void test_4촌_고종형제(void) {
    printf("\n=== 4촌: 고종형제 (고모 경유) ===\n");
    /* 나(M,2004) ↔ 고종매(F,2006): 나 연상 → 역방향 고종매→나 = 연상 M = 고종형 */
    check("나(M,2004)→고종매(F,2006)",     나,      고종매, 4, KW_GOJONGMAE,  KW_GOJONGHYUNG);
    /* 여동생(F,2007) ↔ 고종매(F,2006): 고종매 연상 */
    check("여동생(F,2007)→고종매(F,2006)", 여동생,  고종매, 4, KW_GOJONGJA,   KW_GOJONGMAE);
}

static void test_배우자_호칭(void) {
    printf("\n=== 배우자 후처리 테이블 ===\n");
    /* 나 → 백모: blood_target=백부, blood_hoching="백부(伯父)" → 백모(伯母) */
    Person *path[MAX_PATH_LEN];
    int len = findPath(나, 백부, path, MAX_PATH_LEN);
    const char *blood = getRelation(나, 백부, path, len);
    const char *sp    = applySpouseTable(blood);
    int ok = (strcmp(sp, KW_BAENGMO) == 0);
    if (ok) { printf("[PASS] 나→백모(배우자후처리) : %s\n", sp); g_pass++; }
    else    { printf("[FAIL] 나→백모 : \"%s\" (기대 \"%s\")\n", sp, KW_BAENGMO); g_fail++; }

    /* 나 → 고모부: blood_target=고모, hoching="고모(姑母)" → 고모부(姑母夫) */
    len   = findPath(나, 고모, path, MAX_PATH_LEN);
    blood = getRelation(나, 고모, path, len);
    sp    = applySpouseTable(blood);
    ok    = (strcmp(sp, KW_GOMOBU) == 0);
    if (ok) { printf("[PASS] 나→고모부(배우자후처리) : %s\n", sp); g_pass++; }
    else    { printf("[FAIL] 나→고모부 : \"%s\" (기대 \"%s\")\n", sp, KW_GOMOBU); g_fail++; }

    /* 역방향: 백모→나 = 생질(甥姪) (나→백부 혈족 호칭 기준으로 역방향 교체) */
    len   = findPath(나, 백부, path, MAX_PATH_LEN);
    blood = getRelation(나, 백부, path, len);
    const char *rev_sp = applyReverseSpouseTable(blood);
    ok     = (strcmp(rev_sp, KW_SAENGJIL) == 0);
    if (ok) { printf("[PASS] 백모→나(역방향배우자) : %s\n", rev_sp); g_pass++; }
    else    { printf("[FAIL] 백모→나 : \"%s\" (기대 \"%s\")\n", rev_sp, KW_SAENGJIL); g_fail++; }
}

/* ── 숙부 테스트용 보조 트리 ──────────────────────────────────────────
 * 기존 트리에 숙부(연하 남자 형제)가 없으므로 별도로 구성.          */
static void test_숙부(void) {
    printf("\n=== 3촌: 숙부 (아버지보다 연하) ===\n");

    Person *gp  = createPerson("조부_B", 'M', 1940);
    Person *fa  = createPerson("부_B",   'M', 1968);
    Person *su  = createPerson("숙부_B", 'M', 1975); /* fa 보다 연하 */
    Person *me  = createPerson("자녀_B", 'M', 2000);
    Person *kid = createPerson("조카_B", 'F', 2001);

    addChild(gp, fa);
    addChild(gp, su);
    addChild(fa, me);
    addChild(su, kid);

    check("자녀_B→숙부_B", me, su, 3, KW_SUKBU, KW_JILNAM);
    /* 역: 숙부 → 자녀_B = 질(姪) */

    freeAll(gp);
}

/* ── 메인 ───────────────────────────────────────────────────────────── */

int main(void) {
    printf("========================================\n");
    printf("  촌수 계산기 - 전체 관계 테스트 (친가)\n");
    printf("========================================\n");

    buildTree();

    test_본인();
    test_1촌();
    test_2촌_조부모();
    test_2촌_형제();
    test_3촌();
    test_4촌_종형제();
    test_4촌_고종형제();
    test_배우자_호칭();

    freeTree();

    test_숙부();

    printf("\n========================================\n");
    printf("  결과: PASS %d / FAIL %d / 합계 %d\n",
           g_pass, g_fail, g_pass + g_fail);
    printf("========================================\n");

    return g_fail > 0 ? 1 : 0;
}
