// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <SchedulerNode.h>

/// @brief 隐藏控制台启动
#define	HideConsole	0
#if HideConsole
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif // !HideConsole

#ifndef _WIN32_WINNT
#define UNREFERENCED_PARAMETER(P)	{ (P) = (P); }
#endif // !_WIN32_WINNT

int main(int argc, char* argv[])
{
    if (CSchedulerNode::Instance()->Initialize())
    {
        CSchedulerNode::Instance()->Run();
    }
}
