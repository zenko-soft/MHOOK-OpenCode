#include "RecentFiles.h"
#include "Settings.h"
#include "resource.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <algorithm>
#include <tchar.h>
HWND RecentFiles::hListBox = NULL;
std::vector<RecentFileInfo> RecentFiles::files;
std::vector<RecentFileInfo> RecentFiles::embeddedFiles;
bool RecentFiles::initialized = false;
void RecentFiles::Initialize(HWND parentHwnd, int x, int y, int width, int height) {
	hListBox = CreateWindow(_T("LISTBOX"), NULL,
		WS_VISIBLE | WS_CHILD | LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
		x, y, width, height,
		parentHwnd, (HMENU)1001,
		GetModuleHandle(NULL), NULL);
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
	LoadEmbeddedFiles();
	RefreshList();
}
void RecentFiles::LoadEmbeddedFiles() {
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_EMBEDDEDSETTINGS), RT_RCDATA);
	if (!hRes) return;
	HGLOBAL hData = LoadResource(NULL, hRes);
	if (!hData) return;
	DWORD size = SizeofResource(NULL, hRes);
	const BYTE* pData = (const BYTE*)LockResource(hData);
	if (!pData || size < 4) return;
	DWORD numFiles = *(DWORD*)pData;
	const BYTE* ptr = pData + 4;
	embeddedFiles.clear();
	for (DWORD i = 0; i < numFiles; i++) {
		if (ptr + 4 > pData + size) break;
		DWORD nameLen = *(DWORD*)ptr;
		ptr += 4;
		if (ptr + nameLen * 2 > pData + size) break;
		std::basic_string<TCHAR> name;
		name.resize(nameLen);
		memcpy(&name[0], ptr, nameLen * 2);
		ptr += nameLen * 2;
		if (ptr + 8 > pData + size) break;
		ULONGLONG filetime = *(ULONGLONG*)ptr;
		ptr += 8;
		RecentFileInfo info;
		info.filename = name;
		info.fullpath = name;
		info.lastWriteTime.dwHighDateTime = (DWORD)(filetime >> 32);
		info.lastWriteTime.dwLowDateTime = (DWORD)(filetime & 0xFFFFFFFF);
		info.isEmbedded = true;
		embeddedFiles.push_back(info);
	}
	initialized = true;
}
void RecentFiles::RefreshList() {
	files.clear();
	files = embeddedFiles;
	ScanDirectory();
	SortFiles();
	PopulateList();
}
void RecentFiles::ScanDirectory() {
	std::basic_string<TCHAR> dir = GetExecutableDirectory();
	std::basic_string<TCHAR> pattern = dir + _T("*.mhook");
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(pattern.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				std::basic_string<TCHAR> fname = findData.cFileName;
				bool found = false;
				for (const auto& ef : embeddedFiles) {
					if (fname == ef.filename) {
						found = true;
						break;
					}
				}
				if (!found) {
					RecentFileInfo info;
					info.filename = findData.cFileName;
					info.fullpath = dir + findData.cFileName;
					info.lastWriteTime = findData.ftLastWriteTime;
					info.isEmbedded = false;
					files.push_back(info);
				}
			}
		} while (FindNextFile(hFind, &findData));
		FindClose(hFind);
	}
}
void RecentFiles::SortFiles() {
	std::sort(files.begin(), files.end(),
		[](const RecentFileInfo& a, const RecentFileInfo& b) {
			int cmp = CompareFileTime(&a.lastWriteTime, &b.lastWriteTime);
			if (cmp != 0) return cmp > 0;
			return a.filename < b.filename;
		});
	if (files.size() > MAX_FILES) {
		files.resize(MAX_FILES);
	}
}
void RecentFiles::PopulateList() {
	SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);
	SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
	SendMessage(hListBox, LB_INITSTORAGE, files.size(), files.size() * 64);
	for (size_t i = 0; i < files.size(); i++) {
		std::basic_string<TCHAR> displayName = files[i].filename;
		size_t dotPos = displayName.find_last_of(_T('.'));
		if (dotPos != std::basic_string<TCHAR>::npos) {
			displayName = displayName.substr(0, dotPos);
		}
		LRESULT idx = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)displayName.c_str());
		SendMessage(hListBox, LB_SETITEMDATA, idx, i);
	}
	SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
}
void RecentFiles::PopulateDialogList(HWND hDlg, int comboId) {
	LoadEmbeddedFiles();
	files.clear();
	files = embeddedFiles;
	ScanDirectory();
	SortFiles();
	HWND hCombo = GetDlgItem(hDlg, comboId);
	if (!hCombo) return;
	SendMessage(hCombo, WM_SETREDRAW, FALSE, 0);
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
	SendMessage(hCombo, CB_INITSTORAGE, files.size(), files.size() * 64);
	for (size_t i = 0; i < files.size(); i++) {
		std::basic_string<TCHAR> displayName = files[i].filename;
		size_t dotPos = displayName.find_last_of(_T('.'));
		if (dotPos != std::basic_string<TCHAR>::npos) {
			displayName = displayName.substr(0, dotPos);
		}
		LRESULT idx = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)displayName.c_str());
		SendMessage(hCombo, CB_SETITEMDATA, idx, i);
	}
	SendMessage(hCombo, WM_SETREDRAW, TRUE, 0);
}
void RecentFiles::OnDialogFileSelected(HWND hDlg, int comboId, int index) {
	if (index >= 0 && index < (int)files.size()) {
		const RecentFileInfo& file = files[index];
		if (file.isEmbedded) {
			HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_EMBEDDEDSETTINGS), RT_RCDATA);
			if (hRes) {
				HGLOBAL hData = LoadResource(NULL, hRes);
				if (hData) {
					MHSettings::OpenMHookConfig(hDlg, (TCHAR*)file.fullpath.c_str());
					MHSettings::AfterLoad(hDlg);
				}
			}
		} else {
			if (GetFileAttributes(file.fullpath.c_str()) != INVALID_FILE_ATTRIBUTES) {
				MHSettings::OpenMHookConfig(hDlg, (TCHAR*)file.fullpath.c_str());
				MHSettings::AfterLoad(hDlg);
			}
		}
	}
}
std::basic_string<TCHAR> RecentFiles::GetExecutableDirectory() {
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	PathRemoveFileSpec(path);
	PathAddBackslash(path);
	return std::basic_string<TCHAR>(path);
}
void RecentFiles::Shutdown() {
	if (hListBox) {
		DestroyWindow(hListBox);
		hListBox = NULL;
	}
	files.clear();
	embeddedFiles.clear();
	initialized = false;
}