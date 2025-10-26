#include "Graph.h"
#include <string.h>

// 内部函数：查找最大值
int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * @brief 创建图
 */
Graph* createGraph(int V) {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    if (g == NULL) {
        perror("错误: 无法为图分配内存");
        return NULL;
    }
    g->numVertices = V;

    // +1 是因为顶点ID从1到V
    // 使用 calloc 自动将所有指针初始化为 NULL
    g->adj = (AdjListNode**)calloc(V + 1, sizeof(AdjListNode*));
    if (g->adj == NULL) {
        perror("错误: 无法为邻接表分配内存");
        free(g);
        return NULL;
    }
    
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
            free(temp); // 释放链表中的每个节点
        }
    }
    free(g->adj); // 释放邻接表数组
    free(g);      // 释放图结构体
}

/**
 * @brief 添加边 (在邻接表头部插入)
 */
void graphAddEdge(Graph* g, int u, int v, int weight) {
    if (u < 0 || u > g->numVertices || v < 0 || v > g->numVertices) {
        fprintf(stderr, "警告: 顶点ID %d 或 %d 超出范围 (最大: %d)\n", u, v, g->numVertices);
        return;
    }

    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (newNode == NULL) {
        perror("错误: 无法为新边分配内存");
        return;
    }
    newNode->to = v;
    newNode->weight = weight;
    newNode->next = g->adj[u]; // 插入到链表头部
    g->adj[u] = newNode;
}

/**
 * @brief 从文件加载图
 *
 * 采用两遍扫描法 (Two-Pass):
 * 1. 第一遍: 找到最大的顶点ID, 以确定图的大小
 * 2. 第二遍: 实际读取并添加边
 */
Graph* loadGraphFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("错误: 无法打开图文件");
        return NULL;
    }

    int id1, id2, distance;
    int max_id = 0;
    long long line_count = 0;
    long long edge_count = 0;

    // --- 第一遍: 找到最大顶点ID ---
    printf("加载图: 第一次扫描 (寻找最大顶点ID)...\n");
    // (注意: fscanf 比 sscanf + fgets 效率稍低, 但在C中更简洁)
    while (fscanf(file, "%d %d %d", &id1, &id2, &distance) == 3) {
        max_id = max(max_id, max(id1, id2));
        line_count++;
    }

    if (max_id == 0) {
        fprintf(stderr, "错误: 未在文件中找到任何有效的边数据。\n");
        fclose(file);
        return NULL;
    }

    printf("图重置大小, 最大顶点ID: %d\n", max_id);
    Graph* g = createGraph(max_id);
    if (g == NULL) {
        fclose(file);
        return NULL;
    }

    // --- 第二遍: 添加边 ---
    rewind(file); // 文件指针回到开头
    printf("加载图: 第二次扫描 (添加边)...\n");
    
    while (fscanf(file, "%d %d %d", &id1, &id2, &distance) == 3) {
        graphAddEdge(g, id1, id2, distance);
        edge_count++;
    }

    fclose(file);
    
    if (edge_count != line_count) {
         fprintf(stderr, "警告: 边计数不匹配 (第一遍: %lld, 第二遍: %lld)\n", line_count, edge_count);
    }
    
    printf("图加载成功。\n");
    printf("总顶点数 (最大ID): %d\n", g->numVertices);
    printf("总边数: %lld\n", edge_count);

    return g;
}