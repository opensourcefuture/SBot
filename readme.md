# SBot

[TOC]

## 目的

* 提供一个对接 OneBot 的跨平台C语言SDK
* 这是一个以简单为主的框架，可能不会十分高效

## 编译 & 安装

### 编译环境

* CMake 3.15 及以上
* 支持 c++17 的 gcc 或 clang （windows 下可以是 mingw）
* git以及网络支持（所有依赖会自动从github拉取）

### 编译

在 build（自己新建，github传不了空文件夹...）目录下：

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
4. 若无特殊说明，涉及的字符串均为 utf8 编码。
5. 以上约定对于使用下划线`_`做函数名开头的函数无效，这种函数不建议做测试之外的用途，并且以后会移除。

## 错误码与错误字符串

错误码用来标记函数的执行情况，通过`SBOT_ERR_CODE SBot_GetErrCode()`函数返回。

错误字符串用人类可读的方式解释错误原因，通过`const char * SBot_GetErrStr()`函数返回。

错误码有以下几种：

| 值                | 意义                                                         |
| ----------------- | ------------------------------------------------------------ |
| `SBOT_OK`         | 成功                                                         |
| `SBOT_CLIENT_ERR` | 客户端错误，包括但不限于：配置错误、目录权限错误、参数错误、网络错误 |
| `SBOT_SERVER_ERR` | 服务端错误，包括但不限于：实现端返回超时、实现端返回格式不正确、实现端报告动作执行失败 |
| `SBOT_UNKNOW_ERR` | 未知错误，通常意味着`SBot`包含一个bug或者`SBot`没有对此错误进行分类 |

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
	handle：
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
		线程变量 handle。

#### 返回值：
	返回 handle。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	handle

#### 其它说明：
	此函数需要通过错误码来判断是否执行成功。
	
```
</details>

### 向消息链添加文字节点：SBot_UpTextMsg

<details>

```markdown
#### 函数原型：
	SBOT_BOOL_TYPE SBot_UpTextMsg(const char * text_msg)

#### 函数功能：
	向消息链缓存添加文字节点，此函数不会真正将消息发出去。

#### 参数：
	text_msg：
		要添加的文字，使用 utf8 编码。

#### 返回值：
	成功返回 SBOT_TRUE，失败返回 SBOT_FALSE。

#### 线程变量设置：
	消息链缓存

#### 线程变量依赖：
	消息链缓存

#### 其它说明：
	之后使用 SBot_SendPrivateMsg 或 SBot_SendGroupMsg 可以将消息链发出去。
	
```
</details>

### 向消息链添加At节点：SBot_UpAtMsg

<details>

```markdown
#### 函数原型：
	SBOT_BOOL_TYPE SBot_UpAtMsg(const char * target_id)

#### 函数功能：
	向消息链缓存添加At节点，此函数不会真正将消息发出去。

#### 参数：
	target_id：
		要 At 的人的id。

#### 返回值：
	成功返回 SBOT_TRUE，失败返回 SBOT_FALSE。

#### 线程变量设置：
	消息链缓存

#### 线程变量依赖：
	消息链缓存

#### 其它说明：
	之后使用 SBot_SendPrivateMsg 或 SBot_SendGroupMsg 可以将消息链发出去。
	
```
</details>

### 向消息链添加图片节点：SBot_UpImgMsg

<details>

```markdown
#### 函数原型：
	SBOT_BOOL_TYPE SBot_UpImgMsg(const char * file_id)

#### 函数功能：
	向消息链缓存添加图片节点，此函数不会真正将消息发出去。

#### 参数：
	file_id：
		图片的文件id。

#### 返回值：
	成功返回 SBOT_TRUE，失败返回 SBOT_FALSE。

#### 线程变量设置：
	消息链缓存

#### 线程变量依赖：
	消息链缓存

#### 其它说明：
	之后使用 SBot_SendPrivateMsg 或 SBot_SendGroupMsg 可以将消息链发出去。图片的文件id可以通过 SBot_MakeImgFileIdByPath、SBot_MakeImgFileIdByUrl 获取。
	
```
</details>

### 从本地图片得到文件id：SBot_MakeImgFileIdByPath

<details>

```markdown
#### 函数原型：
	const char * SBot_MakeImgFileIdByPath(const char * path_str)

#### 函数功能：
	从本地图片得到文件id。

#### 参数：
	path_str：
		图片位置，绝对路径、相对路径均可，使用 utf8 编码。

#### 返回值：
	成功返回文件id，失败返回 ""。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	handle、self_id

#### 其它说明：
	目前实现中，不会依赖 handle、self_id，但不排除将来依赖的可能性。
	
```
</details>

### 从网络图片得到文件id：SBot_MakeImgFileIdByUrl

<details>

```markdown
#### 函数原型：
	const char * SBot_MakeImgFileIdByUrl(const char * url_str)

#### 函数功能：
	从网络图片得到文件id。支持 http、https 开头的路径。

#### 参数：
	path_str：
		网络位置，使用 utf8 编码。

#### 返回值：
	成功返回文件id，失败返回 ""。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	handle、self_id

#### 其它说明：
	目前实现中，不会依赖 handle、self_id，但不排除将来依赖的可能性。
	
```
</details>

### 发送私聊消息：SBot_SendPrivateMsg

<details>

```markdown
#### 函数原型：
	const char * SBot_SendPrivateMsg()

#### 函数功能：
	将消息链缓存通过私聊消息的形式发送出去，并清空消息链缓存。

#### 参数：
	无。

#### 返回值：
	成功返回 message_id，失败返回 ""。

#### 线程变量设置：
	消息链缓存

#### 线程变量依赖：
	消息链缓存、handle、self_id、user_id

#### 其它说明：
	无论成功与否，都会清空消息链缓存。
	
```
</details>

### 发送群聊消息：SBot_SendGroupMsg

<details>

```markdown
#### 函数原型：
	const char * SBot_SendGroupMsg()

#### 函数功能：
	将消息链缓存通过群聊消息的形式发送出去，并清空消息链缓存。

#### 参数：
	无。

#### 返回值：
	成功返回 message_id，失败返回 ""。

#### 线程变量设置：
	消息链缓存

#### 线程变量依赖：
	消息链缓存、handle、self_id、group_id

#### 其它说明：
	无论成功与否，都会清空消息链缓存。
	
```
</details>

### 撤回消息：SBot_DelMsg

<details>

```markdown
#### 函数原型：
	SBOT_BOOL_TYPE SBot_DelMsg()

#### 函数功能：
	撤回消息。

#### 参数：
	无。

#### 返回值：
	成功返回 SBOT_TRUE，失败返回 SBOT_FALSE。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	handle、self_id、message_id

#### 其它说明：
	无。
	
```
</details>

### 获取登录号信息：SBot_GetLoginInfo

<details>

```markdown
#### 函数原型：
	SBOT_LOGININFO_TYPE * SBot_GetLoginInfo()

#### 函数功能：
	获取登录号信息，包含id和昵称。

#### 参数：
	无。

#### 返回值：
	成功返回非空 SBOT_LOGININFO_TYPE 指针，失败返回空指针。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	handle、self_id

#### 其它说明：
	无。
	
```
</details>

### 获取好友列表：SBot_GetFriendList

<details>

```markdown
#### 函数原型：
	SBOT_FRIENDINFOLIST_TYPE * SBot_GetFriendList()

#### 函数功能：
	获取好友列表，包含id和昵称。

#### 参数：
	无。

#### 返回值：
	成功且至少有一个好友返回非空 SBOT_FRIENDINFOLIST_TYPE 指针，失败或没有好友返回空指针。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	handle、self_id

#### 其它说明：
	此函数需要通过错误码来判断成功与否。SBOT_FRIENDINFOLIST_TYPE 结构体中，next指针指向下一个元素。
	
```
</details>

### 获取群列表：SBot_GetGroupList

<details>

```markdown
#### 函数原型：
	SBOT_GROUPINFOLIST_TYPE * SBot_GetGroupList()

#### 函数功能：
	获取群列表，包含id和群名字。

#### 参数：
	无。

#### 返回值：
	成功且至少有一个群返回非空 SBOT_GROUPINFOLIST_TYPE 指针，失败或没有群返回空指针。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	handle、self_id

#### 其它说明：
	此函数需要通过错误码来判断成功与否。SBOT_GROUPINFOLIST_TYPE 结构体中，next指针指向下一个元素。
	
```
</details>

### 获得message长度：SBot_GetMsgSize

<details>

```markdown
#### 函数原型：
	unsigned int SBot_GetMsgSize()

#### 函数功能：
	获得 message 长度，一般在调用 SBot_GetEvent 函数后使用。

#### 参数：
	无。

#### 返回值：
	返回 message 长度。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	message

#### 其它说明：
	此函数需要通过错误码来判断成功与否。
	
```
</details>

### 查看message中的节点类型：SBot_GetMsgType

<details>

```markdown
#### 函数原型：
	const char * SBot_GetMsgType(unsigned int pos)

#### 函数功能：
	查看message中的节点类型，目前支持 "text"、"at"

#### 参数：
	要获得的节点在 message 中的位置，从 0 开始。

#### 返回值：
	成功返回节点类型，失败返回""，若节点类型未知，返回 "unknow"。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	message

#### 其它说明：
	返回 "unknow" 不算失败。
	
```
</details>

### 获得text节点数据：SBot_GetTextMsg

<details>

```markdown
#### 函数原型：
	const char * SBot_GetTextMsg(unsigned int pos)

#### 函数功能：
	从 message 中获得 text 节点的文字。

#### 参数：
	要获得的节点在 message 中的位置，从 0 开始。

#### 返回值：
	返回 text 节点的文字。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	message

#### 其它说明：
	此函数需要通过错误码来判断成功与否。
	
```
</details>

### 获得at节点数据：SBot_GetAtMsg

<details>

```markdown
#### 函数原型：
	const char * SBot_GetAtMsg(unsigned int pos)

#### 函数功能：
	从 message 中获得 at 节点中被at的人的id。

#### 参数：
	要获得的节点在 message 中的位置，从 0 开始。

#### 返回值：
	成功返回被at的人的 id ，失败返回""。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	message

#### 其它说明：
	无。
	
```
</details>

### 获得错误描述字符串：SBot_GetErrStr

<details>

```markdown
#### 函数原型：
	const char * SBot_GetErrStr()

#### 函数功能：
	获得其它函数执行失败后的错误描述字符串。

#### 参数：
	无。

#### 返回值：
	错误描述字符串，若没有错误，则返回""。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	无。

#### 其它说明：
	此函数本身不会失败，也不会修改错误码、错误描述字符串。
	
```
</details>

### 获得错误码：SBot_GetErrCode

<details>

```markdown
#### 函数原型：
	SBOT_ERR_CODE SBot_GetErrCode()

#### 函数功能：
	获得其它函数执行后的错误码。

#### 参数：
	无。

#### 返回值：
	SBOT_ERR_CODE 类型的错误码

#### 线程变量设置：
	无。

#### 线程变量依赖：
	无。

#### 其它说明：
	此函数本身不会失败，也不会修改错误码、错误描述字符串。此函数可以用来判断其它函数是否执行成功。
	
```
</details>

### 将字符串转化为本地编码：SBot_ToAnsi

<details>

```markdown
#### 函数原型：
	const char * SBot_ToAnsi(const char * utf8_str)

#### 函数功能：
	将 utf8 字符串转换为本地编码。

#### 参数：
	utf8_str：
		utf8 编码的字符串

#### 返回值：
	按本机编码的字符串。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	无。

#### 其它说明：
	此函数需要通过错误码来判断成功与否。
	若返回非""，则一定成功，此函数会跳过无法转换的字符，不视为错误。
	此函数只在windows下做实际转换，在其它操作系统下会直接返回参数中字符串的拷贝，而不做转换，且不视作错误。
	
```
</details>

### 将字符串转化为utf8编码：SBot_ToUtf8

<details>

```markdown
#### 函数原型：
	const char * SBot_ToUtf8(const char * ansi_str);

#### 函数功能：
	将本机编码的字符串转换为utf8编码。

#### 参数：
	ansi_str：
		本机编码的字符串

#### 返回值：
	按utf8编码的字符串。

#### 线程变量设置：
	无。

#### 线程变量依赖：
	无。

#### 其它说明：
	此函数需要通过错误码来判断成功与否。
	若返回非""，则一定成功，此函数会跳过无法转换的字符，不视为错误。
	此函数只在windows下做实际转换，在其它操作系统下会直接返回参数中字符串的拷贝，而不做转换，且不视作错误。
	
```
</details>

## 当前进度

例子能跑了，但是可能会大改。

国庆~~即将~~已经结束，之后更新~~可能~~一定会较缓慢，请谅解。

下面列出已经实现和计划实现的特性：

#### 连接相关
* [x] ws正向连接 
* [ ] ws反向连接 (低优先级)
* [x] 对接OneBotv11 
* [ ] 对接OneBotv12  (等待第一个OneBotv12实现出现)

#### 消息相关
* [x] 收发文本消息
* [x] 收发At消息
* [x] 发送Image消息
* [x] 撤回消息
* [ ] 接收Image消息  (暂无思路)
* [ ] 收发Reply消息  (高优先级)
* [ ] 收发语音消息  (暂无思路)
* [ ] 收发文件消息  (暂无思路)

#### 管理相关
* [x] 获取登录号信息
* [x] 获取好友列表
* [x] 获取群列表
* [ ] 获取群成员列表  (高优先级)
* [ ] 禁言群成员  (高优先级)

#### 平台相关
* [x] gcc (win、linux、android)
* [ ] msvc  (高优先级)

