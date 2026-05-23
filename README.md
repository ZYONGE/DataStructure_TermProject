<div align="center">

# 촌수 계산기

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

## Project Overview

가족 구성원 정보를 입력하면 **트리 + 이중연결리스트** 혼합 자료구조로 저장하고,
두 사람 사이의 촌수를 혈족 DFS로 계산하여 한국어 호칭(한자어 병기)까지 출력하는 TUI 프로그램이다.

- **세대 범위**: 조부모 ~ 본인 (3세대, 친가)
- **촌수 범위**: 4촌 이내
- **호칭 방식**: 한자어 병기 (예: `백부(伯父)`, `종형(從兄)`)

---

## Data Structure

| 우선순위 | 자료구조 | 역할 | 구현 방식 |
|---|---|---|---|
| ★ Must | **트리** | 세대 간 부모-자식 수직 관계 저장 | `parent` / `child` 포인터 |
| ★ Must | **이중연결리스트** | 형제자매 간 수평 연결 | `prev` / `next` 포인터 |
| Should | **덱** | 세대별 UI 시각화 (출생연도 기준 정렬) | 원형 배열 기반 덱 |

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

## 구현 알고리즘

### 1. 혈족 DFS (Blood-only DFS)

`spouse` 간선을 완전히 제외하고 `parent` / `child` 혈족 간선만으로 경로를 탐색한다.
배우자 호칭은 혈족 경로 탐색 후 **후처리 교체 테이블**로 별도 처리한다.

```
나 → 아버지 → 할아버지 → 백부 → 종형   (혈족 경로 길이 5)
```

### 2. LCA 추출 (Lowest Common Ancestor)

경로에서 올라가다가 처음 내려가는 전환점을 LCA로 확정한다.

```
path = [나, 아버지, 할아버지, 백부, 종형]
         ↑(up)    ↑(up)      ↓(down) ↓(down)

LCA = 할아버지 (lca_idx = 2)
X = 2  (나 → LCA 올라간 세대)
Y = 2  (LCA → 종형 내려간 세대)
```

| lca_idx 판정 조건 | 의미 |
|---|---|
| `last_up == pathLen - 2` | 전체 상향 (조상 직계) → `lca_idx = 0` |
| `last_up == -1` | 전체 하향 (자손 직계) → `lca_idx = pathLen - 1` |
| 혼합 | `lca_idx = last_up + 1` |

### 3. 경로 패턴 매칭 → 호칭 결정

촌수 테이블 조회 방식이 아닌, `(X, Y, 경로 노드)` 패턴으로 호칭을 결정한다.

| X | Y | 조건 | 호칭 (남/녀) |
|---|---|---|---|
| 0 | 1 | 직계 부모 | 부(父) / 모(母) |
| 0 | 2 | 직계 조부모 | 조부(祖父) / 조모(祖母) |
| 1 | 1 | 동항렬 2촌 | 형(兄)/제(弟) / 자(姊)/매(妹) |
| 2 | 1 | 부모 항렬 3촌 | 백부(伯父)/숙부(叔父)/고모(姑母) |
| 1 | 2 | 자녀 항렬 3촌 역방향 | 질(姪) / 질녀(姪女) |
| 2 | 2 | 동항렬 4촌 | 종형제 계열 / 고종형제 계열 |

4촌 계열 분기: `path[lca_idx-1]` 또는 `path[lca_idx+1]` 중 하나라도 여성(`F`)이면 고종형제 계열.

### 4. 배우자 후처리 교체 테이블

| 혈족 호칭 | → 배우자 호칭 | 역방향 |
|---|---|---|
| 백부(伯父) | 백모(伯母) | 생질(甥姪) |
| 숙부(叔父) | 숙모(叔母) | 생질(甥姪) |
| 고모(姑母) | 고모부(姑母夫) | 생질(甥姪) |

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
║  [ MENU ]    ║  [ MAIN UI ]                                  ║
║  1. 추가     ║                                               ║
║  2. 수정     ║  결과 출력 영역                               ║
║  3. 삭제     ║                                               ║
║  4. 조회     ║  > 나와 종형의 관계                           ║
║  5. 촌수계산 ║  4촌 | 나→종형: 종형(從兄) | 종형→나: 종제(從弟)║
║  6. 호칭출력 ║                                               ║
║  0. 종료     ║                                               ║
║  선택 > _    ║                                               ║
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
├── .vscode/
│   └── tasks.json        # VS Code 빌드 태스크 (Ctrl+Shift+B)
├── Makefile
├── LICENSE
├── .gitignore
└── README.md
```

---

## Build

### 사전 준비 (최초 1회)

**macOS**

```bash
xcode-select --install
```

**Windows** — MSYS2 MinGW64 gcc 사용 (MSYS2 터미널 불필요)

MSYS2가 설치되어 있으면 추가 설치 없이 바로 사용할 수 있다.  
gcc 경로는 `.vscode/terminal_profile.ps1` 에 자동으로 설정되어 있다.

```
C:\msys64\mingw64\bin\gcc.exe
```

MSYS2가 없다면 [winlibs.com](https://winlibs.com/) → **UCRT runtime** → 최신 ZIP을 `C:\mingw64` 에 압축 해제 후,  
`terminal_profile.ps1` 첫 줄의 경로를 `C:\mingw64\bin` 으로 변경한다.

---

### Windows 빌드 메뉴얼

VS Code에서 프로젝트 폴더를 연 뒤, 내장 터미널(`Ctrl + `` `)을 새로 열면  
`PowerShell (촌수계산기)` 프로필이 자동으로 로드되어 `build` 명령을 바로 사용할 수 있다.

**1단계 — 컴파일러 설치 확인**

```powershell
gcc --version
```

**2단계 — 최신 소스 받기**

```powershell
git pull
```

**3단계 — .exe 빌드** (alias 한 줄)

```powershell
build
```

빌드 성공 시 프로젝트 루트(`README.md` 와 같은 위치)에 `family_tree.exe` 가 생성된다.

> `Ctrl + Shift + B` → **Build family_tree.exe** 를 선택해도 동일하게 빌드된다. (`.vscode/tasks.json`)

**4단계 — 실행**

```powershell
.\family_tree.exe
```

---

### Windows 테스트 메뉴얼

```powershell
cd test
```

```powershell
gcc -std=c11 -I../include -I../src `
    test_relation.c `
    ../src/family/family_tree.c `
    ../src/algorithm/dfs.c `
    ../src/algorithm/relation.c `
    ../src/data/keywords.c `
    -o test_relation.exe -static
```

```powershell
.\test_relation.exe
```

```powershell
cd ..
```

---

### Windows 빌드 결과물 삭제

```cmd
del family_tree.exe
```

---

### macOS / Linux 빌드 메뉴얼

```bash
git pull
```

```bash
make
```

```bash
./family_tree
```

테스트:

```bash
cd test && gcc -std=c11 -I../include -I../src \
    test_relation.c \
    ../src/family/family_tree.c \
    ../src/algorithm/dfs.c \
    ../src/algorithm/relation.c \
    ../src/data/keywords.c \
    -o test_relation && ./test_relation
```

정리:

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
