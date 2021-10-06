# SBot

[TOC]

## 目的

提供一个对接`OneBot`的跨平台C语言`SDK`

## 编译安装

### 编译环境

* `CMake 3.15`及以上
* 支持`c++17`的`gcc`（`windows` 下可以是`mingw`）
* `git`以及网络支持（某些依赖会自动从`github`拉取）

### 编译命令

在`build`目录下：

```shell
cmake ..
make
make install
```

## 使用

### 在 CMakeLists.txt 中导入 SBot

```cmake
find_package(SBot)
target_link_libraries(${PROJECT_NAME} PRIVATE SBot)
```

### 使用示例

一个`ping/pong`插件：私聊收到`ping`，回复`pong`；收到`pong`，回复`ping`。

```c
/* file : main.c */
#include <string.h>
#include <stdio.h>
#include <SBot_Core.h>

int main()
{
    const char * json_cfg = "\
	{\
        \"adapter\":\"ws_onebot11\",\
        \"ws_url\":\"ws://localhost:6700\",\
        \"access_token\":\"super1207\"\
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

```

## 关于内存释放，指针使用的约定

1. 返回指针的函数，不需要用户手动调用去分配函数。
2. 返回`const char *`的函数，保证返回C语言字符串（以`'\0'`结尾且其它位置不含`'\0'`），不会返回空指针。
3. 返回的指针指向的资源，在下一次在同一线程中调用同名函数时自动释放。
4. 以上约定对于使用下划线`_`做函数名开头的函数无效，这种函数不建议做测试之外的用途，并且以后会移除。

## 错误码与错误字符串

错误码用来标记函数的执行情况，通过`SBOT_ERR_CODE SBot_GetErrCode()`函数返回。

错误字符串用人类可读的方式解释错误原因，通过`const char * SBot_GetErrStr()`函数返回。

错误码有以下几种：

| 值                | 意义                                                         |
| ----------------- | ------------------------------------------------------------ |
| `SBOT_OK`         | 成功，或无法得知成功与否                                     |
| `SBOT_CLIENT_ERR` | 客户端错误，包括但不限于：配置错误、目录权限错误、参数错误、网络错误 |
| `SBOT_SERVER_ERR` | 服务端错误，包括但不限于：实现端返回超时、实现端返回格式不正确、实现端报告动作执行失败 |
| `SBOT_UNKNOW_ERR` | 未知错误，通常意味着`SBot`包含一个bug或者`SBot`没有对此错误进行分类 |

错误码和错误字符串仅在函数执行失败或无法明确判断函数执行结果时有效，对于这两种情况，会在函数说明文档（还没写）中具体说明。

## 当前进度

梳理思路中，例子能跑了，但是可能会大改，随时跑路。