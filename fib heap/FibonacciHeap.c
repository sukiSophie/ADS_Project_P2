#include "FibonacciHeap.h"

// --- 内部辅助函数 (前向声明) ---
static void _fibHeapConsolidate(FibHeap* H);
static void _fibHeapLink(FibHeap* H, FibHeapNode* y, FibHeapNode* x);
static void _fibHeapCut(FibHeap* H, FibHeapNode* x, FibHeapNode* y);
static void _fibHeapCascadingCut(FibHeap* H, FibHeapNode* y);
static void _fibHeapRecursiveDestroy(FibHeapNode* node);

/**
 * @brief 创建堆
 */
FibHeap* createFibHeap() {
    FibHeap* H = (FibHeap*)malloc(sizeof(FibHeap));
    if (H == NULL) {
        perror("错误: 无法为堆分配内存");
        return NULL;
    }
    H->minNode = NULL;
    H->numNodes = 0;
    return H;
}

/**
 * @brief 检查堆是否为空
 */
int fibHeapIsEmpty(const FibHeap* H) {
    return H->minNode == NULL;
}

/**
 * @brief 创建一个新节点
 */
static FibHeapNode* _createNode(long long key, int value) {
    FibHeapNode* node = (FibHeapNode*)malloc(sizeof(FibHeapNode));
    if (node == NULL) {
        perror("错误: 无法为堆节点分配内存");
        return NULL;
    }
    node->key = key;
    node->value = value;
    node->parent = NULL;
    node->child = NULL;
    node->left = node;
    node->right = node;
    node->degree = 0;
    node->mark = FALSE;
    return node;
}

/**
 * @brief 将节点x插入到根链表
 * (插入到 H->minNode 的左侧)
 */
static void _fibHeapAddNodeToRootList(FibHeap* H, FibHeapNode* x) {
    if (H->minNode == NULL) {
        H->minNode = x;
        x->left = x;
        x->right = x;
    } else {
        H->minNode->left->right = x;
        x->left = H->minNode->left;
        H->minNode->left = x;
        x->right = H->minNode;
    }
    x->parent = NULL; // 根链表节点的父指针为NULL
}

/**
 * @brief 插入新节点
 */
FibHeapNode* fibHeapInsert(FibHeap* H, long long key, int value) {
    FibHeapNode* node = _createNode(key, value);
    if (node == NULL) return NULL; // 内存分配失败

    _fibHeapAddNodeToRootList(H, node);

    // 更新最小节点指针
    if (node->key < H->minNode->key) {
        H->minNode = node;
    }
    H->numNodes++;
    return node;
}

/**
 * @brief 提取最小节点
 */
int fibHeapExtractMin(FibHeap* H) {
    FibHeapNode* z = H->minNode;
    if (z == NULL) {
        // 理论上Dijkstra不应该对空堆调用此函数, 但作为健壮性检查
        fprintf(stderr, "错误: 试图从空堆中提取。\n");
        return -1; // 返回错误值
    }

    int minValue = z->value;

    // 1. 将z的所有子节点移动到根链表
    if (z->child != NULL) {
        FibHeapNode* currentChild = z->child;
        do {
            FibHeapNode* nextChild = currentChild->right;
            _fibHeapAddNodeToRootList(H, currentChild);
            currentChild = nextChild;
        } while (currentChild != z->child); // 遍历子节点循环链表
    }

    // 2. 从根链表中移除z
    z->left->right = z->right;
    z->right->left = z->left;

    // 3. 更新minNode指针
    if (z == z->right) {
        // z是唯一的根节点
        H->minNode = NULL;
    } else {
        H->minNode = z->right; // 临时指向
        _fibHeapConsolidate(H); // 3. 合并根链表
    }

    H->numNodes--;
    free(z); // 释放内存
    return minValue;
}

/**
 * @brief 合并根链表
 */
static void _fibHeapConsolidate(FibHeap* H) {
    if (H->minNode == NULL) return;

    // D(n) = O(log n)
    // +2 是为了安全裕度
    int maxDegree = (int)floor(log2(H->numNodes) / log2(1.618)) + 2; 
    
    FibHeapNode** A = (FibHeapNode**)calloc(maxDegree, sizeof(FibHeapNode*));
    if (A == NULL) {
        perror("错误: 无法为Consolidate数组分配内存");
        return;
    }
    
    // 遍历根链表
    FibHeapNode* w = H->minNode;
    FibHeapNode* stop = w->left;
    int processed = 0;
    
    // 我们需要一个方式来安全遍历, 因为节点会被移除
    // C++的vector.push_back很方便, C需要手动构建
    int rootCount = 0;
    FibHeapNode* current = H->minNode;
    do {
        rootCount++;
        current = current->right;
    } while (current != H->minNode);

    FibHeapNode** rootList = (FibHeapNode**)malloc(rootCount * sizeof(FibHeapNode*));
    if (rootList == NULL) {
        perror("错误: 无法为根列表数组分配内存");
        free(A);
        return;
    }
    
    current = H->minNode;
    for(int i = 0; i < rootCount; i++) {
        rootList[i] = current;
        current = current->right;
    }
    
    // 现在遍历这个静态数组
    for (int i = 0; i < rootCount; i++) {
        FibHeapNode* x = rootList[i];
        int d = x->degree;
        
        while (A[d] != NULL) {
            FibHeapNode* y = A[d]; // 另一个度数为d的树

            if (x->key > y->key) {
                // 交换x和y
                FibHeapNode* temp = x;
                x = y;
                y = temp;
            }

            _fibHeapLink(H, y, x); // 将y链接到x
            A[d] = NULL;
            d++;
        }
        A[d] = x;
    }
    
    free(rootList); // 释放临时数组

    // 重建根链表
    H->minNode = NULL;
    for (int i = 0; i < maxDegree; ++i) {
        if (A[i] != NULL) {
            _fibHeapAddNodeToRootList(H, A[i]);
            if (H->minNode == NULL || A[i]->key < H->minNode->key) {
                H->minNode = A[i];
            }
        }
    }
    
    free(A); // 释放辅助数组
}

/**
 * @brief 将节点y链接为节点x的子节点 (y成为x的子节点)
 */
static void _fibHeapLink(FibHeap* H, FibHeapNode* y, FibHeapNode* x) {
    // 1. 从根链表中移除y
    y->left->right = y->right;
    y->right->left = y->left;
    y->right = y->left = y; // y 独立

    // 2. 将y设为x的子节点
    y->parent = x;
    if (x->child == NULL) {
        x->child = y;
    } else {
        // 插入到x的子节点链表 (y 插入到 x->child 的左侧)
        x->child->left->right = y;
        y->left = x->child->left;
        x->child->left = y;
        y->right = x->child;
    }

    // 3. 更新x的度数
    x->degree++;

    // 4. 重置y的标记
    y->mark = FALSE;
}

/**
 * @brief 减小键值
 */
void fibHeapDecreaseKey(FibHeap* H, FibHeapNode* x, long long newKey) {
    if (newKey > x->key) {
        fprintf(stderr, "错误: 新键值大于旧键值。\n");
        return;
    }

    x->key = newKey;
    FibHeapNode* y = x->parent;

    if (y != NULL && x->key < y->key) {
        // 堆属性被破坏
        _fibHeapCut(H, x, y);
        _fibHeapCascadingCut(H, y);
    }

    // 更新最小节点指针
    if (x->key < H->minNode->key) {
        H->minNode = x;
    }
}

/**
 * @brief 切割操作 (将x从其父节点y中切除)
 */
static void _fibHeapCut(FibHeap* H, FibHeapNode* x, FibHeapNode* y) {
    // 1. 从y的子节点链表中移除x
    if (x->right == x) {
        // x是唯一的子节点
        y->child = NULL;
    } else {
        x->left->right = x->right;
        x->right->left = x->left;
        if (y->child == x) {
            y->child = x->right; // 更新y的child指针
        }
    }
    y->degree--;

    // 2. 将x添加到根链表
    _fibHeapAddNodeToRootList(H, x);

    // 3. 重置x的父指针和标记
    x->parent = NULL;
    x->mark = FALSE;
}

/**
 * @brief 级联切割
 */
static void _fibHeapCascadingCut(FibHeap* H, FibHeapNode* y) {
    FibHeapNode* z = y->parent;
    if (z != NULL) {
        if (y->mark == FALSE) {
            // 如果y未被标记, 标记它
            y->mark = TRUE;
        } else {
            // y已被标记, 说明它之前已失去过一个子节点
            // 现在又失去了一个, 所以切除y
            _fibHeapCut(H, y, z);
            _fibHeapCascadingCut(H, z); // 递归检查父节点
        }
    }
}

/**
 * @brief 递归销毁节点 (用于 fibHeapDestroy)
 */
static void _fibHeapRecursiveDestroy(FibHeapNode* node) {
    if (node == NULL) return;
    
    // 安全地遍历循环链表
    FibHeapNode* current = node;
    FibHeapNode* start = node;
    FibHeapNode* next;
    
    do {
        next = current->right;
        
        // 递归销毁子树
        _fibHeapRecursiveDestroy(current->child);
        
        // 释放当前节点
        free(current);
        
        current = next;
    } while (current != start);
}

/**
 * @brief 销毁堆
 */
void fibHeapDestroy(FibHeap* H) {
    if (H == NULL) return;
    _fibHeapRecursiveDestroy(H->minNode);
    free(H);
}