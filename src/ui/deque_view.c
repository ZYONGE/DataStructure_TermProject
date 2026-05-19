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

/* person 한 명(및 배우자)을 나이 기준으로 덱에 삽입 */
static void insertToDeque(Deque *deque, Person *p) {
    if (!p || dequeIsFull(deque) || dequeContains(deque, p)) return;
    if (dequeIsEmpty(deque) || p->age >= dequePeekFront(deque)->age)
        dequePushFront(deque, p);
    else
        dequePushBack(deque, p);

    /* 배우자도 바로 뒤에 삽입 */
    if (p->spouse && !dequeIsFull(deque) && !dequeContains(deque, p->spouse))
        dequePushBack(deque, p->spouse);
}

/* anchor 의 형제자매(+ 배우자)를 덱에 수집 */
static void collectGeneration(Person *anchor, Deque *deque) {
    initDeque(deque);
    if (!anchor) return;

    Person *leftmost = anchor;
    while (leftmost->prev) leftmost = leftmost->prev;

    Person *cur = leftmost;
    while (cur) {
        insertToDeque(deque, cur);
        cur = cur->next;
    }
}

/*
 * 본인 세대 수집: 본인·형제자매 + 사촌(부모의 형제자매의 자녀)
 * 사촌은 부모 세대 형제자매 각각의 child 리스트를 순회해서 모은다.
 */
static void collectSelfGeneration(Person *self, Deque *deque) {
    initDeque(deque);
    if (!self) return;

    /* 1. 본인과 형제자매 */
    Person *leftmost = self;
    while (leftmost->prev) leftmost = leftmost->prev;
    Person *cur = leftmost;
    while (cur) {
        insertToDeque(deque, cur);
        cur = cur->next;
    }

    /* 2. 사촌: 부모의 형제자매(삼촌/고모 등)의 자녀들 */
    if (!self->parent) return;

    /* 부모의 가장 왼쪽 형제로 이동 */
    Person *parentSib = self->parent;
    while (parentSib->prev) parentSib = parentSib->prev;

    while (parentSib) {
        /* 내 직접 부모는 이미 1번에서 처리 */
        if (parentSib != self->parent) {
            Person *cousin = parentSib->child;
            while (cousin) {
                insertToDeque(deque, cousin);
                cousin = cousin->next;
            }
        }
        /* 부모 형제의 배우자 쪽 자녀도 포함 */
        if (parentSib->spouse && parentSib->spouse != self->parent) {
            Person *cousin = parentSib->spouse->child;
            while (cousin) {
                insertToDeque(deque, cousin);
                cousin = cousin->next;
            }
        }
        parentSib = parentSib->next;
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

    /* 본인 세대: self + 사촌 */
    if (generation == 2 || generation == -1) {
        collectSelfGeneration(self, &g_deques[2]);
        renderDequeRow(2, self);
    }
}
