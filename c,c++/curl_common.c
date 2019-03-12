#define MAX_BODY_SIZE (2048)    //接收http响应的最大缓冲区

typedef enum HTTP_REQ_TYPE
{
    GET = 0,
    POST
}HTTP_REQ_TYPE;

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
    rbuf->buf[rbuf->size] = '\0';
    return sizes;
}

/*
 * @brief 发送http请求，获取http响应
 * @param[in] url url地址
 * @param[in] req_type 请求类型
 * @param[in] send_data body数据
 * @param[in] headers 请求头部信息
 * @param[out] recv_data http响应数据
 * @param[out] res_code http响应的状态码
 * @return 0成功，-1失败
 */
static int curl_data(const char *url, const int req_type, const char *send_data,
                         recv_buf *recv_data, struct curl_slist *headers, long *res_code)
{
    assert(url && recv_data);
    CURL *curl = NULL;
    CURLcode res;
    curl = curl_easy_init();
    if (!curl)
    {
        CUSTOM_LOG(ErrLog, "curl_easy_init fail.");
        return -1;
    }
    ON_SCOPE_EXIT(curl, smart_free_curl);
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
    if (res != CURLE_OK)
    {
        CUSTOM_LOG(ErrLog, "curl %s failed : error(%s)", url, curl_easy_strerror(res));
        return -1;
    }
    if (res_code != NULL)
    {
        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, res_code);
        if (res != CURLE_OK)
        {
            CUSTOM_LOG(ErrLog, "get http response code failed : url(%s),error(%s)", url, curl_easy_strerror(res));
            return -1;
        }
    }
    CUSTOM_LOG(DebugLog, "curl success,get data(%s)", recv_data->buf);
    return 0;
}

struct curl_slist *headers = NULL;
headers = curl_slist_append(headers, "Content-Type: text/xml; charset=utf-8");
headers = curl_slist_append(headers, "SOAPAction: \"\"");
ON_SCOPE_EXIT(headers, smart_free_headers);

recv_buf recv_data;
memset(&recv_data, 0, sizeof(recv_data));
curl_data(url, POST, body, &recv_data, headers, NULL);

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