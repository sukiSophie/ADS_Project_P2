#ifndef FIBONACCI_HEAP_H
#define FIBONACCI_HEAP_H

#include <stdlib.h>
#include <stdio.h>
#include <limits.h> // for LLONG_MAX
#include <math.h>   // for log2, floor
#include <string.h> // for NULL

// C语言中布尔值的简易定义
#define TRUE 1
#define FALSE 0

/**
 * @brief 斐波那契堆节点
 * Key (键): long long (Dijkstra中的距离)
 * Value (值): int (Dijkstra中的顶点ID)
 */
typedef struct FibHeapNode {
    long long key;
    int value;
    struct FibHeapNode *parent, *child, *left, *right;
    int degree;
    int mark; // 0 (FALSE) or 1 (TRUE)
} FibHeapNode;

/**
 * @brief 斐波那契堆结构体
 */
typedef struct FibHeap {
    FibHeapNode* minNode;
    int numNodes;
} FibHeap;

/**
 * @brief 创建一个新的空斐波那契堆
 * @return 指向新堆的指针
 */
FibHeap* createFibHeap();

/**
 * @brief 销毁堆 (释放所有节点内存)
 * @param H 指向堆的指针
 */
void fibHeapDestroy(FibHeap* H);

/**
 * @brief 插入一个新节点
 * @param H 堆
 * @param key 键 (距离)
 * @param value 值 (顶点ID)
 * @return 指向新创建节点的指针 (Dijkstra需要用它来DecreaseKey)
 */
FibHeapNode* fibHeapInsert(FibHeap* H, long long key, int value);

/**
 * @brief 提取最小键的节点
 * @param H 堆
 * @return 最小节点的值 (顶点ID)
 */
int fibHeapExtractMin(FibHeap* H);

/**
 * @brief 减小一个节点的键值
 * @param H 堆
 * @param x 要减小键值的节点
 * @param newKey 新的键值 (必须小于旧键值)
 */
void fibHeapDecreaseKey(FibHeap* H, FibHeapNode* x, long long newKey);

/**
 * @brief 检查堆是否为空
 * @param H 堆
 * @return 1 (TRUE) 如果为空, 0 (FALSE) 否则
 */
int fibHeapIsEmpty(const FibHeap* H);

#endif // FIBONACCI_HEAP_H