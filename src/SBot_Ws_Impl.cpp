#include "SBot_Ws_Impl/SBot_Ws_Impl.h"

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _WEBSOCKETPP_CPP11_THREAD_
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <map>
#include "SBot_Private/SBot_Tools.h"

using MessagePtr = websocketpp::config::asio_client::message_type::ptr;
using Client = websocketpp::client<websocketpp::config::asio_client>;
using namespace std;

/* 忽略jsoncpp reader 的弃用警告 */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

struct Con_Def
{
    shared_ptr<Client> client;
    weak_ptr<SBot_Struct> sbot_struct;
    websocketpp::connection_hdl hdl;
    std::thread thrd;
};

static list<shared_ptr<Con_Def>> sg_client_vec;
static mutex sg_mx_client_vec;

static map<string,string> sg_echo_map;
static mutex sg_mx_echo_map;


static weak_ptr<Con_Def> Get_Con_Def_By_Handle(SBOT_HANDLE_TYPE handle)
{
    lock_guard<mutex> lk(sg_mx_client_vec);
    for(auto & it:sg_client_vec)
    {
        auto sbot_struct_ptr = it->sbot_struct.lock();
        if(!sbot_struct_ptr)
        {
            continue;
        }
        lock_guard<mutex> lk(sbot_struct_ptr->mx);
        if(sbot_struct_ptr->handle == handle)
        {
            return it;
        }
    }
    return shared_ptr<Con_Def>();
}

static void Del_Con_By_Handle(SBOT_HANDLE_TYPE handle)
{
    lock_guard<mutex> lk(sg_mx_client_vec);
    for(auto it = sg_client_vec.begin();it != sg_client_vec.end();it++)
    {
        auto sbot_struct_ptr = (*it)->sbot_struct.lock();
        if(!sbot_struct_ptr)
        {
            continue;
        }
        lock_guard<mutex> lk(sbot_struct_ptr->mx);
        if(sbot_struct_ptr->handle == handle)
        {
            sbot_struct_ptr->isConnect = false;
            try
            {
                (*it)->client->get_con_from_hdl((*it)->hdl)->close(websocketpp::close::status::value(), "byebye");
            }
            catch(std::exception &)
            {
                //do nothing
            }
            (*it)->thrd.join();
            sg_client_vec.erase(it);
            return;
        }
    }
}

static void Deal_Str_In_SendJson(Json::Value & send_json)
{
    const char * cvt_str_vec[] = {"user_id","group_id","target_id","self_id","message_id"};
    for(int i = 0;i < sizeof(cvt_str_vec)/sizeof(char *); ++i)
    {
        if(send_json["params"][cvt_str_vec[i]].isString())
        {
            if(!(sizeof(long long int) == 8))
            {
                throw std::runtime_error("the size of long long int not 8，don't support this platform");
            }
            Json::Int64 tempint64 = stoll(send_json["params"][cvt_str_vec[i]].asString());
            send_json["params"][cvt_str_vec[i]] = tempint64;
        }
    }
}

static void Cvt_Json_Int_To_Str(Json::Value & send_json)
{
    if(send_json.isObject())
    {
        Json::Value::Members member = send_json.getMemberNames();
        for (std::vector<std::string>::iterator iter = member.begin(); iter != member.end(); iter++)
		{
            if(send_json[(*iter)].isInt64())
            {
                send_json[(*iter)] = to_string(send_json[(*iter)].asInt64());
            }
            else
            {
                Cvt_Json_Int_To_Str(send_json[(*iter)]);
            }
        }
    }else if(send_json.isArray())
    {
        for(auto & it:send_json)
        {
            if(it.isInt64())
            {
                it = to_string(it.asInt64());
            }
            else
            {
                Cvt_Json_Int_To_Str(it);
            }
        }
    }
}

static void Replace_All_Distinct(std::string& str, const std::string& old_value, const std::string& new_value) 
{
	for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())
	{
		if ((pos = str.find(old_value, pos)) != std::string::npos)
		{
			str.replace(pos, old_value.length(), new_value);
		}
		else 
		{ 
			break;
		}
	}
}

static Json::Value CQ_Str_To_JsonArr(const std::string& cq_str) 
{
	using namespace std;
	std::vector<std::string> dat_vec;
	auto all_vec = match_all(cq_str, "(\\[CQ:[^\\[\\],]+?(,[^\\[\\],]+?=[^\\[\\],]*?)*?\\])|([^\\[\\],]+)");
	for (auto& i : all_vec)
	{
		dat_vec.push_back(i.at(0));
	}
	Json::Value jsonarr = Json::arrayValue;
	for (auto& dat : dat_vec)
	{
		if (dat[0] == '[')
		{
			/* 说明是CQ码 */
			size_t pos1 = dat.find_first_of(",");
			Json::Value node;
			node["type"] = dat.substr(4, pos1 - 4);
			Json::Value dat_node = Json::objectValue;
			
			auto all_vec = match_all(dat, "[:,]([^\\[\\],]+?)=([^\\[\\],]*?)(?=[\\],])");
			for (auto& i : all_vec)
			{
				dat_node[i.at(1)] = i.at(2);
			}
			node["data"] = dat_node;
			jsonarr.append(node);
		}
		else
		{
			/* 说明是text */
			Json::Value node;
			node["type"] = "text";
			Json::Value dat_node;
			Replace_All_Distinct(dat, "&amp;", "&");
			Replace_All_Distinct(dat, "&#91;", "[");
			Replace_All_Distinct(dat, "&#93;", "]");
			Replace_All_Distinct(dat, "&#44;", ",");
			dat_node["text"] = dat;
			node["data"] = dat_node;
			jsonarr.append(node);
		}
	}
	//std::string s = jsonarr.toStyledString();
	return jsonarr;
}

static SBOT_BOOL_TYPE _Ws_Connect(std::shared_ptr<SBot_Struct> bot_handle_shared_ptr)
{
    auto con_def = make_shared<Con_Def>();
    SBOT_HANDLE_TYPE handle;
    {
        lock_guard<mutex> lk(bot_handle_shared_ptr->mx);
        con_def->sbot_struct = bot_handle_shared_ptr;
        if(bot_handle_shared_ptr->isConnect)
        {
            _SBot_SetErr(SBOT_OK,"");
            return SBOT_TRUE;
        }
        handle = bot_handle_shared_ptr->handle;
    }
    
    string ws_url;
    string access_token;
    {
        lock_guard<mutex> lk(bot_handle_shared_ptr->mx);
        ws_url = (*bot_handle_shared_ptr->cfg_json)["ws_url"].asString();
        if((*bot_handle_shared_ptr->cfg_json)["access_token"].isString())
        {
            access_token = (*bot_handle_shared_ptr->cfg_json)["access_token"].asString();
        }
    }
    
    auto client_ptr = make_shared<Client>();
    con_def->client = client_ptr;
    {
        lock_guard<mutex> lk(sg_mx_client_vec);
        sg_client_vec.emplace_back(con_def);
    }
    con_def->client->set_access_channels(websocketpp::log::alevel::none);
    con_def->client->set_error_channels(websocketpp::log::elevel::none);
	con_def->client->init_asio();
    con_def->client->set_message_handler([handle](websocketpp::connection_hdl hdl, MessagePtr msg) ->void
	{
        
        if(msg->get_opcode() != websocketpp::frame::opcode::text)
        {
            return ;
        }
        string event_str = msg->get_payload();
        Json::Reader reader;
        Json::Value recv_json;
        if (!reader.parse(event_str, recv_json))
        {
            //do nothing
            return ;
        }
        try
        {
            if(recv_json["message"].isString())
            {
                 recv_json["message"] = CQ_Str_To_JsonArr(recv_json["message"].asString());
            }
            Cvt_Json_Int_To_Str(recv_json);
        }
        catch(const std::exception &)
        {
            //do nothing
            return ;
        }
        if(recv_json.isObject() && recv_json["post_type"].isString())
        {
            auto con_def_ptr = Get_Con_Def_By_Handle(handle).lock();
            if(!con_def_ptr)
            {
                return ;
            }
            auto sbot_struct_ptr = con_def_ptr->sbot_struct.lock();
            if(!sbot_struct_ptr)
            {
                return ;
            }
            lock_guard<mutex> lk(sbot_struct_ptr->mx);
            while(sbot_struct_ptr->event_list.size() > 1024)
            {
                sbot_struct_ptr->event_list.pop_front();
            }
            sbot_struct_ptr->event_list.emplace_back(make_unique<Json::Value>(recv_json));
        }
        else if(recv_json.isObject() && recv_json["echo"].isString())
        {
            string echo = recv_json["echo"].asString();
            lock_guard<mutex> lk(sg_mx_echo_map);
            if(sg_echo_map.find(echo) != sg_echo_map.end())
            {
                sg_echo_map[echo] = Json::FastWriter().write(recv_json);
            }
        }
        

        //cout << event_str << endl;
    });
    con_def->client->set_open_handler([handle](websocketpp::connection_hdl hdl) ->void
	{
        auto con_def_ptr = Get_Con_Def_By_Handle(handle).lock();
        if(!con_def_ptr)
        {
            //cout << "find handle error" << endl;
            return;
        }
		auto sbot_ptr = con_def_ptr->sbot_struct.lock();
        if(sbot_ptr)
        {
            lock_guard<mutex> lk(sbot_ptr->mx);
            sbot_ptr->isConnect = true;
        }
        //cout << "connect success" << endl;
	});
    websocketpp::lib::error_code ec;
    Client::connection_ptr con = con_def->client->get_connection(ws_url,ec);
    if (ec)
	{
        _SBot_SetErr(SBOT_CLIENT_ERR,"can't connect ws server:" + ec.message());
		return SBOT_FALSE;
	}
    if (access_token != "")
	{
        con->append_header("Authorization", "Bearer " + access_token);
	}
    con_def->hdl = con->get_handle();
    con_def->client->connect(con);
    con_def->thrd = thread([handle]()
	{
        auto con_def_ptr = Get_Con_Def_By_Handle(handle).lock();
        if(!con_def_ptr)
        {
            //cout << "find handle error" << endl;
            return;
        }
		try
		{
			con_def_ptr->client->run();
		}
		catch (const std::exception& e)
		{
			cout << e.what() << endl;
		}
        auto sbot_ptr = con_def_ptr->sbot_struct.lock();
        if(sbot_ptr)
        {
            lock_guard<mutex> lk(sbot_ptr->mx);
            sbot_ptr->isConnect = false;
        }
	});
    for (int i = 0; i < 10; ++i)
	{
		this_thread::sleep_for(chrono::milliseconds(500));
        lock_guard<mutex> lk(bot_handle_shared_ptr->mx);
        if(bot_handle_shared_ptr->isConnect)
        {
            _SBot_SetErr(SBOT_OK,"");
            return SBOT_TRUE;
        }
	}
    Del_Con_By_Handle(handle);
    con_def->thrd.join();
    _SBot_SetErr(SBOT_CLIENT_ERR,"conect ws server err:timeout");
    return SBOT_FALSE;
}

SBOT_BOOL_TYPE Ws_Connect(std::shared_ptr<SBot_Struct> bot_handle_shared_ptr)
{
    try
    {
        return _Ws_Connect(bot_handle_shared_ptr);
    }
    catch (const std::exception & e)
    {
       _SBot_SetErr(SBOT_UNKNOW_ERR,e.what());
    }
    return SBOT_FALSE;
}

static void _Ws_DisConnect(std::shared_ptr<SBot_Struct> bot_handle_shared_ptr)
{
    Del_Con_By_Handle(bot_handle_shared_ptr->handle);
}

void Ws_DisConnect(std::shared_ptr<SBot_Struct> bot_handle_shared_ptr)
{
    try
    {
        return _Ws_DisConnect(bot_handle_shared_ptr);
    }
    catch (const std::exception & e)
    {
       _SBot_SetErr(SBOT_UNKNOW_ERR,e.what());
    }
}



const char * Ws_SendApi(std::shared_ptr<SBot_Struct> bot_handle_shared_ptr,const char * self_id,const char * json_str)
{
    if(!self_id || !json_str)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"self_id or json_str is null");
        return NULL;
    }
    Json::Reader reader;
    Json::Value send_json;
    if (!reader.parse(json_str, send_json))
    {
         _SBot_SetErr(SBOT_CLIENT_ERR,"parse json_str error");
        return NULL;
    }
    string echo = gen_echo();
    send_json["echo"] = echo;

    try
    {
        Deal_Str_In_SendJson(send_json);
    }
    catch(const std::exception & e)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,string("send json is wrong:") + e.what());
        return NULL;
    }
    
    
    try
    {
        auto con_def_ptr = Get_Con_Def_By_Handle(bot_handle_shared_ptr->handle).lock();
        if(con_def_ptr->client)
        {
            {
                lock_guard<mutex> lk(sg_mx_echo_map);
                sg_echo_map[echo] = "";
            }
            con_def_ptr->client->send(con_def_ptr->hdl, Json::FastWriter().write(send_json), websocketpp::frame::opcode::text);
        }
        else
        {
            _SBot_SetErr(SBOT_CLIENT_ERR,"handle not exists");
            return NULL;
        }
    }
    catch(const std::exception &)
    {
        _SBot_SetErr(SBOT_CLIENT_ERR,"can't send,maybe not connect");
        lock_guard<mutex> lk(sg_mx_echo_map);
        sg_echo_map.erase(echo);
        return NULL;
    }


    int i;
    int times = 300;
    for(i = 0;i < 300;++i)
    {
        this_thread::sleep_for(chrono::milliseconds(50));
        {
                lock_guard<mutex> lk(sg_mx_echo_map);
                auto & it = sg_echo_map[echo];
                if(it != "")
                {
                    g_ret_get_api_str = it;
                    break;
                }
        }
    }

    {
        lock_guard<mutex> lk(sg_mx_echo_map);
        sg_echo_map.erase(echo);
    }

    if(i == times)
    {
        _SBot_SetErr(SBOT_SERVER_ERR,"wait for recv timeout");
        return NULL;
    }
    _SBot_SetErr(SBOT_OK,"");
    return g_ret_get_api_str.c_str(); 
}