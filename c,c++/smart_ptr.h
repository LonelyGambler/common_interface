#ifndef CUSTOM_SMART_PTR_H_
#define CUSTOM_SMART_PTR_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef __GNUC__
    #error This can only be used on a compiler supporting GNU extensions
#endif

#define ALWAYS_INLINE    __attribute__ ((always_inline)) inline

typedef void* RESOURCE_PTR;  // 指向拥有资源的对象的指针

typedef void (*FREE_CALLBACK) (RESOURCE_PTR);

typedef struct _smart_free_struct
{
    RESOURCE_PTR pres;                   //保存持有资源的指针的指针
    FREE_CALLBACK free_callback;   //释放资源的回调函数
} smart_free_struct;

ALWAYS_INLINE static void smart_free_memory(RESOURCE_PTR res_ptr)
{
    if (res_ptr == NULL)
    {
        return;
    }
    void **tmp = (void **)res_ptr;
    if (*tmp != NULL)
    {
        free(*tmp);
        *tmp = NULL;
    }
}

ALWAYS_INLINE static void smart_free_fd(RESOURCE_PTR res_ptr)
{
    if (res_ptr == NULL)
    {
        return;
    }
    int *pfd = (int *)res_ptr;
    if (pfd != NULL && *pfd >= 0)
    {
        close(*pfd);
        *pfd = -1;
    }
}

ALWAYS_INLINE static void smart_free_guard(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }
    smart_free_struct *tmp = (smart_free_struct *)ptr;
    if (tmp->free_callback != NULL)
    {
        (tmp->free_callback)(tmp->pres);
    }
}

ALWAYS_INLINE static void smart_free_curl(RESOURCE_PTR res_ptr)
{
    if (res_ptr == NULL)
    {
        return;
    }
    CURL **tmp = (CURL **)res_ptr;
    if (*tmp != NULL)
    {
        curl_easy_cleanup(*tmp);
        *tmp = NULL;
    }
}

ALWAYS_INLINE static void smart_free_headers(RESOURCE_PTR res_ptr)
{
    if (res_ptr == NULL)
    {
        return;
    }
    struct curl_slist **tmp = (struct curl_slist **)res_ptr;
    if (*tmp != NULL)
    {
        curl_slist_free_all(*tmp);
        *tmp = NULL;
    }
}

ALWAYS_INLINE static void smart_free_json(RESOURCE_PTR res_ptr)
{
    if (res_ptr == NULL)
    {
        return;
    }
    json_t **tmp = (json_t **)res_ptr;
    if (*tmp != NULL)
    {
        json_decref(*tmp);
        *tmp = NULL;
    }
}

ALWAYS_INLINE static void smart_free_fopen(RESOURCE_PTR res_ptr)
{
    if (res_ptr == NULL)
    {
        return;
    }
    FILE **pfp = (FILE **)res_ptr;
    if (*pfp != NULL)
    {
        fclose(*pfp);
        *pfp = NULL;
    }
}

#define SMART_COMMON(callback) __attribute__ ((cleanup(callback)))
#define SMART_GUARD SMART_COMMON(smart_free_guard)

#define SCOP_GUARD_LINENAME_CAT(line) smart_guard ## line

#define SCOP_GUARD_LINENAME(resource, callback, line)                          \
    SMART_GUARD smart_free_struct SCOP_GUARD_LINENAME_CAT(line)  = {           \
        .pres = (RESOURCE_PTR)&resource,                                       \
        .free_callback = callback                                              \
    };                                           
#define ON_SCOPE_EXIT(resource, callback) SCOP_GUARD_LINENAME(resource, callback, __LINE__)
#endif