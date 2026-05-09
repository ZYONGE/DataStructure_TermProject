// 원형 덱의 연산 함수들을 구현하는 헤더파일 (원형 배열 기반)
// 본 코드에서 typedef int, float... 등의 자료형을 Element로 정의하여 사용하였음
// 사용 방법: typedef int Element; 선언 후 이 파일을 include

#ifndef DEQUE_H
#define DEQUE_H

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

#ifndef DEQUE_MAX_SIZE
#define DEQUE_MAX_SIZE 100
#endif

typedef struct {
    Element data[DEQUE_MAX_SIZE];
    int front;
    int rear;
    int size;
} Deque;

// ─── 초기화 ───────────────────────────────────────────────────────────────────

static void initDeque(Deque *deque) {
    deque->front = 0;
    deque->rear  = 0;
    deque->size  = 0;
}

// ─── 상태 확인 ───────────────────────────────────────────────────────────────

static int dequeIsEmpty(Deque *deque) {
    return deque->size == 0;
}

static int dequeIsFull(Deque *deque) {
    return deque->size == DEQUE_MAX_SIZE;
}

static int dequeSize(Deque *deque) {
    return deque->size;
}

// ─── 삽입 ────────────────────────────────────────────────────────────────────

static void dequePushFront(Deque *deque, Element data) {
    if (dequeIsFull(deque)) { fprintf(stderr, "[Deque] 덱이 가득 찼습니다\n"); return; }
    deque->front = (deque->front - 1 + DEQUE_MAX_SIZE) % DEQUE_MAX_SIZE;
    deque->data[deque->front] = data;
    deque->size++;
}

static void dequePushBack(Deque *deque, Element data) {
    if (dequeIsFull(deque)) { fprintf(stderr, "[Deque] 덱이 가득 찼습니다\n"); return; }
    deque->data[deque->rear] = data;
    deque->rear = (deque->rear + 1) % DEQUE_MAX_SIZE;
    deque->size++;
}

// ─── 삭제 ────────────────────────────────────────────────────────────────────

static Element dequePopFront(Deque *deque) {
    if (dequeIsEmpty(deque)) { fprintf(stderr, "[Deque] 덱이 비어있습니다\n"); exit(1); }
    Element data = deque->data[deque->front];
    deque->front = (deque->front + 1) % DEQUE_MAX_SIZE;
    deque->size--;
    return data;
}

static Element dequePopBack(Deque *deque) {
    if (dequeIsEmpty(deque)) { fprintf(stderr, "[Deque] 덱이 비어있습니다\n"); exit(1); }
    deque->rear = (deque->rear - 1 + DEQUE_MAX_SIZE) % DEQUE_MAX_SIZE;
    Element data = deque->data[deque->rear];
    deque->size--;
    return data;
}

// ─── 탐색 (peek) ─────────────────────────────────────────────────────────────

static Element dequePeekFront(Deque *deque) {
    if (dequeIsEmpty(deque)) { fprintf(stderr, "[Deque] 덱이 비어있습니다\n"); exit(1); }
    return deque->data[deque->front];
}

static Element dequePeekBack(Deque *deque) {
    if (dequeIsEmpty(deque)) { fprintf(stderr, "[Deque] 덱이 비어있습니다\n"); exit(1); }
    return deque->data[(deque->rear - 1 + DEQUE_MAX_SIZE) % DEQUE_MAX_SIZE];
}

static int dequeSearch(Deque *deque, Element data) {
    for (int i = 0; i < deque->size; i++) {
        int idx = (deque->front + i) % DEQUE_MAX_SIZE;
        if (deque->data[idx] == data) return i;
    }
    return -1;
}

// ─── 정렬 (버블 정렬, 오름차순) ──────────────────────────────────────────────

static void dequeSort(Deque *deque) {
    if (deque->size <= 1) return;
    int swapped;
    do {
        swapped = 0;
        for (int i = 0; i < deque->size - 1; i++) {
            int a = (deque->front + i)     % DEQUE_MAX_SIZE;
            int b = (deque->front + i + 1) % DEQUE_MAX_SIZE;
            if (deque->data[a] > deque->data[b]) {
                Element tmp = deque->data[a];
                deque->data[a] = deque->data[b];
                deque->data[b] = tmp;
                swapped = 1;
            }
        }
    } while (swapped);
}

// ─── 출력 ────────────────────────────────────────────────────────────────────

static void dequeDisplay(Deque *deque) {
    printf("[Deque] FRONT [ ");
    for (int i = 0; i < deque->size; i++) {
        printf(ELEMENT_FORMAT, deque->data[(deque->front + i) % DEQUE_MAX_SIZE]);
        if (i < deque->size - 1) printf(", ");
    }
    printf(" ] REAR  (크기: %d)\n", deque->size);
}

// ─── 파일 가져오기 ────────────────────────────────────────────────────────────

static void dequeImport(Deque *deque, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { fprintf(stderr, "[Deque] 파일 열기 실패: %s\n", filename); return; }
    Element data;
    while (fscanf(fp, ELEMENT_FORMAT, &data) == 1) {
        dequePushBack(deque, data);
    }
    fclose(fp);
    printf("[Deque] '%s' 에서 데이터 가져오기 완료  (크기: %d)\n", filename, deque->size);
}

// ─── 초기화 (비우기) ─────────────────────────────────────────────────────────

static void dequeClear(Deque *deque) {
    deque->front = 0;
    deque->rear  = 0;
    deque->size  = 0;
}

#endif // DEQUE_H
