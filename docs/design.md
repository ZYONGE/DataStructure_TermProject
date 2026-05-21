# 한국 가족 호칭 탐색 논리 명세서

> **구현 범위**: 조부모 세대 ~ 본인 세대 (3세대)
> **외가 제외**: 어머니 측 친족(외조부, 외숙부, 이모 등)은 구현 범위에서 제외
> **하향 제외**: 자녀, 조카, 손자녀 등 본인보다 아래 세대는 구현 범위에서 제외

> **핵심 원칙**: 촌수(寸數)와 호칭(呼稱)은 별개의 개념이다.
> - **촌수**: 경로 길이 `X + Y` 로 계산되는 순수 숫자 인덱스
> - **호칭**: 나와 대상 사이의 경로가 **특정 조건을 만족할 때만** 부여되는 문자열
>
> 따라서 호칭 결정은 "촌수 → 테이블 조회"가 아니라
> **"경로(path) 패턴 매칭 → 조건 충족 → 호칭 부여"** 순서로 동작한다.

---

## 용어 정의

| 기호 | 의미 |
|---|---|
| `X` | 나(me)로부터 LCA(최근 공통 조상)까지 **올라간** 세대 수 |
| `Y` | LCA에서 target까지 **내려간** 세대 수 |
| `LCA` | path 내에서 generation이 가장 높은 노드 (공통 조상) |
| `up_nodes` | 나 → LCA 구간 노드 목록 (나 제외, LCA 포함) |
| `down_nodes` | LCA → target 구간 노드 목록 (LCA 제외, target 포함) |
| `down_nodes[0]` | LCA의 직계 자식. **호칭 분기의 핵심 기준점** |
| `is_spouse` | 배우자 노드 여부 (SPOUSE 간선으로 연결됨) |

---

## 1. 직계 (直系) — 상향만 구현

직계는 촌수 숫자를 호칭에 사용하지 않는다. 세대와 성별만으로 결정된다.

### 1-1. 상향 직계 (X == 0, Y > 0)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **부** | 父 | `X==0, Y==1, target.gender==male` |
| **모** | 母 | `X==0, Y==1, target.gender==female` |
| **조부** | 祖父 | `X==0, Y==2, target.gender==male` |
| **조모** | 祖母 | `X==0, Y==2, target.gender==female` |

> **제외 항목**
> - 외조부 / 외조모 : 외가 제외
> - 증조부 / 증조모 : 세대 범위 초과

---

## 2. +1세대 방계 — 부모 항렬

### 2-1. 3촌 (LCA = 조부, X=2, Y=1)

경로: `나 → 부 → 조부 → target`

분기 기준: `up_nodes[0]` (나의 부)와 target의 나이 비교 및 성별

```
경로 예시:
  나 → 부 → 조부 → 백부    [X=2, Y=1, male, 부보다 연상]
  나 → 부 → 조부 → 숙부    [X=2, Y=1, male, 부보다 연하]
  나 → 부 → 조부 → 고모    [X=2, Y=1, female]
```

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **백부** | 伯父 | `X==2, Y==1` `target.gender==male` `target.birth_year < up_nodes[0].birth_year` *(부보다 연상)* |
| **숙부** | 叔父 | `X==2, Y==1` `target.gender==male` `target.birth_year > up_nodes[0].birth_year` *(부보다 연하)* |
| **고모** | 姑母 | `X==2, Y==1` `target.gender==female` |

> **주의 1**: 백부/숙부는 아버지(부)와의 나이 비교로 결정된다.
> **주의 2**: 구어(삼촌)와 한자어(숙부)가 갈리는 지점이 여기다.

> **제외 항목**
> - 외숙부 / 이모 : 외가 제외

---

## 3. 동항렬 (同行列) — 나와 같은 세대 (X == Y)

### 3-1. 2촌 (LCA = 부/모, X=1, Y=1)

경로: `나 → 부/모 → target`

분기 기준: `me.gender` × `target.gender` × `birth_year` 비교

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **형** | 兄 | `X==1, Y==1` `me.gender==male` `target.gender==male` `target.birth_year < me.birth_year` |
| **제** | 弟 | `X==1, Y==1` `me.gender==male` `target.gender==male` `target.birth_year > me.birth_year` |
| **자** | 姊 | `X==1, Y==1` `me.gender==male` `target.gender==female` `target.birth_year < me.birth_year` *(누나의 한자어)* |
| **매** | 妹 | `X==1, Y==1` `me.gender==male` `target.gender==female` `target.birth_year > me.birth_year` |
| **형** | 兄 | `X==1, Y==1` `me.gender==female` `target.gender==male` `target.birth_year < me.birth_year` *(오빠의 한자어)* |
| **제** | 弟 | `X==1, Y==1` `me.gender==female` `target.gender==male` `target.birth_year > me.birth_year` |
| **자** | 姊 | `X==1, Y==1` `me.gender==female` `target.gender==female` `target.birth_year < me.birth_year` *(언니의 한자어)* |
| **매** | 妹 | `X==1, Y==1` `me.gender==female` `target.gender==female` `target.birth_year > me.birth_year` |

> **주의**: 한자어에서는 형/오빠 → 모두 **형(兄)**, 누나/언니 → 모두 **자(姊)**.
> 구어(오빠, 언니)와 한자어(형, 자)가 갈리는 지점이 여기다.

---

### 3-2. 4촌 (LCA = 조부, X=2, Y=2) — 종형제 계열

경로: `나 → 부 → 조부 → down_nodes[0] → target`

분기 기준: `down_nodes[0]`의 성별 (백부/숙부 경유인가, 고모 경유인가)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **종형** | 從兄 | `X==2, Y==2` `down_nodes[0].gender==male` `target.gender==male` `target.birth_year < me.birth_year` |
| **종제** | 從弟 | `X==2, Y==2` `down_nodes[0].gender==male` `target.gender==male` `target.birth_year > me.birth_year` |
| **종자** | 從姊 | `X==2, Y==2` `down_nodes[0].gender==male` `target.gender==female` `target.birth_year < me.birth_year` |
| **종매** | 從妹 | `X==2, Y==2` `down_nodes[0].gender==male` `target.gender==female` `target.birth_year > me.birth_year` |
| **고종형** | 姑從兄 | `X==2, Y==2` `down_nodes[0].gender==female` *(고모 경유)* `target.gender==male` `target.birth_year < me.birth_year` |
| **고종제** | 姑從弟 | `X==2, Y==2` `down_nodes[0].gender==female` `target.gender==male` `target.birth_year > me.birth_year` |
| **고종자** | 姑從姊 | `X==2, Y==2` `down_nodes[0].gender==female` `target.gender==female` `target.birth_year < me.birth_year` |
| **고종매** | 姑從妹 | `X==2, Y==2` `down_nodes[0].gender==female` `target.gender==female` `target.birth_year > me.birth_year` |

> **종형제 통칭 원리**:
> - 백부/숙부 자녀 → **종형제(從兄弟)**
> - 고모 자녀 → **고종형제(姑從兄弟)**

> **제외 항목**
> - 외종형제 (외숙부 자녀) : 외가 제외
> - 이종형제 (이모 자녀) : 외가 제외

---

## 4. 배우자 호칭 — 후처리 교체 테이블

> 배우자 탐색은 DFS 경로 탐색과 **완전히 분리**된 후처리 단계에서 수행한다.
>
> 1. `target.is_spouse == True` 이면, `target.spouse_of` (배우자의 혈족 원본)를 찾는다.
> 2. 혈족 원본에 대해 DFS 수행 → 혈족 호칭 결정
> 3. 아래 교체 테이블로 최종 호칭 변환

| 혈족 호칭 | 배우자 호칭 | 비고 |
|---|---|---|
| 백부 (伯父) | **백모 (伯母)** | 큰어머니 |
| 숙부 (叔父) | **숙모 (叔母)** | 작은어머니 |
| 고모 (姑母) | **고모부 (姑母夫)** | 혈족이 여성이므로 `부(夫)` 접미 |

> **배우자 호칭 생성 원칙**:
> - 윗 항렬 **남성** 혈족의 배우자: 접미사 `부(父)` → `모(母)` 로 교체
> - 윗 항렬 **여성** 혈족의 배우자: 호칭 뒤에 `부(夫)` 를 추가

> **직접 배우자 (배우자 포인터 사전 체크)**
> - `p1->spouse == p2` 조건을 DFS 호출 전에 먼저 확인
> - 해당 시 "배우자 관계입니다" 출력 후 DFS 생략

---

## 5. 한자어 구성 원리 요약 (구현 범위 내)

| 접두사/접미사 | 의미 | 예시 |
|---|---|---|
| **백(伯)** | 아버지보다 손위 남자 형제 | 백부, 백모 |
| **숙(叔)** | 아버지보다 손아래 남자 형제 | 숙부, 숙모 |
| **고(姑)** | 아버지의 여자 형제 (친가) | 고모, 고모부 |
| **종(從)** | 1단계 방계 (공통 조상: 조부) | 종형, 종제, 종자, 종매 |
| **고종(姑從)** | 고모 경유 방계 | 고종형, 고종제 |

---

## 6. 탐색 알고리즘 전체 흐름

```
function get_hoching(me, target):

    # Step 1: 직접 배우자 여부 사전 체크
    if target == me.spouse or me == target.spouse:
        return "배우자"

    # Step 2: 배우자 노드 여부 확인 (간접 배우자)
    blood_target = target
    is_spouse = False
    if target.is_spouse:
        blood_target = target.spouse_of
        is_spouse = True

    # Step 3: DFS로 혈족 경로 탐색 (BLOOD 간선만, 백트래킹)
    path = dfs_backtrack(me, blood_target)
    if path is None:
        return "지원하지 않는 관계"

    # Step 4: LCA 추출
    lca_idx = index_of_max_generation(path)
    X = lca_idx
    Y = len(path) - 1 - lca_idx
    up_nodes = path[1:lca_idx+1]
    down_nodes = path[lca_idx+1:]

    # Step 5: 혈족 호칭 결정 (경로 패턴 매칭)
    blood_hoching = match_hoching_rule(X, Y, up_nodes, down_nodes, me, blood_target)

    # Step 6: 배우자이면 후처리 교체
    if is_spouse:
        return SPOUSE_TABLE[blood_hoching]
    return blood_hoching
```

---

## 7. 같은 촌수, 다른 호칭 — 핵심 케이스 정리 (구현 범위 내)

| 촌수 | 경로 | 호칭 | 차이 원인 |
|:---:|---|---|---|
| 3촌 | 나→부→조부→**백부(연상 남)** | **백부** | birth_year < 부 |
| 3촌 | 나→부→조부→**숙부(연하 남)** | **숙부** | birth_year > 부 |
| 3촌 | 나→부→조부→**고모(여)** | **고모** | target.gender==female |
| 4촌 | 나→부→조부→**백부/숙부(남)**→종형제 | **종형/종제/종자/종매** | down_nodes[0].gender==male |
| 4촌 | 나→부→조부→**고모(여)**→고종형제 | **고종형/고종제/고종자/고종매** | down_nodes[0].gender==female |

---

## 8. 구현 제외 항목 전체 목록

| 항목 | 제외 이유 |
|---|---|
| 외조부 / 외조모 | 외가 제외 |
| 외숙부 / 이모 / 이모부 | 외가 제외 |
| 외종형제 / 이종형제 | 외가 제외 |
| 증조부 / 증조모 이상 | 세대 범위 초과 |
| 자 / 녀 (자녀) 이하 | 하향 세대 제외 |
| 질 / 질녀 (조카) | 하향 세대 제외 |
| 종손 / 재종손 | 하향 세대 제외 |
| 당숙 / 당고모 | 증조부 경유 — 세대 범위 초과 |
| 종조부 / 대고모 | 증조부 경유 — 세대 범위 초과 |
| 재종형제 / 삼종형제 | 세대 범위 초과 |