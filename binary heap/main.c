#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>

// 图结构定义
typedef struct Edge {
    int dest;           // 目标节点
    int weight;         // 边权重
    struct Edge* next;  // 下一条边
} Edge;

typedef struct {
    int num_nodes;      // 节点数量
    int num_edges;      // 边数量
    Edge** adj_list;    // 邻接表
} Graph;

// 二叉堆节点
typedef struct {
    int node;           // 节点编号
    int distance;       // 到源点的距离
} HeapNode;

// 二叉堆结构
typedef struct {
    HeapNode* heap;     // 堆数组
    int* pos;           // 节点在堆中的位置
    int size;           // 当前堆大小
    int capacity;       // 堆容量
} BinaryHeap;

// ==================== 二叉堆操作 ====================

/**
 * 创建二叉堆
 * @param capacity 堆容量
 * @return 堆指针
 */
BinaryHeap* create_heap(int capacity) {
    BinaryHeap* heap = (BinaryHeap*)malloc(sizeof(BinaryHeap));
    heap->heap = (HeapNode*)malloc(capacity * sizeof(HeapNode));
    heap->pos = (int*)malloc(capacity * sizeof(int));
    heap->size = 0;
    heap->capacity = capacity;
    
    // 初始化位置数组为-1（表示不在堆中）
    for (int i = 0; i < capacity; i++) {
        heap->pos[i] = -1;
    }
    
    return heap;
}

/**
 * 释放堆内存
 * @param heap 堆指针
 */
void free_heap(BinaryHeap* heap) {
    if (heap) {
        free(heap->heap);
        free(heap->pos);
        free(heap);
    }
}

/**
 * 交换堆中两个元素
 * @param heap 堆指针
 * @param i 第一个位置
 * @param j 第二个位置
 */
void swap_nodes(BinaryHeap* heap, int i, int j) {
    // 交换堆元素
    HeapNode temp = heap->heap[i];
    heap->heap[i] = heap->heap[j];
    heap->heap[j] = temp;
    
    // 更新位置信息
    heap->pos[heap->heap[i].node] = i;
    heap->pos[heap->heap[j].node] = j;
}

/**
 * 上浮操作（维护堆性质）
 * @param heap 堆指针
 * @param idx 要上浮的元素位置
 */
void heapify_up(BinaryHeap* heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap->heap[parent].distance <= heap->heap[idx].distance) {
            break;
        }
        swap_nodes(heap, parent, idx);
        idx = parent;
    }
}

/**
 * 下沉操作（维护堆性质）
 * @param heap 堆指针
 * @param idx 要下沉的元素位置
 */
void heapify_down(BinaryHeap* heap, int idx) {
    while (2 * idx + 1 < heap->size) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;
        
        if (left < heap->size && heap->heap[left].distance < heap->heap[smallest].distance) {
            smallest = left;
        }
        if (right < heap->size && heap->heap[right].distance < heap->heap[smallest].distance) {
            smallest = right;
        }
        
        if (smallest == idx) {
            break;
        }
        
        swap_nodes(heap, idx, smallest);
        idx = smallest;
    }
}

/**
 * 插入节点到堆中
 * @param heap 堆指针
 * @param node 节点编号
 * @param distance 距离
 */
void heap_insert(BinaryHeap* heap, int node, int distance) {
    if (heap->size >= heap->capacity) {
        printf("Heap overflow!\n");
        return;
    }
    
    // 插入新节点到堆末尾
    int idx = heap->size;
    heap->heap[idx].node = node;
    heap->heap[idx].distance = distance;
    heap->pos[node] = idx;
    heap->size++;
    
    // 上浮维护堆性质
    heapify_up(heap, idx);
}

/**
 * 提取最小元素
 * @param heap 堆指针
 * @return 最小堆节点
 */
HeapNode heap_extract_min(BinaryHeap* heap) {
    if (heap->size == 0) {
        HeapNode empty = {-1, INT_MAX};
        return empty;
    }
    
    HeapNode min_node = heap->heap[0];
    heap->pos[min_node.node] = -1;  // 标记为不在堆中
    
    // 将最后一个元素移到根位置
    heap->size--;
    if (heap->size > 0) {
        heap->heap[0] = heap->heap[heap->size];
        heap->pos[heap->heap[0].node] = 0;
        heapify_down(heap, 0);
    }
    
    return min_node;
}

/**
 * 减少键值操作
 * @param heap 堆指针
 * @param node 节点编号
 * @param new_distance 新距离
 */
void heap_decrease_key(BinaryHeap* heap, int node, int new_distance) {
    int idx = heap->pos[node];
    if (idx == -1 || heap->heap[idx].distance <= new_distance) {
        return;
    }
    
    heap->heap[idx].distance = new_distance;
    heapify_up(heap, idx);
}

/**
 * 检查堆是否为空
 * @param heap 堆指针
 * @return 1为空，0为非空
 */
int is_heap_empty(BinaryHeap* heap) {
    return heap->size == 0;
}

// ==================== 图操作 ====================

/**
 * 创建图
 * @param num_nodes 节点数量
 * @return 图指针
 */
Graph* create_graph(int num_nodes) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->num_nodes = num_nodes;
    graph->num_edges = 0;
    graph->adj_list = (Edge**)calloc(num_nodes, sizeof(Edge*));
    return graph;
}

/**
 * 释放图内存
 * @param graph 图指针
 */
void free_graph(Graph* graph) {
    if (graph) {
        for (int i = 0; i < graph->num_nodes; i++) {
            Edge* edge = graph->adj_list[i];
            while (edge) {
                Edge* temp = edge;
                edge = edge->next;
                free(temp);
            }
        }
        free(graph->adj_list);
        free(graph);
    }
}

/**
 * 添加边到图中
 * @param graph 图指针
 * @param src 源节点
 * @param dest 目标节点
 * @param weight 边权重
 */
void add_edge(Graph* graph, int src, int dest, int weight) {
    // 添加到源节点的邻接表
    Edge* new_edge = (Edge*)malloc(sizeof(Edge));
    new_edge->dest = dest;
    new_edge->weight = weight;
    new_edge->next = graph->adj_list[src];
    graph->adj_list[src] = new_edge;
    graph->num_edges++;
}

// ==================== Dijkstra算法 ====================

/**
 * 使用二叉堆的Dijkstra算法
 * @param graph 图指针
 * @param source 源节点
 * @param dist 距离数组（输出）
 */
void dijkstra_binary_heap(Graph* graph, int source, int* dist) {
    // 初始化距离数组
    for (int i = 0; i < graph->num_nodes; i++) {
        dist[i] = INT_MAX;
    }
    dist[source] = 0;
    
    // 创建二叉堆
    BinaryHeap* heap = create_heap(graph->num_nodes);
    
    // 将所有节点插入堆中
    for (int i = 0; i < graph->num_nodes; i++) {
        heap_insert(heap, i, dist[i]);
    }
    
    // Dijkstra主循环
    while (!is_heap_empty(heap)) {
        // 提取距离最小的节点
        HeapNode min_node = heap_extract_min(heap);
        int u = min_node.node;
        
        // 如果最小距离是无穷大，说明剩余节点不可达
        if (min_node.distance == INT_MAX) {
            break;
        }
        
        // 遍历所有邻接边
        Edge* edge = graph->adj_list[u];
        while (edge) {
            int v = edge->dest;
            int new_dist = dist[u] + edge->weight;
            
            // 如果找到更短路径
            if (new_dist < dist[v]) {
                dist[v] = new_dist;
                heap_decrease_key(heap, v, new_dist);
            }
            
            edge = edge->next;
        }
    }
    
    free_heap(heap);
}

// ==================== 文件读取和测试 ====================

/**
 * 读取DIMACS格式的图文件
 * @param filename 文件名
 * @return 图指针
 */
Graph* read_dimacs_graph(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Cannot open file: %s\n", filename);
        return NULL;
    }
    
    char line[256];
    int num_nodes = 0, num_edges = 0;
    
    // 第一遍读取：获取节点和边数量
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'p') {
            sscanf(line, "p sp %d %d", &num_nodes, &num_edges);
            break;
        }
    }
    
    if (num_nodes == 0) {
        printf("Invalid file format\n");
        fclose(file);
        return NULL;
    }
    
    printf("Reading graph: %d nodes, %d edges\n", num_nodes, num_edges);
    
    Graph* graph = create_graph(num_nodes);
    
    // 第二遍读取：添加边
    rewind(file);
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'a') {
            int src, dest, weight;
            sscanf(line, "a %d %d %d", &src, &dest, &weight);
            // 注意：文件中的节点编号从1开始，我们内部从0开始
            add_edge(graph, src - 1, dest - 1, weight);
        }
    }
    
    fclose(file);
    return graph;
}

/**
 * 获取当前时间（微秒）
 * @return 当前时间戳
 */
long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

/**
 * 性能测试函数
 * @param graph 图指针
 * @param num_queries 查询数量
 */
void performance_test(Graph* graph, int num_queries) {
    if (!graph) return;
    
    printf("Starting performance test (%d queries)...\n", num_queries);
    
    int* dist = (int*)malloc(graph->num_nodes * sizeof(int));
    long long total_time = 0;
    int valid_queries = 0;
    
    srand(time(NULL));  // 初始化随机种子
    
    for (int i = 0; i < num_queries; i++) {
        // 随机选择源节点
        int source = rand() % graph->num_nodes;
        
        long long start_time = get_time_us();
        dijkstra_binary_heap(graph, source, dist);
        long long end_time = get_time_us();
        
        long long elapsed = end_time - start_time;
        total_time += elapsed;
        valid_queries++;
        
        if ((i + 1) % 100 == 0) {
            printf("Completed %d/%d queries, average time: %.2f microseconds\n", 
                   i + 1, num_queries, (double)total_time / valid_queries);
        }
    }
    
printf("\n=== Performance Test Results ===\n");
    printf("Total queries: %d\n", valid_queries);
    printf("Total time: %.2f seconds\n", total_time / 1000000.0);
    printf("Average time per query: %.2f microseconds\n", (double)total_time / valid_queries);
    printf("Average time per query: %.2f milliseconds\n", (double)total_time / valid_queries / 1000.0);   
    free(dist);
}

// ==================== 主程序 ====================

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <graph_file> <query_count>\n", argv[0]);
        printf("Example: %s USA-road-d.NY.gr 1000\n", argv[0]);
        return 1;
    }
    
    const char* filename = argv[1];
    int num_queries = atoi(argv[2]);
    
    if (num_queries < 1) {
        printf("Query count must be greater than 0\n");
        return 1;
    }
    
    printf("Reading graph file: %s\n", filename);
    Graph* graph = read_dimacs_graph(filename);
    
    if (!graph) {
        printf("Failed to read graph\n");
        return 1;
    }
    
    printf("Graph read successfully!\n");
    printf("Number of nodes: %d\n", graph->num_nodes);
    printf("Number of edges: %d\n", graph->num_edges);
    
    // 运行性能测试
    performance_test(graph, num_queries);
    
    // 清理内存
    free_graph(graph);
    
    return 0;
}