#ifndef __RECENT_FILES_H
#define __RECENT_FILES_H
#include <Windows.h>
#include <vector>
#include <string>
struct RecentFileInfo {
	std::basic_string<TCHAR> filename;
	std::basic_string<TCHAR> fullpath;
	FILETIME lastWriteTime;
	bool isResource;
	DWORD dataLen;
	DWORD dataOffset;
};
class RecentFiles {
public:
	static const int MAX_FILES = 50;
	static std::vector<RecentFileInfo> files;
	static void Initialize(HWND parentHwnd, int x, int y, int width, int height);
	static void Shutdown();
	static void RefreshList();
	static void PopulateDialogList(HWND hDlg, int comboId);
	static void OnDialogFileSelected(HWND hDlg, int comboId, int index);
	static const TCHAR* GetSelectedFilename();
	static void LoadMoreIfNeeded();
private:
	static HWND hListBox;
	static void ScanDirectory();
	static void SortByDate();
	static void PopulateList();
	static std::basic_string<TCHAR> GetExecutableDirectory();
};
#endif