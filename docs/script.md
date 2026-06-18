# AI Agent 인계 문서
## 자료구조 기말 프로젝트 — 가족 호칭 탐색 시스템

> **업데이트:** 2026-06-10

---

## Agent 행동 원칙

| # | 원칙 | 세부 지침 |
|---|---|---|
| 1 | **질문 우선** | 불명확한 명세·논리는 임의 처리 금지. `미결 질문` 섹션에 기록 후 중단. 답변 후 재개. 해결 시 삭제. |
| 2 | **design.md 절대 기준** | `docs/design.md`와 기존 코드가 충돌하면 **기존 코드를 삭제**한다. |
| 3 | **알고리즘 우선** | `src/ui/`, `src/util/`은 수정 가능하나 알고리즘 구현이 항상 우선이다. |

---

## 프로젝트 목표

`docs/design.md` 기준으로 소스코드를 작성한다. 최종 기준은 `test/test_relation.c` 전체 통과.

### 구현 범위 (명시적 제한)

```
조부모 ~ 본인 (3세대 이내), 친가 단일 계열
```

---

## 핵심 설계 사항

### 1. `Person` 구조체

```c
typedef struct Person {
    char name[50];
    char gender;
    int  birth_year;    // 연도값. 값이 작을수록 연상(나이 많음)
    struct Person *parent;  // 혈족 부모(아버지)만 단일 포인터
    struct Person *child;
    struct Person *prev;
    struct Person *next;
    struct Person *spouse;
} Person;
```

---

### 2. DFS — blood 간선만 사용

```c
// parent / child / prev / next 포인터만 사용
// spouse 간선은 DFS에서 완전 제거
// 배우자 호칭은 후처리 교체 테이블로 별도 처리
```

---

### 3. 관계 결정 로직 — LCA 경로 패턴 매칭

```c
// Step 1: LCA 인덱스 추출
static int find_lca_idx(Person **path, int pathLen) {
    int lca = 0;
    for (int i = 0; i < pathLen - 1; i++) {
        if (path[i+1] == path[i]->parent)
            lca = i + 1;
        else
            break;  // 첫 down step에서 LCA 확정
    }
    return lca;
}

// Step 2: X(나→LCA), Y(LCA→target) 산출
int X = lca_idx;
int Y = pathLen - 1 - lca_idx;
Person **up   = &path[1];           // [1 .. lca_idx]
Person **down = &path[lca_idx + 1]; // [lca_idx+1 .. pathLen-1]

// Step 3: design.md 섹션 순서대로 패턴 매칭
```

---

### 4. 키워드 — 한자어 병기

```c
/* 구현 범위 내 키워드 */
"부(父)",  "모(母)"                               // 직계 1촌
"조부(祖父)",  "조모(祖母)"                       // 직계 2촌 상향
"형(兄)",  "제(弟)",  "자(姊)",  "매(妹)"         // 2촌 동항렬
"백부(伯父)",  "숙부(叔父)",  "고모(姑母)"        // 3촌 +1세대 (친가)
"종형(從兄)",  "종제(從弟)",  "종자(從姊)",  "종매(從妹)"              // 4촌 (백부·숙부 경유)
"고종형(姑從兄)",  "고종제(姑從弟)",  "고종자(姑從姊)",  "고종매(姑從妹)" // 4촌 (고모 경유)

/* 배우자 후처리 교체 테이블 */
"백부" → "백모(伯母)"
"숙부" → "숙모(叔母)"
"고모" → "고모부(姑母夫)"

/* 역방향 배우자 호칭 */
"백부·숙부·고모" 배우자 → 나 : "생질(甥姪)"

/* 자기 자신 */
"본인"

/* 미연결 관계 */
"관계 없음"
```

---

### 5. 배우자 호칭 — 2단계 분리 처리

```
Step 1. target이 p1->spouse 또는 p2->spouse인지 사전 체크
Step 2. 혈족 원본(spouse의 혈족)에 대해 blood DFS 수행
Step 3. 혈족 호칭에 배우자 후처리 교체 테이블 적용
```

```c
/* menuChonsu 구현 예시 */
void menuChonsu(Person *p1, Person *p2) {
    if (p1->spouse == p2 || p2->spouse == p1) {
        printf("두 분은 배우자 관계입니다.\n");
        return;
    }
    int chon = computeChonsu(p1, p2);
    printf(chon == -1 ? "지원하지 않는 관계입니다.\n"
                      : "%d촌 관계입니다.\n", chon);
}
```

---

## 구현 세부 참고

### 4촌 세분화 조건 (relation.c, X==2 && Y==2 분기)

```c
Person *dn0 = path[lca_idx + 1];   // target 쪽 분기점
Person *up1 = path[lca_idx - 1];   // 나 쪽 분기점
int through_female = (dn0->gender == 'F' || up1->gender == 'F');

if (!through_female) {
    // 종형(從兄) 계열 — 백부·숙부 경유
    // 백부 조건: target.birth_year < up[0].birth_year (아버지보다 연상)
    // 숙부 조건: target.birth_year > up[0].birth_year (아버지보다 연하)
} else {
    // 고종형(姑從兄) 계열 — 고모 경유
}
```

### 테스트 트리 구성 (birth_year 기준, 외가 없음)

```c
// createPerson(이름, 성별, birth_year)
Person *할아버지 = createPerson("할아버지", 'M', 1945);
Person *할머니   = createPerson("할머니",   'F', 1948);
Person *백부     = createPerson("백부",     'M', 1965);  // 아버지보다 연상
Person *아버지   = createPerson("아버지",   'M', 1970);
Person *어머니   = createPerson("어머니",   'F', 1972);  // spouse로만 연결
Person *숙부     = createPerson("숙부",     'M', 1975);  // 아버지보다 연하
Person *고모     = createPerson("고모",     'F', 1978);
Person *종형     = createPerson("종형",     'M', 2001);  // 백부 자녀, 나보다 연상
Person *나       = createPerson("나",       'M', 2004);
Person *고종매   = createPerson("고종매",   'F', 2006);  // 고모 자녀, 나보다 연하

addSpouse(할아버지, 할머니);
addChild(할아버지, 백부);
addChild(할아버지, 아버지);
addChild(할아버지, 숙부);
addChild(할아버지, 고모);
addSpouse(아버지, 어머니);
addChild(아버지, 나);
addChild(백부, 종형);
addChild(고모, 고종매);
```

---

## 구현 순서

```
1  src/family/person.h         Person 구조체 (birth_year, 단일 parent 포인터)
2  src/family/family_tree.c    createPerson / addChild / addSpouse
3  src/algorithm/dfs.c         blood 간선만 탐색 (spouse 간선 제외)
4  src/data/keywords.h/c       한자어 병기 키워드 + 배우자 교체 테이블
5  src/algorithm/relation.c    LCA 기반 패턴 매칭
6  test/test_relation.c        트리 구성 + 기대값 (4촌 이내)
7  src/main.c                  birth_year 표시, 배우자 사전 체크
8  make && ./test_relation      빌드 및 전체 테스트 통과 확인
```

---

## UI

```
╔══════════════════════════════════════════════════════════════╗
║  [ 조부모 세대 ]    [할아버지]─[할머니]                       ║
╠══════════════════════════════════════════════════════════════╣
║  [ 부모 세대 ]      [백부]─[백모]  [아버지]─[어머니]  [고모]  ║
╠══════════════════════════════════════════════════════════════╣
║  [ 본인 세대 ]      [종형]  [나]  [여동생]  [고종매]          ║
╠══════════════╦═══════════════════════════════════════════════╣
║  [ MENU ]       [ MAIN UI ]                                  
║  1. 추가                                                      
║  2. 수정        결과 출력 영역                                    
║  3. 삭제                                                     
║  4. 조회        > 나와 종형의 관계                               
║  5. 촌수계산     4촌 | 나→종형: 종형(從兄) | 종형→나: 종제(從弟)║
║  6. 호칭출력                                                
║  0. 종료                                                    
║  선택 > _                                                   
╚══════════════╩═══════════════════════════════════════════════╝
```

- 세대별 Deque 3개 사용 (조부모·부모·본인)
- 기준 인물보다 연상 → `pushFront` / 연하 → `pushRear`
- 친가 단일 계열만 표시 (외가 패널 없음)

---

## 주의사항

| 항목 | 세부 내용 |
|---|---|
| `birth_year` 의미 | 값이 작을수록 연상. `백부 조건 : target.birth_year < 아버지.birth_year` |
| 4촌 분기 기준 | `down[0].gender` 단독이 아닌 **양쪽 분기점 모두 확인** (역방향 오판 방지) |
| 구어체 잔존 코드 | `relation_table` 포함 구어체 코드 전체 삭제 |
| 외가 경로 탐색 | 구현 범위 밖 |
| `branch`, `parent2` 필드 | `Person` 구조체에 없음 |

---

## 설계 결정사항

| 항목 | 결정 |
|---|---|
| `birth_year` 역할 | 동항렬(형·동생) 구분용 숫자 |
| 기준 연도 | 2004 (나의 birth_year) |
| 자기 자신 호칭 | `"본인"` |
| 미연결 관계 반환값 | `"관계 없음"` |
| 배우자 역방향 호칭 | `"생질(甥姪)"` |
| 고종형제 세분화 | 고종형·고종제·고종자·고종매 각각 구분 (외종·이종은 외가 제외로 미구현) |
| 호칭 출력 형식 | 한자 병기 (예: `"백부(伯父)"`) |
| `computeChonsu` 배우자 처리 | 호출 전 `p1->spouse == p2` 사전 체크, 나머지 인척은 `"지원하지 않는 관계"` |
| 구현 최대 촌수 | 4촌 이내 (5촌 이상은 세대 범위 초과로 미구현) |

---