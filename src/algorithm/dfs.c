#include <string.h>
#include "../family/person.h"
#include "dfs.h"

#define MAX_VISITED 500

/*
 * 혈족 DFS (BLOOD 간선만: parent / child 목록).
 * spouse 간선은 사용하지 않는다 — design.md 10절 알고리즘 Step 2.
 *
 * 형제는 부모→자식 목록 전체를 순회하여 탐색하므로
 * prev/next 포인터를 직접 경유하지 않는다.
 * 결과적으로 모든 이동은 up(→parent) 또는 down(→child) 중 하나이며,
 * LCA 추출이 정확하게 동작한다.
 */
static int dfsHelper(Person *cur, Person *target,
                     Person **visited, int *vCount,
                     Person **path, int pathLen, int maxLen) {
    if (!cur || pathLen >= maxLen) return -1;

    for (int i = 0; i < *vCount; i++)
        if (visited[i] == cur) return -1;

    visited[(*vCount)++] = cur;
    path[pathLen] = cur;

    if (cur == target) return pathLen + 1;

    int res;
    /* 부모 방향 (up) */
    if ((res = dfsHelper(cur->parent, target, visited, vCount, path, pathLen + 1, maxLen)) != -1)
        return res;
    /* 자식 목록 (down) — 형제는 부모 경유로 탐색됨 */
    Person *ch = cur->child;
    while (ch) {
        if ((res = dfsHelper(ch, target, visited, vCount, path, pathLen + 1, maxLen)) != -1)
            return res;
        ch = ch->next;
    }

    (*vCount)--;
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
