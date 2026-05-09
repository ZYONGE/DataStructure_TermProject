// 이중 연결 리스트의 연산 함수들을 구현하는 헤더파일
// 본 코드에서 typedef int, float... 등의 자료형을 Element로 정의하여 사용하였음
// 사용 방법: typedef int Element; 선언 후 이 파일을 include

#ifndef LIST_H
#define LIST_H

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

typedef struct ListNode {
    Element data;
    struct ListNode *prev;
    struct ListNode *next;
} ListNode;

typedef struct {
    ListNode *head;
    ListNode *tail;
    int size;
} List;

// ─── 초기화 ───────────────────────────────────────────────────────────────────

static void initList(List *list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

// ─── 내부 헬퍼 ───────────────────────────────────────────────────────────────

static ListNode *_createListNode(Element data) {
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    if (!node) { fprintf(stderr, "[List] 메모리 할당 실패\n"); exit(1); }
    node->data = data;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

// ─── 상태 확인 ───────────────────────────────────────────────────────────────

static int listIsEmpty(List *list) {
    return list->size == 0;
}

static int listSize(List *list) {
    return list->size;
}

// ─── 삽입 ────────────────────────────────────────────────────────────────────

static void listInsertFront(List *list, Element data) {
    ListNode *node = _createListNode(data);
    if (listIsEmpty(list)) {
        list->head = list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->size++;
}

static void listInsertBack(List *list, Element data) {
    ListNode *node = _createListNode(data);
    if (listIsEmpty(list)) {
        list->head = list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}

static void listInsertAt(List *list, int index, Element data) {
    if (index < 0 || index > list->size) {
        fprintf(stderr, "[List] 인덱스 범위 초과: %d\n", index);
        return;
    }
    if (index == 0) { listInsertFront(list, data); return; }
    if (index == list->size) { listInsertBack(list, data); return; }

    ListNode *cur = list->head;
    for (int i = 0; i < index; i++) cur = cur->next;

    ListNode *node = _createListNode(data);
    node->next = cur;
    node->prev = cur->prev;
    cur->prev->next = node;
    cur->prev = node;
    list->size++;
}

// ─── 삭제 ────────────────────────────────────────────────────────────────────

static Element listDeleteFront(List *list) {
    if (listIsEmpty(list)) { fprintf(stderr, "[List] 리스트가 비어있습니다\n"); exit(1); }
    ListNode *node = list->head;
    Element data = node->data;
    list->head = node->next;
    if (list->head) list->head->prev = NULL;
    else list->tail = NULL;
    free(node);
    list->size--;
    return data;
}

static Element listDeleteBack(List *list) {
    if (listIsEmpty(list)) { fprintf(stderr, "[List] 리스트가 비어있습니다\n"); exit(1); }
    ListNode *node = list->tail;
    Element data = node->data;
    list->tail = node->prev;
    if (list->tail) list->tail->next = NULL;
    else list->head = NULL;
    free(node);
    list->size--;
    return data;
}

static Element listDeleteAt(List *list, int index) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "[List] 인덱스 범위 초과: %d\n", index);
        exit(1);
    }
    if (index == 0) return listDeleteFront(list);
    if (index == list->size - 1) return listDeleteBack(list);

    ListNode *cur = list->head;
    for (int i = 0; i < index; i++) cur = cur->next;

    Element data = cur->data;
    cur->prev->next = cur->next;
    cur->next->prev = cur->prev;
    free(cur);
    list->size--;
    return data;
}

// 값과 일치하는 첫 번째 노드 삭제
static int listDeleteFirst(List *list, Element data) {
    ListNode *cur = list->head;
    while (cur) {
        if (cur->data == data) {
            if (cur->prev) cur->prev->next = cur->next;
            else list->head = cur->next;
            if (cur->next) cur->next->prev = cur->prev;
            else list->tail = cur->prev;
            free(cur);
            list->size--;
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

// 값과 일치하는 모든 노드 삭제
static void listDeleteAll(List *list, Element data) {
    ListNode *cur = list->head;
    while (cur) {
        ListNode *next = cur->next;
        if (cur->data == data) {
            if (cur->prev) cur->prev->next = cur->next;
            else list->head = cur->next;
            if (cur->next) cur->next->prev = cur->prev;
            else list->tail = cur->prev;
            free(cur);
            list->size--;
        }
        cur = next;
    }
}

// ─── 탐색 ────────────────────────────────────────────────────────────────────

static ListNode *listSearch(List *list, Element data) {
    ListNode *cur = list->head;
    while (cur) {
        if (cur->data == data) return cur;
        cur = cur->next;
    }
    return NULL;
}

static int listSearchIndex(List *list, Element data) {
    ListNode *cur = list->head;
    int idx = 0;
    while (cur) {
        if (cur->data == data) return idx;
        cur = cur->next;
        idx++;
    }
    return -1;
}

static Element listGetAt(List *list, int index) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "[List] 인덱스 범위 초과: %d\n", index);
        exit(1);
    }
    ListNode *cur = list->head;
    for (int i = 0; i < index; i++) cur = cur->next;
    return cur->data;
}

// ─── 정렬 (버블 정렬, 오름차순) ──────────────────────────────────────────────

static void listSort(List *list) {
    if (list->size <= 1) return;
    int swapped;
    do {
        swapped = 0;
        ListNode *cur = list->head;
        while (cur && cur->next) {
            if (cur->data > cur->next->data) {
                Element tmp = cur->data;
                cur->data = cur->next->data;
                cur->next->data = tmp;
                swapped = 1;
            }
            cur = cur->next;
        }
    } while (swapped);
}

// ─── 출력 ────────────────────────────────────────────────────────────────────

static void listDisplay(List *list) {
    printf("[List] HEAD <-> ");
    ListNode *cur = list->head;
    while (cur) {
        printf(ELEMENT_FORMAT, cur->data);
        if (cur->next) printf(" <-> ");
        cur = cur->next;
    }
    printf(" <-> NULL  (크기: %d)\n", list->size);
}

static void listDisplayReverse(List *list) {
    printf("[List] NULL <-> ");
    ListNode *cur = list->tail;
    while (cur) {
        printf(ELEMENT_FORMAT, cur->data);
        if (cur->prev) printf(" <-> ");
        cur = cur->prev;
    }
    printf(" <-> TAIL  (크기: %d)\n", list->size);
}

// ─── 파일 가져오기 ────────────────────────────────────────────────────────────

static void listImport(List *list, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { fprintf(stderr, "[List] 파일 열기 실패: %s\n", filename); return; }
    Element data;
    while (fscanf(fp, ELEMENT_FORMAT, &data) == 1) {
        listInsertBack(list, data);
    }
    fclose(fp);
    printf("[List] '%s' 에서 데이터 가져오기 완료  (크기: %d)\n", filename, list->size);
}

// ─── 메모리 해제 ─────────────────────────────────────────────────────────────

static void listFree(List *list) {
    ListNode *cur = list->head;
    while (cur) {
        ListNode *next = cur->next;
        free(cur);
        cur = next;
    }
    list->head = list->tail = NULL;
    list->size = 0;
}

#endif // LIST_H
