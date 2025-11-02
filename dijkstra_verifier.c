#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

// ==================== 统一的宏定义 ====================
#define TRUE 1
#define FALSE 0
const long long INF = LLONG_MAX;

// ==================== 统一的图结构 (来自 Graph.h/c) ====================

/**
 * @brief 邻接表节点 (边)
 */
typedef struct AdjListNode {
    int to;
    int weight;
    struct AdjListNode* next;
} AdjListNode;

/**
 * @brief 图结构 (邻接表)
 */
typedef struct Graph {
    int numVertices;     // 顶点数量 (最大ID)
    AdjListNode** adj;   // 邻接表数组 (1-based: 忽略 adj[0])
} Graph;

/**
 * @brief 创建一个图
 */
Graph* createGraph(int V) {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    if (g == NULL) return NULL;
    g->numVertices = V;
    // 分配 V+1 大小，以便使用 1-based 索引
    g->adj = (AdjListNode**)calloc(V + 1, sizeof(AdjListNode*));
    if (g->adj == NULL) { free(g); return NULL; }
    return g;
}

/**
 * @brief 销毁图
 */
void graphDestroy(Graph* g) {
    if (g == NULL) return;
    for (int i = 0; i <= g->numVertices; ++i) {
        AdjListNode* current = g->adj[i];
        while (current != NULL) {
            AdjListNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(g->adj);
    free(g);
}

/**
 * @brief 添加边
 */
void graphAddEdge(Graph* g, int u, int v, int weight) {
    if (u <= 0 || u > g->numVertices || v <= 0 || v > g->numVertices) return;

    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (newNode == NULL) return;
    newNode->to = v;
    newNode->weight = weight;
    newNode->next = g->adj[u];
    g->adj[u] = newNode;
}

/**
 * @brief 从文件加载图 (兼容 convert_format.c 的输出)
 */
Graph* loadGraphFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) { perror("错误: 无法打开图文件"); return NULL; }

    int id1, id2, distance;
    int max_id = 0;
    long long edge_count = 0;
    
    // --- 第一遍: 找到最大顶点ID ---
    while (fscanf(file, "%d %d %d", &id1, &id2, &distance) == 3) {
        if (id1 > max_id) max_id = id1;
        if (id2 > max_id) max_id = id2;
    }

    if (max_id == 0) { fprintf(stderr, "错误: 未找到有效边数据。\n"); fclose(file); return NULL; }

    Graph* g = createGraph(max_id);
    if (g == NULL) { fclose(file); return NULL; }

    // --- 第二遍: 添加边 ---
    rewind(file);
    while (fscanf(file, "%d %d %d", &id1, &id2, &distance) == 3) {
        graphAddEdge(g, id1, id2, distance);
        edge_count++;
    }

    fclose(file);
    printf("图加载成功。顶点数(最大ID): %d, 总边数: %lld\n", g->numVertices, edge_count);
    return g;
}


// ==================== 二叉堆实现 (来自 main.c, 适应 1-based ID) ====================

typedef struct {
    int node;
    long long distance; // 距离统一使用 long long
} BinHeapNode;

typedef struct {
    BinHeapNode* heap;
    int* pos;           // 节点在堆中的位置
    int size;
    int capacity;
} BinaryHeap;

BinaryHeap* create_binary_heap(int capacity) {
    // 容量需要为 V+1 以处理 1-based ID
    BinaryHeap* heap = (BinaryHeap*)malloc(sizeof(BinaryHeap));
    heap->heap = (BinHeapNode*)malloc((capacity + 1) * sizeof(BinHeapNode));
    heap->pos = (int*)malloc((capacity + 1) * sizeof(int));
    heap->size = 0;
    heap->capacity = capacity;
    
    for (int i = 0; i <= capacity; i++) {
        heap->pos[i] = -1;
    }
    return heap;
}

void free_binary_heap(BinaryHeap* heap) {
    if (heap) { free(heap->heap); free(heap->pos); free(heap); }
}

void swap_bin_nodes(BinaryHeap* heap, int i, int j) {
    BinHeapNode temp = heap->heap[i];
    heap->heap[i] = heap->heap[j];
    heap->heap[j] = temp;
    
    heap->pos[heap->heap[i].node] = i;
    heap->pos[heap->heap[j].node] = j;
}

void bin_heapify_up(BinaryHeap* heap, int idx) {
    while (idx > 1) { // 堆数组从 1 开始
        int parent = idx / 2;
        if (heap->heap[parent].distance <= heap->heap[idx].distance) break;
        swap_bin_nodes(heap, parent, idx);
        idx = parent;
    }
}

void bin_heapify_down(BinaryHeap* heap, int idx) {
    while (2 * idx <= heap->size) {
        int left = 2 * idx;
        int right = 2 * idx + 1;
        int smallest = idx;
        
        if (left <= heap->size && heap->heap[left].distance < heap->heap[smallest].distance) {
            smallest = left;
        }
        if (right <= heap->size && heap->heap[right].distance < heap->heap[smallest].distance) {
            smallest = right;
        }
        
        if (smallest == idx) break;
        
        swap_bin_nodes(heap, idx, smallest);
        idx = smallest;
    }
}

void bin_heap_insert(BinaryHeap* heap, int node, long long distance) {
    if (heap->size >= heap->capacity) { return; }
    
    int idx = ++heap->size;
    heap->heap[idx].node = node;
    heap->heap[idx].distance = distance;
    heap->pos[node] = idx;
    
    bin_heapify_up(heap, idx);
}

BinHeapNode bin_heap_extract_min(BinaryHeap* heap) {
    if (heap->size == 0) {
        BinHeapNode empty = {-1, INF};
        return empty;
    }
    
    BinHeapNode min_node = heap->heap[1]; // 堆数组从 1 开始
    heap->pos[min_node.node] = -1;
    
    heap->size--;
    if (heap->size > 0) {
        heap->heap[1] = heap->heap[heap->size + 1];
        heap->pos[heap->heap[1].node] = 1;
        bin_heapify_down(heap, 1);
    }
    
    return min_node;
}

void bin_heap_decrease_key(BinaryHeap* heap, int node, long long new_distance) {
    int idx = heap->pos[node];
    if (idx == -1 || heap->heap[idx].distance <= new_distance) return;
    
    heap->heap[idx].distance = new_distance;
    bin_heapify_up(heap, idx);
}

int is_bin_heap_empty(BinaryHeap* heap) {
    return heap->size == 0;
}

// ==================== 二叉堆 Dijkstra 实现 (逻辑来自 main.c) ====================

long long* dijkstra_binary_heap_verifier(Graph* graph, int source) {
    int V = graph->numVertices;
    long long* dist = (long long*)malloc((V + 1) * sizeof(long long));
    if (dist == NULL) return NULL;

    for (int i = 1; i <= V; i++) {
        dist[i] = INF;
    }
    dist[source] = 0;
    
    BinaryHeap* heap = create_binary_heap(V);
    
    // 初始插入所有节点
    for (int i = 1; i <= V; i++) {
        // 由于二叉堆的 insert/decreaseKey 逻辑是耦合的，我们初始时插入 0/INF
        // 并依靠 decreaseKey 来调整位置，这与 main.c 的逻辑一致。
        bin_heap_insert(heap, i, dist[i]); 
    }
    
    while (!is_bin_heap_empty(heap)) {
        BinHeapNode min_node = bin_heap_extract_min(heap);
        int u = min_node.node;
        
        if (min_node.distance == INF) break;
        
        AdjListNode* edge = graph->adj[u];
        while (edge) {
            int v = edge->to;
            // 检查溢出
            if (dist[u] != INF) {
                long long new_dist = dist[u] + edge->weight;
                
                if (new_dist < dist[v]) {
                    dist[v] = new_dist;
                    // 在 main.c 的逻辑中，所有节点初始都在堆中，所以这里总是调用 decreaseKey
                    bin_heap_decrease_key(heap, v, new_dist);
                }
            }
            
            edge = edge->next;
        }
    }
    
    free_binary_heap(heap);
    return dist;
}


// ==================== 斐波那契堆实现 (来自 FibonacciHeap.h/c) ====================

typedef struct FibHeapNode {
    long long key;
    int value;
    struct FibHeapNode *parent, *child, *left, *right;
    int degree;
    int mark;
} FibHeapNode;

typedef struct FibHeap {
    FibHeapNode* minNode;
    int numNodes;
} FibHeap;

static FibHeapNode* _create_fib_node(long long key, int value) {
    FibHeapNode* node = (FibHeapNode*)malloc(sizeof(FibHeapNode));
    if (node == NULL) return NULL;
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

static void _fib_add_node_to_root_list(FibHeap* H, FibHeapNode* x) {
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
    x->parent = NULL;
}

FibHeap* createFibHeap() {
    FibHeap* H = (FibHeap*)malloc(sizeof(FibHeap));
    if (H == NULL) return NULL;
    H->minNode = NULL;
    H->numNodes = 0;
    return H;
}

FibHeapNode* fibHeapInsert(FibHeap* H, long long key, int value) {
    FibHeapNode* node = _create_fib_node(key, value);
    if (node == NULL) return NULL;

    _fib_add_node_to_root_list(H, node);

    if (H->minNode == NULL || node->key < H->minNode->key) {
        H->minNode = node;
    }
    H->numNodes++;
    return node;
}

int fibHeapIsEmpty(const FibHeap* H) { return H->minNode == NULL; }

// --- FibonacciHeapConsolidate & Link 仅做声明，这里仅实现核心 Extract/DecreaseKey 所需的逻辑 ---
static void _fibHeapLink(FibHeap* H, FibHeapNode* y, FibHeapNode* x);
static void _fibHeapCut(FibHeap* H, FibHeapNode* x, FibHeapNode* y);
static void _fibHeapCascadingCut(FibHeap* H, FibHeapNode* y);
static void _fibHeapRecursiveDestroy(FibHeapNode* node) {
    if (node == NULL) return;
    FibHeapNode* current = node;
    FibHeapNode* start = node;
    FibHeapNode* next;
    do {
        next = current->right;
        _fibHeapRecursiveDestroy(current->child);
        free(current);
        current = next;
    } while (current != start);
}
void fibHeapDestroy(FibHeap* H) {
    if (H == NULL) return;
    _fibHeapRecursiveDestroy(H->minNode);
    free(H);
}

static void _fibHeapConsolidate(FibHeap* H) {
    if (H->minNode == NULL) return;

    int maxDegree = (int)floor(log2(H->numNodes) / log2(1.618)) + 2; 
    FibHeapNode** A = (FibHeapNode**)calloc(maxDegree, sizeof(FibHeapNode*));
    if (A == NULL) return;
    
    int rootCount = 0;
    FibHeapNode* current = H->minNode;
    do { rootCount++; current = current->right; } while (current != H->minNode);

    FibHeapNode** rootList = (FibHeapNode**)malloc(rootCount * sizeof(FibHeapNode*));
    if (rootList == NULL) { free(A); return; }
    
    current = H->minNode;
    for(int i = 0; i < rootCount; i++) {
        rootList[i] = current;
        current = current->right;
    }
    
    for (int i = 0; i < rootCount; i++) {
        FibHeapNode* x = rootList[i];
        int d = x->degree;
        
        while (A[d] != NULL) {
            FibHeapNode* y = A[d];

            if (x->key > y->key) {
                FibHeapNode* temp = x; x = y; y = temp;
            }

            _fibHeapLink(H, y, x);
            A[d] = NULL;
            d++;
        }
        A[d] = x;
    }
    
    free(rootList);

    H->minNode = NULL;
    for (int i = 0; i < maxDegree; ++i) {
        if (A[i] != NULL) {
            _fib_add_node_to_root_list(H, A[i]);
            if (H->minNode == NULL || A[i]->key < H->minNode->key) {
                H->minNode = A[i];
            }
        }
    }
    
    free(A);
}

static void _fibHeapLink(FibHeap* H, FibHeapNode* y, FibHeapNode* x) {
    y->left->right = y->right;
    y->right->left = y->left;
    y->right = y->left = y;

    y->parent = x;
    if (x->child == NULL) {
        x->child = y;
    } else {
        x->child->left->right = y;
        y->left = x->child->left;
        x->child->left = y;
        y->right = x->child;
    }

    x->degree++;
    y->mark = FALSE;
}

int fibHeapExtractMin(FibHeap* H) {
    FibHeapNode* z = H->minNode;
    if (z == NULL) return -1;

    int minValue = z->value;

    if (z->child != NULL) {
        FibHeapNode* currentChild = z->child;
        do {
            FibHeapNode* nextChild = currentChild->right;
            _fib_add_node_to_root_list(H, currentChild);
            currentChild = nextChild;
        } while (currentChild != z->child);
    }

    z->left->right = z->right;
    z->right->left = z->left;

    if (z == z->right) {
        H->minNode = NULL;
    } else {
        H->minNode = z->right;
        _fibHeapConsolidate(H);
    }

    H->numNodes--;
    free(z);
    return minValue;
}

static void _fibHeapCut(FibHeap* H, FibHeapNode* x, FibHeapNode* y) {
    if (x->right == x) {
        y->child = NULL;
    } else {
        x->left->right = x->right;
        x->right->left = x->left;
        if (y->child == x) y->child = x->right;
    }
    y->degree--;

    _fib_add_node_to_root_list(H, x);

    x->parent = NULL;
    x->mark = FALSE;
}

static void _fibHeapCascadingCut(FibHeap* H, FibHeapNode* y) {
    FibHeapNode* z = y->parent;
    if (z != NULL) {
        if (y->mark == FALSE) {
            y->mark = TRUE;
        } else {
            _fibHeapCut(H, y, z);
            _fibHeapCascadingCut(H, z);
        }
    }
}

void fibHeapDecreaseKey(FibHeap* H, FibHeapNode* x, long long newKey) {
    if (newKey > x->key) return;

    x->key = newKey;
    FibHeapNode* y = x->parent;

    if (y != NULL && x->key < y->key) {
        _fibHeapCut(H, x, y);
        _fibHeapCascadingCut(H, y);
    }

    if (x->key < H->minNode->key) {
        H->minNode = x;
    }
}

// ==================== 斐波那契堆 Dijkstra 实现 (逻辑来自 main_fib.c) ====================

long long* dijkstra_fib_heap_verifier(Graph* g, int startNode) {
    int V = g->numVertices;
    long long* dist = (long long*)malloc((V + 1) * sizeof(long long));
    if (dist == NULL) return NULL;

    FibHeapNode** nodePtrs = (FibHeapNode**)calloc(V + 1, sizeof(FibHeapNode*));
    if (nodePtrs == NULL) { free(dist); return NULL; }

    for (int i = 1; i <= V; ++i) {
        dist[i] = INF;
    }

    FibHeap* pq = createFibHeap();
    if (pq == NULL) { free(dist); free(nodePtrs); return NULL; }

    dist[startNode] = 0;
    nodePtrs[startNode] = fibHeapInsert(pq, 0, startNode);

    while (!fibHeapIsEmpty(pq)) {
        int u = fibHeapExtractMin(pq);
        nodePtrs[u] = NULL; 

        if (dist[u] == INF) break; 

        AdjListNode* current = g->adj[u];
        while (current != NULL) {
            int v = current->to;
            int weight = current->weight;
            
            if (dist[u] != INF) {
                long long newDist = dist[u] + weight;

                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    
                    if (nodePtrs[v] != NULL) {
                        fibHeapDecreaseKey(pq, nodePtrs[v], newDist);
                    } else {
                        nodePtrs[v] = fibHeapInsert(pq, newDist, v);
                    }
                }
            }
            current = current->next;
        }
    }

    fibHeapDestroy(pq);
    free(nodePtrs);

    return dist;
}


// ==================== 主程序：正确性验证 ====================

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "用法: %s <graph_file.txt> <start_id> <target_id>\n", argv[0]);
        fprintf(stderr, "  <graph_file.txt>: 格式为 'id1 id2 distance' 的图文件\n");
        fprintf(stderr, "  <start_id>: 起始城市ID\n");
        fprintf(stderr, "  <target_id>: 目标城市ID\n");
        return 1;
    }

    const char* filename = argv[1];
    int start_node = atoi(argv[2]);
    int target_node = atoi(argv[3]);

    printf("--- 开始正确性验证 ---\n");
    printf("图文件: %s\n", filename);
    printf("起始节点: %d, 目标节点: %d\n\n", start_node, target_node);
    
    // 1. 加载图
    Graph* g = loadGraphFromFile(filename);
    if (!g) return 1;

    if (start_node <= 0 || start_node > g->numVertices || target_node <= 0 || target_node > g->numVertices) {
        fprintf(stderr, "错误: 起始或目标节点ID超出图范围 [1, %d]\n", g->numVertices);
        graphDestroy(g);
        return 1;
    }

    long long* dist_fib = NULL;
    long long* dist_bin = NULL;

    // 2. 斐波那契堆 Dijkstra
    printf("--- 1. 运行 Fibonacci Heap Dijkstra ---\n");
    dist_fib = dijkstra_fib_heap_verifier(g, start_node);

    // 3. 二叉堆 Dijkstra
    printf("--- 2. 运行 Binary Heap Dijkstra ---\n");
    dist_bin = dijkstra_binary_heap_verifier(g, start_node);

    // 4. 结果输出与对比
    if (dist_fib == NULL || dist_bin == NULL) {
        fprintf(stderr, "致命错误: 内存分配失败或Dijkstra运行失败。\n");
    } else {
        long long result_fib = dist_fib[target_node];
        long long result_bin = dist_bin[target_node];
        
        printf("\n=== 结果对比 ===\n");
        printf("Fibonacci Heap 结果: ");
        if (result_fib == INF) {
            printf("目标节点 (%d) 不可达\n", target_node);
        } else {
            printf("最短路径长度: %lld\n", result_fib);
        }

        printf("Binary Heap 结果: ");
        if (result_bin == INF) {
            printf("目标节点 (%d) 不可达\n", target_node);
        } else {
            printf("最短路径长度: %lld\n", result_bin);
        }

        printf("\n=== 正确性验证 ===\n");
        if (result_fib == result_bin) {
            printf("两种算法的结果一致!\n");
        } else {
            printf("警告: 两种算法的结果不一致 (差异: %lld)\n", result_fib - result_bin);
        }
        
        free(dist_fib);
        free(dist_bin);
    }

    graphDestroy(g);
    printf("\n--- 验证结束 ---\n");
    return 0;
}