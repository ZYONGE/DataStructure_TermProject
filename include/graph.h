// 그래프의 연산 함수들을 구현하는 헤더파일 (인접 리스트 기반)
// 정점(vertex)은 0부터 numVertices-1까지의 정수 인덱스로 관리
// 본 코드에서 typedef int, float... 등의 자료형을 Element로 정의하여 사용하였음
// 사용 방법: typedef int Element; 선언 후 이 파일을 include

#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 기본 Element 타입: int (각 정점에 저장되는 데이터)
// 다른 타입 사용 시 include 전에 ELEMENT_DEFINED 매크로와 함께 typedef 선언
// 예: typedef float Element; #define ELEMENT_DEFINED
#ifndef ELEMENT_DEFINED
#define ELEMENT_DEFINED
typedef int Element;
#endif

#ifndef ELEMENT_FORMAT
#define ELEMENT_FORMAT "%d"
#endif

#ifndef GRAPH_MAX_VERTICES
#define GRAPH_MAX_VERTICES 100
#endif

// 인접 리스트 노드 (간선 표현)
typedef struct AdjNode {
    int vertex;
    struct AdjNode *next;
} AdjNode;

// 정점 노드 (데이터 + 인접 리스트 헤드)
typedef struct {
    Element data;
    AdjNode *head;
} VertexNode;

typedef struct {
    VertexNode vertices[GRAPH_MAX_VERTICES];
    int numVertices;
    int numEdges;
    int directed;  // 1: 방향 그래프, 0: 무방향 그래프
} Graph;

// ─── 초기화 ───────────────────────────────────────────────────────────────────

static void initGraph(Graph *g, int directed) {
    g->numVertices = 0;
    g->numEdges    = 0;
    g->directed    = directed;
    for (int i = 0; i < GRAPH_MAX_VERTICES; i++) {
        g->vertices[i].head = NULL;
    }
}

// ─── 상태 확인 ───────────────────────────────────────────────────────────────

static int graphIsEmpty(Graph *g) {
    return g->numVertices == 0;
}

// ─── 정점 추가 ────────────────────────────────────────────────────────────────

static int graphAddVertex(Graph *g, Element data) {
    if (g->numVertices >= GRAPH_MAX_VERTICES) {
        fprintf(stderr, "[Graph] 정점 수 초과 (최대: %d)\n", GRAPH_MAX_VERTICES);
        return -1;
    }
    int v = g->numVertices;
    g->vertices[v].data = data;
    g->vertices[v].head = NULL;
    g->numVertices++;
    return v;
}

// ─── 간선 추가 ────────────────────────────────────────────────────────────────

static void _addAdjNode(Graph *g, int from, int to) {
    AdjNode *node = (AdjNode *)malloc(sizeof(AdjNode));
    if (!node) { fprintf(stderr, "[Graph] 메모리 할당 실패\n"); exit(1); }
    node->vertex = to;
    node->next = g->vertices[from].head;
    g->vertices[from].head = node;
}

static void graphAddEdge(Graph *g, int from, int to) {
    if (from < 0 || from >= g->numVertices || to < 0 || to >= g->numVertices) {
        fprintf(stderr, "[Graph] 잘못된 정점 인덱스: %d -> %d\n", from, to);
        return;
    }
    _addAdjNode(g, from, to);
    if (!g->directed) _addAdjNode(g, to, from);
    g->numEdges++;
}

// ─── 간선 삭제 ────────────────────────────────────────────────────────────────

static void _removeAdjNode(Graph *g, int from, int to) {
    AdjNode *cur = g->vertices[from].head;
    AdjNode *prev = NULL;
    while (cur) {
        if (cur->vertex == to) {
            if (prev) prev->next = cur->next;
            else      g->vertices[from].head = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur  = cur->next;
    }
}

static int graphRemoveEdge(Graph *g, int from, int to) {
    if (from < 0 || from >= g->numVertices || to < 0 || to >= g->numVertices) return 0;
    _removeAdjNode(g, from, to);
    if (!g->directed) _removeAdjNode(g, to, from);
    g->numEdges--;
    return 1;
}

// ─── 탐색 ────────────────────────────────────────────────────────────────────

static int graphHasEdge(Graph *g, int from, int to) {
    AdjNode *cur = g->vertices[from].head;
    while (cur) {
        if (cur->vertex == to) return 1;
        cur = cur->next;
    }
    return -1;
}

// ─── DFS (깊이 우선 탐색) ─────────────────────────────────────────────────────

static void _dfs(Graph *g, int v, int *visited) {
    visited[v] = 1;
    printf(ELEMENT_FORMAT, g->vertices[v].data);
    printf("(%d) ", v);
    AdjNode *cur = g->vertices[v].head;
    while (cur) {
        if (!visited[cur->vertex]) _dfs(g, cur->vertex, visited);
        cur = cur->next;
    }
}

static void graphDFS(Graph *g, int start) {
    if (start < 0 || start >= g->numVertices) {
        fprintf(stderr, "[Graph] 잘못된 시작 정점: %d\n", start);
        return;
    }
    int visited[GRAPH_MAX_VERTICES] = {0};
    printf("[Graph] DFS (시작: %d): ", start);
    _dfs(g, start, visited);
    printf("\n");
}

// ─── BFS (너비 우선 탐색) ─────────────────────────────────────────────────────

static void graphBFS(Graph *g, int start) {
    if (start < 0 || start >= g->numVertices) {
        fprintf(stderr, "[Graph] 잘못된 시작 정점: %d\n", start);
        return;
    }
    int visited[GRAPH_MAX_VERTICES] = {0};
    int queue[GRAPH_MAX_VERTICES];
    int front = 0, rear = 0;

    visited[start] = 1;
    queue[rear++]  = start;

    printf("[Graph] BFS (시작: %d): ", start);
    while (front < rear) {
        int v = queue[front++];
        printf(ELEMENT_FORMAT, g->vertices[v].data);
        printf("(%d) ", v);
        AdjNode *cur = g->vertices[v].head;
        while (cur) {
            if (!visited[cur->vertex]) {
                visited[cur->vertex] = 1;
                queue[rear++] = cur->vertex;
            }
            cur = cur->next;
        }
    }
    printf("\n");
}

// ─── 출력 ────────────────────────────────────────────────────────────────────

static void graphDisplay(Graph *g) {
    printf("[Graph] 정점 수: %d, 간선 수: %d, %s\n",
           g->numVertices, g->numEdges,
           g->directed ? "방향 그래프" : "무방향 그래프");
    for (int i = 0; i < g->numVertices; i++) {
        printf("  [%d] ", i);
        printf(ELEMENT_FORMAT, g->vertices[i].data);
        printf(" -> ");
        AdjNode *cur = g->vertices[i].head;
        while (cur) {
            printf("%d", cur->vertex);
            if (cur->next) printf(", ");
            cur = cur->next;
        }
        printf("\n");
    }
}

// ─── 파일 가져오기 ────────────────────────────────────────────────────────────
// 파일 형식:
//   첫 줄: 정점 수
//   이후: 각 정점의 데이터 (한 줄에 하나)
//   그 다음: 간선 수
//   이후: from to (한 줄에 하나)

static void graphImport(Graph *g, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { fprintf(stderr, "[Graph] 파일 열기 실패: %s\n", filename); return; }

    int vCount;
    fscanf(fp, "%d", &vCount);
    for (int i = 0; i < vCount; i++) {
        Element data;
        fscanf(fp, ELEMENT_FORMAT, &data);
        graphAddVertex(g, data);
    }

    int eCount;
    fscanf(fp, "%d", &eCount);
    for (int i = 0; i < eCount; i++) {
        int from, to;
        fscanf(fp, "%d %d", &from, &to);
        graphAddEdge(g, from, to);
    }
    fclose(fp);
    printf("[Graph] '%s' 에서 데이터 가져오기 완료  (정점: %d, 간선: %d)\n",
           filename, g->numVertices, g->numEdges);
}

// ─── 메모리 해제 ─────────────────────────────────────────────────────────────

static void graphFree(Graph *g) {
    for (int i = 0; i < g->numVertices; i++) {
        AdjNode *cur = g->vertices[i].head;
        while (cur) {
            AdjNode *next = cur->next;
            free(cur);
            cur = next;
        }
        g->vertices[i].head = NULL;
    }
    g->numVertices = 0;
    g->numEdges    = 0;
}

#endif // GRAPH_H
