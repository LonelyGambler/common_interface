#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
根据文件路径获取文件内容,切记用完之后需要释放内存
[in] fname:文件名
[return] 返回文件内容 
*/
char* get_file_cont(char *fname)
{
    if (fname == NULL)
    {
        printf("file name is NULL.\n");
        return NULL;
    }
    FILE *fp;
    int filesize;
    if ((fp = fopen(fname,"r")) == NULL)
    {
        printf("open file %s error.\n",fname);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    char *file_cont = (char *)malloc(filesize+1);
    if (file_cont == NULL)
    {
        fclose(fp);
        printf("failed to apply memory resource,file name(%s)",fname);
        return NULL;
    }
    rewind(fp);   //指向文件开头
    size_t num = 1;
    size_t len = fread(file_cont, filesize, num, fp);
    if (len != num)
    {
        fclose(fp);
        free(file_cont);
        file_cont = NULL;
        printf("failed to read file(%s)", fname);
        return NULL;
    } 
    fclose(fp); 
    file_cont[filesize] = 0;
    return file_cont;
}

int main(int argc, const char *argv[])
{
    char *ch = NULL;
    ch = get_file_cont(argv[1]);
    printf("%s\n", ch);
    if (ch)
        free(ch);
    return 0;
}
