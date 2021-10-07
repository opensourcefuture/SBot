#include <SBot_Core.h>
#include <json/json.h>
#include <SBot_Private/SBot_Def.h>
#include <thread>
#include <chrono>
#include <base64.h>
#include <SBot_Private/SBot_Tools.h>

/* 忽略jsoncpp reader 的弃用警告 */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

using namespace std;

#ifdef _WIN32
	#define SBOT_EXPORT_API __declspec(dllexport)
#else
	#define SBOT_EXPORT_API __attribute ((visibility("default")))
#endif

thread_local SBOT_HANDLE_TYPE g_handle;
thread_local Json::Value g_event_json;
thread_local Json::Value g_send_msg;
thread_local std::string g_get_evt_value_str;
thread_local std::string g_get_msg_type;
thread_local std::string g_get_text_msg;
thread_local std::string g_make_img_id_by_path;
thread_local std::string g_to_ansi_str;
thread_local std::string g_to_utf8_str;

using namespace std;

extern "C" SBOT_EXPORT_API SBOT_HANDLE_TYPE SBot_Connect(const char * cfg_json_str)
{
	g_handle =  _SBot_GetHandle(cfg_json_str);
    if(g_handle == SBOT_HANDLE_NULL)
    {
        return SBOT_HANDLE_NULL;
    }
    SBOT_BOOL_TYPE is_connect =  _SBot_Connect(g_handle);
    if(is_connect == SBOT_TRUE)
    {
        return g_handle;
    }
    _SBot_DelHandle(g_handle);
    return SBOT_HANDLE_NULL;
}

extern "C" SBOT_EXPORT_API SBOT_BOOL_TYPE SBot_IsConnect()
{
    return _SBot_IsConnect(g_handle);
}

extern "C" SBOT_EXPORT_API const char * SBot_GetEvent()
{
    g_event_json = Json::nullValue;
	const char * evt_str = _SBot_GetEvent(g_handle);
    if(evt_str == NULL)
    {
        this_thread::sleep_for(chrono::milliseconds(1));
        return "";
    }
    Json::Reader reader;
	reader.parse(evt_str, g_event_json); //must success
    return evt_str;
}

extern "C" SBOT_EXPORT_API SBOT_BOOL_TYPE SBot_UpTextMsg(const char * text_msg)
{
    if(!text_msg)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"text_msg is null");
        return SBOT_FALSE;
    }
    Json::Value send_json;
    send_json["type"] = "text";
    send_json["data"]["text"] = text_msg;
    g_send_msg.append(send_json);
    _SBot_SetErr(SBOT_OK,"");
    return SBOT_TRUE;
}

extern "C" SBOT_EXPORT_API SBOT_BOOL_TYPE SBot_SendPrivateMsg()
{
    Json::Value send_json;
    send_json["action"] = "send_private_msg";
    send_json["params"]["user_id"] = g_event_json["user_id"];
    send_json["params"]["message"] = g_send_msg;
    g_send_msg = Json::Value();
    string self_id;
    if(g_event_json["self_id"].isInt64())
    {
        self_id = to_string(g_event_json["self_id"].asInt64());
    }
    else if(g_event_json["self_id"].isString())
    {
        self_id = g_event_json["self_id"].asString();
    }
    else
    {
        _SBot_SetErr(SBOT_SERVER_ERR,"self_id not found");
        return SBOT_FALSE;
    }
    
    const char * ret_str =  \
        _SBot_SendApi(g_handle,self_id.c_str(),Json::FastWriter().write(send_json).c_str());
    if(!ret_str)
    {
        return SBOT_FALSE;
    }
    Json::Reader reader;
    Json::Value ret_json;
	if(!reader.parse(ret_str, ret_json))
    {
        _SBot_SetErr(SBOT_SERVER_ERR,"parse recv json err");
        return SBOT_FALSE;
    }
    if( !ret_json.isObject() || 
        !ret_json["retcode"].isInt() || 
        (ret_json["retcode"].asInt() != 0)
    )
    {
        _SBot_SetErr(SBOT_SERVER_ERR,"retcode in recv json not 0");
        return SBOT_FALSE;
    }
    return SBOT_TRUE;
}

extern "C" SBOT_EXPORT_API SBOT_BOOL_TYPE SBot_SendGroupMsg()
{
    Json::Value send_json;
    send_json["action"] = "send_group_msg";
    send_json["params"]["user_id"] = g_event_json["user_id"];
    send_json["params"]["group_id"] = g_event_json["group_id"];
    send_json["params"]["message"] = g_send_msg;
    g_send_msg = Json::Value();
    string self_id;
    if(g_event_json["self_id"].isInt64())
    {
        self_id = to_string(g_event_json["self_id"].asInt64());
    }
    else if(g_event_json["self_id"].isString())
    {
        self_id = g_event_json["self_id"].asString();
    }
    else
    {
        _SBot_SetErr(SBOT_SERVER_ERR,"self_id not found");
        return SBOT_FALSE;
    }
    
    const char * ret_str =  \
        _SBot_SendApi(g_handle,self_id.c_str(),Json::FastWriter().write(send_json).c_str());
    if(!ret_str)
    {
        return SBOT_FALSE;
    }
    Json::Reader reader;
    Json::Value ret_json;
	if(!reader.parse(ret_str, ret_json))
    {
        _SBot_SetErr(SBOT_SERVER_ERR,"parse recv json err");
        return SBOT_FALSE;
    }
    if( !ret_json.isObject() || 
        !ret_json["retcode"].isInt() || 
        (ret_json["retcode"].asInt() != 0)
    )
    {
        _SBot_SetErr(SBOT_SERVER_ERR,"retcode in recv json not 0");
        return SBOT_FALSE;
    }
    return SBOT_TRUE;
}


extern "C" SBOT_EXPORT_API const char * SBot_GetEvtValue(const char * key)
{
    if(!key)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"key is null");
        return "";
    }
    if(!g_event_json.isObject())
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"SBot_GetEvent not call before");
        return "";
    }
    auto ret_json = g_event_json[key];
    if(ret_json.isString())
    {
        g_get_evt_value_str = ret_json.asString();
    }
    else if(ret_json.isInt64())
    {
        g_get_evt_value_str = to_string(ret_json.asInt64());
    }
    else
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"the key ` " + string(key) + "` not exist");
        return "";
    }
    _SBot_SetErr(SBOT_OK,"");
    return g_get_evt_value_str.c_str();
}

extern "C" SBOT_EXPORT_API SBOT_BOOL_TYPE SBot_SetEvtValue(const char * key,const char * value)
{
    if(!key)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"key is null");
        return SBOT_FALSE;
    }
    if(!g_event_json.isObject())
    {
        g_event_json = Json::objectValue;
    }
    if(!value)
    {
        g_event_json[key] = Json::nullValue;
    }
    else
    {
        g_event_json[key] = value;
    }
    _SBot_SetErr(SBOT_OK,"");
    return SBOT_TRUE;
}

extern "C" SBOT_EXPORT_API void SBot_DelHandle(SBOT_HANDLE_TYPE handle)
{
    _SBot_DelHandle(handle);
    g_handle = SBOT_HANDLE_NULL;
}

extern "C" SBOT_EXPORT_API SBOT_BOOL_TYPE SBot_SetHandle(SBOT_HANDLE_TYPE handle)
{
	g_handle = handle;
    _SBot_SetErr(SBOT_OK,"");
	return SBOT_TRUE;
}

extern "C" SBOT_EXPORT_API SBOT_HANDLE_TYPE SBot_GetHandle(SBOT_HANDLE_TYPE handle)
{
    _SBot_SetErr(SBOT_OK,"");
	return g_handle;
}

extern "C" SBOT_EXPORT_API unsigned int SBot_GetMsgSize()
{
    if(!g_event_json.isObject())
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"SBot_GetEvent not call before");
        return  0;
    }
	Json::Value msg_vec = g_event_json["message"];
    if(msg_vec.isNull())
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"not a message");
        return 0;
    }
    if(!msg_vec.isArray())
    {
        _SBot_SetErr(SBOT_SERVER_ERR,"not a array message");
        return 0;
    }
    _SBot_SetErr(SBOT_OK,"");
    return msg_vec.size();
}

extern "C" SBOT_EXPORT_API const char * SBot_GetMsgType(unsigned int pos)
{
    try
    {
        g_get_msg_type = g_event_json["message"][pos]["type"].asString();
        if(/* g_get_msg_type == "image" || */ g_get_msg_type == "text")
        {
            return g_get_msg_type.c_str();
        }
        return "unknow";
    }
    catch(const std::exception & e)
    {
         _SBot_SetErr(SBOT_UNKNOW_ERR,e.what());
         return "";
    }
    _SBot_SetErr(SBOT_OK,"");
}

extern "C" SBOT_EXPORT_API const char * SBot_GetTextMsg(unsigned int pos)
{
    try
    {
        g_get_text_msg = g_event_json["message"][pos]["data"]["text"].asString();
       return g_get_text_msg.c_str();
    }
    catch(const std::exception & e)
    {
        _SBot_SetErr(SBOT_UNKNOW_ERR,e.what());
         return "";
    }
    _SBot_SetErr(SBOT_OK,"");
}

extern "C" SBOT_EXPORT_API SBOT_ERR_CODE SBot_GetErrCode()
{
    return g_err_code;
}

extern "C" SBOT_EXPORT_API const char * SBot_GetErrStr()
{
    return g_err_msg.c_str();
}

extern "C" SBOT_EXPORT_API const char * SBot_MakeImgFileIdByPath(const char * path_str)
{
    if(!path_str)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"path_str is null");
        return "";
    }
    std::string ansi_path_str = to_ansi(path_str);
    FILE * fp = fopen(ansi_path_str.c_str(),"rb");
    if(!fp)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"img file: ` " + string(ansi_path_str) + "` can't open");
         return "";
    }
    fseek(fp,0 ,SEEK_END);  
    long f_size = ftell(fp);
    if(f_size == 0)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"img file: ` " + string(ansi_path_str) + "` can't get size");
        fclose(fp);
         return "";
    }
    rewind(fp);
    std::string file_buf(f_size, '\0');
    size_t read_size = fread((void * )file_buf.data(),1,f_size,fp);
    if(read_size != f_size)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"img file: ` " + string(ansi_path_str) + "` can't read");
         fclose(fp);
         return "";
    }
    fclose(fp);
    try
    {
        g_make_img_id_by_path =  base64_encode(file_buf,false);
    }
    catch(const std::exception&)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"img file: ` " + string(ansi_path_str) + "` can't conver to base64");
        return "";
    }
    return  g_make_img_id_by_path.c_str();
}

extern "C" SBOT_EXPORT_API SBOT_BOOL_TYPE SBot_UpImgMsg(const char * file_id)
{
    if(!file_id)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"file_id is null");
        return SBOT_FALSE;
    }
    Json::Value send_json;
    send_json["type"] = "image";
    send_json["data"]["file"] = string("base64://") +  file_id;
    g_send_msg.append(send_json);
    _SBot_SetErr(SBOT_OK,"");
    return SBOT_TRUE;
}

extern "C" SBOT_EXPORT_API const char * SBot_ToAnsi(const char * utf8_str)
{
    if(!utf8_str)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"utf8_str is null");
        return SBOT_FALSE;
    }
    g_to_ansi_str = to_ansi(utf8_str);
    return g_to_ansi_str.c_str();
}

extern "C" SBOT_EXPORT_API const char * SBot_ToUtf8(const char * ansi_str)
{
    if(!ansi_str)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"ansi_str is null");
        return SBOT_FALSE;
    }
    g_to_utf8_str = to_utf8(ansi_str);
    return g_to_utf8_str.c_str();
}