#pragma once
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <filesystem>

#include "resource.h"

#define MAX_LOADSTRING 100
// �����ԴĿ¼
#define IDC_BUTTON_BROWSE_SOURCE		1001
// ���Ŀ��Ŀ¼
#define IDC_BUTTON_BROWSE_TARGET		1002
// ����
#define IDC_BUTTON_LINK_CREATE			1003
// �ƶ�������
#define IDC_BUTTON_LINK_MOVE			1004
// ������Դ��ѡ��
#define IDC_CHECKBOX_HIDE_SOURCE		2001
// ·������
#define IDC_CHECKBOX_PATH_RECTIFY		2002
// ��ԴĿ¼��
#define IDC_EDIT_SOURCE					3001
// Ŀ��Ŀ¼��
#define IDC_EDIT_TARGET					3002
// ��ԴĿ¼��ʾ
#define IDC_STATIC_SOURCE				4001
// Ŀ��Ŀ¼��ʾ
#define IDC_STATIC_TARGET				4002
// ������ʾ
#define IDC_PROCESSBAR					5001