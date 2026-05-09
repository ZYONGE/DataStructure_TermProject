// 원형 큐의 연산 함수들을 구현하는 헤더파일 (원형 배열 기반)
// 본 코드에서 typedef int, float... 등의 자료형을 Element로 정의하여 사용하였음
// 사용 방법: typedef int Element; 선언 후 이 파일을 include

#ifndef QUEUE_H
#define QUEUE_H

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

#ifndef QUEUE_MAX_SIZE
#define QUEUE_MAX_SIZE 100
#endif

typedef struct {
    Element data[QUEUE_MAX_SIZE];
    int front;
    int rear;
    int size;
} Queue;

// ─── 초기화 ───────────────────────────────────────────────────────────────────

static void initQueue(Queue *queue) {
    queue->front = 0;
    queue->rear  = 0;
    queue->size  = 0;
}

// ─── 상태 확인 ───────────────────────────────────────────────────────────────

static int queueIsEmpty(Queue *queue) {
    return queue->size == 0;
}

static int queueIsFull(Queue *queue) {
    return queue->size == QUEUE_MAX_SIZE;
}

static int queueSize(Queue *queue) {
    return queue->size;
}

// ─── 삽입 (enqueue) ──────────────────────────────────────────────────────────

static void enqueue(Queue *queue, Element data) {
    if (queueIsFull(queue)) { fprintf(stderr, "[Queue] 큐가 가득 찼습니다\n"); return; }
    queue->data[queue->rear] = data;
    queue->rear = (queue->rear + 1) % QUEUE_MAX_SIZE;
    queue->size++;
}

// ─── 삭제 (dequeue) ──────────────────────────────────────────────────────────

static Element dequeue(Queue *queue) {
    if (queueIsEmpty(queue)) { fprintf(stderr, "[Queue] 큐가 비어있습니다\n"); exit(1); }
    Element data = queue->data[queue->front];
    queue->front = (queue->front + 1) % QUEUE_MAX_SIZE;
    queue->size--;
    return data;
}

// ─── 탐색 (peek) ─────────────────────────────────────────────────────────────

static Element queuePeek(Queue *queue) {
    if (queueIsEmpty(queue)) { fprintf(stderr, "[Queue] 큐가 비어있습니다\n"); exit(1); }
    return queue->data[queue->front];
}

static int queueSearch(Queue *queue, Element data) {
    for (int i = 0; i < queue->size; i++) {
        int idx = (queue->front + i) % QUEUE_MAX_SIZE;
        if (queue->data[idx] == data) return i;
    }
    return -1;
}

// ─── 정렬 (버블 정렬, 오름차순) ──────────────────────────────────────────────

static void queueSort(Queue *queue) {
    if (queue->size <= 1) return;
    int swapped;
    do {
        swapped = 0;
        for (int i = 0; i < queue->size - 1; i++) {
            int a = (queue->front + i)     % QUEUE_MAX_SIZE;
            int b = (queue->front + i + 1) % QUEUE_MAX_SIZE;
            if (queue->data[a] > queue->data[b]) {
                Element tmp = queue->data[a];
                queue->data[a] = queue->data[b];
                queue->data[b] = tmp;
                swapped = 1;
            }
        }
    } while (swapped);
}

// ─── 출력 ────────────────────────────────────────────────────────────────────

static void queueDisplay(Queue *queue) {
    printf("[Queue] FRONT [ ");
    for (int i = 0; i < queue->size; i++) {
        printf(ELEMENT_FORMAT, queue->data[(queue->front + i) % QUEUE_MAX_SIZE]);
        if (i < queue->size - 1) printf(", ");
    }
    printf(" ] REAR  (크기: %d)\n", queue->size);
}

// ─── 파일 가져오기 ────────────────────────────────────────────────────────────

static void queueImport(Queue *queue, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { fprintf(stderr, "[Queue] 파일 열기 실패: %s\n", filename); return; }
    Element data;
    while (fscanf(fp, ELEMENT_FORMAT, &data) == 1) {
        enqueue(queue, data);
    }
    fclose(fp);
    printf("[Queue] '%s' 에서 데이터 가져오기 완료  (크기: %d)\n", filename, queue->size);
}

// ─── 초기화 (비우기) ─────────────────────────────────────────────────────────

static void queueClear(Queue *queue) {
    queue->front = 0;
    queue->rear  = 0;
    queue->size  = 0;
}

#endif // QUEUE_H
