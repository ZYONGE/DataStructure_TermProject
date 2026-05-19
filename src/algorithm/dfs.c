#include <string.h>
#include "../family/person.h"
#include "dfs.h"

#define MAX_VISITED 500

/*
 * 재귀 DFS.
 * 형제 포인터(prev/next)를 직접 탐색하지 않고,
 * 부모->자식 리스트 전체를 순회한다.
 * 덕분에 모든 이동이 up/down/spouse 중 하나로 분류되어
 * countSteps 가 정확하게 동작한다.
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
    /* 부모 */
    if ((res = dfsHelper(cur->parent, target, visited, vCount, path, pathLen+1, maxLen)) != -1) return res;
    /* 배우자 */
    if ((res = dfsHelper(cur->spouse, target, visited, vCount, path, pathLen+1, maxLen)) != -1) return res;
    /* 자식 리스트 전체 (형제는 부모를 경유하여 탐색됨) */
    Person *ch = cur->child;
    while (ch) {
        if ((res = dfsHelper(ch, target, visited, vCount, path, pathLen+1, maxLen)) != -1) return res;
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
