#ifndef KEYWORDS_H
#define KEYWORDS_H

/* ── 특수 ───────────────────────────────────────────────────── */
#define KW_SELF           "본인"
#define KW_UNKNOWN        "관계 없음"
#define KW_SPOUSE         "배우자"

/* ── 직계 1촌 ────────────────────────────────────────────────── */
#define KW_FATHER         "부(父)"
#define KW_MOTHER         "모(母)"

/* ── 직계 2촌 상향 ───────────────────────────────────────────── */
#define KW_GRANDFATHER    "조부(祖父)"
#define KW_GRANDMOTHER    "조모(祖母)"

/* ── 2촌 동항렬 (형제자매) ───────────────────────────────────── */
#define KW_HYUNG          "형(兄)"    /* M→M 연상 / F→M 연상(오빠) */
#define KW_JE             "제(弟)"    /* →M 연하 */
#define KW_JA             "자(姊)"    /* M→F 연상(누나) / F→F 연상(언니) */
#define KW_MAE            "매(妹)"    /* →F 연하 */

/* ── 3촌 +1세대 (친가) ───────────────────────────────────────── */
#define KW_BAEKBU         "백부(伯父)"   /* 부보다 연상 남자 형제 */
#define KW_SUKBU          "숙부(叔父)"   /* 부보다 연하 남자 형제 */
#define KW_GOMO           "고모(姑母)"   /* 부의 여자 형제 */

/* ── 3촌 -1세대 (역방향 전용) ───────────────────────────────── */
#define KW_JILNAM         "질(姪)"       /* 남조카 */
#define KW_JILNYEO        "질녀(姪女)"   /* 여조카 */

/* ── 배우자 호칭 (후처리) ────────────────────────────────────── */
#define KW_BAENGMO        "백모(伯母)"
#define KW_SUNGMO         "숙모(叔母)"
#define KW_GOMOBU         "고모부(姑母夫)"

/* ── 역방향 배우자 호칭 ──────────────────────────────────────── */
#define KW_SAENGJIL       "생질(甥姪)"   /* 백모/숙모/고모부 → 나 */

/* ── 4촌 동항렬: 종형제 (백부/숙부 경유) ────────────────────── */
#define KW_JONGHYUNG      "종형(從兄)"
#define KW_JONGJE         "종제(從弟)"
#define KW_JONGJA         "종자(從姊)"
#define KW_JONGMAE        "종매(從妹)"

/* ── 4촌 동항렬: 고종형제 (고모 경유) ───────────────────────── */
#define KW_GOJONGHYUNG    "고종형(姑從兄)"
#define KW_GOJONGJE       "고종제(姑從弟)"
#define KW_GOJONGJA       "고종자(姑從姊)"
#define KW_GOJONGMAE      "고종매(姑從妹)"

#endif /* KEYWORDS_H */
