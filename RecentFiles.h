#ifndef __RECENT_FILES_H
#define __RECENT_FILES_H
#include <Windows.h>
#include <vector>
#include <string>
struct RecentFileInfo {
	std::basic_string<TCHAR> filename;
	std::basic_string<TCHAR> fullpath;
	FILETIME lastWriteTime;
	bool isEmbedded;
};
class RecentFiles {
public:
	static const int MAX_FILES = 5000;
	static void Initialize(HWND parentHwnd, int x, int y, int width, int height);
	static void Shutdown();
	static void RefreshList();
	static void PopulateDialogList(HWND hDlg, int listboxId);
	static void OnDialogFileSelected(HWND hDlg, int listboxId, int index);
private:
	static HWND hListBox;
	static std::vector<RecentFileInfo> files;
	static std::vector<RecentFileInfo> embeddedFiles;
	static bool initialized;
	static void LoadEmbeddedFiles();
	static void ScanDirectory();
	static void SortFiles();
	static void PopulateList();
	static std::basic_string<TCHAR> GetExecutableDirectory();
};
#endif