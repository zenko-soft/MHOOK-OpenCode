#define _CRT_SECURE_NO_WARNINGS
#include "RecentFiles.h"
#include "Settings.h"
#include "resource.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <algorithm>
#include <tchar.h>
static TCHAR selectedFilename[1258];
HWND RecentFiles::hListBox = NULL;
std::vector<RecentFileInfo> RecentFiles::files;
static bool allLoaded = false;
static bool LoadFromResource() {
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_MHOOKDATA), RT_RCDATA);
	if (!hRes) return false;
	HGLOBAL hMem = LoadResource(NULL, hRes);
	if (!hMem) return false;
	DWORD size = SizeofResource(NULL, hRes);
	char* pData = (char*)LockResource(hMem);
	if (!pData || size == 0) return false;
	char* p = pData;
	char* end = pData + size;
	while (p + 8 < end) {
		DWORD nameLen = *(DWORD*)p;
		p += 4;
		if (nameLen > 256 || nameLen < 2) break;
		std::basic_string<TCHAR> name;
		name.assign((TCHAR*)p, nameLen / 2 - 1);
		p += nameLen;
		if (p + 4 > end) break;
		DWORD dataLen = *(DWORD*)p;
		p += 4;
		if (p + dataLen > end) break;
		RecentFileInfo info;
		info.filename = name;
		info.fullpath = name;
		info.lastWriteTime.dwHighDateTime = 0;
		info.lastWriteTime.dwLowDateTime = 0;
		info.isResource = true;
		info.dataLen = dataLen;
		info.dataOffset = p - pData;
		RecentFiles::files.push_back(info);
		p += dataLen;
	}
	UnlockResource(hMem);
	FreeResource(hMem);
	return RecentFiles::files.size() > 0;
}
static WNDPROC oldListBoxProc = NULL;
static LRESULT CALLBACK ListBoxSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_KEYDOWN) {
		int vk = (int)wParam;
		if (vk == VK_NEXT || vk == VK_PRIOR || vk == VK_END || vk == VK_HOME || vk == VK_DOWN || vk == VK_UP) {
			RecentFiles::LoadMoreIfNeeded();
		}
	}
	return CallWindowProc(oldListBoxProc, hwnd, msg, wParam, lParam);
}
void RecentFiles::Initialize(HWND parentHwnd, int x, int y, int width, int height) {
	hListBox = CreateWindow(_T("LISTBOX"), NULL,
		WS_VISIBLE | WS_CHILD | LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
		x, y, width, height,
		parentHwnd, (HMENU)1001,
		GetModuleHandle(NULL), NULL);
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
	oldListBoxProc = (WNDPROC)SetWindowLongPtr(hListBox, GWLP_WNDPROC, (LONG_PTR)ListBoxSubclassProc);
	RefreshList();
}
void RecentFiles::RefreshList() {
	files.clear();
	LoadFromResource();
	ScanDirectory();
	SortByDate();
	PopulateList();
}
void RecentFiles::ScanDirectory() {
	std::basic_string<TCHAR> dir = _T("C:\\Programs\\mhook\\");
	std::basic_string<TCHAR> pattern = dir + _T("*.MHOOK");
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(pattern.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				std::basic_string<TCHAR> fname(findData.cFileName);
				bool exists = false;
				for (auto& f : files) {
					if (f.filename == fname) {
						exists = true;
						break;
					}
				}
				if (!exists) {
					RecentFileInfo info;
					info.filename = findData.cFileName;
					info.fullpath = dir + findData.cFileName;
					info.lastWriteTime = findData.ftLastWriteTime;
					info.isResource = false;
					files.push_back(info);
				}
			}
		} while (FindNextFile(hFind, &findData));
		FindClose(hFind);
	}
}
void RecentFiles::SortByDate() {
	std::vector<RecentFileInfo> resFiles;
	std::vector<RecentFileInfo> diskFiles;
	for (auto& f : files) {
		if (f.isResource) resFiles.push_back(f);
		else diskFiles.push_back(f);
	}
	std::sort(diskFiles.begin(), diskFiles.end(),
		[](const RecentFileInfo& a, const RecentFileInfo& b) {
			return CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) > 0;
		});
	if (diskFiles.size() > 50) {
		std::vector<RecentFileInfo> first50(diskFiles.begin(), diskFiles.begin() + 50);
		std::vector<RecentFileInfo> rest(diskFiles.begin() + 50, diskFiles.end());
		std::sort(rest.begin(), rest.end(),
			[](const RecentFileInfo& a, const RecentFileInfo& b) {
				return _tcsicmp(a.filename.c_str(), b.filename.c_str()) < 0;
			});
		diskFiles = first50;
		diskFiles.insert(diskFiles.end(), rest.begin(), rest.end());
	}
	files.clear();
	files.insert(files.end(), diskFiles.begin(), diskFiles.end());
	files.insert(files.end(), resFiles.begin(), resFiles.end());
}
void RecentFiles::PopulateList() {
	allLoaded = false;
	SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
	int maxShow = 100;
	int count = 0;
	SendMessage(hListBox, LB_INITSTORAGE, (WPARAM)files.size(), (LPARAM)256);
	for (const auto& file : files) {
		if (count >= maxShow) break;
		std::basic_string<TCHAR> displayName = file.filename;
		size_t dotPos = displayName.find_last_of(_T('.'));
		if (dotPos != std::basic_string<TCHAR>::npos) {
			displayName = displayName.substr(0, dotPos);
		}
		SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)displayName.c_str());
		count++;
	}
}
void RecentFiles::LoadMoreIfNeeded() {
	if (allLoaded) return;
	int count = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
	if (count >= (int)files.size()) {
		allLoaded = true;
		return;
	}
	int toLoad = min(100, (int)files.size() - count);
	for (int i = 0; i < toLoad; i++) {
		std::basic_string<TCHAR> displayName = files[count + i].filename;
		size_t dotPos = displayName.find_last_of(_T('.'));
		if (dotPos != std::basic_string<TCHAR>::npos) {
			displayName = displayName.substr(0, dotPos);
		}
		SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)displayName.c_str());
	}
	if (SendMessage(hListBox, LB_GETCOUNT, 0, 0) >= (int)files.size()) {
		allLoaded = true;
	}
}
void RecentFiles::PopulateDialogList(HWND hDlg, int comboId) {
	files.clear();
	LoadFromResource();
	ScanDirectory();
	SortByDate();
	HWND hCombo = GetDlgItem(hDlg, comboId);
	if (!hCombo) return;
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
	SendMessage(hCombo, CB_INITSTORAGE, (WPARAM)files.size(), (LPARAM)256);
	for (const auto& file : files) {
		std::basic_string<TCHAR> displayName = file.filename;
		size_t dotPos = displayName.find_last_of(_T('.'));
		if (dotPos != std::basic_string<TCHAR>::npos) {
			displayName = displayName.substr(0, dotPos);
		}
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)displayName.c_str());
	}
}
void RecentFiles::OnDialogFileSelected(HWND hDlg, int comboId, int index) {
	if (index >= 0 && index < (int)files.size()) {
		std::basic_string<TCHAR> displayName = files[index].filename;
		_tcsncpy_s(selectedFilename, displayName.c_str(), _TRUNCATE);
		if (files[index].isResource) {
			HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_MHOOKDATA), RT_RCDATA);
			if (hRes) {
				HGLOBAL hMem = LoadResource(NULL, hRes);
				if (hMem) {
					char* pData = (char*)LockResource(hMem);
					if (pData) {
						char* fileData = pData + files[index].dataOffset;
						HANDLE hFile = CreateFile(files[index].fullpath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						if (hFile != INVALID_HANDLE_VALUE) {
							DWORD written;
							WriteFile(hFile, fileData, files[index].dataLen, &written, NULL);
							CloseHandle(hFile);
						}
						UnlockResource(hMem);
					}
					FreeResource(hMem);
				}
			}
		}
		SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, 0, (LPARAM)files[index].fullpath.c_str());
	}
}
std::basic_string<TCHAR> RecentFiles::GetExecutableDirectory() {
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	std::basic_string<TCHAR> exePath(path);
	size_t pos = exePath.find_last_of(_T('\\'));
	if (pos != std::basic_string<TCHAR>::npos) {
		exePath = exePath.substr(0, pos + 1);
	}
	return exePath;
}
const TCHAR* RecentFiles::GetSelectedFilename() {
	if (selectedFilename[0]) return selectedFilename;
	return NULL;
}
void RecentFiles::Shutdown() {
	if (hListBox) {
		DestroyWindow(hListBox);
		hListBox = NULL;
	}
}