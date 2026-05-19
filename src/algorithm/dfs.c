#include <string.h>
#include "../family/person.h"

/* include/stack.h 를 Person* Element 로 사용 */
typedef struct Person *Element;
#define ELEMENT_DEFINED
#define ELEMENT_FORMAT "%p"
#include "../../include/stack.h"

#include "dfs.h"

#define MAX_VISITED 500

/*
 * 재귀 DFS: 현재 경로를 path[0..pathLen-1] 에 쌓으며 target 탐색.
 * visited[] 는 현재 경로 상의 노드를 추적해 사이클을 방지한다.
 * 백트래킹 시 visited 도 함께 복원한다.
 */
static int dfsHelper(Person *cur, Person *target,
                     Person **visited, int *vCount,
                     Person **path, int pathLen, int maxLen) {
    if (!cur || pathLen >= maxLen) return -1;

    /* 현재 경로에 이미 있는 노드면 스킵 */
    for (int i = 0; i < *vCount; i++)
        if (visited[i] == cur) return -1;

    visited[(*vCount)++] = cur;
    path[pathLen] = cur;

    if (cur == target) return pathLen + 1;

    int res;
    /* 인접 노드: 부모, 배우자, 첫째 자식, 왼쪽/오른쪽 형제 */
    if ((res = dfsHelper(cur->parent, target, visited, vCount, path, pathLen+1, maxLen)) != -1) return res;
    if ((res = dfsHelper(cur->spouse, target, visited, vCount, path, pathLen+1, maxLen)) != -1) return res;
    if ((res = dfsHelper(cur->child,  target, visited, vCount, path, pathLen+1, maxLen)) != -1) return res;
    if ((res = dfsHelper(cur->prev,   target, visited, vCount, path, pathLen+1, maxLen)) != -1) return res;
    if ((res = dfsHelper(cur->next,   target, visited, vCount, path, pathLen+1, maxLen)) != -1) return res;

    (*vCount)--;    /* 백트래킹: 방문 표시 해제 */
    return -1;
}

int findPath(Person *start, Person *target,
             Person **path, int maxLen) {
    if (!start || !target) return -1;
    if (start == target) { path[0] = start; return 1; }

    Person *visited[MAX_VISITED];
    int vCount = 0;
    return dfsHelper(start, target, visited, &vCount, path, 0, maxLen);
}
