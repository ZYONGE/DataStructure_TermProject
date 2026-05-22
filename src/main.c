#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>   /* isatty */
#endif

#include "family/person.h"
#include "family/family_tree.h"
#include "algorithm/dfs.h"
#include "algorithm/relation.h"
#include "data/keywords.h"
#include "ui/ui.h"
#include "ui/deque_view.h"
#include "util/util.h"

/* ── 전역 상태 ───────────────────────────────────────────────────── */
static Person *g_self = NULL;   /* 기준 인물 (나) */

/* ── 입력 헬퍼 ────────────────────────────────────────────────────── */

static void readLine(const char *prompt, char *buf, int size) {
    /* TTY 인터랙티브 모드에서만 잔여 입력 비우기
       파이프/리디렉션 시에는 flushInput 을 호출하면 데이터가 유실된다 */
#ifndef _WIN32
    if (isatty(STDIN_FILENO)) flushInput();
#else
    flushInput();
#endif

    /* 프롬프트 행 갱신, 입력 행 초기화 */
    printMain(0, "%s", prompt);
    clearArea(MAIN_X, MAIN_Y + 1, MAIN_W);

    /* 입력 행에 커서 이동 후 대기 */
    gotoxy(MAIN_X, MAIN_Y + 1);
    setColor(C_GREEN);
    printf("> ");
    resetColor();
    showCursor();
    fflush(stdout);

    buf[0] = '\0';
    if (fgets(buf, size, stdin))
        buf[strcspn(buf, "\n")] = '\0';

    /* 입력 에코 잔상 제거 */
    clearArea(MAIN_X, MAIN_Y + 1, MAIN_W);
    hideCursor();
}

/* 성별 입력: 1=남성, 2=여성. 반환값 0은 입력 취소를 의미한다. */
static char readGender(void) {
    char buf[8];
    while (1) {
        readLine("성별 (1=남성 / 2=여성 / 0=취소): ", buf, sizeof(buf));
        if (buf[0] == '0') return 0;   /* 취소 */
        if (buf[0] == '1') return 'M';
        if (buf[0] == '2') return 'F';
        printMain(2, "1(남성) 또는 2(여성)를 입력하세요.");
    }
}

static int readBirthYear(void) {
    char buf[16];
    readLine("출생연도 (예: 2004): ", buf, sizeof(buf));
    int y = atoi(buf);
    return (y > 1000) ? y : 2000;
}

/* ── 메뉴 기능 구현 ────────────────────────────────────────────────── */

/* 1. 구성원 추가 */
static void menuAdd(void) {
    clearMainArea();
    printMain(0, "=== 구성원 추가 ===");
    if (!g_self)
        printMain(1, "입력 순서: 본인 → 부모 → 조부모 → 기타 순으로 등록하세요.");
    else
        printMain(1, "TIP: '0' 입력으로 언제든지 취소할 수 있습니다.");

    char name[50], relName[50], relBuf[4];

    readLine("새 구성원 이름 (0=취소): ", name, sizeof(name));
    if (name[0] == '\0' || name[0] == '0') { printMain(3, "취소되었습니다."); return; }

    char gender = readGender();
    if (gender == 0) { printMain(3, "취소되었습니다."); return; }

    int  birth_year = readBirthYear();

    if (!g_self) {
        g_self = createPerson(name, gender, birth_year);
        printMain(3, "[%s] 을(를) 기준 인물(나)로 등록했습니다.", name);
        printMain(4, "다음으로 부모님을 등록해보세요!");
        dequeViewUpdate(g_self, -1);
        return;
    }

    printMain(3, "관계: 1.자녀  2.부모  3.배우자");
    readLine("선택 (0=취소): ", relBuf, sizeof(relBuf));
    if (relBuf[0] == '0') { printMain(5, "취소되었습니다."); return; }
    int rel = atoi(relBuf);

    readLine("연결할 기존 구성원 이름: ", relName, sizeof(relName));
    if (relName[0] == '0') { printMain(5, "취소되었습니다."); return; }
    Person *existing = findPerson(g_self, relName);
    if (!existing) {
        printMain(5, "'%s' 을(를) 찾을 수 없습니다.", relName);
        return;
    }

    Person *newP = createPerson(name, gender, birth_year);

    if (rel == 1) {
        addChild(existing, newP);
        printMain(5, "[%s] 의 자녀로 [%s] 추가 완료.", relName, name);
    } else if (rel == 2) {
        if (existing->parent) {
            printMain(5, "이미 부모가 있습니다.");
            freePerson(newP);
            return;
        }
        addChild(newP, existing);
        printMain(5, "[%s] 의 부모로 [%s] 추가 완료.", relName, name);
    } else if (rel == 3) {
        if (existing->spouse) {
            printMain(5, "이미 배우자가 있습니다.");
            freePerson(newP);
            return;
        }
        addSpouse(existing, newP);
        printMain(5, "[%s] 의 배우자로 [%s] 추가 완료.", relName, name);
    } else {
        printMain(5, "잘못된 선택입니다.");
        freePerson(newP);
        return;
    }

    dequeViewUpdate(g_self, -1);
}

/* 2. 구성원 수정 */
static void menuEdit(void) {
    clearMainArea();
    printMain(0, "=== 구성원 수정 ===");

    char name[50];
    readLine("수정할 구성원 이름: ", name, sizeof(name));
    Person *p = findPerson(g_self, name);
    if (!p) { printMain(2, "'%s' 을(를) 찾을 수 없습니다.", name); return; }

    printMain(2, "현재: [%s]  %c  %d년생", p->name, p->gender, p->birth_year);

    char newName[50];
    readLine("새 이름 (그대로: Enter): ", newName, sizeof(newName));
    if (newName[0] != '\0') {
        strncpy(p->name, newName, sizeof(p->name) - 1);
        p->name[sizeof(p->name) - 1] = '\0';
    }

    char gBuf[4];
    readLine("새 성별 M/F (그대로: Enter): ", gBuf, sizeof(gBuf));
    if (gBuf[0] == 'M' || gBuf[0] == 'm') p->gender = 'M';
    else if (gBuf[0] == 'F' || gBuf[0] == 'f') p->gender = 'F';

    char byBuf[8];
    readLine("새 출생연도 (그대로: Enter): ", byBuf, sizeof(byBuf));
    if (byBuf[0] != '\0') { int y = atoi(byBuf); if (y > 1000) p->birth_year = y; }

    printMain(5, "수정 완료: [%s] %c %d년생", p->name, p->gender, p->birth_year);
    dequeViewUpdate(g_self, -1);
}

/* 3. 구성원 삭제 */
static void menuDelete(void) {
    clearMainArea();
    printMain(0, "=== 구성원 삭제 ===");

    char name[50];
    readLine("삭제할 구성원 이름: ", name, sizeof(name));
    Person *p = findPerson(g_self, name);
    if (!p) { printMain(2, "'%s' 을(를) 찾을 수 없습니다.", name); return; }
    if (p == g_self) { printMain(2, "기준 인물(나)은 삭제할 수 없습니다."); return; }

    if (!removePerson(g_self, p)) {
        printMain(2, "자식이 있는 구성원은 삭제할 수 없습니다.");
        return;
    }
    printMain(2, "[%s] 삭제 완료.", name);
    dequeViewUpdate(g_self, -1);
}

/* 4. 전체 조회 */
static void menuView(void) {
    clearMainArea();
    printMain(0, "=== 구성원 조회 ===");
    if (!g_self) { printMain(1, "등록된 구성원이 없습니다."); return; }

    /* 최상위 조상 탐색 */
    Person *root = g_self;
    while (root->parent) root = root->parent;

    /* BFS 로 전체 출력 */
    Person *queue[500];
    int qHead = 0, qTail = 0;
    Person *seen[500];
    int sCount = 0;
    queue[qTail++] = root;

    int printRow = 1;
    while (qHead < qTail && printRow + MAIN_Y <= ROW_BOTTOM_END) {
        Person *cur = queue[qHead++];
        int alreadySeen = 0;
        for (int i = 0; i < sCount; i++) if (seen[i] == cur) { alreadySeen = 1; break; }
        if (alreadySeen) continue;
        seen[sCount++] = cur;

        clearArea(MAIN_X, MAIN_Y + printRow, MAIN_W);
        gotoxy(MAIN_X, MAIN_Y + printRow);
        printf("[%s] %c %d년생", cur->name, cur->gender, cur->birth_year);
        if (cur->spouse) printf("  ↔ [%s]", cur->spouse->name);
        if (cur == g_self) { setColor(C_YELLOW); printf(" ★나"); resetColor(); }
        fflush(stdout);
        printRow++;

        Person *ch = cur->child;
        while (ch) { queue[qTail++] = ch; ch = ch->next; }
        if (cur->spouse && cur->spouse != g_self) queue[qTail++] = cur->spouse;
    }
}

/* 5. 촌수 계산 */
static void menuChonsu(void) {
    clearMainArea();
    printMain(0, "=== 촌수 계산 ===");

    char name1[50], name2[50];
    readLine("첫 번째 사람 이름 (0=취소): ", name1, sizeof(name1));
    if (name1[0] == '0') { printMain(2, "취소되었습니다."); return; }
    readLine("두 번째 사람 이름 (0=취소): ", name2, sizeof(name2));
    if (name2[0] == '0') { printMain(2, "취소되었습니다."); return; }

    Person *p1 = findPerson(g_self, name1);
    Person *p2 = findPerson(g_self, name2);
    if (!p1) { printMain(3, "'%s' 을(를) 찾을 수 없습니다.", name1); return; }
    if (!p2) { printMain(3, "'%s' 을(를) 찾을 수 없습니다.", name2); return; }

    /* 배우자 사전 체크 */
    if (p1->spouse == p2 || p2->spouse == p1) {
        printMain(3, "[%s] 와(과) [%s] 는 배우자(무촌) 관계입니다.", name1, name2);
        return;
    }

    Person *path[MAX_PATH_LEN];
    int len = findPath(p1, p2, path, MAX_PATH_LEN);
    if (len < 0) {
        printMain(3, "[%s] 와(과) [%s] 는 연결되지 않습니다.", name1, name2);
        return;
    }

    int chonsu = computeChonsu(path, len);
    printMain(3, "[%s] 와(과) [%s] 는 %d촌입니다.", name1, name2, chonsu);

    /* 경로 출력 */
    char pathStr[512] = {0};
    for (int i = 0; i < len; i++) {
        strncat(pathStr, path[i]->name, sizeof(pathStr) - strlen(pathStr) - 1);
        if (i < len - 1)
            strncat(pathStr, "->", sizeof(pathStr) - strlen(pathStr) - 1);
    }
    printMain(4, "경로: %s", pathStr);
}

/* 6. 호칭 출력 */
static void menuRelation(void) {
    clearMainArea();
    printMain(0, "=== 호칭 출력 ===");
    if (!g_self) { printMain(1, "기준 인물이 없습니다."); return; }

    char name[50];
    readLine("상대방 이름 (0=취소): ", name, sizeof(name));
    if (name[0] == '0') { printMain(2, "취소되었습니다."); return; }
    Person *target = findPerson(g_self, name);
    if (!target) { printMain(2, "'%s' 을(를) 찾을 수 없습니다.", name); return; }

    /* 배우자 사전 체크 (design.md §8: blood DFS 와 분리) */
    if (g_self->spouse == target || target->spouse == g_self) {
        printMain(2, "배우자 관계");
        printMain(3, "나 -> [%s] : %s", name, KW_SPOUSE);
        printMain(4, "[%s] -> 나 : %s", name, KW_SPOUSE);
        return;
    }

    /* blood_target 결정: target 이 배우자 노드이면 배우자의 혈족 원본으로 교체 */
    Person *blood_target = target;
    int is_spouse_node   = 0;
    if (target->spouse) {
        Person *sp_path[MAX_PATH_LEN];
        int sp_len = findPath(g_self, target->spouse, sp_path, MAX_PATH_LEN);
        if (sp_len > 0) { blood_target = target->spouse; is_spouse_node = 1; }
    }

    Person *path[MAX_PATH_LEN];
    int len = findPath(g_self, blood_target, path, MAX_PATH_LEN);
    if (len < 0) { printMain(2, "경로를 찾을 수 없습니다."); return; }

    const char *blood1 = getRelation(g_self, blood_target, path, len);
    const char *blood2 = getRelationReverse(g_self, blood_target, path, len);
    const char *rel1   = is_spouse_node ? applySpouseTable(blood1) : blood1;
    /* 역방향 배우자 호칭: 전용 테이블에 없으면(KW_UNKNOWN) 혈족 역방향(blood2)으로 대체
     * 예) 조모→나 = applyReverseSpouseTable("조부") → KW_UNKNOWN → blood2("손자") 사용 */
    const char *rev_sp = is_spouse_node ? applyReverseSpouseTable(blood1) : KW_UNKNOWN;
    const char *rel2   = (strcmp(rev_sp, KW_UNKNOWN) == 0) ? blood2 : rev_sp;
    int chonsu = computeChonsu(path, len);

    printMain(2, "%d촌", chonsu);
    printMain(3, "나 -> [%s] : %s", name, rel1);
    printMain(4, "[%s] -> 나 : %s", name, rel2);

    const char *msg = getRelationMessage(rel1);
    if (msg) printMain(6, ">> %s", msg);
}

/* 7. 초기화 */
static void menuReset(void) {
    clearMainArea();
    printMain(0, "=== 초기화 ===");

    char buf[4];
    readLine("모든 데이터를 삭제합니다. 계속? (y/n): ", buf, sizeof(buf));
    if (buf[0] != 'y' && buf[0] != 'Y') { printMain(2, "취소되었습니다."); return; }

    if (g_self) { freeAll(g_self); g_self = NULL; }
    dequeViewUpdate(NULL, -1);
    printMain(2, "초기화 완료.");
}

/* ── 메인 루프 ─────────────────────────────────────────────────────── */

int main(void) {
    initUI();

    clearMainArea();
    printMain(0, "친가(아버지 가계) 촌수 계산기");
    printMain(1, "지원 범위: 조부모까지 3대 / 4촌 이내 / 친가 전용");
    printMain(2, "1번을 눌러 처음 구성원(나)을 등록하세요.");

    while (1) {
        updateMenuCursor(0);
        char ch = readKey();

        switch (ch) {
        case '1': menuAdd();      break;
        case '2': menuEdit();     break;
        case '3': menuDelete();   break;
        case '4': menuView();     break;
        case '5': menuChonsu();   break;
        case '6': menuRelation(); break;
        case '7': menuReset();    break;
        case '0': {
            clearMainArea();
            printMain(0, "종료하려면 'quit'를 입력하세요.");
            char confirm[16];
            readLine("확인: ", confirm, sizeof(confirm));
            if (strcmp(confirm, "quit") == 0) {
                if (g_self) freeAll(g_self);
                restoreTerminal();
                gotoxy(0, UI_HEIGHT + 1);
                printf("\n");
                return 0;
            }
            printMain(2, "취소되었습니다.");
            break;
        }
        default:
            break;
        }
    }
}
