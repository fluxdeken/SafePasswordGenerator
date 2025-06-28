#ifndef UNICODE
#define UNICODE
#endif

#define NOMINMAX

#define WM_SUBMIT_REQUEST (WM_USER + 1)

#include <windows.h>
#include <shobjidl.h>
#include <openssl/rand.h>
#include <limits>
#include <string>
#include <vector>

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HWND hwndMain = nullptr;
HWND hwndPassEditBox = nullptr;
HWND hwndCheck1 = nullptr;
HWND hwndCheck2 = nullptr;
HWND hwndCheck3 = nullptr;
HWND hwndCheck4 = nullptr;
HWND hwndCheck5 = nullptr;
HWND hwndLengthLabel = nullptr;
HWND hwndLengthEdit = nullptr;
HWND hwndBtnCreate = nullptr;

HHOOK hhkLowLevelKybd;

HFONT hFont = 0;

LRESULT __stdcall KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void SetHook();
void ReleaseHook();

int Submit();
bool isSubmitInProgress = false;

uint32_t get_secure_random_uint32();
int get_secure_random_in_range(int x, int y);
void getPass(std::string alph, char* buff, int sz);

void dLog(const char* format, ...);
bool CopyToClipboardW(HWND hwnd, const std::wstring& text);

int passState = -1;
std::vector<std::wstring> passArr;

int __stdcall wWinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	PWSTR pCmdLine, 
	int nCmdShow) {

	const wchar_t CLASS_NAME[] = L"PasswordGenerator";

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.style = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = nullptr;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = 0;
	wc.hIconSm = nullptr;

	RegisterClassEx(&wc);

	hwndMain = CreateWindowExW(
		0, CLASS_NAME, L"Password Generator",
		WS_OVERLAPPED | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, 500, 370,
		nullptr, nullptr, hInstance, nullptr
	);

	hwndPassEditBox = CreateWindowExW(
		0, WC_EDIT, L"",
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
		ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
		20, 20, 450, 130,
		hwndMain, nullptr, nullptr, nullptr
	);

	int checkX = 20, checkY = 160, checkW = 150, checkH = 25, checkGap = 30;

	hwndCheck1 = CreateWindowExW(0, WC_BUTTON, L"ABC", WS_CHILD | WS_VISIBLE | BS_CHECKBOX, checkX, checkY + 0 * checkGap, checkW, checkH, hwndMain, nullptr, nullptr, nullptr);
	hwndCheck2 = CreateWindowExW(0, WC_BUTTON, L"abc", WS_CHILD | WS_VISIBLE | BS_CHECKBOX, checkX, checkY + 1 * checkGap, checkW, checkH, hwndMain, nullptr, nullptr, nullptr);
	hwndCheck3 = CreateWindowExW(0, WC_BUTTON, L"123", WS_CHILD | WS_VISIBLE | BS_CHECKBOX, checkX, checkY + 2 * checkGap, checkW, checkH, hwndMain, nullptr, nullptr, nullptr);
	hwndCheck4 = CreateWindowExW(0, WC_BUTTON, L"![\\<", WS_CHILD | WS_VISIBLE | BS_CHECKBOX, checkX, checkY + 3 * checkGap, checkW, checkH, hwndMain, nullptr, nullptr, nullptr);
	hwndCheck5 = CreateWindowExW(0, WC_BUTTON, L"' '", WS_CHILD | WS_VISIBLE | BS_CHECKBOX, checkX, checkY + 4 * checkGap, checkW, checkH, hwndMain, nullptr, nullptr, nullptr);

	hwndLengthLabel = CreateWindowExW(
		0, WC_STATIC, L"Length:",
		WS_CHILD | WS_VISIBLE,
		250, 160, 60, 25,
		hwndMain, nullptr, nullptr, nullptr
	);

	hwndLengthEdit = CreateWindowExW(
		0, WC_EDIT, L"",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		320, 160, 60, 25,
		hwndMain, nullptr, nullptr, nullptr
	);

	hwndBtnCreate = CreateWindowExW(
		0, WC_BUTTON, L"Create",
		WS_CHILD | WS_VISIBLE,
		250, 200, 130, 35,
		hwndMain, nullptr, nullptr, nullptr
	);

	hFont = CreateFontW(18, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Arial");


	SendMessage(hwndCheck1, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hwndCheck2, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hwndCheck3, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hwndCheck4, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hwndCheck5, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hwndLengthLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hwndLengthEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hwndBtnCreate, WM_SETFONT, (WPARAM)hFont, TRUE);

	if (hwndMain == NULL)
		return 0;

	ShowWindow(hwndMain, nCmdShow);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT __stdcall WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND: {
		if (HIWORD(wParam) == BN_CLICKED) {
			if (reinterpret_cast<HWND>(lParam) == hwndBtnCreate) {
				Submit();
			}
			else if (reinterpret_cast<HWND>(lParam) == hwndCheck1 ||
				reinterpret_cast<HWND>(lParam) == hwndCheck2 ||
				reinterpret_cast<HWND>(lParam) == hwndCheck3 ||
				reinterpret_cast<HWND>(lParam) == hwndCheck4 ||
				reinterpret_cast<HWND>(lParam) == hwndCheck5) {
				HWND currCheck = reinterpret_cast<HWND>(lParam);
				int state = SendMessageW(currCheck, BM_GETSTATE, 0, 0);
				if (state & BST_CHECKED) {
					SendMessageW(currCheck, BM_SETCHECK, BST_UNCHECKED, 0);
				}
				else {
					SendMessageW(currCheck, BM_SETCHECK, BST_CHECKED, 0);
				}
			}
		}
	}break;

	case WM_CTLCOLORSTATIC:
	{
		HDC hdc = (HDC)wParam;
		SetBkMode(hdc, TRANSPARENT);
		return (LRESULT)(HBRUSH)(COLOR_WINDOW + 1);
	}

	case WM_SUBMIT_REQUEST:
		Submit();
		break;

	case WM_CLOSE: {
		DestroyWindow(hwndMain);
	}break;
	case WM_CREATE: {
		SetHook();
	}break;

	case WM_DESTROY: {
		DeleteObject(hFont);
		ReleaseHook();
		PostQuitMessage(0);
	}break;

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void dLog(const char* format, ...) {
#ifdef _DEBUG
	char buffer[512];
	char* args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	OutputDebugStringA(buffer);
#endif
}

uint32_t get_secure_random_uint32() {
	uint32_t value;
	RAND_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));
	return value;
}

int get_secure_random_in_range(int x, int y) {
	if (x >= y) return 0;

	uint32_t range = y - x;
	uint32_t max_value = std::numeric_limits<uint32_t>::max();
	uint32_t limit = max_value - (max_value % range);

	uint32_t r;
	do {
		r = get_secure_random_uint32();
	} while (r >= limit);

	return x + (r % range);
}

void getPass(std::string alph, char* buff, int sz) {
	for (int i = 0; i < sz; ++i) {
		int random = get_secure_random_in_range(0, alph.length());
		buff[i] = alph[random];
	}
	buff[sz-1] = 0;
}

void SetHook() {
	hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
}
void ReleaseHook() {
	UnhookWindowsHookEx(hhkLowLevelKybd);
}

int Submit() {
	std::string alph1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::string alph2 = "abcdefghijklmnopqrstuvwxyz";
	std::string alph3 = "0123456789";
	std::string alph4 = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
	std::string alph5 = " ";

	std::string fullAlph = "";
	int length = -1;


	if (SendMessageW(hwndCheck1, BM_GETSTATE, 0, 0) == BST_CHECKED) {
		fullAlph += alph1;
	}
	if (SendMessageW(hwndCheck2, BM_GETSTATE, 0, 0) == BST_CHECKED) {
		fullAlph += alph2;
	}
	if (SendMessageW(hwndCheck3, BM_GETSTATE, 0, 0) == BST_CHECKED) {
		fullAlph += alph3;
	}
	if (SendMessageW(hwndCheck4, BM_GETSTATE, 0, 0) == BST_CHECKED) {
		fullAlph += alph4;
	}
	if (SendMessageW(hwndCheck5, BM_GETSTATE, 0, 0) == BST_CHECKED) {
		fullAlph += alph5;
	}

	WCHAR lengthBuff[10];
	GetWindowTextW(hwndLengthEdit, lengthBuff, 10);

	try {
		length = std::stoi(lengthBuff);
	}
	catch (...) {
		MessageBoxW(hwndMain, L"Error converting length", L"Error", MB_OK);
		return 0;
	}


	if (length < 1) {
		MessageBoxW(hwndMain, L"Password length error.", L"Error", MB_OK);
		return 0;
	}
	else if (fullAlph == "") {
		MessageBoxW(hwndMain, L"Symbols for password aren't chosen.", L"Error", MB_OK);
		return 0;
	}

	if (length > 4096) length = 4096;

	++length;

	char* password = new char[length];
	wchar_t* wPassword = new wchar_t[length];

	try {
		getPass(fullAlph, password, length);
		size_t converted = 0;
		mbstowcs_s(&converted, wPassword, length, password, length);
		SetWindowTextW(hwndPassEditBox, wPassword);

		std::wstring wStr(wPassword);

		passArr.push_back(wStr);
		passState = passArr.size() - 1;

		CopyToClipboardW(hwndMain, wStr);
	}
	catch (...) {
		delete[] password;
		delete[] wPassword;
		MessageBoxW(hwndMain, L"Error creating a password.", L"Error", MB_OK);
		return 0;
	}

	delete[] password;
	delete[] wPassword;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

		if (GetForegroundWindow() == hwndMain && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
			bool ctrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

			switch (pKeyboard->vkCode) {
				case 'Z':
					if (ctrlPressed) {
						if (passState > 0) {
							--passState;
							SetWindowTextW(hwndPassEditBox, passArr[passState].data());
							CopyToClipboardW(hwndMain, passArr[passState]);
						}
						return 1;
					}
					break;
				case 'X':
					if (ctrlPressed) {
						if (passState > -1 && passState < passArr.size() - 1) {
							++passState;
							SetWindowTextW(hwndPassEditBox, passArr[passState].data());
							CopyToClipboardW(hwndMain, passArr[passState]);
						}
						return 1;
					}
					break;
				case VK_RETURN:
					PostMessage(hwndMain, WM_SUBMIT_REQUEST, 0, 0);
					return 1;
			}
		}
	}

	return CallNextHookEx(hhkLowLevelKybd, nCode, wParam, lParam);
}

bool CopyToClipboardW(HWND hwnd, const std::wstring& text)
{
	if (text.empty())
		return false;

	if (!OpenClipboard(hwnd))
		return false;

	EmptyClipboard();

	size_t length = text.length();

	size_t sizeInBytes = (length + 1) * sizeof(wchar_t);
	HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, sizeInBytes);
	if (!hGlob) {
		CloseClipboard();
		return false;
	}

	void* pMem = GlobalLock(hGlob);
	memcpy(pMem, text.c_str(), length * sizeof(wchar_t));
	((wchar_t*)pMem)[length] = L'\0';
	GlobalUnlock(hGlob);

	SetClipboardData(CF_UNICODETEXT, hGlob);

	CloseClipboard();
	return true;
}