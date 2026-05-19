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

/* self 의 조부모 세대 수집: self->parent->parent 와 그 형제자매 */
static void collectGeneration(Person *anchor, Deque *deque) {
    initDeque(deque);
    if (!anchor) return;

    /* anchor 의 가장 왼쪽 형제로 이동 */
    Person *leftmost = anchor;
    while (leftmost->prev) leftmost = leftmost->prev;

    /* 형제자매 전체를 나이 기준으로 덱에 삽입 */
    Person *cur = leftmost;
    while (cur) {
        if (!dequeIsFull(deque)) {
            if (dequeIsEmpty(deque) || cur->age >= dequePeekFront(deque)->age)
                dequePushFront(deque, cur);
            else
                dequePushBack(deque, cur);
        }
        /* 배우자도 함께 (쌍으로 표시) */
        if (cur->spouse && !dequeIsFull(deque)) {
            /* 이미 삽입된 배우자인지 확인 */
            int found = 0;
            for (int i = 0; i < deque->size; i++) {
                int idx = (deque->front + i) % DEQUE_MAX_SIZE;
                if (deque->data[idx] == cur->spouse) { found = 1; break; }
            }
            if (!found) dequePushBack(deque, cur->spouse);
        }
        cur = cur->next;
    }
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

        /* 배우자가 바로 다음 인덱스에 있으면 ─ 로 연결 */
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

    /* 본인 세대: self */
    if (generation == 2 || generation == -1) {
        collectGeneration(self, &g_deques[2]);
        renderDequeRow(2, self);
    }
}
