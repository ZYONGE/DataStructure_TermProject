# 한국 가족 호칭 탐색 논리 명세서

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
| `branch` | `paternal`(친가·아버지 쪽) / `maternal`(외가·어머니 쪽) |
| `is_spouse` | 배우자 노드 여부 (SPOUSE 간선으로 연결됨) |

---

## 1. 직계 (直系) — X == 0 or Y == 0

직계는 촌수 숫자를 호칭에 사용하지 않는다. 세대와 성별만으로 결정된다.

### 1-1. 상향 직계 (X == 0, Y > 0)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **부** | 父 | `X==0, Y==1, target.gender==male` |
| **모** | 母 | `X==0, Y==1, target.gender==female` |
| **조부** | 祖父 | `X==0, Y==2, target.gender==male, target.branch==paternal` |
| **조모** | 祖母 | `X==0, Y==2, target.gender==female, target.branch==paternal` |
| **외조부** | 外祖父 | `X==0, Y==2, target.gender==male, target.branch==maternal` |
| **외조모** | 外祖母 | `X==0, Y==2, target.gender==female, target.branch==maternal` |
| **증조부** | 曾祖父 | `X==0, Y==3, target.gender==male, target.branch==paternal` |
| **증조모** | 曾祖母 | `X==0, Y==3, target.gender==female, target.branch==paternal` |
| **외증조부** | 外曾祖父 | `X==0, Y==3, target.gender==male, target.branch==maternal` |
| **외증조모** | 外曾祖母 | `X==0, Y==3, target.gender==female, target.branch==maternal` |

> **규칙**: 외가(어머니 쪽)는 `외(外)` 접두사를 붙인다.

---

### 1-2. 하향 직계 (X > 0, Y == 0)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **자** | 子 | `X==1, Y==0, target.gender==male` |
| **녀** | 女 | `X==1, Y==0, target.gender==female` |
| **손** | 孫 | `X==2, Y==0, target.gender==male, up_nodes[0].gender==male` *(아들 경유)* |
| **손녀** | 孫女 | `X==2, Y==0, target.gender==female, up_nodes[0].gender==male` |
| **외손** | 外孫 | `X==2, Y==0, target.gender==male, up_nodes[0].gender==female` *(딸 경유)* |
| **외손녀** | 外孫女 | `X==2, Y==0, target.gender==female, up_nodes[0].gender==female` |
| **증손** | 曾孫 | `X==3, Y==0, target.gender==male, up_nodes[0].gender==male` |
| **증손녀** | 曾孫女 | `X==3, Y==0, target.gender==female, up_nodes[0].gender==male` |
| **외증손** | 外曾孫 | `X==3, Y==0, target.gender==male, up_nodes[0].gender==female` |
| **외증손녀** | 外曾孫女 | `X==3, Y==0, target.gender==female, up_nodes[0].gender==female` |

> **규칙**: 아들 계열은 손/증손, 딸 계열(up_nodes[0].gender == female)은 외손/외증손.

---

## 2. +1세대 방계 — 부모 항렬

### 2-1. 3촌 (LCA = 조부, X=2, Y=1)

경로: `나 → 부/모 → 조부/조모 → target`

분기 기준: `up_nodes[0].branch` (나의 부 또는 모가 어느 계열인가)

```
경로 예시:
  나 → 부(paternal) → 조부 → 백부    [X=2, Y=1, paternal, male, 부보다 연상]
  나 → 부(paternal) → 조부 → 숙부    [X=2, Y=1, paternal, male, 부보다 연하, 기혼]
  나 → 부(paternal) → 조부 → 고모    [X=2, Y=1, paternal, female]
  나 → 모(maternal) → 외조부 → 외숙부 [X=2, Y=1, maternal, male]
  나 → 모(maternal) → 외조모 → 이모  [X=2, Y=1, maternal, female]
```

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **백부** | 伯父 | `X==2, Y==1` `up_nodes[0].branch==paternal` `target.gender==male` `target.birth_year < up_nodes[0].birth_year` *(부보다 연상)* |
| **숙부** | 叔父 | `X==2, Y==1` `up_nodes[0].branch==paternal` `target.gender==male` `target.birth_year > up_nodes[0].birth_year` *(부보다 연하)* |
| **고모** | 姑母 | `X==2, Y==1` `up_nodes[0].branch==paternal` `target.gender==female` |
| **외숙부** | 外叔父 | `X==2, Y==1` `up_nodes[0].branch==maternal` `target.gender==male` *(손위·손아래 구분 없이 모두 외숙부)* |
| **이모** | 姨母 | `X==2, Y==1` `up_nodes[0].branch==maternal` `target.gender==female` |

> **주의 1**: 백부/숙부는 아버지(부)와의 나이 비교로 결정된다.  
> **주의 2**: 외숙부는 결혼 여부·나이 무관하게 항상 `외숙부`. 외백부(外伯父)는 **틀린 표현**.  
> **주의 3**: 구어(삼촌/외삼촌)와 한자어(숙부/외숙부)가 갈리는 지점이 여기다.

---

### 2-2. 5촌 (LCA = 증조부, X=3, Y=2)

경로: `나 → 부 → 조부 → 증조부 → down_nodes[0] → target`

분기 기준: **`down_nodes[0]`** (증조부의 직계 자식이 누구인가)

```
경로 예시:
  증조부 → [조부의 형제(남)] → target  →  당숙/당고모
  증조부 → [조부의 자매(여)] → target  →  내종숙/내종고모  ← 대고모 경유
  외증조부 → [외조부의 형제(남)] → target  →  외당숙
```

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **당숙** (종숙) | 堂叔 (從叔) | `X==3, Y==2` `down_nodes[0].branch==paternal` `down_nodes[0].gender==male` `target.gender==male` |
| **당고모** (종고모) | 堂姑母 (從姑母) | `X==3, Y==2` `down_nodes[0].branch==paternal` `down_nodes[0].gender==male` `target.gender==female` |
| **내종숙** | 內從叔 | `X==3, Y==2` `down_nodes[0].branch==paternal` `down_nodes[0].gender==female` *(대고모 경유)* `target.gender==male` |
| **내종고모** | 內從姑母 | `X==3, Y==2` `down_nodes[0].branch==paternal` `down_nodes[0].gender==female` `target.gender==female` |
| **외당숙** (외종숙) | 外堂叔 | `X==3, Y==2` `down_nodes[0].branch==maternal` `down_nodes[0].gender==male` `target.gender==male` |
| **외당고모** | 外堂姑母 | `X==3, Y==2` `down_nodes[0].branch==maternal` `down_nodes[0].gender==male` `target.gender==female` |
| **이종숙** | 姨從叔 | `X==3, Y==2` `down_nodes[0].branch==maternal` `down_nodes[0].gender==female` *(이모 경유)* `target.gender==male` |
| **당이모** | 堂姨母 | `X==3, Y==2` `down_nodes[0].branch==maternal` `down_nodes[0].gender==female` `target.gender==female` |

> **핵심**: `down_nodes[0]`이 남자(조부의 형제)이면 당숙 계열,  
> `down_nodes[0]`이 여자(조부의 자매 = 대고모)이면 내종숙 계열.  
> 같은 5촌이라도 경로가 다르면 호칭이 완전히 달라진다.

---

### 2-3. 7촌 (LCA = 고조부, X=4, Y=3)

경로: `나 → 부 → 조부 → 증조부 → 고조부 → down_nodes[0] → down_nodes[1] → target`

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **재당숙** (재종숙) | 再堂叔 | `X==4, Y==3` `target.branch==paternal` `target.gender==male` |
| **재당고모** | 再堂姑母 | `X==4, Y==3` `target.branch==paternal` `target.gender==female` |

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

경로: `나 → 부/모 → 조부/조모 → down_nodes[0] → target`

분기 기준: `down_nodes[0]`의 성별과 계열 (어느 경로로 내려왔는가)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **종형** | 從兄 | `X==2, Y==2` `down_nodes[0].branch==paternal, gender==male` `target.gender==male` `target.birth_year < me.birth_year` |
| **종제** | 從弟 | `X==2, Y==2` `down_nodes[0].branch==paternal, gender==male` `target.gender==male` `target.birth_year > me.birth_year` |
| **종자** | 從姊 | `X==2, Y==2` `down_nodes[0].branch==paternal, gender==male` `target.gender==female` `target.birth_year < me.birth_year` |
| **종매** | 從妹 | `X==2, Y==2` `down_nodes[0].branch==paternal, gender==male` `target.gender==female` `target.birth_year > me.birth_year` |
| **고종형제** | 姑從兄弟 | `X==2, Y==2` `down_nodes[0].branch==paternal, gender==female` *(고모 경유)* `나이/성별에 따라 형제 접미사` |
| **외종형제** | 外從兄弟 | `X==2, Y==2` `down_nodes[0].branch==maternal, gender==male` *(외숙부 경유)* `나이/성별에 따라 형제 접미사` |
| **이종형제** | 姨從兄弟 | `X==2, Y==2` `down_nodes[0].branch==maternal, gender==female` *(이모 경유)* `나이/성별에 따라 형제 접미사` |

> **종형제 통칭 원리**:  
> - 백부/숙부 자녀 → **종형제(從兄弟)**  
> - 고모 자녀 → **고종형제(姑從兄弟)**  
> - 외숙부 자녀 → **외종형제(外從兄弟)** (= 외사촌)  
> - 이모 자녀 → **이종형제(姨從兄弟)**

---

### 3-3. 6촌 (LCA = 증조부, X=3, Y=3)

경로: `나 → 부 → 조부 → 증조부 → down_nodes[0] → down_nodes[1] → target`

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **재종형** | 再從兄 | `X==3, Y==3` `target.gender==male` `target.birth_year < me.birth_year` |
| **재종제** | 再從弟 | `X==3, Y==3` `target.gender==male` `target.birth_year > me.birth_year` |
| **재종자** | 再從姊 | `X==3, Y==3` `target.gender==female` `target.birth_year < me.birth_year` |
| **재종매** | 再從妹 | `X==3, Y==3` `target.gender==female` `target.birth_year > me.birth_year` |

---

### 3-4. 8촌 (LCA = 고조부, X=4, Y=4) — 당내친의 마지노선

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **삼종형** | 三從兄 | `X==4, Y==4` `target.gender==male` `target.birth_year < me.birth_year` |
| **삼종제** | 三從弟 | `X==4, Y==4` `target.gender==male` `target.birth_year > me.birth_year` |
| **삼종자** | 三從姊 | `X==4, Y==4` `target.gender==female` `target.birth_year < me.birth_year` |
| **삼종매** | 三從妹 | `X==4, Y==4` `target.gender==female` `target.birth_year > me.birth_year` |

---

## 4. -1세대 방계 — 자녀 항렬

> **주의**: 하향 탐색에서는 LCA가 나 자신(me) 또는 나의 직계 선조가 된다.  
> LCA == me 이면 `X==0`이고 down_nodes[0]이 형제자매 노드이다.

### 4-1. 3촌 (LCA = me, X=0, Y=2 이나 실질적으로 경로상 형제 → 자녀)

실제 경로 재정의:
```
나 → [형/제/자/매] → target
LCA = 조부(X=2), Y=1 방향이 아닌,
나의 직계 선조 없이 나 → 형제 → 조카 로 내려가는 경우
→ LCA = 부/모 기준 X=1, 형제 경유 Y=2 로 처리
```

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **질** | 姪 | `X==1, Y==2` `down_nodes[0]`이 나의 형제자매 `target.gender==male` |
| **질녀** | 姪女 | `X==1, Y==2` `down_nodes[0]`이 나의 형제자매 `target.gender==female` |

---

### 4-2. 5촌 (LCA = 조부, X=2, Y=3)

경로: `나 → 부 → 조부 → 백부/숙부 → 종형제 → target`

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **당질** (종질) | 堂姪 (從姪) | `X==2, Y==3` `down_nodes[0].branch==paternal` `target.gender==male` |
| **당질녀** (종질녀) | 堂姪女 | `X==2, Y==3` `down_nodes[0].branch==paternal` `target.gender==female` |

---

### 4-3. 7촌 (LCA = 증조부, X=3, Y=4)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **재당질** (재종질) | 再堂姪 | `X==3, Y==4` `target.gender==male` |
| **재당질녀** | 再堂姪女 | `X==3, Y==4` `target.gender==female` |

---

## 5. +2세대 방계 — 조부모 항렬

### 5-1. 4촌 (LCA = 증조부, X=3, Y=1)

경로: `나 → 부 → 조부 → 증조부 → target`

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **종조부** | 從祖父 | `X==3, Y==1` `target.branch==paternal` `target.gender==male` *(손위·손아래 구분 없이 동일)* |
| **대고모** | 大姑母 | `X==3, Y==1` `target.branch==paternal` `target.gender==female` |
| **외종조부** | 外從祖父 | `X==3, Y==1` `target.branch==maternal` `target.gender==male` |
| **외대고모** | 外大姑母 | `X==3, Y==1` `target.branch==maternal` `target.gender==female` |

> **주의**: 종조부는 백종조부/숙종조부로 세분하지 않고 통칭 **종조부** 사용.

---

### 5-2. 6촌 (LCA = 고조부, X=4, Y=2)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **재종조부** | 再從祖父 | `X==4, Y==2` `target.gender==male` |
| **재종조모** | 再從祖母 | `X==4, Y==2` `target.gender==female` |

---

## 6. -2세대 방계 — 손자녀 항렬

### 6-1. 4촌 (LCA = 부, X=1, Y=3)

경로: `나 → 부 → 형/제 → 질 → 종손`

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **종손** | 從孫 | `X==1, Y==3` `down_nodes[0]`이 나의 형제 `target.gender==male` |
| **종손녀** | 從孫女 | `X==1, Y==3` `down_nodes[0]`이 나의 형제 `target.gender==female` |

---

### 6-2. 6촌 (LCA = 조부, X=2, Y=4)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **재종손** | 再從孫 | `X==2, Y==4` `target.gender==male` |
| **재종손녀** | 再從孫女 | `X==2, Y==4` `target.gender==female` |

---

## 7. -3세대 방계 — 증손자녀 항렬

### 7-1. 5촌 (LCA = 부, X=1, Y=4)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **종증손** | 從曾孫 | `X==1, Y==4` `target.gender==male` |
| **종증손녀** | 從曾孫女 | `X==1, Y==4` `target.gender==female` |

---

### 7-2. 7촌 (LCA = 조부, X=2, Y=5)

| 호칭 | 한자 | 탐색 조건 |
|---|---|---|
| **재종증손** | 再從曾孫 | `X==2, Y==5` `target.gender==male` |
| **재종증손녀** | 再從曾孫女 | `X==2, Y==5` `target.gender==female` |

---

## 8. 배우자 호칭 — 후처리 교체 테이블

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
| 외숙부 (外叔父) | **외숙모 (外叔母)** | |
| 이모 (姨母) | **이모부 (姨母夫)** | |
| 당숙 (堂叔) | **당숙모 (堂叔母)** | |
| 당고모 (堂姑母) | **당고모부 (堂姑母夫)** | |
| 내종숙 (內從叔) | **내종숙모 (內從叔母)** | |
| 종조부 (從祖父) | **종조모 (從祖母)** | |
| 대고모 (大姑母) | **대고모부 (大姑母夫)** | |
| 재당숙 (再堂叔) | **재당숙모 (再堂叔母)** | |

> **배우자 호칭 생성 원칙 (나무위키 출처)**:
> - 윗 항렬 **남성** 혈족의 배우자: 접미사 `부(父)` → `모(母)` 로 교체
> - 윗 항렬 **여성** 혈족의 배우자: 호칭 뒤에 `부(夫)` 를 추가

---

## 9. 한자어 구성 원리 요약

| 접두사/접미사 | 의미 | 예시 |
|---|---|---|
| **백(伯)** | 아버지보다 손위 남자 형제 | 백부, 백모 |
| **숙(叔)** | 아버지보다 손아래 남자 형제 / 외가는 무조건 숙 | 숙부, 외숙부 |
| **고(姑)** | 아버지의 여자 형제 (친가) | 고모, 대고모 |
| **이(姨)** | 어머니의 여자 형제 (외가) | 이모, 이종숙 |
| **종(從)** | 1단계 방계 (공통 조상: 조부) | 종형, 종조부, 종질 |
| **재종(再從)** | 2단계 방계 (공통 조상: 증조부) | 재종형, 재종조부 |
| **삼종(三從)** | 3단계 방계 (공통 조상: 고조부) | 삼종형 |
| **당(堂)** | 종(從)의 동의어 (일상 혼용) | 당숙 = 종숙 |
| **외(外)** | 어머니 쪽 계열 | 외조부, 외종형제 |
| **증(曾)** | +3 / -3 세대 | 증조부, 증손 |

---

## 10. 탐색 알고리즘 전체 흐름

```
function get_hoching(me, target):

    # Step 1: 배우자 여부 확인
    blood_target = target
    is_spouse = False
    if target.is_spouse:
        blood_target = target.spouse_of
        is_spouse = True

    # Step 2: DFS로 혈족 경로 탐색 (BLOOD 간선만 사용, 백트래킹)
    path = dfs_backtrack(me, blood_target)
    if path is None:
        return "관계 없음"

    # Step 3: LCA 추출
    lca_idx = index_of_max_generation(path)
    X = lca_idx
    Y = len(path) - 1 - lca_idx
    up_nodes = path[1:lca_idx+1]
    down_nodes = path[lca_idx+1:]

    # Step 4: 혈족 호칭 결정 (경로 패턴 매칭)
    blood_hoching = match_hoching_rule(X, Y, up_nodes, down_nodes, me, blood_target)

    # Step 5: 배우자이면 후처리 교체
    if is_spouse:
        return SPOUSE_TABLE[blood_hoching]
    return blood_hoching
```

---

## 11. 같은 촌수, 다른 호칭 — 핵심 케이스 정리

| 촌수 | 경로 | 호칭 | 차이 원인 |
|:---:|---|---|---|
| 5촌 | 나→부→조부→**증조부→조부형제(남)**→당숙 | **당숙** | down_nodes[0].gender==male |
| 5촌 | 나→부→조부→**증조부→조부자매(여)**→내종숙 | **내종숙** | down_nodes[0].gender==female |
| 5촌 | 나→모→외조부→**외증조부→외조부형제(남)**→외당숙 | **외당숙** | branch==maternal |
| 4촌 | 나→부→조부→**백부(남)**→종형제 | **종형제** | down_nodes[0].gender==male (paternal) |
| 4촌 | 나→부→조부→**고모(여)**→고종형제 | **고종형제** | down_nodes[0].gender==female (paternal) |
| 4촌 | 나→모→외조부→**외숙부(남)**→외종형제 | **외종형제** | down_nodes[0].gender==male (maternal) |
| 4촌 | 나→모→외조모→**이모(여)**→이종형제 | **이종형제** | down_nodes[0].gender==female (maternal) |