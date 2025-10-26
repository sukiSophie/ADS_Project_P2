#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   // 用于计时 (clock) 和随机数 (srand)
#include <limits.h> // 用于 LLONG_MAX

#include "Graph.h"
#include "FibonacciHeap.h"

// 定义无穷大
const long long INF = LLONG_MAX;

/**
 * @brief 使用斐波那契堆实现Dijkstra算法 (C语言版)
 *
 * @param g 图对象
 * @param startNode 起始顶点ID
 * @return long long* 数组, 包含从startNode到所有其他节点的最短距离
 * 调用者必须手动 free() 此数组
 */
long long* dijkstra_fib_heap(Graph* g, int startNode) {
    if (startNode <= 0 || startNode > g->numVertices) {
        fprintf(stderr, "错误: 起始节点 %d 无效。\n", startNode);
        return NULL;
    }

    // 1. 初始化
    long long* dist = (long long*)malloc((g->numVertices + 1) * sizeof(long long));
    if (dist == NULL) {
        perror("错误: 无法为距离数组分配内存");
        return NULL;
    }

    // 'nodePtrs' 用于存储从顶点ID到堆中节点的映射, 以便执行 decreaseKey
    // 使用 calloc 自动初始化为 NULL
    FibHeapNode** nodePtrs = (FibHeapNode**)calloc(g->numVertices + 1, sizeof(FibHeapNode*));
    if (nodePtrs == NULL) {
        perror("错误: 无法为节点指针数组分配内存");
        free(dist);
        return NULL;
    }

    for (int i = 0; i <= g->numVertices; ++i) {
        dist[i] = INF;
    }

    // 2. 创建优先队列
    FibHeap* pq = createFibHeap();
    if (pq == NULL) {
        free(dist);
        free(nodePtrs);
        return NULL;
    }

    // 3. 设置起始节点
    dist[startNode] = 0;
    nodePtrs[startNode] = fibHeapInsert(pq, 0, startNode);

    // 4. Dijkstra主循环
    while (!fibHeapIsEmpty(pq)) {
        // 4.1 提取最小距离的顶点 u
        int u = fibHeapExtractMin(pq);
        nodePtrs[u] = NULL; // (关键!) 标记为已提取, 防止在 decreaseKey 中误用

        if (dist[u] == INF) {
            // 优化: 剩余节点不可达
            break; 
        }

        // 4.2 遍历 u 的所有邻居 v
        AdjListNode* current = g->adj[u];
        while (current != NULL) {
            int v = current->to;
            int weight = current->weight;
            
            // 确保不会溢出
            if (dist[u] == INF) {
                current = current->next;
                continue;
            }

            long long newDist = dist[u] + weight;

            // 4.3 松弛操作
            if (newDist < dist[v]) {
                dist[v] = newDist;
                
                if (nodePtrs[v] != NULL) {
                    // 节点 v 已在堆中, 执行 decreaseKey
                    fibHeapDecreaseKey(pq, nodePtrs[v], newDist);
                } else {
                    // 节点 v 首次被发现, 插入堆中
                    nodePtrs[v] = fibHeapInsert(pq, newDist, v);
                }
            }
            current = current->next;
        }
    }

    // 5. 清理
    // 注意: nodePtrs 中剩余的非NULL指针指向的节点
    // 会在 fibHeapDestroy 中被统一释放
    fibHeapDestroy(pq);
    free(nodePtrs);

    return dist; // 返回距离数组
}

/**
 * @brief 主程序: 性能测试
 * * 编译: gcc -o test_fib main_fib.c Graph.c FibonacciHeap.c -std=c11 -O3 -lm
 * * 运行: ./test_fib <graph_file.txt> <n>
 *
 * <graph_file.txt> 是 convert_format.c 的输出文件 ("id1 id2 距离")
 * <n> 是要测试的随机源节点数量
 */
int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc != 3) {
        fprintf(stderr, "错误: 参数数量不正确。\n");
        fprintf(stderr, "用法: %s <graph_file.txt> <n>\n", argv[0]);
        fprintf(stderr, "  <n>: 要测试的随机查询次数 (例如: 1000)\n");
        return 1;
    }

    const char* graph_filename = argv[1];
    int n = atoi(argv[2]); // 将字符串参数转为整数

    if (n <= 0) {
        fprintf(stderr, "错误: 查询次数 'n' 必须是正整数。\n");
        return 1;
    }

    // --- 1. 加载图 ---
    Graph* g = loadGraphFromFile(graph_filename);
    if (g == NULL) {
        fprintf(stderr, "错误: 图加载失败。\n");
        return 1;
    }

    // --- 2. 准备查询 ---
    printf("\n正在生成 %d 个随机源节点用于测试...\n", n);
    
    int* source_nodes = (int*)malloc(n * sizeof(int));
    if (source_nodes == NULL) {
        perror("错误: 无法为源节点数组分配内存");
        graphDestroy(g);
        return 1;
    }

    // 初始化随机数生成器
    srand(time(NULL)); 
    for (int i = 0; i < n; ++i) {
        // 生成 [1, g->numVertices] 范围内的随机ID
        source_nodes[i] = (rand() % g->numVertices) + 1;
    }

    // --- 3. 执行性能测试 ---
    printf("开始性能测试 (Dijkstra + Fibonacci Heap)...\n");
    
    // 使用 <time.h> 中的 clock() 计时
    clock_t start_time = clock();

    for (int i = 0; i < n; ++i) {
        int startNode = source_nodes[i];
        
        // 调用Dijkstra算法
        long long* distances = dijkstra_fib_heap(g, startNode);

        if (distances != NULL) {
            // 必须释放dijkstra返回的距离数组
            free(distances);
        } else {
             fprintf(stderr, "警告: 第 %d 次查询 (源: %d) 失败。\n", i+1, startNode);
        }
        
        // (可选) 打印进度
        // if ((i + 1) % 100 == 0) {
        //     printf("已完成 %d / %d 次查询...\n", i + 1, n);
        // }
    }

    clock_t end_time = clock();
    
    // --- 4. 输出结果 ---
    double time_spent_seconds = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("\n--- 性能测试结果 (Fibonacci Heap) ---\n");
    printf("总共执行查询: %d\n", n);
    printf("总耗时: %.4f 秒\n", time_spent_seconds);
    printf("平均每次查询耗时: %.4f 毫秒\n", (time_spent_seconds / n) * 1000.0);

    // --- 5. 最终清理 ---
    free(source_nodes);
    graphDestroy(g);

    return 0; // 成功退出
}