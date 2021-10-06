#include "SBot_Private/SBot_Def.h"

using namespace std;


vector<shared_ptr<SBot_Struct>> g_sbot_struct_vec;
mutex mx_g_sbot_struct_vec;
thread_local std::string g_ret_get_event_str;
thread_local std::string g_ret_get_api_str;
thread_local std::string g_err_msg;
thread_local SBOT_ERR_CODE g_err_code;

void _SBot_SetErr(SBOT_ERR_CODE err_code,const std::string & err_msg)
{
    g_err_code = err_code;
    g_err_msg = err_msg;
}

