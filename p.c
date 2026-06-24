#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// 全局变量
long long total_lines = 0;
pthread_mutex_t mutex;

// 线程参数：文件路径
typedef struct
{
    char *filepath
} ThreadArg;

int count_lines(char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp)
        perror("无法打开文件\n");
    int lines = 0;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), fp))
    {
        lines++;
    }
    fclose(fp);
    return lines;
}

void *process_file(void *arg)
{
    ThreadArg *args = (ThreadArg *)arg;

    int lines = count_lines(args->filepath);
    pthread_mutex_lock(&mutex);
    total_lines += lines;
    pthread_mutex_unlock(&mutex);

    printf("文件：%s 的行数是 %d\n", args->filepath, lines);

    free(args->filepath);
    free(arg);

    return NULL;
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("正确输入：本文件夹和目标文件\n");
        return 1;
    }
    // 定义需要用道德变量
    char **files;
    int file_count = 0;
    collect_files(&files, argv[1], file_count);
    printf("共找到了 %d 个文件\n", file_count);

    pthread_mutex_init(&mutex, NULL);

    // 创建线程数组
    pthread_t *threads = malloc(file_count * sizeof(pthread_t));
    for (int i = 0; i < file_count; i++)
    {
        // 为每个线程分配内存空间
        ThreadArg *arg = malloc(sizeof(ThreadArg));
        // 复制文件路径到参数中
        arg->filepath = strdup(files[i]);

        pthread_create(&threads[i], NULL, process_file, arg);
    }
    // 等待所有线程执行完毕
    for (int i = 0; i < file_count; i++)
    {
        pthread_join(threads[i], NULL);
    }
    // 输出结果
    printf("\n所有文件的总行数是：%lld\n", total_lines);
    // 清理资源
    for (int i = 0; i < file_count; i++)
    {
        free(files[i]);
    }
    free(files);
    free(threads);

    return 0;
}
