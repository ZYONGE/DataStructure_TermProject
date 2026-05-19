#ifndef DFS_H
#define DFS_H

#include "../family/person.h"

#define MAX_PATH_LEN 200

/*
 * start 에서 target 까지의 경로를 DFS 로 탐색한다.
 * path[] 에 경로를 저장하고 경로 길이(노드 수)를 반환한다.
 * 탐색 실패 시 -1 반환.
 * 촌수 = 반환값 - 1
 */
int findPath(Person *start, Person *target,
             Person **path, int maxLen);

#endif /* DFS_H */
