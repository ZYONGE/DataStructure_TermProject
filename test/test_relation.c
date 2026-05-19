/*
 * test_relation.c
 * 촌수 계산 / 한국어 호칭 정상 작동 테스트
 * UI 없이 로직 레이어만 검증한다.
 *
 * 컴파일 (MSYS2 MinGW64 or macOS):
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

/* ── 테스트 헬퍼 ─────────────────────────────────────────────────── */

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

    int chonsu = computeChonsu(path, len);
    const char *rel_a2b = getRelation(a, b, path, len);
    const char *rel_b2a = getRelationReverse(a, b, path, len);

    int ok = 1;
    if (chonsu != expectedChonsu) {
        printf("[FAIL] %s : 촌수 %d (기대 %d)\n", label, chonsu, expectedChonsu);
        ok = 0;
    }
    if (strcmp(rel_a2b, expectedRel_a2b) != 0) {
        printf("[FAIL] %s : %s→%s 호칭 \"%s\" (기대 \"%s\")\n",
               label, a->name, b->name, rel_a2b, expectedRel_a2b);
        ok = 0;
    }
    if (strcmp(rel_b2a, expectedRel_b2a) != 0) {
        printf("[FAIL] %s : %s→%s 호칭 \"%s\" (기대 \"%s\")\n",
               label, b->name, a->name, rel_b2a, expectedRel_b2a);
        ok = 0;
    }
    if (ok) {
        printf("[PASS] %s : %d촌 | %s→%s=\"%s\" | %s→%s=\"%s\"\n",
               label, chonsu,
               a->name, b->name, rel_a2b,
               b->name, a->name, rel_b2a);
        g_pass++;
    } else {
        g_fail++;
    }
}

/* ── 가족 트리 구성 ──────────────────────────────────────────────── */
/*
 * 4세대 가족 트리
 *
 * 증조부(M,90)─증조모(F,88)
 *   └── 할아버지(M,65)─할머니(F,63)
 *           ├── 큰아버지(M,42)─큰어머니(F,40)
 *           │     ├── 사촌형(M,22)
 *           │     └── 사촌누나(F,20)
 *           ├── 아버지(M,38)─어머니(F,36)
 *           │     ├── 나(M,18)
 *           │     └── 여동생(F,15)
 *           └── 고모(F,35)─고모부(M,37)
 *                 └── 고모사촌(F,16)
 *
 * 외가:
 * 외할아버지(M,62)─외할머니(F,60)
 *   ├── 어머니(F,36)  ← 친가 아버지의 배우자와 동일 인물
 *   ├── 외삼촌(M,33)─외숙모(F,31)
 *   │     └── 외사촌남동생(M,10)
 *   └── 이모(F,30)─이모부(M,32)
 *         └── 이모사촌(F,12)
 */

static Person *증조부, *증조모;
static Person *할아버지, *할머니;
static Person *큰아버지, *큰어머니;
static Person *아버지, *어머니;
static Person *고모, *고모부;
static Person *나, *여동생;
static Person *사촌형, *사촌누나, *고모사촌;
static Person *외할아버지, *외할머니;
static Person *외삼촌, *외숙모;
static Person *이모, *이모부;
static Person *외사촌남동생, *이모사촌;

static void buildTree(void) {
    증조부  = createPerson("증조부",  'M', 90);
    증조모  = createPerson("증조모",  'F', 88);
    addSpouse(증조부, 증조모);

    할아버지 = createPerson("할아버지", 'M', 65);
    할머니   = createPerson("할머니",   'F', 63);
    addSpouse(할아버지, 할머니);
    addChild(증조부, 할아버지);

    큰아버지  = createPerson("큰아버지", 'M', 42);
    큰어머니  = createPerson("큰어머니", 'F', 40);
    addSpouse(큰아버지, 큰어머니);
    addChild(할아버지, 큰아버지);

    아버지 = createPerson("아버지", 'M', 38);
    어머니 = createPerson("어머니", 'F', 36);
    addSpouse(아버지, 어머니);
    addChild(할아버지, 아버지);

    고모   = createPerson("고모",   'F', 35);
    고모부 = createPerson("고모부", 'M', 37);
    addSpouse(고모, 고모부);
    addChild(할아버지, 고모);

    나      = createPerson("나",      'M', 18);
    여동생  = createPerson("여동생",  'F', 15);
    addChild(아버지, 나);
    addChild(아버지, 여동생);

    사촌형   = createPerson("사촌형",   'M', 22);
    사촌누나 = createPerson("사촌누나", 'F', 20);
    addChild(큰아버지, 사촌형);
    addChild(큰아버지, 사촌누나);

    고모사촌 = createPerson("고모사촌", 'F', 16);
    addChild(고모, 고모사촌);

    외할아버지 = createPerson("외할아버지", 'M', 62);
    외할머니   = createPerson("외할머니",   'F', 60);
    addSpouse(외할아버지, 외할머니);
    addChild(외할아버지, 어머니);

    외삼촌  = createPerson("외삼촌", 'M', 33);
    외숙모  = createPerson("외숙모", 'F', 31);
    addSpouse(외삼촌, 외숙모);
    addChild(외할아버지, 외삼촌);

    이모   = createPerson("이모",   'F', 30);
    이모부 = createPerson("이모부", 'M', 32);
    addSpouse(이모, 이모부);
    addChild(외할아버지, 이모);

    외사촌남동생 = createPerson("외사촌남동생", 'M', 10);
    addChild(외삼촌, 외사촌남동생);

    이모사촌 = createPerson("이모사촌", 'F', 12);
    addChild(이모, 이모사촌);
}

static void freeTree(void) {
    /* 증조부에서 출발하면 어머니 → 외할아버지로 연결되므로 외가도 함께 해제됨 */
    freeAll(증조부);
}

/* ── 테스트 케이스 ───────────────────────────────────────────────── */

static void test_본인(void) {
    printf("\n=== 본인 ===\n");
    Person *path[MAX_PATH_LEN];
    int len = findPath(나, 나, path, MAX_PATH_LEN);
    const char *rel = getRelation(나, 나, path, len);
    if (strcmp(rel, "본인") == 0) { printf("[PASS] 나→나 : 본인\n"); g_pass++; }
    else { printf("[FAIL] 나→나 : \"%s\" (기대 \"본인\")\n", rel); g_fail++; }
}

static void test_배우자(void) {
    printf("\n=== 0촌: 배우자 ===\n");
    check("아버지↔어머니",   아버지, 어머니, 0, "아내", "남편");
    check("할아버지↔할머니", 할아버지, 할머니, 0, "아내", "남편");
}

static void test_1촌(void) {
    printf("\n=== 1촌: 부모-자녀 ===\n");
    check("나(M)→아버지(M)",   나, 아버지, 1, "아버지", "아들");
    check("나(M)→어머니(F)",   나, 어머니, 1, "어머니", "아들");
    check("여동생(F)→아버지(M)", 여동생, 아버지, 1, "아버지", "딸");
    check("여동생(F)→어머니(F)", 여동생, 어머니, 1, "어머니", "딸");
}

static void test_2촌_조부모(void) {
    printf("\n=== 2촌: 조부모-손자녀 ===\n");
    check("나→할아버지(친가)", 나, 할아버지, 2, "할아버지", "손자");
    check("나→할머니(친가)",   나, 할머니,   2, "할머니",   "손자");
    check("여동생→할아버지",   여동생, 할아버지, 2, "할아버지", "손녀");
    check("여동생→할머니",     여동생, 할머니,   2, "할머니",   "손녀");
    check("나→외할아버지",     나, 외할아버지, 2, "외할아버지", "손자");
    check("나→외할머니",       나, 외할머니,   2, "외할머니",   "손자");
    check("여동생→외할아버지", 여동생, 외할아버지, 2, "외할아버지", "손녀");
    check("여동생→외할머니",   여동생, 외할머니,   2, "외할머니",   "손녀");
}

static void test_2촌_형제(void) {
    printf("\n=== 2촌: 형제자매 ===\n");
    /* 나(M,18) ↔ 여동생(F,15): 나이 나>여동생 */
    check("나(M,18)↔여동생(F,15)", 나, 여동생, 2, "여동생", "오빠");
    /* 사촌형(M,22) ↔ 사촌누나(F,20): 사촌형이 나이 많으므로 사촌누나는 연하 */
    check("사촌형(M,22)↔사촌누나(F,20)", 사촌형, 사촌누나, 2, "여동생", "오빠");
}

static void test_3촌(void) {
    printf("\n=== 3촌: 삼촌/고모/외삼촌/이모/조카 ===\n");
    check("나(M)→큰아버지(M,부계)", 나, 큰아버지, 3, "큰아버지", "조카");
    check("나(M)→고모(F,부계)",     나, 고모,      3, "고모",     "조카");
    check("나(M)→외삼촌(M,모계)",   나, 외삼촌,    3, "외삼촌",   "조카");
    check("나(M)→이모(F,모계)",     나, 이모,       3, "이모",     "조카");
    check("여동생(F)→큰아버지",     여동생, 큰아버지, 3, "큰아버지", "조카딸");
    check("여동생(F)→고모",         여동생, 고모,     3, "고모",     "조카딸");
    check("여동생(F)→외삼촌",       여동생, 외삼촌,   3, "외삼촌",   "조카딸");
    check("여동생(F)→이모",         여동생, 이모,      3, "이모",     "조카딸");
    /* 증조부: 나→아버지→할아버지→증조부 = 3촌 */
    check("나→증조부(3촌)",  나, 증조부, 3, "증조할아버지", "증손자");
    check("나→증조모(3촌)",  나, 증조모, 3, "증조할머니",   "증손자");
    check("여동생→증조부",   여동생, 증조부, 3, "증조할아버지", "증손녀");
}

static void test_4촌(void) {
    printf("\n=== 4촌: 사촌 ===\n");
    /* 나(M,18) ↔ 사촌형(M,22): 사촌형이 연상 */
    check("나(M,18)↔사촌형(M,22)",       나, 사촌형,       4, "사촌형",     "사촌동생");
    /* 나(M,18) ↔ 사촌누나(F,20): 사촌누나가 연상 */
    check("나(M,18)↔사촌누나(F,20)",     나, 사촌누나,     4, "사촌누나",   "사촌동생");
    /* 나(M,18) ↔ 고모사촌(F,16): 고모사촌이 연하 */
    check("나(M,18)↔고모사촌(F,16)",     나, 고모사촌,     4, "사촌여동생", "사촌오빠");
    /* 여동생(F,15) ↔ 사촌형(M,22): 사촌형이 연상 */
    check("여동생(F,15)↔사촌형(M,22)",   여동생, 사촌형,   4, "사촌오빠",   "사촌여동생");
    /* 여동생(F,15) ↔ 사촌누나(F,20): 사촌누나가 연상 */
    check("여동생(F,15)↔사촌누나(F,20)", 여동생, 사촌누나, 4, "사촌언니",   "사촌여동생");
    /* 여동생(F,15) ↔ 고모사촌(F,16): 고모사촌이 연상 */
    check("여동생(F,15)↔고모사촌(F,16)", 여동생, 고모사촌, 4, "사촌언니",   "사촌여동생");
    /* 나(M,18) ↔ 외사촌남동생(M,10): 연하 */
    check("나(M,18)↔외사촌남동생(M,10)", 나, 외사촌남동생, 4, "사촌동생",   "사촌형");
    /* 나(M,18) ↔ 이모사촌(F,12): 연하 */
    check("나(M,18)↔이모사촌(F,12)",     나, 이모사촌,     4, "사촌여동생", "사촌오빠");
}

/* ── 메인 ─────────────────────────────────────────────────────────── */

int main(void) {
    printf("========================================\n");
    printf("  촌수 계산기 - 전체 관계 테스트\n");
    printf("========================================\n");

    buildTree();

    test_본인();
    test_배우자();
    test_1촌();
    test_2촌_조부모();
    test_2촌_형제();
    test_3촌();
    test_4촌();

    freeTree();

    printf("\n========================================\n");
    printf("  결과: PASS %d / FAIL %d / 합계 %d\n",
           g_pass, g_fail, g_pass + g_fail);
    printf("========================================\n");

    return g_fail > 0 ? 1 : 0;
}
