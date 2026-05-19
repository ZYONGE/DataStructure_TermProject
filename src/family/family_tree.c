#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "family_tree.h"

Person *createPerson(const char *name, char gender, int age) {
    Person *p = (Person *)malloc(sizeof(Person));
    if (!p) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    p->gender = gender;
    p->age    = age;
    p->parent = p->child = p->prev = p->next = p->spouse = NULL;
    return p;
}

/* parent의 자식 목록 맨 뒤에 child 삽입 (이중연결리스트) */
void addChild(Person *parent, Person *child) {
    child->parent = parent;
    child->prev = child->next = NULL;

    if (!parent->child) {
        parent->child = child;
        return;
    }

    Person *last = parent->child;
    while (last->next) last = last->next;
    last->next  = child;
    child->prev = last;
}

void addSpouse(Person *a, Person *b) {
    a->spouse = b;
    b->spouse = a;
}

/* ─── 탐색 (그래프 DFS, 방문 배열 사용) ───────────────────────────── */

#define MAX_MEMBERS 500

static void findHelper(Person *cur, const char *name,
                       Person **visited, int *vCount, Person **result) {
    if (!cur || *result) return;
    for (int i = 0; i < *vCount; i++)
        if (visited[i] == cur) return;
    visited[(*vCount)++] = cur;

    if (strcmp(cur->name, name) == 0) { *result = cur; return; }

    findHelper(cur->parent, name, visited, vCount, result);
    findHelper(cur->spouse, name, visited, vCount, result);
    findHelper(cur->child,  name, visited, vCount, result);
    findHelper(cur->prev,   name, visited, vCount, result);
    findHelper(cur->next,   name, visited, vCount, result);
}

Person *findPerson(Person *any, const char *name) {
    if (!any) return NULL;
    Person *visited[MAX_MEMBERS];
    int vCount = 0;
    Person *result = NULL;
    findHelper(any, name, visited, &vCount, &result);
    return result;
}

/* ─── 노드 제거 ─────────────────────────────────────────────────────── */

int removePerson(Person *any, Person *target) {
    if (!target) return 0;
    if (target->child) return 0;  /* 자식 있으면 삭제 불가 */

    /* 형제자매 연결 해제 */
    if (target->prev) target->prev->next = target->next;
    if (target->next) target->next->prev = target->prev;

    /* 부모의 child 포인터 갱신 */
    if (target->parent && target->parent->child == target)
        target->parent->child = target->next;

    /* 배우자 연결 해제 */
    if (target->spouse) target->spouse->spouse = NULL;

    (void)any;
    freePerson(target);
    return 1;
}

/* ─── 메모리 해제 ────────────────────────────────────────────────────── */

void freePerson(Person *p) {
    free(p);
}

static void freeHelper(Person *cur, Person **visited, int *vCount) {
    if (!cur) return;
    for (int i = 0; i < *vCount; i++)
        if (visited[i] == cur) return;
    visited[(*vCount)++] = cur;

    freeHelper(cur->parent, visited, vCount);
    freeHelper(cur->spouse, visited, vCount);
    freeHelper(cur->child,  visited, vCount);
    freeHelper(cur->prev,   visited, vCount);
    freeHelper(cur->next,   visited, vCount);

    free(cur);
}

void freeAll(Person *any) {
    if (!any) return;
    Person *visited[MAX_MEMBERS];
    int vCount = 0;
    freeHelper(any, visited, &vCount);
}

/* ─── 디버그 출력 ─────────────────────────────────────────────────────── */

void printTree(Person *root, int depth) {
    if (!root) return;
    for (int i = 0; i < depth * 2; i++) putchar(' ');
    printf("[%s(%c/%d)]", root->name, root->gender, root->age);
    if (root->spouse) printf("─[%s]", root->spouse->name);
    putchar('\n');
    Person *ch = root->child;
    while (ch) {
        printTree(ch, depth + 1);
        ch = ch->next;
    }
}
