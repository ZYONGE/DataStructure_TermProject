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

/*
 * 경로로부터 촌수를 계산한다.
 * 형제 포인터(prev/next) 이동을 1up+1down으로 처리하므로
 * pathLen-1 대신 이 함수를 사용해야 정확하다.
 */
int computeChonsu(Person **path, int pathLen);

/*
 * 호칭에 대응하는 짧은 관계 메시지를 반환한다.
 * 없으면 NULL 반환.
 */
const char *getRelationMessage(const char *relation);

#endif /* RELATION_H */
