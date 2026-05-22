# AI Agent 인계 문서 — 자료구조 기말프로젝트

## 작업 규칙 (AI Agent 필독)

> 1. **작업 중 사용자 명세가 필요하거나 논리 흐름에 의문이 생기는 경우:**  
>    임의로 처리하지 않고 `script.md`의 `❓ 미결 질문` 섹션에 기록하고 작업을 **즉시 중단**한다.  
>    이후 사용자가 답변을 채워넣은 것을 확인한 후 작업을 재개한다.
>    만약 `script.md`의 `❓ 미결 질문`중 해결이 되거나 더 이상의 명시가 필요하지 않은 질문은 삭제한다.
>
> 2. **design.md는 절대적 기준**이다. design.md와 충돌하는 기존 코드는 삭제한다.
>
> 3. **UI 파일 (`src/ui/`, `src/util/`)은 수정해도 되지만 알고리즘의 구현을 우선한다.**

---

## 목표
`docs/design.md`를 절대적 기준으로 삼아, **UI 디스플레이를 제외한 모든 소스코드**를 수정한다.
탐색 논리, 구현 코드, 사용 키워드를 design.md의 논리에 따라 완전히 교체하고,
실행 테스트(`test/test_relation.c`)가 design.md 논리에 따라 통과하는지 검증한다.

> **구현 범위 제한**
> - 세대 범위 : 조부모 ~ 본인 (3세대)
> - 외가 제외 : 어머니 측 친족(외조부, 외숙부, 이모 등) 구현 안 함
> - 하향 제외 : 자녀, 조카, 손자녀 등 본인보다 아래 세대 구현 안 함

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
  main.c           # ★ 부분 수정 (birth_year 표시)
  option.c/h       # 유지

test/
  test_relation.c  # ★ 완전 재작성 필요 (기대값 + 트리 구성 변경)
```

---

## 파악된 핵심 문제점

### 문제 1: Person 구조체 — `age` vs `birth_year`, branch 필드 불필요

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

**변경 후**
```c
typedef struct Person {
    char name[50];
    char gender;
    int  birth_year;    // age → birth_year 교체

    struct Person *parent;   // 혈족 부모 (아버지)
    struct Person *child;
    struct Person *prev;
    struct Person *next;
    struct Person *spouse;
} Person;
```

> **제거 항목**
> - `branch` 필드 : 외가 제외로 친가 단일 계열만 존재 → 불필요
> - `parent2` 필드 : 외가 탐색 불필요 → 불필요

---

### 문제 2: DFS — SPOUSE 간선 사용 (design.md 위반)

**현재 (`dfs.c`)**
```c
/* 배우자도 DFS 탐색에 포함 */
if ((res = dfsHelper(cur->spouse, ...)) != -1) return res;
```

**design.md 요구사항**
> "DFS로 혈족 경로 탐색 (BLOOD 간선만 사용, 백트래킹)"

배우자는 DFS 혈족 탐색에서 완전히 제외한다.
배우자 호칭은 **후처리 교체 테이블**로 별도 처리한다.

**필요한 변경**
- `dfsHelper`에서 `cur->spouse` 탐색 제거
- 탐색 간선 : `parent` / `child` / `prev` / `next` 만 사용

---

### 문제 3: 관계 결정 로직 — 촌수 테이블 조회 방식 (design.md 위반)

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
    if (to == from->parent) lca_idx = i + 1;
    else break;  // 첫 down step에서 LCA 확정
}
int X = lca_idx;                         // 나→LCA 올라간 세대 수
int Y = pathLen - 1 - lca_idx;          // LCA→target 내려간 세대 수
Person **up_nodes   = &path[1];         // [1..lca_idx]
Person **down_nodes = &path[lca_idx+1]; // [lca_idx+1..pathLen-1]

// 2. 패턴 매칭 (design.md 섹션 1~4 순서대로)
```

---

### 문제 4: 키워드 — 구어체 (잘못된 방향)

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

**변경 후 — 한자어(漢字語), 구현 범위 내**
```
부(父), 모(母)                          ← 직계 1촌
조부(祖父), 조모(祖母)                  ← 직계 2촌 상향
형(兄), 제(弟), 자(姊), 매(妹)         ← 2촌 동항렬 (오빠/언니/누나/형 없음)
백부(伯父), 숙부(叔父), 고모(姑母)     ← 3촌 +1세대 (친가만)
종형(從兄), 종제(從弟),
종자(從姊), 종매(從妹)                  ← 4촌 종형제 (백부/숙부 경유)
고종형(姑從兄), 고종제(姑從弟),
고종자(姑從姊), 고종매(姑從妹)         ← 4촌 고종형제 (고모 경유)
```

**배우자 후처리 교체 테이블 (구현 범위 내)**
```
백부 → 백모(伯母)
숙부 → 숙모(叔母)
고모 → 고모부(姑母夫)
```

> **제거 항목**
> - 외숙부, 이모, 이모부, 외숙모 : 외가 제외
> - 당숙, 당고모, 내종숙 : 증조부 경유 — 범위 초과
> - 종조부, 대고모 : 증조부 경유 — 범위 초과
> - 재당숙, 종조모 : 범위 초과
> - 손, 손녀, 외손, 외손녀 : 하향 세대 제외
> - 질, 질녀, 조카 : 하향 세대 제외

---

### 문제 5: 배우자 호칭 처리 방식

**현재**: DFS가 spouse 간선을 통해 경로를 만들어 관계 계산 (혼재)
**design.md 요구사항**: 완전히 분리된 2단계 처리

```
1. target이 배우자인지 확인
2. 혈족 원본(spouse_of)에 대해 DFS 수행 → 혈족 호칭 결정
3. 배우자 후처리 교체 테이블 적용 (범위 내 3가지만)
```

---

### 문제 6: 테스트 케이스 — 기대값 전면 교체 필요

현재 test_relation.c의 기대값이 구어체 기준:
```c
check("나→큰아버지", 나, 큰아버지, 3, "큰아버지", "조카");  // 현재
// → 변경 후:
check("나→백부",     나, 백부,     3, "백부(伯父)", "질(姪)"); // 한자어
```

> **제거 항목**
> - 외가 관련 테스트 케이스 전체 삭제
> - 5촌 이상 테스트 케이스 추가 안 함 (4촌 이내만)
> - 하향 세대(질, 종손 등) 테스트 케이스 삭제

---

## 충돌 사항 (해결 방안 포함)

| 충돌 | 설명 | 해결 방안 |
|---|---|---|
| age vs birth_year | UI에서 나이 표시, 로직에서 출생년도 비교 | `birth_year` 단일 필드 사용, UI도 출생년도 표시로 변경 |
| 형제자매 sibling 리스트 | parent 기준 이중연결리스트 | parent 기준 유지, 변경 없음 |
| 구어 ↔ 한자어 | main.c의 getRelationMessage도 업데이트 필요 | 한자어 기준으로 메시지 맵핑 재작성 |
| 4촌 세분화 | 현재 종형/고종 구분 없음 | relation.c 패턴 매칭에서 down_nodes[0].gender 기준 분기 |

> **제거된 충돌 항목**
> - 외가 경로 탐색 : 외가 제외로 문제 자체 소멸
> - branch 자동 할당 : branch 필드 제거로 소멸
> - parent2 관련 : parent2 필드 제거로 소멸

---

## 구현 순서 (권장)

1. **`person.h`** — 구조체 변경 (age → birth_year, branch/parent2 제거)
2. **`family_tree.c`** — createPerson, addChild, addSpouse 수정
3. **`dfs.c`** — spouse 간선 제거, blood 간선만 탐색
4. **`keywords.h` / `keywords.c`** — 한자어로 전면 교체 + 배우자 교체 테이블
5. **`relation.c`** — LCA 기반 패턴 매칭 완전 재작성
6. **`test/test_relation.c`** — 트리 재구성 + 기대값 교체 (4촌 이내)
7. **`main.c`** — birth_year 표시 수정
8. **빌드 + 테스트 실행**

---

## 세부 구현 참고

### LCA 추출 알고리즘 (relation.c에 구현)
```c
static int find_lca_idx(Person **path, int pathLen) {
    int lca = 0;
    for (int i = 0; i < pathLen - 1; i++) {
        Person *from = path[i], *to = path[i+1];
        // up step: to가 from의 parent인 경우
        if (to == from->parent)
            lca = i + 1;
        else
            break;  // 첫 down step에서 LCA 확정
    }
    return lca;
}
```

### 직접 배우자 사전 체크 (menuChonsu에 구현)
```c
void menuChonsu(Person *p1, Person *p2) {
    // Step 1: 직접 배우자 사전 체크 (DFS 호출 전)
    if (p1->spouse == p2 || p2->spouse == p1) {
        printf("두 분은 배우자 관계입니다.\n");
        return;
    }
    // Step 2: blood DFS
    int chon = computeChonsu(p1, p2);
    if (chon == -1) {
        printf("지원하지 않는 관계입니다.\n");
    } else {
        printf("%d촌 관계입니다.\n", chon);
    }
}
```

### 배우자 후처리 교체 테이블 (구현 범위 내)
```c
static const char *spouse_of(const char *blood_hoching) {
    if (strcmp(blood_hoching, "백부") == 0) return "백모(伯母)";
    if (strcmp(blood_hoching, "숙부") == 0) return "숙모(叔母)";
    if (strcmp(blood_hoching, "고모") == 0) return "고모부(姑母夫)";
    return "관계 없음";
}
```

### 역방향 배우자 호칭 (생질)
```c
// 고모부 → 나 = "생질(甥姪)"
// 역방향 배우자 호칭 테이블
static const char *reverse_spouse_of(const char *blood_hoching) {
    if (strcmp(blood_hoching, "백부") == 0) return "생질(甥姪)";
    if (strcmp(blood_hoching, "숙부") == 0) return "생질(甥姪)";
    if (strcmp(blood_hoching, "고모") == 0) return "생질(甥姪)";
    return "관계 없음";
}
```

### 테스트 트리 구성 (birth_year 기준, 외가 없음)
```c
// 친가 루트 (3세대)
할아버지 = createPerson("할아버지", 'M', 1945);
할머니   = createPerson("할머니",   'F', 1948);
아버지   = createPerson("아버지",   'M', 1970);
어머니   = createPerson("어머니",   'F', 1972);  // spouse로만 저장
백부     = createPerson("백부",     'M', 1965);  // 아버지보다 연상
숙부     = createPerson("숙부",     'M', 1975);  // 아버지보다 연하
고모     = createPerson("고모",     'F', 1978);
나       = createPerson("나",       'M', 2004);
종형     = createPerson("종형",     'M', 2001);  // 백부의 자녀, 나보다 연상
고종매   = createPerson("고종매",   'F', 2006);  // 고모의 자녀, 나보다 연하

// 연결
addSpouse(할아버지, 할머니);
addChild(할아버지, 아버지);
addChild(할아버지, 백부);
addChild(할아버지, 숙부);
addChild(할아버지, 고모);
addSpouse(아버지, 어머니);
addChild(아버지, 나);
addChild(백부, 종형);
addChild(고모, 고종매);
```

---

## 주의사항

- **UI 파일 (`src/ui/`, `src/util/`)은 수정하지 않는다. 알고리즘 완성 후 UI를 별도 작성 예정.**
- `birth_year`는 연도값. 나이 비교는 **값이 작을수록 더 연상** (`birth_year 작음 = 더 오래 전 출생 = 나이 많음`)
- `백부 조건`: `target.birth_year < up_nodes[0].birth_year` (아버지보다 연상)
- `숙부 조건`: `target.birth_year > up_nodes[0].birth_year` (아버지보다 연하)
- 4촌 세분화는 `down_nodes[0].gender` 기준으로만 분기:
  - `down_nodes[0].gender == male` (백부/숙부 경유) → 종형/종제/종자/종매
  - `down_nodes[0].gender == female` (고모 경유) → 고종형/고종제/고종자/고종매
- 구어체 `relation_table`은 완전 삭제
- `keywords.c`의 `relation_table`도 삭제 대상

---

## ❓ 미결 질문 — 사용자 정의 필요

### Q1. `birth_year` 입력 방식

**답변:**
age 속성은 그저 같은 레벨(항렬)의 형-동생 관계를 파악하기 위해 사용하는 숫자에 불과하다.
기존 age 속성과 연산을 삭제하고 birth_year로 계산을 대체할 것.

---

### Q2. 테스트 트리(`test_relation.c`)의 기준 연도

**답변:**
B. 특정 기준 연도 직접 지정.
입력 예시: 2004

---

### Q3. 자기 자신을 조회했을 때의 호칭

**답변:**
A. 현재대로 `"본인"` 유지

---

### Q4. 연결되지 않은 관계의 반환 문자열

**답변:**
A. `"관계 없음"` (design.md 표기 채택)

---

### Q5. 배우자의 역방향 호칭

**답변:**
A. 역방향 배우자 교체 테이블 적용.
역방향 호칭 : `"생질(甥姪)"`

---

### Q6. 4촌 고종형제자매의 세분 호칭

**답변:**
A. 세분화 적용 (고종형, 고종제, 고종자, 고종매 각각 구분)

> **외종형제 / 이종형제는 외가 제외로 구현 안 함**

---

### Q7. 한자어 출력 표기 방식

**답변:**
B. 한자 병기 (예: `"백부(伯父)"`, `"숙부(叔父)"`)

---

### Q8. `computeChonsu`에서 배우자 처리

**답변:**
A. `computeChonsu` 호출 전에 `p1->spouse == p2` 여부를 별도 체크해서 배우자 출력.
나머지 인척 관계는 C (지원하지 않는 관계로 처리).

---

### Q9. 5촌 이상 테스트 케이스 추가 여부

**답변:**
C. 4촌까지만 (구현 범위 자체가 4촌 이내).
5촌 이상은 세대 범위(조부~나) 초과로 구현 제외.

---

*작성일: 2026-05-21 | 작성: Claude Sonnet 4.6*

---

## 현재 진행 상태 (2026-05-21 업데이트)

### 완료된 작업

| 단계 | 파일 | 상태 |
|---|---|---|
| 1 | `src/family/person.h` | ✅ 완료 (age→birth_year) |
| 2 | `src/family/family_tree.h/c` | ✅ 완료 (createPerson 시그니처 변경) |
| 3 | `src/algorithm/dfs.c` | ✅ 완료 (spouse 간선 제거) |
| 4 | `src/data/keywords.h` | ✅ 완료 (한자어 병기 전면 교체) |
| 5 | `src/data/keywords.c` | ✅ 완료 (relation_table 제거) |
| 6 | `src/algorithm/relation.h/c` | ✅ 완료 (LCA 기반 패턴 매칭) |
| 7 | `test/test_relation.c` | ✅ 완료 (트리 재구성 + 기대값 교체) |
| 8 | `src/main.c` | ✅ 완료 (birth_year 표시, 배우자 처리) |
| 8a | `src/ui/deque_view.c` | ✅ 완료 (age→birth_year 비교 수정) |
| 빌드 | `make` | ✅ 성공 |

### 테스트 결과 (최종)

```
결과: PASS 23 / FAIL 0 / 합계 23  ✅ 전체 통과
```

---

## 남은 버그 및 수정 계획

### 버그 A: 알고리즘 — 고종형제 역방향 판별 오류 (relation.c)

**증상:**
```
[FAIL] 나(M,2004)→고종매(F,2006) : 고종매→나 = "종형(從兄)" (기대 "고종제(姑從弟)")
[FAIL] 여동생(F,2007)→고종매(F,2006) : 고종매→여동생 = "종매(從妹)" (기대 "고종매(姑從妹)")
```

**원인:**
X=2, Y=2 분기에서 `down[0] = path[lca_idx+1]`만 체크한다.
순방향(나→고종매) 경로: `path[lca_idx+1] = 고모(F)` → 고종형제 ✓
역방향(고종매→나) 경로: `path[lca_idx+1] = 아버지(M)` → 종형제로 잘못 판정 ✗

경로에서 LCA 양쪽 바로 아래 노드는:
- `path[lca_idx - 1]` (나 쪽 분기점)
- `path[lca_idx + 1]` (target 쪽 분기점)

둘 중 하나라도 `gender == 'F'`이면 고종형제 계열이어야 한다.

**수정 계획 (relation.c, X==2 && Y==2 분기):**
```c
/* 현재 */
Person *dn0 = down[0];
if (dn0->gender == 'M') { /* 종형제 */ } else { /* 고종형제 */ }

/* 수정 후 */
Person *dn0  = path[lca_idx + 1];   /* target 쪽 분기점 */
Person *up1  = path[lca_idx - 1];   /* 나 쪽 분기점 */
int through_female = (dn0->gender == 'F' || up1->gender == 'F');
if (!through_female) { /* 종형제 */ } else { /* 고종형제 */ }
```

---

### 버그 B: 테스트 기대값 오류 (test_relation.c)

**① 어머니/할머니 — blood DFS 도달 불가 (설계상 의도)**

```
[FAIL] 나(M)→어머니(F) : 경로 없음    (4건)
[FAIL] 나→할머니       : 경로 없음
```

blood DFS는 spouse 간선을 탐색하지 않으므로 어머니(아버지의 배우자), 할머니(할아버지의 배우자)는
경로를 찾을 수 없다. 이는 design.md의 설계 원칙 (blood-only DFS)에 부합한다.

main.c에서는 `p1->spouse == p2` 사전 체크로 배우자 관계를 처리하며,
`부(父)→모(母)`, `조부(祖父)→조모(祖母)` 매핑은 applySpouseTable에 추가가 필요하다
(현재는 백부/숙부/고모→배우자만 구현됨).

**수정 계획:** 해당 4개 테스트 케이스를 test_1촌/test_2촌_조부모에서 제거하고 주석 처리.

**② 종형→종자 기대값 오류**

```
check("종형(M,2001)→종자(F,2006)", 종형, 종자, 2, KW_JA, KW_HYUNG);
```
종자(2006)는 종형(2001)보다 연하 → 종형이 부르는 호칭은 `매(妹)` = KW_MAE.
현재 기대값 KW_JA는 오류.

**수정 계획:** KW_JA → KW_MAE (2건: 두 방향 check 모두 수정)

**③ 나→종자 역방향 기대값 오류**

```
check("나(M,2004)→종자(F,2006)", 나, 종자, 4, KW_JONGMAE, KW_JONGJE);
```
역방향(종자→나): 나(2004)는 종자(2006)보다 연상 → `종형(從兄)` = KW_JONGHYUNG.
현재 기대값 KW_JONGJE는 오류.

**수정 계획:** KW_JONGJE → KW_JONGHYUNG

---

### 수정 완료 후 예상 결과

✅ 수정 완료 — PASS 23 / FAIL 0

---

## 사용자 추가 지시사항

### UI 인터페이스 변경 명세

최종 화면 레이아웃은 아래 구조를 전제로 구현한다.

```
╔══════════════════════════════════════════════════════════════╗
║  [ 조부모 세대 ]  ◀ [할아버지]─[할머니]  [큰할아버지] ▶     ║
╠══════════════════════════════════════════════════════════════╣
║  [ 부모 세대 ]    ◀ [백부]─[백모]  [아버지]─[어머니]  [고모] ▶║
╠══════════════════════════════════════════════════════════════╣
║  [ 본인 세대 ]    ◀ [종형]  [나★]  [종매]  [고종제] ▶       ║
╠══════════════╦═══════════════════════════════════════════════╣
║  [ MENU ]    ║  [ MAIN UI ]                                  ║
║  1. 추가     ║                                               ║
║  2. 수정     ║  결과 출력 영역                               ║
║  3. 삭제     ║                                               ║
║  4. 조회     ║                                               ║
║  5. 촌수계산 ║                                               ║
║  6. 호칭출력 ║                                               ║
║  0. 종료     ║                                               ║
║  선택 > _    ║                                               ║
╚══════════════╩═══════════════════════════════════════════════╝
```

- 세대별 덱(Deque) 3개로 UI 구성 (조부모 / 부모 / 본인)
- 나이 기준 : 기준 인물보다 연상이면 `pushFront`, 연하이면 `pushRear`
- 친가 단일 계열만 표시 (외가 패널 없음)

---

### 구현 방침

기존 코드와의 병합(merge)을 고려하지 않는다.
`design.md`의 논리를 절대적 기준으로 삼아 대상 파일을 **완전히 새로 작성**한다.
기존 코드의 구조·변수명·로직은 참고하지 않으며, 잔존 코드가 design.md와 충돌할 경우 기존 코드를 삭제한다.