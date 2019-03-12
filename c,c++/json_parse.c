#include <jansson.h>
json_t *jdata = json_loads(recv_data, 0, NULL);
ON_SCOPE_EXIT(jdata, smart_free_json);

if (!json_is_object(jdata))
{
    CUSTOM_LOG(InfoLog, "receive data isn't json object:%s.", recv_data);
}

json_t *jcode = json_object_get(jdata, "code");
if (!json_is_integer(jcode))
{
    CUSTOM_LOG(InfoLog, "receive code isn't right key-value.");
}
int code = json_integer_value(jcode);

json_t *j_account = json_object_get(jdata, "account");
if (json_is_string(j_account))
{
    const char *account = json_string_value(j_account);
    if (!account)
    {
        CUSTOM_LOG(InfoLog, "receive account is null.");
    }
}

//json嵌套,{"data":{"cellphone":"1231232543"}}
void *iter = NULL;
json_t *data = json_object_get(jdata, "data");
iter = json_object_iter_at(data, "cellphone");
json_t *jcode = json_object_iter_value(iter);
const char *cellphone = json_string_value(jcode);

json_t *j_true = json_object_get(jdata, "true");
json_is_true(j_true);

