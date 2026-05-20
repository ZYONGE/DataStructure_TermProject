# AI Agent 인계 문서 — 자료구조 기말프로젝트

## 목표
`docs/design.md`를 절대적 기준으로 삼아, **UI 디스플레이를 제외한 모든 소스코드**를 수정한다.  
탐색 논리, 구현 코드, 사용 키워드를 design.md의 논리에 따라 완전히 교체하고,  
실행 테스트(`test/test_relation.c`)가 design.md 논리에 따라 통과하는지 검증한다.

---

## 프로젝트 구조 (수정 대상 파일 목록)

```
include/
  stack.h          # 수정 불필요 (UI 자료구조)
  deque.h          # 수정 불필요 (UI 자료구조)
  ds.h             # 수정 불필요

src/
  family/
    person.h       # ★ 구조체 변경 필요
    family_tree.h  # ★ 함수 시그니처 변경 필요
    family_tree.c  # ★ 로직 변경 필요
  algorithm/
    dfs.h          # ★ 변경 필요
    dfs.c          # ★ 완전 재작성 필요
    relation.h     # ★ 변경 필요
    relation.c     # ★ 완전 재작성 필요
  data/
    keywords.h     # ★ 완전 재작성 필요 (구어 → 한자어)
    keywords.c     # ★ 완전 재작성 필요
  ui/              # 수정 불필요 (UI 제외)
  util/            # 수정 불필요
  main.c           # ★ 부분 수정 (birth_year 표시, branch 입력 처리)
  option.c/h       # 유지

test/
  test_relation.c  # ★ 완전 재작성 필요 (기대값 + 트리 구성 변경)
```

---

## 파악된 핵심 문제점

### 문제 1: Person 구조체 — `age` vs `birth_year`, branch 필드 없음

**현재 (`person.h`)**
```c
typedef struct Person {
    char name[50];
    char gender;
    int  age;           // ← 문제: 나이로 비교
    struct Person *parent;
    struct Person *child;
    struct Person *prev;
    struct Person *next;
    struct Person *spouse;
} Person;
```

**design.md 요구사항**
- `birth_year` 사용 (백부/숙부 구분: `target.birth_year < up_nodes[0].birth_year`)
- `branch` 필드 필요 (`PATERNAL` / `MATERNAL`) — 외가/친가 분기 판단
- `parent2` 필드 필요 (두 번째 혈족 부모 포인터, 아래 문제 3 참조)

**필요한 변경**
```c
typedef enum { BRANCH_NONE = 0, BRANCH_PATERNAL, BRANCH_MATERNAL } Branch;

typedef struct Person {
    char name[50];
    char gender;
    int  birth_year;    // age → birth_year 교체
    Branch branch;      // 친가/외가 계열

    struct Person *parent;   // 혈족 부모1 (주로 아버지 쪽)
    struct Person *parent2;  // 혈족 부모2 (주로 어머니 쪽) ← 신규
    struct Person *child;
    struct Person *prev;
    struct Person *next;
    struct Person *spouse;
} Person;
```

---

### 문제 2: DFS — SPOUSE 간선 사용 (design.md 위반)

**현재 (`dfs.c`)**
```c
/* 배우자도 DFS 탐색에 포함 */
if ((res = dfsHelper(cur->spouse, ...)) != -1) return res;
```

**design.md 요구사항**
> "DFS로 혈족 경로 탐색 (BLOOD 간선만 사용, 백트래킹)"

배우자는 DFS 혈족 탐색에서 완전히 제외해야 한다.  
배우자 호칭은 **후처리 교체 테이블**로 별도 처리한다.

**필요한 변경**
- `dfsHelper`에서 `cur->spouse` 탐색 제거
- `cur->parent2` 탐색 추가 (parent와 동등한 혈족 부모)

---

### 문제 3: 어머니 쪽 가계 도달 불가 — 구조적 충돌

**현재 문제**
```
나->parent = 아버지
어머니 = 아버지->spouse
```
DFS에서 spouse를 제거하면, 나 → 어머니 → 외할아버지 경로가 끊긴다.

**해결 방안: `parent2` 필드 추가**
```
나->parent  = 아버지   (혈족 부모1, PATERNAL)
나->parent2 = 어머니   (혈족 부모2, MATERNAL)
```
DFS는 `parent`, `parent2`, children 모두 혈족 간선으로 탐색한다.

**`addChild` 수정**: parent에게 spouse가 있으면 자동으로 parent2 설정
```c
void addChild(Person *parent, Person *child) {
    child->parent = parent;
    child->branch = parent->branch;
    if (parent->spouse) child->parent2 = parent->spouse;  // ← 신규
    // ... 기존 sibling 리스트 연결
}
```
반대로 `addSpouse` 수정: spouse 추가 시 기존 자녀들의 parent2도 설정

---

### 문제 4: 관계 결정 로직 — 촌수 테이블 조회 방식 (design.md 위반)

**현재 (`relation.c`)**
```c
// 촌수 + genDiff 기반 테이블 조회 → design.md가 "틀린 방법"으로 명시
int chonsu  = upSteps + downSteps;
int genDiff = downSteps - upSteps;
```

**design.md 요구사항**
> "호칭 결정은 '촌수 → 테이블 조회'가 아니라  
> '경로(path) 패턴 매칭 → 조건 충족 → 호칭 부여' 순서로 동작한다."

**필요한 변경: LCA 추출 + 경로 패턴 매칭**
```c
// 1. LCA 추출: path에서 올라가다가 처음 내려가는 지점
int lca_idx = 0;
for (int i = 0; i < pathLen - 1; i++) {
    if (is_up_step(path[i], path[i+1])) lca_idx = i + 1;
    else break;
}
int X = lca_idx;                        // 나→LCA 올라간 세대 수
int Y = pathLen - 1 - lca_idx;         // LCA→target 내려간 세대 수
Person **up_nodes   = &path[1];        // [1..lca_idx]
Person **down_nodes = &path[lca_idx+1]; // [lca_idx+1..pathLen-1]

// 2. branch 판별
Branch branch = (X > 0) ? up_nodes[0]->branch : BRANCH_NONE;

// 3. 패턴 매칭 (design.md 섹션 1~8 순서대로)
```

---

### 문제 5: 키워드 — 구어체 (잘못된 방향)

**현재 (`keywords.h`) — 구어(口語)**
```c
#define KW_FATHER         "아버지"
#define KW_UNCLE          "삼촌"
#define KW_BIG_UNCLE      "큰아버지"
#define KW_OLDER_BRO      "형"
#define KW_OLDER_SIS      "누나"
#define KW_OPPA           "오빠"
#define KW_COUSIN_M_OLD_M "사촌형"
```

**design.md 요구사항 — 한자어(漢字語)**
```
부(父), 모(母), 자(子), 녀(女)          ← 직계
조부, 조모, 외조부, 외조모              ← 2촌 상향
증조부, 증조모, 외증조부, 외증조모     ← 3촌 상향
손, 손녀, 외손, 외손녀                 ← 2촌 하향
형(兄), 제(弟), 자(姊), 매(妹)         ← 2촌 동항렬 (오빠/언니/누나 없음)
백부, 숙부, 고모, 외숙부, 이모         ← 3촌 +1세대
질, 질녀                               ← 3촌 -1세대
종형, 종제, 종자, 종매                 ← 4촌 (종형제)
고종형제, 외종형제, 이종형제           ← 4촌 (계열별)
종조부, 대고모                         ← 4촌 +2세대
당숙, 당고모, 내종숙, 외당숙 등       ← 5촌
...
```

design.md 섹션 8: **배우자 후처리 교체 테이블** 별도 구현 필요
```
백부 → 백모, 숙부 → 숙모, 고모 → 고모부 등
```

---

### 문제 6: 배우자 호칭 처리 방식

**현재**: DFS가 spouse 간선을 통해 경로를 만들어 관계 계산 (혼재)  
**design.md 요구사항**: 완전히 분리된 2단계 처리
```
1. target이 배우자인지 확인 (target의 spouse가 혈족 DFS로 도달 가능한지)
2. 혈족 호칭 결정 (DFS로 blood_target 탐색)
3. 배우자 후처리 교체 테이블 적용
```

---

### 문제 7: 외손/외손녀 분기 로직 없음

**현재**: 손자/손녀만 있음, 외손/외손녀 없음  
**design.md 1-2절**:
```
손   (X=2, Y=0, target.gender==male,   up_nodes[0].gender==male)   ← 아들 경유
외손 (X=2, Y=0, target.gender==male,   up_nodes[0].gender==female) ← 딸 경유
```

---

### 문제 8: 테스트 케이스 — 기대값 전면 교체 필요

현재 test_relation.c의 기대값이 구어체 기준:
```c
check("나→큰아버지", 나, 큰아버지, 3, "큰아버지", "조카");  // 현재
// → 변경 후:
check("나→백부",     나, 백부,     3, "백부",     "질");      // 한자어
```

전체 테스트 케이스의 기대값을 한자어로 교체 필요.  
또한 5촌, 6촌, 7촌, 8촌 테스트 케이스 신규 추가 필요.

---

## 충돌 사항 (해결 방안 포함)

| 충돌 | 설명 | 해결 방안 |
|---|---|---|
| age vs birth_year | UI에서 나이 표시, 로직에서 출생년도 비교 | `birth_year` 단일 필드 사용, UI도 출생년도 표시로 변경 |
| 외가 경로 탐색 | spouse 제거 후 어머니 경유 탐색 불가 | `parent2` 필드 추가, addChild에서 자동 설정 |
| branch 자동 할당 | 인터랙티브 입력 시 branch 누가 설정? | addChild 시 parent->branch 상속, 루트 노드는 FAMILY_SIDE 옵션으로 결정 |
| 형제자매 sibling 리스트 | parent2 추가 후 sibling 리스트는 parent1 기준 유지 | parent2는 DFS 탐색 전용, sibling 리스트는 parent 기준 유지 |
| 구어 ↔ 한자어 | main.c의 getRelationMessage도 업데이트 필요 | 한자어 기준으로 메시지 맵핑 재작성 |
| 4촌 세분화 | 현재 종형/고종/외종/이종 구분 없음 → down_nodes[0] 기반으로 분기 필요 | relation.c 패턴 매칭에서 down_nodes[0].branch + down_nodes[0].gender 조합으로 분기 |

---

## 구현 순서 (권장)

1. **`person.h`** — 구조체 변경 (birth_year, branch, parent2)
2. **`family_tree.c`** — createPerson, addChild, addSpouse 수정
3. **`dfs.c`** — spouse 제거, parent2 추가
4. **`keywords.h` / `keywords.c`** — 한자어로 전면 교체 + 배우자 교체 테이블
5. **`relation.c`** — LCA 기반 패턴 매칭 완전 재작성
6. **`test/test_relation.c`** — 트리 재구성 + 기대값 교체 + 5촌 이상 케이스 추가
7. **`main.c`** — birth_year 표시 수정, branch 입력 처리
8. **빌드 + 테스트 실행**

---

## 세부 구현 참고

### LCA 추출 알고리즘 (relation.c에 구현)
```c
static int find_lca_idx(Person **path, int pathLen) {
    int lca = 0;
    for (int i = 0; i < pathLen - 1; i++) {
        Person *from = path[i], *to = path[i+1];
        // up step: to가 from의 parent 또는 parent2인 경우
        if (to == from->parent || to == from->parent2)
            lca = i + 1;
        else
            break;  // 첫 down step에서 LCA 확정
    }
    return lca;
}
```

### 배우자 감지 (relation.c에 구현)
```c
// target이 배우자인지 확인
// target->spouse가 blood DFS로 나에게서 도달 가능하면 배우자 관계
static int is_spouse_target(Person *me, Person *target) {
    if (!target->spouse) return 0;
    Person *path[MAX_PATH_LEN];
    int len = findPath(me, target->spouse, path, MAX_PATH_LEN);
    return len > 0;
}
```

### 배우자 후처리 교체 테이블 (design.md 섹션 8)
```c
static const char *spouse_of(const char *blood_hoching) {
    if (strcmp(blood_hoching, "백부") == 0)    return "백모";
    if (strcmp(blood_hoching, "숙부") == 0)    return "숙모";
    if (strcmp(blood_hoching, "고모") == 0)    return "고모부";
    if (strcmp(blood_hoching, "외숙부") == 0)  return "외숙모";
    if (strcmp(blood_hoching, "이모") == 0)    return "이모부";
    if (strcmp(blood_hoching, "당숙") == 0)    return "당숙모";
    if (strcmp(blood_hoching, "당고모") == 0)  return "당고모부";
    if (strcmp(blood_hoching, "내종숙") == 0)  return "내종숙모";
    if (strcmp(blood_hoching, "종조부") == 0)  return "종조모";
    if (strcmp(blood_hoching, "대고모") == 0)  return "대고모부";
    if (strcmp(blood_hoching, "재당숙") == 0)  return "재당숙모";
    return "미상";
}
```

### 테스트 트리 재구성 핵심 (birth_year + branch + parent2)
```c
// 친가 루트: 증조부 = PATERNAL, birth_year 사용
증조부  = createPerson("증조부",  'M', 1935, BRANCH_PATERNAL);
할아버지 = createPerson("할아버지",'M', 1960, BRANCH_PATERNAL);
아버지  = createPerson("아버지",  'M', 1987, BRANCH_PATERNAL);
나      = createPerson("나",      'M', 2007, BRANCH_PATERNAL);

// 외가 루트: 외할아버지 = MATERNAL
외할아버지 = createPerson("외할아버지",'M', 1963, BRANCH_MATERNAL);
어머니  = createPerson("어머니",  'F', 1989, BRANCH_MATERNAL);

// 연결
addChild(아버지, 나);      // 나->parent=아버지, 자동으로 나->parent2=어머니(spouse)
addSpouse(아버지, 어머니); // + 기존 자녀들의 parent2 설정
addChild(외할아버지, 어머니);
```

---

## 주의사항

- **UI 파일 (`src/ui/`, `src/util/`)은 수정하지 않는다.**
- `birth_year`는 연도값이므로 나이 비교는 **값이 작을수록 더 연상** (`birth_year 작음 = 더 오래 전 출생 = 나이 많음`)
- `백부 조건`: `target.birth_year < up_nodes[0].birth_year` (아버지보다 연상)
- `숙부 조건`: `target.birth_year > up_nodes[0].birth_year` (아버지보다 연하)
- `외숙부`: 나이/결혼 무관하게 무조건 `외숙부` (외백부는 틀린 표현)
- design.md 섹션 3-2 (4촌)에서 down_nodes[0] 기준 분기가 핵심:
  - paternal + male → 종형제
  - paternal + female (고모 경유) → 고종형제
  - maternal + male (외숙부 경유) → 외종형제
  - maternal + female (이모 경유) → 이종형제
- 구어체 `relation_table[5][2][2][3]`은 완전 삭제 (design.md와 방식이 근본적으로 다름)
- `keywords.c`의 `relation_table`도 삭제 대상 (사용 안 함)

---

## 현재 빌드 상태

```bash
# 메인 빌드
make

# 테스트 빌드 및 실행 (현재 상당수 FAIL 예상)
cd test && gcc -std=c11 -I../include -I../src \
    test_relation.c \
    ../src/family/family_tree.c \
    ../src/algorithm/dfs.c \
    ../src/algorithm/relation.c \
    ../src/data/keywords.c \
    -o test_relation && ./test_relation
```

---

*작성일: 2026-05-20 | 작성: Claude Sonnet 4.6*

---

## 사용자 추가 지시사항

### UI 인터페이스 변경 명세

최종 화면 레이아웃은 아래 구조를 전제로 구현한다.
┌─────────────────┬─────────────────┐
│   친가 가계도    │   외가 가계도    │
│  (아버지 기준)   │  (어머니 기준)   │
├─────────────────┴─────────────────┤
│          계산 결과 출력창           │
├─────────────────┬─────────────────┤
│    사용자 입력창  │     옵션 창      │
└─────────────────┴─────────────────┘

- **친가 가계**: "나"의 아버지(부, 父) 계열을 루트로 하는 가계도
- **외가 가계**: "나"의 어머니(모, 母) 계열을 루트로 하는 가계도
- 두 가계도는 UI 상에서 좌우로 분리된 독립 패널로 표시한다
- `branch` 필드(`PATERNAL` / `MATERNAL`)가 이 두 패널의 데이터 분리 기준이 된다

---

### 구현 방침

기존 코드와의 병합(merge)을 고려하지 않는다.  
`design.md`의 논리를 절대적 기준으로 삼아 대상 파일을 **완전히 새로 작성**한다.  
기존 코드의 구조·변수명·로직은 참고하지 않으며, 잔존 코드가 design.md와 충돌할 경우 기존 코드를 삭제한다.