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


    typedef struct SBOT_LOGININFO_TYPE
    {
        const char * user_id;
        const char * nickname;
    }__attribute__((aligned(1))) SBOT_LOGININFO_TYPE;

    typedef struct SBOT_FRIENDINFOLIST_TYPE
    {
        const char * user_id;
        const char * nickname;
        struct SBOT_FRIENDINFOLIST_TYPE * next;
    }__attribute__((aligned(1))) SBOT_FRIENDINFOLIST_TYPE;

    typedef struct SBOT_GROUPINFOLIST_TYPE
    {
        const char * group_id;
        const char * group_name;
        struct SBOT_GROUPINFOLIST_TYPE * next;
    }__attribute__((aligned(1))) SBOT_GROUPINFOLIST_TYPE;

    typedef struct SBOT_GROUPMEMBERINFOLIST_TYPE
    {
        const char * member_id;
        const char * member_name;
        struct SBOT_GROUPMEMBERINFOLIST_TYPE * next;
    }__attribute__((aligned(1))) SBOT_GROUPMEMBERINFOLIST_TYPE;

    SBOT_HANDLE_TYPE SBot_Connect(const char *cfg_json_str);
    void SBot_DelHandle(SBOT_HANDLE_TYPE handle);
    SBOT_BOOL_TYPE SBot_IsConnect();

    const char * SBot_GetEvent();
    SBOT_BOOL_TYPE SBot_UpTextMsg(const char * text_msg);
    const char * SBot_MakeImgFileIdByPath(const char * path_str);
    const char * SBot_MakeImgFileIdByUrl(const char * url_str);
    SBOT_BOOL_TYPE SBot_UpImgMsg(const char * file_id);
    SBOT_BOOL_TYPE SBot_UpAtMsg();
    SBOT_BOOL_TYPE SBot_UpReplyMsg();
    const char * SBot_SendPrivateMsg();
    const char * SBot_SendGroupMsg();
    SBOT_BOOL_TYPE SBot_GroupBan(unsigned int sec);

    SBOT_LOGININFO_TYPE * SBot_GetLoginInfo();
    SBOT_FRIENDINFOLIST_TYPE * SBot_GetFriendList();
    SBOT_GROUPINFOLIST_TYPE * SBot_GetGroupList();
    SBOT_GROUPMEMBERINFOLIST_TYPE * SBot_GetGroupMemberList();


    SBOT_BOOL_TYPE SBot_DelMsg();


    //线程变量控制函数
    const char * SBot_GetEvtValue(const char *key);
    SBOT_BOOL_TYPE SBot_SetEvtValue(const char *key, const char *value);
    SBOT_BOOL_TYPE SBot_SetHandle(SBOT_HANDLE_TYPE handle);
    SBOT_HANDLE_TYPE SBot_GetHandle();
    SBOT_ERR_CODE SBot_GetErrCode();
    const char * SBot_GetErrStr();

    const char * SBot_ToAnsi(const char * utf8_str);
    const char * SBot_ToUtf8(const char * ansi_str);
    /* const char * SBot_Quote(const char * url); */

    //用于获取Message的函数
    unsigned int SBot_GetMsgSize();
    const char * SBot_GetMsgType(unsigned int pos);
    const char * SBot_GetTextMsg(unsigned int pos);
    const char * SBot_GetAtMsg(unsigned int pos);
    const char * SBot_GetReplyMsg(unsigned int pos);

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
