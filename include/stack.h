// 스택의 연산을 구현하는 헤더 파일 (연결 리스트 기반)
// 본 코드에서 typedef int, float... 등의 자료형을 Element로 정의하여 사용하였음
// 사용 방법: typedef int Element; 선언 후 이 파일을 include

#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>

// 기본 Element 타입: int
// 다른 타입 사용 시 include 전에 ELEMENT_DEFINED 매크로와 함께 typedef 선언
// 예: typedef float Element; #define ELEMENT_DEFINED
#ifndef ELEMENT_DEFINED
#define ELEMENT_DEFINED
typedef int Element;
#endif

#ifndef ELEMENT_FORMAT
#define ELEMENT_FORMAT "%d"
#endif

typedef struct StackNode {
    Element data;
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *top;
    int size;
} Stack;

// ─── 초기화 ───────────────────────────────────────────────────────────────────

static void initStack(Stack *stack) {
    stack->top = NULL;
    stack->size = 0;
}

// ─── 상태 확인 ───────────────────────────────────────────────────────────────

static int stackIsEmpty(Stack *stack) {
    return stack->size == 0;
}

static int stackSize(Stack *stack) {
    return stack->size;
}

// ─── 삽입 (push) ─────────────────────────────────────────────────────────────

static void stackPush(Stack *stack, Element data) {
    StackNode *node = (StackNode *)malloc(sizeof(StackNode));
    if (!node) { fprintf(stderr, "[Stack] 메모리 할당 실패\n"); exit(1); }
    node->data = data;
    node->next = stack->top;
    stack->top = node;
    stack->size++;
}

// ─── 삭제 (pop) ──────────────────────────────────────────────────────────────

static Element stackPop(Stack *stack) {
    if (stackIsEmpty(stack)) { fprintf(stderr, "[Stack] 스택이 비어있습니다\n"); exit(1); }
    StackNode *node = stack->top;
    Element data = node->data;
    stack->top = node->next;
    free(node);
    stack->size--;
    return data;
}

// ─── 탐색 (peek) ─────────────────────────────────────────────────────────────

static Element stackPeek(Stack *stack) {
    if (stackIsEmpty(stack)) { fprintf(stderr, "[Stack] 스택이 비어있습니다\n"); exit(1); }
    return stack->top->data;
}

// 스택 내 특정 값 존재 여부 확인 (top → bottom 순서로 탐색)
static int stackSearch(Stack *stack, Element data) {
    StackNode *cur = stack->top;
    int pos = 1;
    while (cur) {
        if (cur->data == data) return pos;
        cur = cur->next;
        pos++;
    }
    return -1;
}

// ─── 정렬 ────────────────────────────────────────────────────────────────────
// 스택을 임시 배열로 꺼낸 후 정렬하여 다시 push (오름차순, top이 최솟값)

static void stackSort(Stack *stack) {
    if (stack->size <= 1) return;
    int n = stack->size;
    Element *arr = (Element *)malloc(n * sizeof(Element));
    if (!arr) { fprintf(stderr, "[Stack] 메모리 할당 실패\n"); exit(1); }

    for (int i = 0; i < n; i++) arr[i] = stackPop(stack);

    // 버블 정렬 (내림차순 → 오름차순으로 push하면 top이 최솟값)
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (arr[j] < arr[j + 1]) {
                Element tmp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = tmp;
            }

    for (int i = 0; i < n; i++) stackPush(stack, arr[i]);
    free(arr);
}

// ─── 출력 ────────────────────────────────────────────────────────────────────

static void stackDisplay(Stack *stack) {
    printf("[Stack] TOP -> ");
    StackNode *cur = stack->top;
    while (cur) {
        printf(ELEMENT_FORMAT, cur->data);
        if (cur->next) printf(" -> ");
        cur = cur->next;
    }
    printf(" -> BOTTOM  (크기: %d)\n", stack->size);
}

// ─── 메모리 해제 ─────────────────────────────────────────────────────────────

static void stackFree(Stack *stack) {
    while (!stackIsEmpty(stack)) stackPop(stack);
}

#endif // STACK_H
