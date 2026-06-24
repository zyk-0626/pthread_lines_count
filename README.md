# pthread_lines_count
多线程统计Linux中.h、.c文件的行数

## 具体流程
1. 收集.h和.c的所有文件
2. 创建线程数组
3. 编写线程执行的任务（一个线程统计一个文件的函数）
4. 编写统计单个文件行数的函数

## 每个部分的算法

### 收集.h和.c文件
```c
char **files = NULL;
    int file_count = 0;
    collect_files(&files, argv[1], &file_count);
    printf("共找到了 %d 个文件\n", file_count);
```
```c
// 递归遍历目录收集文件
void collect_files(const char *dirpath, char ***files, int *count) {
    DIR *dir = opendir(dirpath);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);
        
        struct stat st;
        stat(fullpath, &st);
        
        if (S_ISDIR(st.st_mode)) {
            collect_files(fullpath, files, count);  // 递归进入子目录
        } else {
            // 检查是否为 .c 或 .h 文件
            char *ext = strrchr(entry->d_name, '.');
            if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0)) {
                *files = realloc(*files, (*count + 1) * sizeof(char *));
                (*files)[*count] = strdup(fullpath);
                (*count)++;
            }
        }
    }
    closedir(dir);
}
```

### 创建线程数组
- 创建线程数组，元素数量是文件的数量
- 为每个线程的参数进行动态内存
- 复制文件路径到参数值这种
- 将每个线程开始进行任务
```c
pthread_t *threads = malloc(file_count * sizeof(pthread_t));
    for (int i = 0; i < file_count; i++)
    {
        ThreadArg *arg = malloc(sizeof(ThreadArg));
        arg->filepath = strdup(files[i]);
        pthread_create(&threads[i], NULL, process_file, arg);
    }
```

### 编写线程执行的任务
- 把县城参数转化为定义的结构体
- 统计这个线程所对应管理的整个文件的行数
- 运用到互斥锁，避免多个线程同时占用到全局变量，导致计数错误
- 输出此线程所对应的文件行数
- 释放动态的内存
```c
void *process_file(void *arg)
{
    ThreadArg *args = (ThreadArg *)arg;
    int lines = count_lines(args->filepath);
    pthread_mutex_lock(&mutex);
    total_lines += lines;
    pthread_mutex_unlock(&mutex);
    printf("文件：%s 的行数是 %d\n", args->filepath, lines);
    free(args->filepath);
    free(args);
    return NULL;
}
```

### 统计单个文件的行数
- 打开某个文件
- 每一行一行来读，读一行，行数加加
```c
int count_lines(const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp)
    {
        return 0;
    }
    int lines = 0;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), fp))
    {
        lines++;
    }
    fclose(fp);
    return lines;
}
```
