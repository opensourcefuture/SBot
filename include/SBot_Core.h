#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define SBOT_HANDLE_TYPE int
#define SBOT_HANDLE_NULL 0
#define SBOT_BOOL_TYPE int
#define SBOT_TRUE 1
#define SBOT_FALSE 0
#define SBOT_ERR_CODE int
#define SBOT_OK 0
#define SBOT_CLIENT_ERR 1
#define SBOT_SERVER_ERR 2
#define SBOT_UNKNOW_ERR 3

    /* 连接Bot */
    SBOT_BOOL_TYPE SBot_Connect(const char *cfg_json_str);

    /* 释放Bot */
    void SBot_DelHandle(SBOT_HANDLE_TYPE handle);

    /* 判断Bot是否连接 */
    SBOT_BOOL_TYPE SBot_IsConnect();

    /* 获取事件 */
    const char * SBot_GetEvent();

    /* 提交文本信息 */
    SBOT_BOOL_TYPE SBot_UpTextMsg(const char *text_msg);

    /* 提交图片信息 */
    /* SBOT_BOOL_TYPE SBot_UpImageMsg(const char * file_id); */

    /* 发送私聊消息 */
    SBOT_BOOL_TYPE SBot_SendPrivateMsg();

    /* 发送群聊消息 */
    SBOT_BOOL_TYPE SBot_SendGroupMsg();


    //线程变量控制函数
    const char * SBot_GetEvtValue(const char *key);
    SBOT_BOOL_TYPE SBot_SetEvtValue(const char *key, const char *value);
    SBOT_BOOL_TYPE SBot_SetHandle(SBOT_HANDLE_TYPE handle);
    SBOT_HANDLE_TYPE SBot_GetHandle(SBOT_HANDLE_TYPE handle);
    SBOT_ERR_CODE SBot_GetErrCode();
    const char * SBot_GetErrStr();


    //用于获取Message的函数
    unsigned int SBot_GetMsgSize();
    const char * SBot_GetMsgType(unsigned int pos);
    const char * SBot_GetTextMsg(unsigned int pos);

    /* 内部使用的函数 */
    SBOT_HANDLE_TYPE _SBot_GetHandle(const char *cfg_json_str);
    SBOT_BOOL_TYPE _SBot_Connect(SBOT_HANDLE_TYPE handle);
    SBOT_BOOL_TYPE _SBot_IsConnect(SBOT_HANDLE_TYPE handle);
    const char *_SBot_GetEvent(SBOT_HANDLE_TYPE handle);
    const char *_SBot_SendApi(SBOT_HANDLE_TYPE handle, const char *self_id, const char *json_str);
    void _SBot_DelHandle(SBOT_HANDLE_TYPE handle);

#ifdef __cplusplus
}

#endif /* __cplusplus */
