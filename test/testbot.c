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
		if( strcmp(SBot_GetEvtValue("message_type"),"group") == 0 &&
			SBot_GetMsgSize() == 1 &&
			strcmp(SBot_GetMsgType(0),"text") == 0)
		{
			const char * text_msg = SBot_GetTextMsg(0);
			if(strcmp(text_msg,"ping") == 0)
			{
				printf("recv:ping,send pong\n");
				const char * file_id =SBot_MakeImgFileIdByPath("D:\\1图片.jpg");
				if(strcmp(file_id,"") == 0)
				{
					printf("upload file err:%s\n",SBot_GetErrStr());
					continue;
				}
				SBot_UpAtMsg(SBot_GetEvtValue("user_id"));
				SBot_UpImgMsg(file_id);
				SBot_UpTextMsg("pong");
				if(strcmp(SBot_SendGroupMsg(),"") == 0)
				{
					printf("SBot_SendGroupMsg err:%s\n",SBot_GetErrStr());
				}
			}
			else if(strcmp(text_msg,"pong") == 0)
			{
				printf("recv:pong,send ping\n");
				const char * file_id =SBot_MakeImgFileIdByUrl("https://1bot.dev/assets/logo-white.png");
				if(strcmp(file_id,"") == 0)
				{
					printf("upload file err:%s\n",SBot_GetErrStr());
					continue;
				}
				SBot_UpAtMsg(SBot_GetEvtValue("user_id"));
				SBot_UpImgMsg(file_id);
				SBot_UpTextMsg("ping");
				SBot_SendGroupMsg();
			}

		}
	}
	SBot_DelHandle(handle);
	printf("connect lost\n");
    return 0;
}
