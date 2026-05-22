#ifndef RELATION_H
#define RELATION_H

#include "../family/person.h"

/*
 * path[0]=me ~ path[pathLen-1]=target 경로(혈족 DFS 결과)를 받아
 * me 기준 한국어 호칭 문자열을 반환한다.
 *
 * 배우자 호칭이 필요한 경우 getRelation 호출 전에
 * 호출자가 target->spouse 여부를 확인하고 blood_target으로 교체한 뒤
 * applySpouseTable 로 후처리한다.
 */
const char *getRelation(Person *me, Person *target,
                        Person **path, int pathLen);

/*
 * target 기준 me 에 대한 호칭 (역방향).
 * path 를 반전시켜 getRelation 을 재호출한다.
 */
const char *getRelationReverse(Person *me, Person *target,
                               Person **path, int pathLen);

/*
 * 혈족 호칭 → 배우자 호칭 교체 테이블 (design.md §8).
 * 매칭되지 않으면 KW_UNKNOWN 반환.
 */
const char *applySpouseTable(const char *blood_hoching);

/*
 * 혈족 호칭(역방향) → 역방향 배우자 호칭 교체.
 * 예: 백부→나(질) 관계에서 백모→나 = 생질.
 */
const char *applyReverseSpouseTable(const char *blood_reverse_hoching);

/*
 * 촌수 계산: X + Y.
 * path 가 NULL 이거나 배우자 관계이면 0 반환.
 */
int computeChonsu(Person **path, int pathLen);

/*
 * 호칭에 대응하는 짧은 관계 메시지를 반환한다.
 * 없으면 NULL 반환.
 */
const char *getRelationMessage(const char *relation);

#endif /* RELATION_H */
