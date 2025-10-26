## 文件结构

```

├── Graph.h             \# 图数据结构 (头文件)
├── Graph.c             \# 图数据结构 (实现文件)
├── FibonacciHeap.h     \# 斐波那契堆 (头文件)
├── FibonacciHeap.c     \# 斐波那契堆 (实现文件)
├── main\_fib.c          \# 性能测试主程序 (包含Dijkstra实现)
└── README.md           \# 本说明文件

```

## 编译指南

你需要使用 C 编译器 (如 `gcc`) 来编译所有 `.c` 文件。

* 推荐使用 `-O3` 优化选项以获得准确的性能测试结果。
* 需要链接数学库 `-lm` (因为 `FibonacciHeap.c` 中使用了 `log2` 和 `floor`)。

**编译命令:**

```bash
gcc -o test_fib main_fib.c Graph.c FibonacciHeap.c -std=c11 -O3 -lm
```

## 使用示例

1. **准备数据** (假设你已下载 `USA-road-d.NY.gr`并有 `convert_format.c`)

    ```bash
    gcc convert_format.c -o convert_format
    ./convert_format USA-road-d.NY.gr graph_input.txt
    ```

2.  **编译**

    ```bash
    gcc -o test_fib main_fib.c Graph.c FibonacciHeap.c -std=c11 -O3 -lm
    ```

3.  **运行性能测试** (使用 `graph_input.txt` 文件，测试 1000 次查询)

    ```bash
    ./test_fib graph_input.txt 1000
    ```