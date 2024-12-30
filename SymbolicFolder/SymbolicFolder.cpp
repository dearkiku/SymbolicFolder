// SymbolicFolder.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "SymbolicFolder.h"

// 全局变量:
HINSTANCE hInst;                                // 当前实例				
HWND MhWnd, hButton[4], hStatic[2], hEdit[2], hCheckBox[2];// , hProgressBar;
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_SYMBOLICFOLDER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SYMBOLICFOLDER));

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SYMBOLICFOLDER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MAINICON));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	//HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	//	CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	HWND hWnd = CreateWindowEx(
		WS_EX_TOPMOST,// | WS_EX_CONTEXTHELP, 
		szWindowClass, szTitle,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, 400, 210,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}
	MhWnd = hWnd;
	// 设置窗口为始终置顶
	// SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

// 检查文件是否被占用
bool IsFileInUse(const std::filesystem::path& filePath) {
	HANDLE hFile = CreateFile(
		filePath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		return true;
		/*	if (GetLastError() == ERROR_SHARING_VIOLATION) {
			}*/
	}

	CloseHandle(hFile);
	return false;
}

// 检查文件或文件夹权限
bool HasAccessPermission(const std::filesystem::path& path) {
	std::error_code ec;
	(void)std::filesystem::status(path, ec);
	return !ec; // 如果没有错误代码，表示有权限
}

// 遍历文件夹获取权限和占用状态
bool GetPathsState(const std::filesystem::path& srcDir) {
	// 总文件数
	size_t totalItems = 0;
	// 存储所有的错误信息
	std::vector<std::wstring> errorMessages;
	// 计算文件和文件夹总数，并同时检查文件和文件夹是否被占用或无权限
	for (const auto& entry : std::filesystem::recursive_directory_iterator(srcDir)) {
		const auto& path = entry.path();
		// 检查权限
		if (!HasAccessPermission(path)) {
			errorMessages.push_back(L"无访问权限: " + path.wstring());
			continue;
		}
		// 如果是文件需要额外检查是否占用
		if (entry.is_regular_file() && IsFileInUse(path))
		{
			errorMessages.push_back(L"文件被占用: " + path.wstring());
			continue;
		}
	}
	// 如果有错误信息，统一显示
	if (!errorMessages.empty()) {
		std::wstring errorMessage = L"以下文件无法处理:\n";
		for (const auto& msg : errorMessages) {
			errorMessage += msg + L"\n";
		}
		MessageBox(MhWnd, errorMessage.c_str(), L"错误", MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

// 显示 IFileOperation 错误信息的函数
void ShowErrorMessage(IFileOperation* pfo, HRESULT hr) {
	if (pfo) {
		// 获取错误信息
		IErrorInfo* pErrorInfo;
		if (GetErrorInfo(0, &pErrorInfo) == S_OK) {
			BSTR description;
			if (pErrorInfo->GetDescription(&description) == S_OK) {
				MessageBoxW(NULL, description, L"Error", MB_OK);
				SysFreeString(description);
			}
			pErrorInfo->Release();
		}
	}
}

// 移动文件的函数 IFileOperation 替代 SHFileOperation
HRESULT MoveFileWithDialog(const std::wstring& sourcePath, const std::wstring& targetPath) {
	IFileOperation* pfo = nullptr;
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		// 创建 IFileOperation 实例
		hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
		if (SUCCEEDED(hr)) {
			// 设置操作标志
			//
			//
			// 如果在移动或复制过程中遇到同名文件，会自动重命名新文件，而不是覆盖或提示用户。
			hr = pfo->SetOperationFlags(FOF_NOCONFIRMMKDIR | FOF_RENAMEONCOLLISION);
			if (FAILED(hr)) {
				pfo->Release();
				CoUninitialize();
				return hr;
			}

			// 创建源和目标的 IShellItem 对象
			IShellItem* psiSource = nullptr;
			IShellItem* psiTarget = nullptr;
			hr = SHCreateItemFromParsingName(sourcePath.c_str(), NULL, IID_PPV_ARGS(&psiSource));
			if (SUCCEEDED(hr)) {
				hr = SHCreateItemFromParsingName(targetPath.c_str(), NULL, IID_PPV_ARGS(&psiTarget));
				if (SUCCEEDED(hr)) {
					// 执行文件移动操作
					hr = pfo->MoveItem(psiSource, psiTarget, nullptr, nullptr); // 使用 nullptr 作为新源和新目标
					if (FAILED(hr)) {
						psiTarget->Release();
						psiSource->Release();
						pfo->Release();
						CoUninitialize();
						return hr;
					}
					psiTarget->Release();
				}
				psiSource->Release();
			}

			// 执行操作
			if (SUCCEEDED(hr)) {
				hr = pfo->PerformOperations();
				if (FAILED(hr)) {
					ShowErrorMessage(pfo, hr);  // 如果执行操作失败，则显示错误信息
				}
			}

			// 释放 IFileOperation 接口
			pfo->Release();
		}
		CoUninitialize();
	}

	if (FAILED(hr)) {
		ShowErrorMessage(pfo, hr);  // 如果初始化失败，则显示错误信息
	}
	return hr;
}

// 显示API调用错误信息
void ShowErrorMsg(DWORD errorId) {
	WCHAR errorMsg[512];
	// 获取错误信息
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errorMsg,
		sizeof(errorMsg) / sizeof(WCHAR),
		NULL
	);
	// 将错误码和具体的错误信息一并显示
	WCHAR fullMsg[1024];
	wsprintf(fullMsg, L"创建符号链接失败，错误码: %lu\n错误信息: %s", errorId, errorMsg);
	MessageBox(MhWnd, fullMsg, L"错误", MB_OK | MB_ICONERROR);
}

// 初始化窗口组件
static void InitControls(HWND hWnd) {
	// 创建按钮
	hButton[0] = CreateWindowEx(
		0,
		WC_BUTTON, L"...",// 来源浏览
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		341, 24, 32, 32,
		hWnd, (HMENU)IDC_BUTTON_BROWSE_SOURCE, hInst, NULL);
	hButton[1] = CreateWindowEx(
		0,
		WC_BUTTON, L"...",// 目标浏览
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		341, 77, 32, 32,
		hWnd, (HMENU)IDC_BUTTON_BROWSE_TARGET, hInst, NULL);
	hButton[2] = CreateWindowEx(
		0,
		WC_BUTTON, L"仅链接",
		WS_CHILD | WS_VISIBLE,
		168, 130, 96, 32,
		hWnd, (HMENU)IDC_BUTTON_LINK_CREATE, hInst, NULL);
	hButton[3] = CreateWindowEx(
		0,
		WC_BUTTON, L"移动并链接",
		WS_CHILD | WS_VISIBLE,
		277, 130, 96, 32,
		hWnd, (HMENU)IDC_BUTTON_LINK_MOVE, hInst, NULL);
	// 提示标签
	hStatic[0] = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		WC_STATIC, L"来源目录",// 提示标签
		WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
		16, 24, 80, 32,
		hWnd, (HMENU)IDC_STATIC_SOURCE, hInst, NULL);

	hStatic[1] = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		WC_STATIC, L"目标目录",// 提示标签
		WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
		16, 77, 80, 32,
		hWnd, (HMENU)IDC_STATIC_TARGET, hInst, NULL);
	// 目录输入框
	hEdit[0] = CreateWindowEx(
		WS_EX_STATICEDGE,
		WC_EDIT, L"",// 来源目录输入框

		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		96, 24, 245, 32,
		hWnd, (HMENU)IDC_EDIT_SOURCE, hInst, NULL);

	hEdit[1] = CreateWindowEx(
		WS_EX_STATICEDGE,
		WC_EDIT, L"",// 目标目录输入框
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		96, 77, 245, 32,
		hWnd, (HMENU)IDC_EDIT_TARGET, hInst, NULL);
	// 复选框
	hCheckBox[0] = CreateWindowEx(0,
		WC_BUTTON, L"隐介藏形",
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		16, 130, 80, 32,
		hWnd, (HMENU)IDC_CHECKBOX_HIDE_SOURCE, hInst, NULL);
	hCheckBox[1] = CreateWindowEx(0,
		WC_BUTTON, L"+\\?",
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		103, 130, 60, 32,
		hWnd, (HMENU)IDC_CHECKBOX_PATH_RECTIFY, hInst, NULL);
	// 默认选中 +\?
	SendMessage(hCheckBox[1], BM_SETCHECK, BST_CHECKED, 0);
	//// 进度条
	//hProgressBar = CreateWindowEx(0,
	//	PROGRESS_CLASS, NULL,
	//	WS_CHILD | WS_VISIBLE,
	//	16, 168, 357, 16,
	//	hWnd, (HMENU)IDC_PROCESSBAR, hInst, NULL);
	//SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); // 设置进度条范围
	// SendMessage(hProgressBar, PBM_SETPOS, 50, 0); // 更新进度条
}

// 规范化路径
std::wstring NormalizePath(const std::wstring& wpath) {
	// 将 std::wstring 转换为 std::filesystem::path
	std::filesystem::path path(wpath);

	// 使用 absolute 函数返回规范化后的绝对路径
	std::filesystem::path normalizedPath = std::filesystem::absolute(path);
	//OutputDebugString(L"\n normalizedPath: ");
	//OutputDebugString(normalizedPath.c_str());
	if (normalizedPath.has_root_name() && normalizedPath.filename().empty())
	{
		return L"";
	}
	// 将规范化的路径转换回 std::wstring
	return normalizedPath.wstring();
}

// 获取编辑框中的文本
std::wstring GetEditText(HWND hwndEdit) {
	// 获取编辑框中的文本长度
	int length = GetWindowTextLength(hwndEdit);
	if (length == 0) {
		return std::wstring();
	}

	// 分配缓冲区，额外加1用于存储字符串的结束符'\0'
	std::wstring buffer(length + 1, L'\0');

	// 获取编辑框中的文本
	GetWindowText(hwndEdit, &buffer[0], length + 1);

	// 调整wstring的大小，去掉多余的空字符
	buffer.resize(length);

	return buffer;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH hbrBackground;  // 用于保存背景色的画刷
	switch (message)
	{
	case WM_CREATE:
	{
		hbrBackground = CreateSolidBrush(GetSysColor(COLOR_WINDOW));  // 获取系统背景颜色
		InitControls(hWnd);
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDC_BUTTON_BROWSE_SOURCE:
		case IDC_BUTTON_BROWSE_TARGET:
		{
			// 获取目标编辑框句柄
			HWND hEditTarget = (wmId == IDC_BUTTON_BROWSE_SOURCE) ? hEdit[0] : hEdit[1];

			IFileDialog* pfd = nullptr;
			HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
			// 浏览目录，并且在确定后将目录放入编辑框
			if (SUCCEEDED(hr))
			{
				DWORD dwOptions;
				pfd->GetOptions(&dwOptions);
				pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

				hr = pfd->Show(hWnd);
				if (SUCCEEDED(hr))
				{
					IShellItem* psiResult;
					hr = pfd->GetResult(&psiResult);
					if (SUCCEEDED(hr))
					{
						PWSTR pszPath = nullptr;
						hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
						if (SUCCEEDED(hr))
						{
							// 将选择的路径设置到编辑框
							SetWindowText(hEditTarget, pszPath);
							CoTaskMemFree(pszPath);
						}
						psiResult->Release();
					}
				}
				pfd->Release();
			}
		}
		break;
		case IDC_BUTTON_LINK_CREATE:
		{
			// 创建一个将 {sourcePath} 指向 {targetPath} 的符号链接

			std::wstring sourcePath = NormalizePath(GetEditText(hEdit[0]));
			std::wstring targetPath = NormalizePath(GetEditText(hEdit[1]));

			//OutputDebugString(L"\n targetPath: ");
			//OutputDebugString(sourcePath.c_str());
			//OutputDebugString(L"\n targetPath: ");
			//OutputDebugString(targetPath.c_str());

			if (sourcePath.empty() || targetPath.empty() || sourcePath == targetPath)
			{
				MessageBox(hWnd, L"确保来源目录和目标目录都已填写，且不是相同的路径或根目录。", L"目录错误", MB_OK | MB_ICONERROR);
				break;
			}
			int msgboxRes;
			std::wstring fullMsg;
			// 来源目录检查 不能存在 不能是符号链接
			if (std::filesystem::exists(sourcePath))
			{
				// 目录存在 判断是否是符号链接
				if (std::filesystem::is_symlink(sourcePath))
				{
					fullMsg = L"来源目录: \n" + sourcePath + L"\n已经是符号链接";
					MessageBox(hWnd, fullMsg.c_str(), L"提示", MB_OK | MB_ICONQUESTION);
					break;
				}
				else if (std::filesystem::is_directory(sourcePath))
				{
					// 目录是普通目录
					fullMsg = L"来源目录: \n" + sourcePath + L"\n已存在，无法给已存在于来源目录的文件夹创建符号链接，是否删除？";
					msgboxRes = MessageBox(hWnd, fullMsg.c_str(), L"错误", MB_YESNO | MB_ICONQUESTION);
					// 点击确认执行删除动作
					if (msgboxRes == IDYES)
					{
						try {
							// 删除目录及其内容
							std::filesystem::remove_all(sourcePath);
						}
						catch (const std::filesystem::filesystem_error& e) {
							// 如果删除失败，显示错误消息
							std::string errorMsg = e.what();
							std::wstring wideErrorMsg(errorMsg.begin(), errorMsg.end());
							MessageBox(hWnd, wideErrorMsg.c_str(), L"删除错误", MB_OK | MB_ICONERROR);
							break;
						}
					}
					else
					{
						// 如果不删除则无法继续创建
						break;
					}
				}
				else
				{
					fullMsg = L"来源目录: \n" + sourcePath + L"\n不是一个目录";
					MessageBox(hWnd, fullMsg.c_str(), L"删除错误", MB_OK | MB_ICONERROR);
					break;
				}
			}
			// 如果勾选了目录补全选项
			if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_PATH_RECTIFY) == BST_CHECKED) {
				// 获取 sourcePath 的长度
				size_t sourceLen = sourcePath.size();
				// 如果 sourcePath 以 '\\' 结尾，移除最后的 '\\'
				if (sourcePath[sourceLen - 1] == L'\\') {
					sourcePath.pop_back();  // 移除最后的 '\\'
				}
				// 获取 targetPath 的长度
				size_t targetLen = targetPath.size();
				// 如果 targetPath 不以 '\\' 结尾，添加 '\\'
				if (targetPath[targetLen - 1] != L'\\') {
					targetPath.append(L"\\");
				}
				// 存放最终要拼接的源路径部分（文件夹名）
				std::wstring finalSource;
				// 查找最后一个 '\\' 并提取其后部分（即文件夹名）
				const WCHAR* lastSlash = wcsrchr(sourcePath.c_str(), L'\\');
				if (lastSlash) {
					finalSource = std::wstring(lastSlash + 1);
				}
				// 拼接最终的 sourcePath 部分到 targetPath
				targetPath.append(finalSource);
			}
			// 目标目录检查 必须要存在
			if (!std::filesystem::exists(targetPath))
			{
				fullMsg = L"目标目录\n" + targetPath + L"\n不存在，是否创建？";
				// MessageBox(hWnd, L"目标目录不存在", L"错误", MB_OK | MB_ICONERROR);
				msgboxRes = MessageBox(hWnd, fullMsg.c_str(), L"提示", MB_YESNO | MB_ICONQUESTION);
				if (msgboxRes == IDYES) {
					try {
						std::filesystem::create_directory(targetPath); // 创建目标目录
					}
					catch (const std::filesystem::filesystem_error& e) {
						std::string errorMsg = e.what();
						std::wstring wideErrorMsg(errorMsg.begin(), errorMsg.end());
						MessageBox(hWnd, wideErrorMsg.c_str(), L"错误", MB_OK | MB_ICONERROR);
						break;
					}
				}
				else {
					break;
				}
			}
			else if (!std::filesystem::is_directory(targetPath))
			{
				fullMsg = L"目标目录\n" + targetPath + L"\n不合法！";
				MessageBox(hWnd, fullMsg.c_str(), L"错误", MB_OK | MB_ICONERROR);
				break;
			}
			fullMsg = L"接下来所有访问:\n" + sourcePath + L"\n目录的文件都会被重定向到: \n" + targetPath + L"\n是否继续";
			//OutputDebugString(L"重定向提示文本：\n");
			//OutputDebugString(fullMsg.c_str());
			msgboxRes = MessageBox(MhWnd, fullMsg.c_str(), L"提示", MB_YESNO | MB_ICONQUESTION);
			if (msgboxRes != IDYES)
			{
				break;
			}
			if (CreateSymbolicLink(sourcePath.c_str(), targetPath.c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY))
			{
				// 创建成功则判断是否勾选隐藏来源
				if (IsDlgButtonChecked(MhWnd, IDC_CHECKBOX_HIDE_SOURCE) == BST_CHECKED)
				{
					// 隐藏来源目录
					SetFileAttributes(sourcePath.c_str(), FILE_ATTRIBUTE_HIDDEN);
				}
				MessageBox(MhWnd, L"符号链接创建成功", L"提示", MB_OK | MB_ICONINFORMATION);
				break;
			}
			else
			{
				// 如果创建失败
				ShowErrorMsg(GetLastError());
				break;
			}
		}
		break;
		case IDC_BUTTON_LINK_MOVE:
		{
			// 将 {sourcePath} 移动到 {targetPath} 再创建一个将 {sourcePath} 指向 {targetPath} 的符号链接

			std::wstring sourcePath = NormalizePath(GetEditText(hEdit[0]));
			std::wstring targetPath = NormalizePath(GetEditText(hEdit[1]));

			if (sourcePath.empty() || targetPath.empty() || sourcePath == targetPath)
			{
				MessageBox(hWnd, L"确保来源目录和目标目录都已填写，且不是相同的路径或根目录。", L"目录错误", MB_OK | MB_ICONERROR);
				break;
			}
			int msgboxRes;
			std::wstring fullMsg;
			std::wstring finalTargetPath = targetPath;
			// 如果勾选了目录补全选项
			if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_PATH_RECTIFY) == BST_CHECKED) {
				// 获取 sourcePath 的长度
				size_t sourceLen = sourcePath.size();
				// 如果 sourcePath 以 '\\' 结尾，移除最后的 '\\'
				if (sourcePath[sourceLen - 1] == L'\\') {
					sourcePath.pop_back();  // 移除最后的 '\\'
				}
				// 获取 finalTargetPath 的长度
				size_t targetLen = finalTargetPath.size();
				// 如果 finalTargetPath 不以 '\\' 结尾，添加 '\\'
				if (finalTargetPath[targetLen - 1] != L'\\') {
					finalTargetPath.append(L"\\");
				}
				// 存放最终要拼接的源路径部分（文件夹名）
				std::wstring finalSource;
				// 从 sourcePath 查找最后一个 '\\' 并提取其后部分（即文件夹名）
				const WCHAR* lastSlash = wcsrchr(sourcePath.c_str(), L'\\');
				if (lastSlash) {
					finalSource = std::wstring(lastSlash + 1);
				}
				// 拼接最终的 sourcePath 中 文件夹名 的部分到 finalTargetPath
				finalTargetPath.append(finalSource);
				//if (finalTargetPath.back() != L'\\') {
				//	finalTargetPath += L'\\';
				//}
			}
			// 来源目录检查
			if (std::filesystem::exists(sourcePath))
			{
				// 目录存在 判断是否是符号链接
				if (std::filesystem::is_symlink(sourcePath))
				{
					fullMsg = L"来源目录: \n" + sourcePath + L"\n已经是一个符号链接目录";
					//WCHAR fullMsg[1024];
					//wsprintf(fullMsg, L"来源目录:\n%ls\n", sourcePath);
					MessageBox(hWnd, fullMsg.c_str(), L"提示", MB_OK | MB_ICONQUESTION);
					break;
				}
				else if (!std::filesystem::is_directory(sourcePath))
				{
					// 如果是文件而不是目录，进行错误处理
					fullMsg = L"来源目录: \n" + sourcePath + L"\n不是一个有效的目录";
					//WCHAR fullMsg[1024];
					//wsprintf(fullMsg, L"来源目录:\n%ls\n不是一个有效的目录", sourcePath);
					MessageBox(hWnd, fullMsg.c_str(), L"错误", MB_OK | MB_ICONERROR);
					break;
				}
			}
			// 目标目录检查
			if (std::filesystem::exists(finalTargetPath)) {
				fullMsg = L"目标目录: \n" + finalTargetPath + L"\n已有同名文件夹，移动时文件会被覆盖或移动失败，是否继续？";
				//WCHAR fullMsg[1024];
				//wsprintf(fullMsg, L"目标目录:\n%ls\n已有同名文件夹，移动时文件会被覆盖或移动失败，是否继续？", targetPath);
				msgboxRes = MessageBox(hWnd, fullMsg.c_str(), L"提示", MB_YESNO | MB_ICONQUESTION);
				if (msgboxRes != IDYES)
				{
					break;
				}
			}
			fullMsg = L"是否需要对:\n" + sourcePath + L"\n中的文件&文件夹进行占用&权限检查\n这样做会降低移动效率";
			msgboxRes = MessageBox(MhWnd, fullMsg.c_str(), L"提示", MB_YESNO | MB_ICONQUESTION);
			if (msgboxRes != IDYES)
			{
				if (!GetPathsState(sourcePath))
				{
					break;
				}
			}
			// 拼接提示信息
			fullMsg = L"接下来会迁移文件，并且所有访问:\n" + sourcePath + L"\n都会被重定向到: \n" + finalTargetPath + L"\n是否继续？";
			msgboxRes = MessageBox(MhWnd, fullMsg.c_str(), L"提示", MB_YESNO | MB_ICONQUESTION);
			if (msgboxRes != IDYES)
			{
				break;
			}
			// finalTargetPath 会自动创建同名目录 用 targetPath 避免套娃
			HRESULT result = MoveFileWithDialog(sourcePath, targetPath);
			if (!SUCCEEDED(result)) {

				break;
			}
			if (CreateSymbolicLink(sourcePath.c_str(), finalTargetPath.c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY))
			{
				// 创建成功则判断是否勾选隐藏来源
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_HIDE_SOURCE) == BST_CHECKED)
				{
					// 隐藏来源目录
					SetFileAttributes(sourcePath.c_str(), FILE_ATTRIBUTE_HIDDEN);
				}
				MessageBox(MhWnd, L"符号链接创建成功", L"提示", MB_OK | MB_ICONINFORMATION);
				break;
			}
			else
			{
				// 如果创建失败
				ShowErrorMsg(GetLastError());
				break;
			}
		}
		break;
		//case IDM_EXIT:
		//	DestroyWindow(hWnd);
		//	break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CTLCOLORSTATIC:
	{
		HDC hdcStatic = (HDC)wParam;
		SetBkMode(hdcStatic, TRANSPARENT);  // 透明背景
		SetTextColor(hdcStatic, RGB(0, 0, 0));  // 黑色文本
		return (LRESULT)hbrBackground;  // 设置标签控件的背景色
	}
	break;
	case WM_CTLCOLORBTN:
	{
		HDC hdcButton = (HDC)wParam;
		SetBkMode(hdcButton, TRANSPARENT);  // 透明背景
		SetTextColor(hdcButton, RGB(0, 0, 0));  // 黑色文本
		return (LRESULT)hbrBackground;  // 设置按钮控件的背景色
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}