#pragma once
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <filesystem>

#include "resource.h"

#define MAX_LOADSTRING 100
// 浏览来源目录
#define IDC_BUTTON_BROWSE_SOURCE		1001
// 浏览目标目录
#define IDC_BUTTON_BROWSE_TARGET		1002
// 链接
#define IDC_BUTTON_LINK_CREATE			1003
// 移动并链接
#define IDC_BUTTON_LINK_MOVE			1004
// 隐藏来源复选框
#define IDC_CHECKBOX_HIDE_SOURCE		2001
// 路径纠正
#define IDC_CHECKBOX_PATH_RECTIFY		2002
// 来源目录框
#define IDC_EDIT_SOURCE					3001
// 目标目录框
#define IDC_EDIT_TARGET					3002
// 来源目录提示
#define IDC_STATIC_SOURCE				4001
// 目标目录提示
#define IDC_STATIC_TARGET				4002
// 进度提示
#define IDC_PROCESSBAR					5001