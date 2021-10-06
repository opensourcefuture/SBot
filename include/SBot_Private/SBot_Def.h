#pragma once

#include "../SBot_Core.h"

#include <json/json.h>
#include <memory>
#include <list>
#include <mutex>


struct SBot_Struct
{
	std::mutex mx;
	SBOT_HANDLE_TYPE handle = 0;
	std::unique_ptr<Json::Value> cfg_json;
	bool isConnect = false;
	std::list<std::unique_ptr<Json::Value>> event_list;
};

/* Core使用的全局变量 */
extern std::vector<std::shared_ptr<SBot_Struct>> g_sbot_struct_vec;
extern std::mutex mx_g_sbot_struct_vec;
extern thread_local std::string g_ret_get_event_str;
extern thread_local std::string g_ret_get_api_str;
extern thread_local std::string g_err_msg;
extern thread_local SBOT_ERR_CODE g_err_code;

void _SBot_SetErr(SBOT_ERR_CODE err_code,const std::string & err_msg);


