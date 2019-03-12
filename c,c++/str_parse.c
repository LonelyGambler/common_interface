/*
 * @brief 基于字符串解析，解析返回报文中所需要的字段  
 * @param[in] src:待解析的字符串
 * @param[in] str_begin:标签头
 * @param[in] str_end:标签尾
 * @param[in] dest_len:支持解析的最大长度
 * @param[out] dest:解析后得到的字符串
 * return 0成功，其它失败
 */
static int str_parse(const char *src, const char *str_begin, const char *str_end, char *dest, int dest_len)
{
    assert(src && str_begin && str_end && dest);
    char *begin = strstr(src, str_begin);
    char *end = strstr(src, str_end);
    int str_begin_len = strlen(str_begin);
    if (!begin || !end)
    {
        CUSTOM_LOG(InfoLog, "string parse fail,can't find str_begin(%s) and str_end(%s) from response.",
                    str_begin, str_end);
        return -1;
    }
    begin += str_begin_len;
    int cp_len = end - begin;
    if (cp_len <= 0)
    {
        CUSTOM_LOG(InfoLog, "string parse fail,maybe str_end(%s) is in front of str_begin(%s) or value is NULL",
                    str_end, str_begin);
        return -1;
    }
    //预留一个'\0'
    if (cp_len > dest_len - 1)
    {
        CUSTOM_LOG(InfoLog, "string parse fail,copy length(%d) is longer than buffer reserved length(%d).",
                    cp_len, dest_len - 1);
        return -1;
    }
    strncpy(dest, begin, cp_len);
    dest[cp_len] = '\0';
    return 0;
}
