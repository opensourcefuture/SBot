# SBot

[TOC]

## 目的

* 提供一个对接`OneBot`的跨平台C语言SDK
* 这是一个以简单为主的框架，可能不会十分高效

## 编译 & 安装

### 编译环境

* `CMake 3.15`及以上
* 支持`c++17`的`gcc`或`clang`（`windows` 下可以是`mingw`）
* `git`以及网络支持（所有依赖会自动从`github`拉取）

### 编译

在`build`（自己新建，github传不了空文件夹...）目录下：

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

一个 ping/pong 插件：私聊收到`ping`，回复`pong`；收到`pong`，回复`ping`。

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
更详细的示例参见 test
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

## 函数说明

### 连接 OneBot 实现：SBot_Connect

<details>

```markdown
#### 函数原型：
	SBOT_HANDLE_TYPE SBot_Connect(const char *cfg_json_str)

#### 函数功能：
	用于连接 OneBot 实现。

#### 参数：
	cfg_json_str：
		连接 OneBot所需的参数，必须为 json 格式的字符串，字符串的具体内容见示例。

#### 返回值：
	若成功，返回连接句柄，若失败，返回 SBOT_HANDLE_NULL。

#### 线程变量设置：
	handle
	
#### 线程变量依赖：
	无。

#### 其它说明：
	建议 cfg_json_str 从配置文件读取。
	
```
</details>

### 释放连接句柄：SBot_DelHandle

<details>

```markdown
#### 函数原型：
	void SBot_DelHandle(SBOT_HANDLE_TYPE handle)

#### 函数功能：
	用于释放连接句柄。

#### 参数：
	handle
		连接句柄。

#### 返回值：
	无，且一定成功。

#### 线程变量设置：
	handle

#### 线程变量依赖：
	无。

#### 其它说明：
	必须调用这个函数来释放。
	
```
</details>

### 判断是否连接成功：SBot_IsConnect

<details>

```markdown
#### 函数原型：
	SBOT_BOOL_TYPE SBot_IsConnect()

#### 函数功能：
	用于释放连接句柄。

#### 参数：
	无。

#### 返回值：
	成功连接返回 SBOT_TRUE，没有连接返回 SBOT_FALSE。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	handle

#### 其它说明：
	无。
	
```
</details>

### 获取事件：SBot_GetEvent

<details>

```markdown
#### 函数原型：
	const char * SBot_GetEvent()

#### 函数功能：
	获取事件。

#### 参数：
	无。

#### 返回值：
	失败返回 ""，成功返回非 ""。

#### 线程变量设置：
	user_id、target_id、group_id、nickname、self_id、time、post_type、<post_type>_type、message_id、sub_type、message。

#### 线程变量依赖：
	handle

#### 其它说明：
	无
	
```
</details>

### 获取线程变量：SBot_GetEvtValue

<details>

```markdown
#### 函数原型：
	const char * SBot_GetEvtValue(const char *key)

#### 函数功能：
	获取 SBot_GetEvent 函数设置的部分线程变量：user_id、target_id、group_id、nickname、self_id、post_type、<post_type>_type、message_id、sub_type。

#### 参数：
	key：
		要获取的线程变量的名字。

#### 返回值：
	返回线程变量的值，恒为字符串。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	可以被此函数获取的线程变量。

#### 其它说明：
	此函数需要通过错误码来判断是否执行成功。
	
```
</details>

### 设置线程变量：SBot_SetEvtValue

<details>

```markdown
#### 函数原型：
	SBOT_BOOL_TYPE SBot_SetEvtValue(const char *key, const char *value)

#### 函数功能：
	重新设置 SBot_GetEvent 函数设置的部分线程变量：user_id、target_id、group_id、nickname、self_id、post_type、<post_type>_type、message_id、sub_type。

#### 参数：
	key：
		要设置的线程变量的名字。
	value:
		要设置的值。

#### 返回值：
	成功返回 SBOT_TRUE，失败返回 SBOT_FALSE。

#### 线程变量设置：
	这个函数可设置的线程变量

#### 线程变量依赖：
	无。

#### 其它说明：
	无
	
```
</details>

### 设置线程变量handle：SBot_SetHandle

<details>

```markdown
#### 函数原型：
	SBOT_BOOL_TYPE SBot_SetHandle(SBOT_HANDLE_TYPE handle)

#### 函数功能：
	设置线程变量 handle

#### 参数：
	handle：
		线程变量 handle

#### 返回值：
	成功返回 SBOT_TRUE，失败返回 SBOT_FALSE。

#### 线程变量设置：
	handle

#### 线程变量依赖：
	无。

#### 其它说明：
	无
	
```
</details>

### 获取线程变量handle：SBot_GetHandle

<details>

```markdown
#### 函数原型：
	SBOT_BOOL_TYPE SBot_SetHandle()

#### 函数功能：
	获取线程变量 handle

#### 参数：
	handle：
		线程变量 handle

#### 返回值：
	返回 handle

#### 线程变量设置：
	无。

#### 线程变量依赖：
	handle

#### 其它说明：
	此函数需要通过错误码来判断是否执行成功。
	
```
</details>

## 当前进度

梳理思路（还没啥想法...）中，例子能跑了，但是可能会大改。国庆即将结束，之后更新可能会较为缓慢，请谅解。