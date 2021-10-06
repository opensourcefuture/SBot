#include "SBot_Core.h"
#include "SBot_Ws_Impl/SBot_Ws_Impl.h"
#include "SBot_Private/SBot_Def.h"

#include <thread>
#include <chrono>

/* 忽略jsoncpp reader 的弃用警告 */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

using namespace std;

#ifdef _WIN32
	#define SBOT_EXPORT_API __declspec(dllexport)
#else
	#define SBOT_EXPORT_API __attribute ((visibility("default")))
#endif

/* 获取SBot_Struct */
static weak_ptr<SBot_Struct> Get_SBot_Struct(SBOT_HANDLE_TYPE handle)
{
	lock_guard<mutex> lk(mx_g_sbot_struct_vec);
	for(size_t i = 0;i < g_sbot_struct_vec.size();++i)
	{
		lock_guard<mutex> lk(g_sbot_struct_vec[i]->mx);
		if(g_sbot_struct_vec[i]->handle == handle)
		{
			return g_sbot_struct_vec[i];
		}
	}
	return shared_ptr<SBot_Struct>();
}

extern "C" SBOT_EXPORT_API SBOT_HANDLE_TYPE _SBot_GetHandle(const char * cfg_json_str)
{
	Json::Value cfg_json;
	Json::Reader reader;
	if(cfg_json_str == NULL)
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"cfg_json_str is a null ptr");
		goto ERR_DEAL;
	}
	if (!reader.parse(cfg_json_str, cfg_json))
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"cfg_json_str not a json str");
		goto ERR_DEAL; 
	}
	{
		shared_ptr<SBot_Struct> sbot_struct = make_shared<SBot_Struct>();
		sbot_struct->cfg_json = make_unique<Json::Value>(move(cfg_json));
		{
			static mutex mx;
			static SBOT_HANDLE_TYPE handle_num = SBOT_HANDLE_NULL;
			lock_guard<mutex> lk(mx);
			++handle_num;
			sbot_struct->handle = handle_num;
		}
		lock_guard<mutex> lk(mx_g_sbot_struct_vec);
		g_sbot_struct_vec.emplace_back(sbot_struct);
		_SBot_SetErr(SBOT_OK,"");
		return sbot_struct->handle;
	}
ERR_DEAL:
	return SBOT_HANDLE_NULL;
}
extern "C" SBOT_EXPORT_API SBOT_BOOL_TYPE _SBot_IsConnect(SBOT_HANDLE_TYPE handle)
{
	auto bot_handle_weak_ptr = Get_SBot_Struct(handle);
	auto bot_handle_shared_ptr = bot_handle_weak_ptr.lock();
	if(!bot_handle_shared_ptr)
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"handle not exists");
		return SBOT_FALSE;
	}
	_SBot_SetErr(SBOT_OK,"");
	lock_guard<mutex> lk(bot_handle_shared_ptr->mx);
	return ((bot_handle_shared_ptr->isConnect)?SBOT_TRUE:SBOT_FALSE);
}
extern "C" SBOT_EXPORT_API const char * _SBot_GetEvent(SBOT_HANDLE_TYPE handle)
{
	auto bot_handle_weak_ptr = Get_SBot_Struct(handle);
	auto bot_handle_shared_ptr = bot_handle_weak_ptr.lock();
	if(!bot_handle_shared_ptr)
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"handle not exists");
		return NULL;
	}
	lock_guard<mutex> lk(bot_handle_shared_ptr->mx);
	if(bot_handle_shared_ptr->event_list.size() == 0)
	{
		_SBot_SetErr(SBOT_OK,"");
		return NULL;
	}
	auto ret_json = move(bot_handle_shared_ptr->event_list.back());
	bot_handle_shared_ptr->event_list.pop_back();
	g_ret_get_event_str = Json::FastWriter().write(*ret_json);
	_SBot_SetErr(SBOT_OK,"");
	return g_ret_get_event_str.c_str();
}

/* should impl */
extern "C" SBOT_EXPORT_API SBOT_BOOL_TYPE _SBot_Connect(SBOT_HANDLE_TYPE handle)
{
	auto bot_handle_shared_ptr = Get_SBot_Struct(handle).lock();
	if(!bot_handle_shared_ptr)
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"handle not exists");
		return SBOT_FALSE;
	}
	std::string connect_way;
	try
	{
		lock_guard<mutex> lk(bot_handle_shared_ptr->mx);
		connect_way = (*bot_handle_shared_ptr->cfg_json)["adapter"].asString();
	}
	catch(const std::exception &)
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"can't get adapter from cfg_json");
		return SBOT_FALSE;
	}
	if(connect_way == "ws_onebot11")
	{
		return Ws_Connect(bot_handle_shared_ptr);
	}
	_SBot_SetErr(SBOT_CLIENT_ERR,"the adapter in cfg_json is wrong");
	return SBOT_FALSE;
}
extern "C" SBOT_EXPORT_API const char * _SBot_SendApi(SBOT_HANDLE_TYPE handle,const char * self_id,const char * json_str)
{
	auto bot_handle_shared_ptr = Get_SBot_Struct(handle).lock();
	if(!bot_handle_shared_ptr)
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"handle not exists");
		return NULL;
	}
	std::string connect_way;
	try
	{
		lock_guard<mutex> lk(bot_handle_shared_ptr->mx);
		connect_way = (*bot_handle_shared_ptr->cfg_json)["adapter"].asString();
	}
	catch(const std::exception &)
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"can't get adapter from cfg_json");
		return NULL;
	}
	if(connect_way == "ws_onebot11")
	{
		return Ws_SendApi(bot_handle_shared_ptr,self_id,json_str);
	}
	_SBot_SetErr(SBOT_CLIENT_ERR,"the adapter in cfg_json is wrong");
	return NULL;
}

extern "C" SBOT_EXPORT_API void _SBot_DelHandle(SBOT_HANDLE_TYPE handle)
{
	auto bot_handle_shared_ptr = Get_SBot_Struct(handle).lock();
	if(!bot_handle_shared_ptr)
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"handle not exists");
		return ;
	}
	std::string connect_way;
	try
	{
		lock_guard<mutex> lk(bot_handle_shared_ptr->mx);
		connect_way = (*bot_handle_shared_ptr->cfg_json)["adapter"].asString();
	}
	catch(const std::exception &)
	{
		_SBot_SetErr(SBOT_CLIENT_ERR,"the adapter in cfg_json is wrong");
		return ;
	}
	if(connect_way == "ws_onebot11")
	{
		Ws_DisConnect(bot_handle_shared_ptr);
	}
	lock_guard<mutex> lk(mx_g_sbot_struct_vec);
	for(size_t i = 0;i < g_sbot_struct_vec.size();++i)
	{
		if(g_sbot_struct_vec[i]->handle == handle)
		{
			g_sbot_struct_vec.erase(g_sbot_struct_vec.begin()+i);
			break;
		}
	}
	_SBot_SetErr(SBOT_OK,"");
	return ;
}
