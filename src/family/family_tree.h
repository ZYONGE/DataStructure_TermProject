#ifndef FAMILY_TREE_H
#define FAMILY_TREE_H

#include "person.h"

/* 노드 생성 / 해제 */
Person *createPerson(const char *name, char gender, int birth_year);
void    freePerson(Person *p);
void    freeAll(Person *any);

/* 트리 조작 */
void addChild(Person *parent, Person *child);
void addSpouse(Person *a, Person *b);

/* 노드 제거 (자식 없는 경우만 허용, 성공 시 1 반환) */
int  removePerson(Person *any, Person *target);

/* 탐색 */
Person *findPerson(Person *any, const char *name);

/* 디버그 출력 */
void printTree(Person *root, int depth);

#endif /* FAMILY_TREE_H */
