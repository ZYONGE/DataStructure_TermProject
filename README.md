<div align="center">

# 호칭 출력기

</div>

#### Languages

<img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white"/>

#### Tools

<img src="https://img.shields.io/badge/Git-F05032?style=for-the-badge&logo=git&logoColor=white"/>
<img src="https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white"/>

#### Platform

<img src="https://img.shields.io/badge/macOS-000000?style=for-the-badge&logo=apple&logoColor=white"/>
<img src="https://img.shields.io/badge/Windows-0078D4?style=for-the-badge&logo=windows&logoColor=white"/>

#### AI Agent

<img src="https://img.shields.io/badge/Claude_Code-D97757?style=for-the-badge&logo=anthropic&logoColor=white"/>

---
## Table of Contents

- [Project Overview](#project-overview)
- [자료구조 활용 명세](#자료구조-활용-명세data-structures)
  - [트리](#트리tree)
  - [이중연결리스트](#이중-연결-리스트doubly-linked-list)
  - [덱](#덱deque)
  - [스택](#스택stack)
- [알고리즘 구현 명세](#알고리즘-구현-명세algorithms)
  - [혈족 경로 깊이 탐색](#혈족-경로-깊이탐색blood-only-dfs)
  - [최소 공통조상 탐색](#lca-lowest-common-ancestor)
  - [호칭 계산](#relation-matching-lca-pattern-matching)
  - [배우자 연산 처리](#spouse-post-processing)
  - [Node Structure](#node-structure)
- [UI](#ui)
- [Project Structure](#project-structure)
- [Build](#build)
  - [Prerequisites](#prerequisites)
  - [Compile](#compile)
  - [Run](#run)
  - [Test](#test)
  - [Clean](#clean)
- [License](#license)
- [Author](#author)
- [Motivation](#motivation)
---

## Project Overview

가족 구성원 정보를 입력하면 **트리 + 이중연결리스트** 혼합 자료구조로 저장하고,
두 사람 사이의 촌수를 혈족 DFS로 계산하여 한국어 호칭(한자어 병기)까지 출력하는 프로그램입니다.

- **세대 범위**: 조부모 ~ 본인 (3세대, 친가)
- **촌수 범위**: 4촌 이내
- **호칭 방식**: 한자어 병기 (예: `백부(伯父)`, `종형(從兄)`)

---

## 자료구조 활용 명세(Data Structures)

이 프로젝트는 `Person` 노드 하나에 **트리 포인터**와 **이중연결리스트 포인터**를 함께 담아, 두 자료구조를 하나의 구조체로 혼합 운용합니다.

```c
typedef struct Person {
    char name[50];
    char gender;          /* 'M' / 'F'               */
    int  birth_year;      /* 출생연도 — 작을수록 연상  */

    struct Person *parent; /* 트리 — 혈족 부모         */
    struct Person *child;  /* 트리 — 첫째 자식         */
    struct Person *prev;   /* 이중연결리스트 — 왼쪽 형제 */
    struct Person *next;   /* 이중연결리스트 — 오른쪽 형제 */
    struct Person *spouse; /* 배우자 (혈족 탐색 제외)   */
} Person;
```

### 트리(Tree)

`parent`와 `child` 포인터가 세대 간 수직 관계를 표현합니다.
루트는 조부이며, 3세대(조부모 → 부모 → 본인)를 내려가는 단일 계통 트리입니다.

- `addChild(parent, child)` — 부모 노드의 자식 리스트 끝에 새 노드를 연결합니다.
- 탐색(`dfs.c`)과 호칭 결정(`relation.c`)은 이 blood 간선만 사용합니다.
- 외가를 구현 범위에서 제외했기 때문에 부모 포인터는 단일(`*parent`)로 충분합니다.

```
할아버지 (루트)
├── 백부   (1965)
├── 아버지 (1970)
├── 숙부   (1975)
└── 고모   (1978)
      └── 고종매 (2006)
아버지
└── 나 (2004, 기준 인물)
백부
└── 종형 (2001)
```

### 이중 연결 리스트(Doubly Linked List)

`prev`와 `next` 포인터가 동일 세대의 형제자매를 수평으로 연결합니다.

`addChild`를 호출할 때 기존 자식 리스트의 맨 끝 노드에 `next`로 이어 붙이고, 새 노드의 `prev`를 역방향으로 연결합니다.

- 세대 수집 시 `leftmost->prev`를 따라 맨 앞 형제를 찾은 뒤, `cur = cur->next`로 오른쪽을 순회합니다.
- 삽입과 순회 모두 O(1) ~ O(n)으로, 형제 수에 비례하는 선형 시간만 소요됩니다.

```
백부 ↔ 아버지 ↔ 숙부 ↔ 고모   (부모 세대, prev/next 연결)
종형 ↔ 나                      (본인 세대, prev/next 연결)
```

### 덱(Deque)

`include/deque.h`에 **원형 배열 기반 덱**이 구현되어 있으며, UI 렌더링(`src/ui/deque_view.c`)에서 세대별 인물을 정렬 표시하는 데 사용합니다.

프로그램은 세대별 덱 3개(`g_deques[3]`)를 유지합니다.

| 인덱스 | 세대 |
|---|---|
| `g_deques[0]` | 조부모 세대 |
| `g_deques[1]` | 부모 세대 |
| `g_deques[2]` | 본인 세대 |

인물을 덱에 삽입할 때 `birth_year`를 기준으로 삼습니다.
기준 인물보다 연상(birth_year 작음)이면 `dequePushFront`, 연하(birth_year 큼)이면 `dequePushBack`으로 삽입합니다.
그 결과 별도의 정렬 단계 없이 화면 왼쪽부터 연상 순으로 자동 배치됩니다.

원형 배열 구조이므로 양쪽 삽입·삭제가 모두 O(1)이며, 인덱스 계산에 모듈러 연산(`% DEQUE_MAX_SIZE`)을 사용합니다.

### 스택(Stack)

`include/stack.h`에 **연결 리스트 기반 스택**이 구현되어 있습니다.
`push`, `pop`, `peek`, `search` 연산을 제공하며, 모든 연산은 O(1)입니다.

DFS(`dfs.c`)가 재귀로 구현되어 있어 경로 탐색 중에는 C의 호출 스택이 백트래킹 프레임을 암묵적으로 누적합니다.
탐색이 완료된 경로는 `path[]` 배열로 추출되어 이후 LCA 인덱스 계산과 호칭 결정에 사용됩니다.

---

## 알고리즘 구현 명세(Algorithms)

### 혈족 경로 깊이탐색(Blood-only DFS)

`src/algorithm/dfs.c`의 `findPath` 함수는 두 인물 사이의 혈족 경로를 깊이 우선 탐색으로 구합니다.

핵심 설계 원칙은 **spouse 간선을 탐색에서 완전히 제외**하는 것입니다.
배우자 포인터를 DFS에 포함하면 혈족이 아닌 경로가 생성되어 호칭 계산이 어긋납니다.
대신 배우자 호칭은 혈족 경로를 먼저 구한 뒤 후처리 교체 테이블로 별도 처리합니다.

```
탐색 간선: parent / child / prev / next   (blood 간선만)
제외 간선: spouse
```

탐색 흐름은 다음과 같습니다.

1. 출발 노드에서 `parent`, `child`, `prev`, `next`를 재귀적으로 방문합니다.
2. 목표 노드를 찾으면 현재까지의 `path[]`를 반환합니다.
3. 찾지 못하면 백트래킹하여 다음 간선을 시도합니다.

경로 예시 — 나에서 종형까지:

```
path = [나, 아버지, 할아버지, 백부, 종형]
```

### LCA (Lowest Common Ancestor)

`src/algorithm/relation.c`의 `find_lca_idx` 함수는 `findPath`가 반환한 경로 배열에서 최소 공통 조상의 위치를 추출합니다.

경로를 순서대로 읽으면서 `path[i+1] == path[i]->parent`가 성립하는 동안은 상향(up) 구간이고, 처음으로 방향이 바뀌는 지점이 LCA입니다.

```
path  = [나,    아버지,  할아버지,  백부,    종형]
방향  =     ↑(up)  ↑(up)      ↓(down)  ↓(down)
lca_idx = 2   →   LCA = 할아버지
X = 2  (나 → LCA, 올라간 세대 수)
Y = 2  (LCA → 종형, 내려간 세대 수)
```

| lca_idx 판정 조건 | 의미 |
|---|---|
| 경로 전체가 상향 | 직계 조상 방향 |
| 경로 전체가 하향 | 직계 자손 방향 |
| 혼합 | LCA = 방향 전환점 |

### Relation Matching (LCA Pattern Matching)

`(X, Y, 경로 노드 성별)` 세 가지를 조합한 **경로 패턴 매칭**으로 호칭을 결정합니다.

| X | Y | 조건 | 호칭 (남 / 녀) |
|---|---|---|---|
| 0 | 1 | 직계 부모 | 부(父) / 모(母) |
| 0 | 2 | 직계 조부모 | 조부(祖父) / 조모(祖母) |
| 1 | 1 | 동항렬 2촌 | 형(兄) · 제(弟) / 자(姊) · 매(妹) |
| 2 | 1 | 부모 항렬 3촌 | 백부(伯父) · 숙부(叔父) / 고모(姑母) |
| 2 | 2 | 동항렬 4촌 | 종형제 계열 / 고종형제 계열 |

4촌(`X == 2, Y == 2`) 분기는 LCA 양쪽 분기점의 성별을 모두 확인합니다.

- `down[0]`(LCA에서 내려가는 첫 노드) 또는 `up[-1]`(나에서 올라가는 마지막 노드) 중 하나라도 여성(`'F'`)이면 **고종형제 계열**
- 둘 다 남성(`'M'`)이면 **종형제 계열**

백부·숙부 구분은 `birth_year`로 판정합니다.
`target.birth_year < father.birth_year`이면 백부(아버지보다 연상), 크면 숙부(아버지보다 연하)입니다.

### Spouse Post-processing

배우자 호칭은 DFS에서 처리하지 않고, 혈족 경로 탐색 이후 2단계로 처리합니다.

1. `target == p->spouse` 여부를 사전 체크합니다.
2. 배우자의 혈족 원본에 대해 blood DFS를 수행합니다.
3. 혈족 호칭에 교체 테이블을 적용하여 최종 호칭을 결정합니다.

```
백부  →  백모(伯母)
숙부  →  숙모(叔母)
고모  →  고모부(姑母夫)
```

역방향(배우자 측에서 나를 부르는 호칭)은 `생질(甥姪)`로 반환합니다.

### Node Structure

```c
typedef struct Person {
    char  name[50];
    char  gender;           /* 'M' / 'F'             */
    int   birth_year;       /* 출생연도 (작을수록 연상) */

    struct Person *parent;  /* 부모 (트리)            */
    struct Person *child;   /* 첫째 자식 (트리)        */
    struct Person *prev;    /* 왼쪽 형제 (이중연결)    */
    struct Person *next;    /* 오른쪽 형제 (이중연결)  */
    struct Person *spouse;  /* 배우자                 */
} Person;
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

- 조부모/부모/본인 세대별 덱(Deque) 3개로 구성
- 출생연도 기준 연상이면 `pushFront`, 연하이면 `pushBack`
- 현재 선택된 인물 강조 표시 (노란색)

---

## Project Structure

```
자료구조_기말프로젝트/
├── include/
│   ├── ds.h              # Umbrella header
│   ├── stack.h           # Linked-list based stack
│   └── deque.h           # Circular array based deque
├── src/
│   ├── main.c            # Entry point, menu 처리
│   ├── option.c/h        # 메뉴 옵션 핸들러
│   ├── family/
│   │   ├── person.h      # Person 구조체 정의
│   │   ├── family_tree.h
│   │   └── family_tree.c # createPerson / addChild / addSpouse / findPerson
│   ├── algorithm/
│   │   ├── dfs.h/c       # 혈족 DFS (findPath)
│   │   ├── relation.h/c  # LCA 추출 / 호칭 결정 / 촌수 계산
│   ├── data/
│   │   ├── keywords.h    # 한자어 호칭 상수 정의
│   │   └── keywords.c    # getRelationMessage (호칭별 출력 메시지)
│   ├── ui/
│   │   ├── ui.h/c        # TUI 렌더링 (gotoxy, setColor, clearArea)
│   │   └── deque_view.h/c# 세대별 덱 뷰 (3-row deque display)
│   └── util/
│       └── util.h/c      # 유틸리티 함수
├── test/
│   └── test_relation.c   # 촌수·호칭 단위 테스트 (23 케이스)
├── docs/
│   ├── design.md         # 알고리즘 설계 문서 (구현 기준)
│   └── script.md         # 개발 인계 문서
├── Makefile
├── LICENSE
├── .gitignore
└── README.md
```

---

## Build

### Prerequisites

**macOS**

```bash
xcode-select --install
```

**Windows** — [MSYS2](https://www.msys2.org/) 설치 후 MINGW64 터미널에서 실행:

```bash
pacman -S mingw-w64-x86_64-gcc make
```

### Compile

프로젝트 루트 디렉터리에서 실행합니다.

```bash
make
```

빌드가 완료되면 프로젝트 루트에 실행 파일이 생성됩니다.

### Run

**macOS / Linux**

```bash
./family_tree
```

**Windows** (MSYS2 MinGW64 터미널)

```bash
./family_tree.exe
```

### Test

```bash
cd test
gcc -std=c11 -I../include -I../src \
    test_relation.c \
    ../src/family/family_tree.c \
    ../src/algorithm/dfs.c \
    ../src/algorithm/relation.c \
    ../src/data/keywords.c \
    -o test_relation && ./test_relation
```

### Clean

```bash
make clean
```

---

## License

This project is licensed under the MIT License.

See the [LICENSE](LICENSE) file for details.

---

## Author

Name: Jiyong Kim (ZYONGE)

Profile: https://github.com/ZYONGE

## Motivation

A+를 위하여 !!!!
