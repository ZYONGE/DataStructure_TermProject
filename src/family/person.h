#ifndef PERSON_H
#define PERSON_H

typedef struct Person {
    char name[50];
    char gender;        /* 'M' 남자 / 'F' 여자 */
    int  age;

    struct Person *parent;  /* 부모 (트리 - 위) */
    struct Person *child;   /* 첫째 자식 (트리 - 아래) */
    struct Person *prev;    /* 왼쪽 형제자매 (이중연결리스트) */
    struct Person *next;    /* 오른쪽 형제자매 (이중연결리스트) */
    struct Person *spouse;  /* 배우자 */
} Person;

#endif /* PERSON_H */
