#pragma once

#include "../SBot_Core.h"
#include "SBot_Private/SBot_Def.h"

SBOT_BOOL_TYPE Ws_Connect(std::shared_ptr<SBot_Struct> bot_handle_shared_ptr);
void Ws_DisConnect(std::shared_ptr<SBot_Struct> bot_handle_shared_ptr);
const char * Ws_SendApi(std::shared_ptr<SBot_Struct> bot_handle_shared_ptr,const char * self_id,const char * json_str);