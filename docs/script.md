# AI Agent 인계 문서
## 자료구조 기말 프로젝트 — 가족 호칭 탐색 시스템

> **작성일:** 2026-05-21 | **버전:** 2.0 (최종 정리본)

---

## ⚙️ Agent 행동 원칙 (필독 후 작업 시작)

| # | 원칙 | 세부 지침 |
|---|---|---|
| 1 | **질문 우선** | 명세가 불명확하거나 논리에 의문이 생기면 임의로 처리하지 않는다. `❓ 미결 질문` 섹션에 기록 후 **즉시 중단**. 사용자 답변 확인 후 재개. 해결된 질문은 즉시 삭제. |
| 2 | **design.md 절대 기준** | `docs/design.md`와 기존 코드가 충돌하면 **기존 코드를 삭제**한다. |
| 3 | **알고리즘 우선** | `src/ui/`, `src/util/`은 수정 가능하나 알고리즘 구현이 항상 우선이다. |

---

## 🎯 프로젝트 목표

`docs/design.md` 기준으로 모든 소스코드를 작성한다.
탐색 논리·구현 코드·호칭 키워드를 design.md의 논리를 기준으로 삼고,
`test/test_relation.c` 전체 통과를 최종 기준으로 삼는다.

### 구현 범위 (명시적 제한)

```
조부모 ~ 본인 (3세대 이내), 친가 단일 계열
```

---

## 📂 수정 대상 파일 목록

```
include/
  stack.h          ─ 수정 불필요 (UI 자료구조)
  deque.h          ─ 수정 불필요 (UI 자료구조)
  ds.h             ─ 수정 불필요

src/
  family/
    person.h       ★ 구조체 변경 (age → birth_year)
    family_tree.h  ★ 함수 시그니처 변경
    family_tree.c  ★ 로직 변경
  algorithm/
    dfs.h          ★ 변경
    dfs.c          ★ 완전 재작성 (spouse 간선 제거)
    relation.h     ★ 변경
    relation.c     ★ 완전 재작성 (LCA 패턴 매칭)
  data/
    keywords.h     ★ 완전 재작성 (구어 → 한자어 병기)
    keywords.c     ★ 완전 재작성 (relation_table 제거)
  ui/              ─ 수정 불필요
  util/            ─ 수정 불필요
  main.c           ★ 부분 수정 (birth_year 표시, 배우자 사전 체크)
  option.c/h       ─ 유지

test/
  test_relation.c  ★ 완전 재작성 (트리 재구성 + 기대값 교체)
```

---

## 핵심 설계 사항 (5가지)

### 1. `Person` 구조체 — `age` → `birth_year`

```c
/* 변경 전 */
typedef struct Person {
    char name[50];
    char gender;
    int  age;           // ← 제거
    int  branch;        // ← 제거 (외가 제외로 불필요)
    struct Person *parent;
    struct Person *parent2; // ← 제거 (외가 탐색 불필요)
    struct Person *child;
    struct Person *prev;
    struct Person *next;
    struct Person *spouse;
} Person;

/* 변경 후 */
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

> `branch`, `parent2` 필드는 설계 범위 축소(외가 제외)로 완전 제거.

---

### 2. DFS — blood 간선만 사용 (spouse 완전 배제)

```c
/* 변경 전 — 잘못된 방식 */
if ((res = dfsHelper(cur->spouse, ...)) != -1) return res;  // ← 삭제

/* 변경 후 — blood 간선만 탐색 */
// parent / child / prev / next 포인터만 사용
// spouse 간선은 DFS에서 완전 제거
// 배우자 호칭은 후처리 교체 테이블로 별도 처리
```

---

### 3. 관계 결정 로직 — 촌수 테이블 → LCA 경로 패턴 매칭

```c
/* 변경 전 — design.md가 명시적으로 "잘못된 방법"으로 지정 */
int chonsu  = upSteps + downSteps;
int genDiff = downSteps - upSteps;
// → 촌수+genDiff 기반 테이블 조회

/* 변경 후 — LCA 기반 경로 패턴 매칭 */
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

### 4. 키워드 — 구어체 → 한자어 병기

```c
/* 제거 대상 (구어체) */
"아버지", "삼촌", "큰아버지", "형", "누나", "오빠", "사촌형" ...

/* 구현 범위 내 키워드 (한자어 병기) */
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

> **완전 삭제 대상:** 외숙부·이모·이모부·외숙모 (외가 제외) / 당숙·당고모·종조부·대고모·재당숙 (증조부 경유, 범위 초과) / 손·손녀·질·질녀 (하향 세대 제외)

---

### 5. 배우자 호칭 — 2단계 분리 처리

```
[기존] DFS가 spouse 간선으로 경로 생성 → 관계 계산 혼재

[변경 후]
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

## 📐 구현 세부 참고

### 4촌 세분화 조건 (relation.c, X==2 && Y==2 분기)

```c
// ⚠️ 역방향 판별 버그 수정 포함
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

## 🔢 구현 순서

```
1  src/family/person.h         구조체 변경 (age→birth_year, branch/parent2 제거)
2  src/family/family_tree.c    createPerson / addChild / addSpouse 수정
3  src/algorithm/dfs.c         spouse 간선 제거, blood 간선만 탐색
4  src/data/keywords.h/c       한자어 병기 전면 교체 + relation_table 제거
5  src/algorithm/relation.c    LCA 기반 패턴 매칭 완전 재작성
6  test/test_relation.c        트리 재구성 + 기대값 교체 (4촌 이내)
7  src/main.c                  birth_year 표시, 배우자 사전 체크 추가
8  make && ./test_relation      빌드 및 전체 테스트 통과 확인
```

---

## UI

```
╔══════════════════════════════════════════════════════════════╗
║  [ 조부모 세대 ]  ◀ [할아버지]─[할머니]                    ▶ ║
╠══════════════════════════════════════════════════════════════╣
║  [ 부모 세대 ]    ◀ [백부]─[백모]  [아버지]─[어머니]  [고모]▶║
╠══════════════════════════════════════════════════════════════╣
║  [ 본인 세대 ]    ◀ [종형]  [나★]  [여동생]  [고종매]      ▶ ║
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

## ⚠️ 주의사항 요약

| 항목 | 세부 내용 |
|---|---|
| `birth_year` 의미 | 값이 작을수록 연상. `백부 조건 : target.birth_year < 아버지.birth_year` |
| 4촌 분기 기준 | `down[0].gender` 단독이 아닌 **양쪽 분기점 모두 확인** (역방향 버그 방지) |
| 구어체 잔존 코드 | `relation_table` 포함 구어체 코드 전체 삭제 |
| 외가 경로 탐색 | 제거 완료. 문제 자체 소멸 |
| `branch`, `parent2` | 제거 완료. 문제 자체 소멸 |


---

## 📋 확정된 설계 결정사항 (Q&A 요약)

| 항목 | 결정 |
|---|---|
| `birth_year` 역할 | 동항렬(형·동생) 구분용 숫자. 기존 `age`의 연산을 `birth_year`로 대체 |
| 기준 연도 | 2004 (나의 birth_year) |
| 자기 자신 호칭 | `"본인"` 유지 |
| 미연결 관계 반환값 | `"관계 없음"` |
| 배우자 역방향 호칭 | `"생질(甥姪)"` |
| 고종형제 세분화 | 고종형·고종제·고종자·고종매 각각 구분 (외종·이종은 외가 제외로 미구현) |
| 호칭 출력 형식 | 한자 병기 (예: `"백부(伯父)"`) |
| `computeChonsu` 배우자 처리 | 호출 전 `p1->spouse == p2` 사전 체크, 나머지 인척은 `"지원하지 않는 관계"` |
| 구현 최대 촌수 | 4촌 이내 (5촌 이상은 세대 범위 초과로 미구현) |

---

*작성일: 2026-05-21 | 버전: 2.0*