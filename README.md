<div align="center">

# 촌수 계산기 

</div>

#### Languages

<img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white"/>

#### Tools

<img src="https://img.shields.io/badge/Git-F05032?style=for-the-badge&logo=git&logoColor=white"/>
<img src="https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white"/>

#### Platform

<img src="https://img.shields.io/badge/Windows-0078D4?style=for-the-badge&logo=windows&logoColor=white"/>

#### AI Agent

<img src="https://img.shields.io/badge/GitHub%20Copilot-22272E?style=for-the-badge&logo=githubcopilot&logoColor=white"/>
<img src="https://img.shields.io/badge/Claude_Code-D97757?style=for-the-badge&logo=anthropic&logoColor=white"/>

---

## Project Overview

가족 구성원 정보를 입력하면 **트리 + 이중연결리스트** 혼합 자료구조로 저장하고,
두 사람 사이의 촌수를 DFS로 계산하여 한국어 호칭까지 출력하는 프로그램이다.
Windows Console API(TUI)를 통해 가족 구성도가 실시간으로 시각화된다.

---

## Data Structure

| 우선순위 | 자료구조 | 역할 | 구현 방식 |
|---|---|---|---|
| ★ Must | **트리** | 세대 간 부모-자식 수직 관계 저장 | `parent` / `child` 포인터 |
| ★ Must | **이중연결리스트** | 형제자매 간 수평 연결 | `prev` / `next` 포인터 |
| Should | **스택** | DFS 기반 촌수 탐색 경로 추적 | 연결리스트 기반 스택 |
| Nice | **덱** | 세대별 UI 시각화 (나이 기준 정렬) | 원형 배열 기반 덱 |
| Nice | **리스트** | 전체 구성원 목록 관리 및 검색 | 이중 연결리스트 |

### Node Structure

```c
typedef struct Person {
    char  name[50];
    char  gender;           /* 'M' / 'F' */
    int   age;

    struct Person *parent;  /* 부모 (트리)          */
    struct Person *child;   /* 첫째 자식 (트리)      */
    struct Person *prev;    /* 왼쪽 형제 (이중연결)  */
    struct Person *next;    /* 오른쪽 형제 (이중연결)*/
    struct Person *spouse;  /* 배우자               */
} Person;
```

---

## 구현 알고리즘

DFS(깊이 우선 탐색)로 기준점에서 목표 인물까지의 경로를 탐색한다.
스택으로 경로를 추적하고 이동 횟수를 촌수로 환산한다.

```
나 → 아버지 → 할아버지 → 큰아버지 → 사촌  →  4촌
```

촌수 결정 변수: 이동 횟수 / 본인 성별 / 상대 성별 / 세대 방향 / 부계·모계 여부

---

## UI

```
╔══════════════════════════════════════════════════════════════╗
║  [ 조부모 세대 ]                                             ║
║  ◀ [큰할아버지]─[큰할머니]  [할아버지]─[할머니]             ▶║
╠══════════════════════════════════════════════════════════════╣
║  [ 부모 세대 ]                                               ║
║  ◀ [큰아버지]─[큰어머니]  [아버지]─[어머니]  [고모]         ▶║
╠══════════════════════════════════════════════════════════════╣
║  [ 본인 세대 ]                                               ║
║  ◀ [사촌형]  [형]  [나★]─[배우자]  [동생]  [사촌여동생]     ▶║
╠══════════════╦═══════════════════════════════════════════════╣
║  [ MENU ]    ║  [ MAIN UI ]                                  ║
║              ║                                               ║
║  1. 추가     ║  > 선택하신 기능 :                            ║
║  2. 수정     ║                                               ║
║  3. 삭제     ║  이름을 입력하세요 :                          ║
║  4. 조회     ║  _                                            ║
║  ──────────  ║                                               ║
║  5. 촌수계산 ║  결과 :                                       ║
║  6. 호칭출력 ║  당신과 [영희]님은 4촌입니다.                 ║
║  ──────────  ║  당신 기준  : 사촌 여동생                     ║
║  7. 초기화   ║  영희 기준  : 사촌 오빠                       ║
║  0. 종료     ║  경로 : 나→아버지→할아버지→큰아버지→영희      ║
║  선택 > _    ║                                               ║
╚══════════════╩═══════════════════════════════════════════════╝
```

---

## Project Structure

```
자료구조_기말프로젝트/
├── include/              # Header-only data structure library
│   ├── ds.h              # Umbrella header (include this in your source)
│   ├── stack.h           # Linked-list based stack
│   ├── queue.h           # Circular array based queue
│   ├── deque.h           # Circular array based deque
│   ├── list.h            # Doubly linked list
│   ├── graph.h           # Adjacency-list based graph
│   └── tree.h            # Binary search tree (BST)
├── src/
│   ├── main.c            # Entry point
│   └── example.c         # Usage reference
├── docs/
│   └── design.md         # Project design document
├── Makefile
├── LICENSE
├── .gitignore
└── README.md
```

---

## Build

```bash
make
./build/main
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

A+을 위하여 !!!!
