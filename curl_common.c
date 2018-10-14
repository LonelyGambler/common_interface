#include <stdio.h>
#include <curl/curl.h>

#define MAX_BODY_SIZE (2048)    //接收http响应的最大缓冲区
#define MAX_URL_SIZE (1024)     //url地址的最大长度

typedef enum HTTP_REQ_TYPE
{
    GET = 0,
    POST
} HTTP_REQ_TYPE;

typedef struct recv_buf_t 
{
    char buf[MAX_BODY_SIZE];
    int size;
} recv_buf;

/*
 * @brief curl的回调函数
 * @param[in] buffer 响应数据
 * @param[in] size * nmemb 响应数据块的大小
 * @param[out] ptr 返回数据
 */
size_t write_data(void *buffer, size_t size, size_t nmemb, void *ptr) 
{
    unsigned int sizes = size * nmemb;
    if (sizes <= 0)
    {
        return 0;
    }
    recv_buf *rbuf = (recv_buf*)ptr;
    if (sizes >= sizeof(rbuf->buf) - 1 - rbuf->size)
    {
        sizes = sizeof(rbuf->buf) - 1 - rbuf->size;
    }
    strncpy(rbuf->buf + rbuf->size, buffer, sizes);
    rbuf->size += sizes;
    return sizes;
}

/*
 * @brief 发送http请求，获取http响应
 * @param[in] url url地址
 * @param[in] req_type 请求类型
 * @param[in] send_data body数据
 * @param[out] recv_data http响应数据
 * @return 0成功，-1失败
 */
static int curl_data(const char *url, const int req_type, const char *send_data,
                        recv_buf *recv_data, struct curl_slist *headers)
{
    assert(url && recv_data);
    int ret = -1;
    CURL *curl = NULL;
    CURLcode res;
    curl = curl_easy_init();
    if (!curl)
    {
        printf("curl_easy_init fail.");
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    if (headers != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    if (req_type == POST)
    {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);   //设置此次为POST请求
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, send_data);
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  //不要验证证书
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);   //禁止连接复用
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);       //timeout不要产生信号 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, recv_data);
    res = curl_easy_perform(curl);
    if (res == CURLE_OK)
    {
        printf("curl success,get data(%s)", recv_data->buf);
        ret = 0;
    }
    else
    {
        ret = -1;
        printf("curl %s failed,error %s", url, curl_easy_strerror(res));
    }

    if(headers)
    {
        curl_slist_free_all(headers);
    }
    if(curl)
    {
        curl_easy_cleanup(curl);
    }
    return ret;
}

int main(int argc, const char *argv[])
{
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "SOAPAction: \"\"");
    headers = curl_slist_append(headers, "Content-Type: charset=utf-8");
    recv_buf recv_data;
    memset(&recv_data, 0, sizeof(recv_data));
    char url[MAX_URL_SIZE] = {0};
    if (argv[1] != NULL)
    {
        strncpy(url, argv[1], sizeof(url) - 1);
    }
    char body[MAX_BODY_SIZE] = {0};
    if (argv[2] != NULL)
    {
        strncpy(body, argv[2], sizeof(body) - 1);
        curl_data(url, POST, body, &recv_data, headers);
    }
    else
        curl_data(url, GET, NULL, &recv_data, headers);
    return 0;
}