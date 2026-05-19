#ifndef RELATION_H
#define RELATION_H

#include "../family/person.h"

/*
 * path[0]=self ~ path[pathLen-1]=target 경로를 받아
 * self 기준 한국어 호칭 문자열을 반환한다.
 * 4촌 이내만 처리하며, 범위 초과 시 "미상" 반환.
 */
const char *getRelation(Person *self, Person *target,
                        Person **path, int pathLen);

/*
 * target 기준 self 에 대한 호칭 (역방향)
 */
const char *getRelationReverse(Person *self, Person *target,
                               Person **path, int pathLen);

#endif /* RELATION_H */
