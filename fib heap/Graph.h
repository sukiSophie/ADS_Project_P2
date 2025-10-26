#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief 邻接表节点 (边)
 */
typedef struct AdjListNode {
    int to;                 // 目标顶点ID
    int weight;             // 边权重 (距离)
    struct AdjListNode* next; // 指向下一个邻居
} AdjListNode;

/**
 * @brief 图结构 (邻接表)
 */
typedef struct Graph {
    int numVertices;     // 顶点数量 (基于最大ID)
    AdjListNode** adj;   // 邻接表数组 (数组的每个元素是一个 AdjListNode 链表的头指针)
} Graph;

/**
 * @brief 创建一个图
 * @param V 顶点数 (最大ID)
 * @return 指向新图的指针
 */
Graph* createGraph(int V);

/**
 * @brief 销毁图, 释放所有内存
 * @param g 指向图的指针
 */
void graphDestroy(Graph* g);

/**
 * @brief 添加一条有向边
 * @param g 图
 * @param u 起始顶点
 * @param v 目标顶点
 * @param weight 权重
 */
void graphAddEdge(Graph* g, int u, int v, int weight);

/**
 * @brief 从文件加载图
 *
 * 预期文件格式: "id1 id2 距离"
 * (这是 convert_format.c 的输出格式)
 *
 * @param filename 输入文件名
 * @return 指向加载好的图的指针, 失败则返回 NULL
 */
Graph* loadGraphFromFile(const char* filename);

#endif // GRAPH_H