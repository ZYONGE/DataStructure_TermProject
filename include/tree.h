// 이진 탐색 트리(BST)의 연산 함수들을 구현하는 헤더파일 (부모 포인터 포함 이중 연결)
// 본 코드에서 typedef int, float... 등의 자료형을 Element로 정의하여 사용하였음
// 사용 방법: typedef int Element; 선언 후 이 파일을 include

#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>

// 기본 Element 타입: int
// 다른 타입 사용 시 include 전에 ELEMENT_DEFINED 매크로와 함께 typedef 선언
// 예: typedef float Element; #define ELEMENT_DEFINED
#ifndef ELEMENT_DEFINED
#define ELEMENT_DEFINED
typedef int Element;
#endif

#ifndef ELEMENT_FORMAT
#define ELEMENT_FORMAT "%d"
#endif

typedef struct TreeNode {
    Element data;
    struct TreeNode *left;
    struct TreeNode *right;
    struct TreeNode *parent;  // 이중 연결 구조
} TreeNode;

typedef struct {
    TreeNode *root;
    int size;
} BST;

// ─── 초기화 ───────────────────────────────────────────────────────────────────

static void initBST(BST *tree) {
    tree->root = NULL;
    tree->size = 0;
}

// ─── 내부 헬퍼 ───────────────────────────────────────────────────────────────

static TreeNode *_createTreeNode(Element data) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (!node) { fprintf(stderr, "[Tree] 메모리 할당 실패\n"); exit(1); }
    node->data   = data;
    node->left   = NULL;
    node->right  = NULL;
    node->parent = NULL;
    return node;
}

// 서브트리에서 최솟값 노드 반환
static TreeNode *_treeMin(TreeNode *node) {
    while (node && node->left) node = node->left;
    return node;
}

// 서브트리에서 최댓값 노드 반환
static TreeNode *_treeMax(TreeNode *node) {
    while (node && node->right) node = node->right;
    return node;
}

// ─── 상태 확인 ───────────────────────────────────────────────────────────────

static int bstIsEmpty(BST *tree) {
    return tree->root == NULL;
}

static int bstSize(BST *tree) {
    return tree->size;
}

// ─── 삽입 ────────────────────────────────────────────────────────────────────

static void bstInsert(BST *tree, Element data) {
    TreeNode *node = _createTreeNode(data);
    if (bstIsEmpty(tree)) {
        tree->root = node;
        tree->size++;
        return;
    }
    TreeNode *cur = tree->root;
    TreeNode *parent = NULL;
    while (cur) {
        parent = cur;
        if (data < cur->data)       cur = cur->left;
        else if (data > cur->data)  cur = cur->right;
        else { free(node); return; }  // 중복값 무시
    }
    node->parent = parent;
    if (data < parent->data) parent->left  = node;
    else                     parent->right = node;
    tree->size++;
}

// ─── 탐색 ────────────────────────────────────────────────────────────────────

static TreeNode *bstSearch(BST *tree, Element data) {
    TreeNode *cur = tree->root;
    while (cur) {
        if (data == cur->data)       return cur;
        else if (data < cur->data)   cur = cur->left;
        else                         cur = cur->right;
    }
    return NULL;
}

static Element bstMin(BST *tree) {
    if (bstIsEmpty(tree)) { fprintf(stderr, "[Tree] 트리가 비어있습니다\n"); exit(1); }
    return _treeMin(tree->root)->data;
}

static Element bstMax(BST *tree) {
    if (bstIsEmpty(tree)) { fprintf(stderr, "[Tree] 트리가 비어있습니다\n"); exit(1); }
    return _treeMax(tree->root)->data;
}

// ─── 삭제 ────────────────────────────────────────────────────────────────────

static void _transplant(BST *tree, TreeNode *u, TreeNode *v) {
    if (!u->parent)              tree->root = v;
    else if (u == u->parent->left) u->parent->left  = v;
    else                           u->parent->right = v;
    if (v) v->parent = u->parent;
}

static int bstDelete(BST *tree, Element data) {
    TreeNode *node = bstSearch(tree, data);
    if (!node) return 0;

    if (!node->left) {
        _transplant(tree, node, node->right);
    } else if (!node->right) {
        _transplant(tree, node, node->left);
    } else {
        // 오른쪽 서브트리의 최솟값(중위 후계자)으로 대체
        TreeNode *successor = _treeMin(node->right);
        if (successor->parent != node) {
            _transplant(tree, successor, successor->right);
            successor->right = node->right;
            successor->right->parent = successor;
        }
        _transplant(tree, node, successor);
        successor->left = node->left;
        successor->left->parent = successor;
    }
    free(node);
    tree->size--;
    return 1;
}

// ─── 순회 (재귀) ─────────────────────────────────────────────────────────────

static void _inorder(TreeNode *node) {
    if (!node) return;
    _inorder(node->left);
    printf(ELEMENT_FORMAT, node->data);
    printf(" ");
    _inorder(node->right);
}

static void _preorder(TreeNode *node) {
    if (!node) return;
    printf(ELEMENT_FORMAT, node->data);
    printf(" ");
    _preorder(node->left);
    _preorder(node->right);
}

static void _postorder(TreeNode *node) {
    if (!node) return;
    _postorder(node->left);
    _postorder(node->right);
    printf(ELEMENT_FORMAT, node->data);
    printf(" ");
}

// ─── 출력 ────────────────────────────────────────────────────────────────────

static void bstDisplayInorder(BST *tree) {
    printf("[Tree] 중위순회 (오름차순): ");
    _inorder(tree->root);
    printf(" (크기: %d)\n", tree->size);
}

static void bstDisplayPreorder(BST *tree) {
    printf("[Tree] 전위순회: ");
    _preorder(tree->root);
    printf(" (크기: %d)\n", tree->size);
}

static void bstDisplayPostorder(BST *tree) {
    printf("[Tree] 후위순회: ");
    _postorder(tree->root);
    printf(" (크기: %d)\n", tree->size);
}

// 트리 구조를 시각적으로 출력 (오른쪽 → 루트 → 왼쪽, 들여쓰기로 깊이 표현)
static void _printTree(TreeNode *node, int depth) {
    if (!node) return;
    _printTree(node->right, depth + 1);
    for (int i = 0; i < depth; i++) printf("    ");
    printf(ELEMENT_FORMAT, node->data);
    printf("\n");
    _printTree(node->left, depth + 1);
}

static void bstDisplay(BST *tree) {
    printf("[Tree] 구조 (크기: %d)\n", tree->size);
    _printTree(tree->root, 0);
}

// ─── 정렬 ────────────────────────────────────────────────────────────────────
// BST의 중위순회 결과를 배열에 저장하여 정렬된 배열 반환 (호출자가 free 필요)

static void _inorderToArray(TreeNode *node, Element *arr, int *idx) {
    if (!node) return;
    _inorderToArray(node->left, arr, idx);
    arr[(*idx)++] = node->data;
    _inorderToArray(node->right, arr, idx);
}

static Element *bstSort(BST *tree) {
    if (bstIsEmpty(tree)) return NULL;
    Element *arr = (Element *)malloc(tree->size * sizeof(Element));
    if (!arr) { fprintf(stderr, "[Tree] 메모리 할당 실패\n"); exit(1); }
    int idx = 0;
    _inorderToArray(tree->root, arr, &idx);
    return arr;
}

// ─── 파일 가져오기 ────────────────────────────────────────────────────────────

static void bstImport(BST *tree, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { fprintf(stderr, "[Tree] 파일 열기 실패: %s\n", filename); return; }
    Element data;
    while (fscanf(fp, ELEMENT_FORMAT, &data) == 1) {
        bstInsert(tree, data);
    }
    fclose(fp);
    printf("[Tree] '%s' 에서 데이터 가져오기 완료  (크기: %d)\n", filename, tree->size);
}

// ─── 메모리 해제 ─────────────────────────────────────────────────────────────

static void _freeBSTNodes(TreeNode *node) {
    if (!node) return;
    _freeBSTNodes(node->left);
    _freeBSTNodes(node->right);
    free(node);
}

static void bstFree(BST *tree) {
    _freeBSTNodes(tree->root);
    tree->root = NULL;
    tree->size = 0;
}

#endif // TREE_H
