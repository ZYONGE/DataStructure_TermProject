#include <stdio.h>
#include <string.h>

/* Person* 를 Element 로 사용해 deque.h 포함 */
#include "../family/person.h"
typedef struct Person *Element;
#define ELEMENT_DEFINED
#define ELEMENT_FORMAT "%p"
#include "../../include/deque.h"

#include "deque_view.h"
#include "ui.h"
#include "../util/util.h"

/* 세대별 덱 (인덱스: 0=조부모, 1=부모, 2=본인) */
static Deque g_deques[3];

/* 덱 출력 행 */
static const int deque_rows[3] = { ROW_GRAND_DEQUE, ROW_PARENT_DEQUE, ROW_SELF_DEQUE };

/* ── 세대 수집 헬퍼 ──────────────────────────────────────────── */

/* 덱에 이미 있는지 확인 */
static int dequeContains(Deque *deque, Person *p) {
    for (int i = 0; i < deque->size; i++) {
        int idx = (deque->front + i) % DEQUE_MAX_SIZE;
        if (deque->data[idx] == p) return 1;
    }
    return 0;
}

/*
 * 혈족 배열(blood[])에 p를 birth_year 오름차순으로 삽입.
 * 이미 있으면 무시한다.
 */
static void insertSortedBlood(Person **blood, int *count, Person *p) {
    if (!p) return;
    for (int i = 0; i < *count; i++)
        if (blood[i] == p) return;

    int pos = *count;
    for (int i = 0; i < *count; i++) {
        if (p->birth_year < blood[i]->birth_year) { pos = i; break; }
    }
    for (int i = *count; i > pos; i--)
        blood[i] = blood[i - 1];
    blood[pos] = p;
    (*count)++;
}

/*
 * 정렬된 혈족 배열로 덱을 구성한다.
 * 각 혈족 바로 뒤에 배우자를 삽입하여 쌍이 항상 인접하도록 한다.
 */
static void buildDequeFromBlood(Deque *deque, Person **blood, int count) {
    for (int i = 0; i < count; i++) {
        if (!dequeIsFull(deque) && !dequeContains(deque, blood[i]))
            dequePushBack(deque, blood[i]);
        if (blood[i]->spouse &&
            !dequeIsFull(deque) &&
            !dequeContains(deque, blood[i]->spouse))
            dequePushBack(deque, blood[i]->spouse);
    }
}

/* anchor 의 형제자매를 birth_year 순으로 덱에 수집 */
static void collectGeneration(Person *anchor, Deque *deque) {
    initDeque(deque);
    if (!anchor) return;

    Person *leftmost = anchor;
    while (leftmost->prev) leftmost = leftmost->prev;

    Person *blood[DEQUE_MAX_SIZE];
    int count = 0;
    Person *cur = leftmost;
    while (cur && count < DEQUE_MAX_SIZE) {
        insertSortedBlood(blood, &count, cur);
        cur = cur->next;
    }

    buildDequeFromBlood(deque, blood, count);
}

/*
 * 본인 세대 수집: 본인·형제자매 + 사촌(부모의 형제자매의 자녀)
 * 모든 혈족을 birth_year 기준으로 정렬한 뒤 배우자를 바로 뒤에 배치한다.
 */
static void collectSelfGeneration(Person *self, Deque *deque) {
    initDeque(deque);
    if (!self) return;

    Person *blood[DEQUE_MAX_SIZE];
    int count = 0;

    /* 1. 본인과 형제자매 */
    Person *leftmost = self;
    while (leftmost->prev) leftmost = leftmost->prev;
    Person *cur = leftmost;
    while (cur && count < DEQUE_MAX_SIZE) {
        insertSortedBlood(blood, &count, cur);
        cur = cur->next;
    }

    /* 2. 사촌: 부모의 형제자매(삼촌/고모 등)의 자녀들 */
    if (!self->parent) goto build;

    {
        Person *parentSib = self->parent;
        while (parentSib->prev) parentSib = parentSib->prev;

        while (parentSib) {
            if (parentSib != self->parent) {
                Person *cousin = parentSib->child;
                while (cousin && count < DEQUE_MAX_SIZE) {
                    insertSortedBlood(blood, &count, cousin);
                    cousin = cousin->next;
                }
            }
            /* 부모 형제의 배우자 쪽 자녀도 포함 */
            if (parentSib->spouse && parentSib->spouse != self->parent) {
                Person *cousin = parentSib->spouse->child;
                while (cousin && count < DEQUE_MAX_SIZE) {
                    insertSortedBlood(blood, &count, cousin);
                    cousin = cousin->next;
                }
            }
            parentSib = parentSib->next;
        }
    }

build:
    buildDequeFromBlood(deque, blood, count);
}

/* ── 덱 한 행 출력 ────────────────────────────────────────────── */

static void renderDequeRow(int gen, Person *self) {
    int row = deque_rows[gen];
    clearArea(1, row, UI_WIDTH - 2);
    gotoxy(1, row);

    Deque *dq = &g_deques[gen];
    if (dequeIsEmpty(dq)) {
        printf("  (없음)");
        fflush(stdout);
        return;
    }

    printf(" \xe2\x97\x80 ");  /* ◀ */
    for (int i = 0; i < dq->size; i++) {
        int idx = (dq->front + i) % DEQUE_MAX_SIZE;
        Person *p = dq->data[idx];
        if (!p) continue;

        /* 본인 강조 */
        if (p == self) setColor(C_YELLOW);

        printf("[%s]", p->name);

        if (p == self) resetColor();

        /* 배우자가 바로 다음 인덱스에 있으면 ─ 로 연결, 아니면 공백 두 칸 */
        if (i + 1 < dq->size) {
            int nidx = (dq->front + i + 1) % DEQUE_MAX_SIZE;
            Person *next = dq->data[nidx];
            if (next && (p->spouse == next || next->spouse == p))
                printf("\xe2\x94\x80");  /* ─ */
            else
                printf("  ");
        }
    }
    printf(" \xe2\x96\xb6 ");  /* ▶ */
    fflush(stdout);
}

/* ── 공개 함수 ────────────────────────────────────────────────── */

void dequeViewInit(Person *self) {
    for (int i = 0; i < 3; i++) initDeque(&g_deques[i]);
    dequeViewUpdate(self, -1);
}

void dequeViewUpdate(Person *self, int generation) {
    if (!self) {
        for (int i = 0; i < 3; i++) {
            initDeque(&g_deques[i]);
            clearArea(1, deque_rows[i], UI_WIDTH - 2);
        }
        return;
    }

    /* 조부모 세대: self->parent->parent */
    if (generation == 0 || generation == -1) {
        Person *grandParent = NULL;
        if (self->parent && self->parent->parent)
            grandParent = self->parent->parent;
        else if (self->parent && self->parent->spouse && self->parent->spouse->parent)
            grandParent = self->parent->spouse->parent;
        collectGeneration(grandParent, &g_deques[0]);
        renderDequeRow(0, self);
    }

    /* 부모 세대: self->parent */
    if (generation == 1 || generation == -1) {
        collectGeneration(self->parent, &g_deques[1]);
        renderDequeRow(1, self);
    }

    /* 본인 세대: self + 사촌 */
    if (generation == 2 || generation == -1) {
        collectSelfGeneration(self, &g_deques[2]);
        renderDequeRow(2, self);
    }
}
