#ifndef FAMILY_TREE_H
#define FAMILY_TREE_H

#include "person.h"

/* 노드 생성 / 해제 */
Person *createPerson(const char *name, char gender, int age);
void    freePerson(Person *p);
void    freeAll(Person *any);       /* 연결된 모든 노드 해제 */

/* 트리 조작 */
void addChild(Person *parent, Person *child);
void addSpouse(Person *a, Person *b);

/* 노드 제거 (자식 없는 경우만 허용, 성공 시 1 반환) */
int  removePerson(Person *any, Person *target);

/* 탐색 */
Person *findPerson(Person *any, const char *name);

/* 유틸리티 */
void printTree(Person *root, int depth); /* 디버그용 트리 출력 */

#endif /* FAMILY_TREE_H */
