#ifndef DEQUE_VIEW_H
#define DEQUE_VIEW_H

#include "../family/person.h"

/*
 * self 기준 3세대 덱 뷰를 초기화한다.
 * self 가 NULL 이면 전체 클리어.
 */
void dequeViewInit(Person *self);

/*
 * self 기준으로 3세대 덱 뷰를 전체 갱신한다.
 * generation: 0=조부모, 1=부모, 2=본인 세대 (-1 이면 전체)
 */
void dequeViewUpdate(Person *self, int generation);

#endif /* DEQUE_VIEW_H */
