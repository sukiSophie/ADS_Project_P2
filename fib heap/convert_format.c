#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256 // 定义每行最大长度

/**
 * @brief 主程序：将 DIMACS .gr 格式文件转换为 "id1 id2 距离" 格式
 * * 用法: ./convert_format <input_file.gr> <output_file.txt>
 */
int main(int argc, char* argv[]) {
    
    // --- 1. 参数检查 ---
    // 检查命令行参数数量是否正确 (程序名 + 输入文件 + 输出文件)
    if (argc != 3) {
        fprintf(stderr, "错误: 参数数量不正确。\n");
        fprintf(stderr, "用法: %s <input_file.gr> <output_file.txt>\n", argv[0]);
        return 1; // 返回错误码
    }

    const char* input_filename = argv[1];
    const char* output_filename = argv[2];

    FILE* input_file = NULL;
    FILE* output_file = NULL;

    // --- 2. 打开文件 ---
    // 打开原始数据文件 (输入)
    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        perror("错误: 无法打开输入文件");
        return 1;
    }

    // 创建或覆盖约定格式的文件 (输出)
    output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        perror("错误: 无法创建输出文件");
        fclose(input_file); // 关闭已打开的输入文件
        return 1;
    }

    // --- 3. 逐行处理 ---
    char line[MAX_LINE_LENGTH];
    int id1, id2, distance;
    long long line_count = 0;
    long long arc_count = 0;

    printf("开始处理文件: %s\n", input_filename);

    // 逐行读取输入文件
    while (fgets(line, sizeof(line), input_file) != NULL) {
        line_count++;

        // 检查行是否以 'a ' (arc definition) 开头
        if (line[0] == 'a' && line[1] == ' ') {
            
            // 解析 "a %d %d %d" 格式
            // 使用 sscanf 从 'a' 字符之后开始解析
            if (sscanf(line + 2, "%d %d %d", &id1, &id2, &distance) == 3) {
                // 成功解析，按约定格式写入输出文件
                fprintf(output_file, "%d %d %d\n", id1, id2, distance);
                arc_count++;
            } else {
                // 解析失败
                fprintf(stderr, "警告: 格式错误的 'a' 行在第 %lld 行: %s", line_count, line);
            }
        }
        // 忽略所有其他行 (如 'c ' 和 'p ')
    }

    // --- 4. 清理和收尾 ---
    fclose(input_file);
    fclose(output_file);

    printf("处理完成。\n");
    printf("总共读取 %lld 行。\n", line_count);
    printf("成功转换 %lld 条边数据到 %s\n", arc_count, output_filename);

    return 0; // 成功退出
}