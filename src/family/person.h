#ifndef PERSON_H
#define PERSON_H

typedef struct Person {
    char name[50];
    char gender;        /* 'M' 남자 / 'F' 여자 */
    int  birth_year;    /* 출생연도 (연장자 비교용: 작을수록 연상) */

    struct Person *parent;  /* 혈족 부모 (부모 방향) */
    struct Person *child;   /* 첫째 자식 */
    struct Person *prev;    /* 왼쪽 형제자매 */
    struct Person *next;    /* 오른쪽 형제자매 */
    struct Person *spouse;  /* 배우자 */
} Person;

#endif /* PERSON_H */
