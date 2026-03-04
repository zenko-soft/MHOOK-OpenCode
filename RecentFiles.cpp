#include "RecentFiles.h"
#include "Settings.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <algorithm>
#include <tchar.h>
HWND RecentFiles::hListBox = NULL;
std::vector<RecentFileInfo> RecentFiles::files;
// Для главного окна (создает отдельный ListBox)
void RecentFiles::Initialize(HWND parentHwnd, int x, int y, int width, int height) {
	hListBox = CreateWindow(_T("LISTBOX"), NULL,
		WS_VISIBLE | WS_CHILD | LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
		x, y, width, height,
		parentHwnd, (HMENU)1001,
		GetModuleHandle(NULL), NULL);
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
	RefreshList();
}
void RecentFiles::RefreshList() {
	files.clear();
	ScanDirectory();
	SortByDate();
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
				RecentFileInfo info;
				info.filename = findData.cFileName;
				info.fullpath = dir + findData.cFileName;
				info.lastWriteTime = findData.ftLastWriteTime;
				files.push_back(info);
			}
		} while (FindNextFile(hFind, &findData));
		FindClose(hFind);
	}
}
void RecentFiles::SortByDate() {
	std::sort(files.begin(), files.end(),
		[](const RecentFileInfo& a, const RecentFileInfo& b) {
			return CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) > 0;
		});
	if (files.size() > MAX_FILES) {
		files.resize(MAX_FILES);
	}
}
void RecentFiles::PopulateList() {
	SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
	for (const auto& file : files) {
		std::basic_string<TCHAR> displayName = file.filename;
		size_t dotPos = displayName.find_last_of(_T('.'));
		if (dotPos != std::basic_string<TCHAR>::npos) {
			displayName = displayName.substr(0, dotPos);
		}
		SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)displayName.c_str());
	}
}
// Для диалога (заполняет существующий ComboBox)
void RecentFiles::PopulateDialogList(HWND hDlg, int comboId) {
	files.clear();
	ScanDirectory();
	SortByDate();
	HWND hCombo = GetDlgItem(hDlg, comboId);
	if (!hCombo) return;
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
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
		if (GetFileAttributes(files[index].fullpath.c_str()) != INVALID_FILE_ATTRIBUTES) {
			MHSettings::OpenMHookConfig(hDlg, (TCHAR*)files[index].fullpath.c_str());
			// Обновляем диалог после загрузки
			MHSettings::AfterLoad(hDlg);
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
}