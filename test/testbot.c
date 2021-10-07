#include <string.h>
#include <stdio.h>
#include <SBot_Core.h>

int main()
{
    const char * json_cfg = "\
	{\
        \"adapter\":\"ws_onebot11\",\
        \"ws_url\":\"ws://localhost:6700\",\
        \"access_token\":\"77156\"\
    }";
	SBOT_HANDLE_TYPE handle = SBot_Connect(json_cfg);
	if(handle == SBOT_HANDLE_NULL)
	{
        printf("connect false\n");
		return 0;
	}
	printf("connect success\n");
	for(;;)
	{
		if(SBot_IsConnect() == SBOT_FALSE)
		{
			break;
		}
		if(strcmp(SBot_GetEvent(),"") == 0)
		{
			continue;
		}
		if( strcmp(SBot_GetEvtValue("message_type"),"private") == 0 &&
			SBot_GetMsgSize() == 1 &&
			strcmp(SBot_GetMsgType(0),"text") == 0)
		{
			const char * text_msg = SBot_GetTextMsg(0);
			if(strcmp(text_msg,"ping") == 0)
			{
				printf("recv:ping,send pong\n");
				SBot_UpTextMsg("pong");
				SBot_SendPrivateMsg();
			}
			else if(strcmp(text_msg,"pong") == 0)
			{
				printf("recv:pong,send ping\n");
				SBot_UpTextMsg("ping");
				SBot_SendPrivateMsg();
			}

		}
	}
	SBot_DelHandle(handle);
	printf("connect lost\n");
    return 0;
}
