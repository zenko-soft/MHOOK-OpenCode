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
	DWORD loaded = 0;
	for (DWORD i = 0; i < numFiles && loaded < 50; i++) {
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
		loaded++;
	}
	initialized = true;
}
std::basic_string<TCHAR> RecentFiles::GetExecutableDirectory() {
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	PathRemoveFileSpec(path);
	PathAddBackslash(path);
	return std::basic_string<TCHAR>(path);
}
void RecentFiles::PopulateDialogList(HWND hDlg, int comboId) {
	LoadEmbeddedFiles();
	files.clear();
	files = embeddedFiles;
	std::sort(files.begin(), files.end(),
		[](const RecentFileInfo& a, const RecentFileInfo& b) {
			return CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) > 0;
		});
	HWND hCombo = GetDlgItem(hDlg, comboId);
	if (!hCombo) return;
	SendMessage(hCombo, WM_SETREDRAW, FALSE, 0);
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
	size_t count = files.size() > 50 ? 50 : files.size();
	SendMessage(hCombo, CB_INITSTORAGE, count, count * 64);
	for (size_t i = 0; i < count; i++) {
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
			std::basic_string<TCHAR> tryPath = file.fullpath;
			std::basic_string<TCHAR> dir = GetExecutableDirectory();
			bool fileFound = false;
			size_t dotPos = tryPath.find_last_of(_T('.'));
			if (dotPos != std::basic_string<TCHAR>::npos) {
				std::basic_string<TCHAR> baseName = tryPath.substr(0, dotPos);
				std::basic_string<TCHAR> searchPattern = dir + _T("*.mhook");
				WIN32_FIND_DATA fd;
				HANDLE hFind = FindFirstFile(searchPattern.c_str(), &fd);
				if (hFind != INVALID_HANDLE_VALUE) {
					do {
						std::basic_string<TCHAR> fname = fd.cFileName;
						if (fname.find(baseName) == 0) {
							std::basic_string<TCHAR> fullPath = dir + fname;
							if (GetFileAttributes(fullPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
								MHSettings::OpenMHookConfig(hDlg, (TCHAR*)fullPath.c_str());
								MHSettings::AfterLoad(hDlg);
								fileFound = true;
								break;
							}
						}
					} while (FindNextFile(hFind, &fd));
					FindClose(hFind);
				}
			}
		}
	}
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