#include <windows.h>
//#include <stdio.h>
//#include <tchar.h>
#include <psapi.h>
#include <Memoryapi.h>
#include <iostream>
#include <tlhelp32.h>
#include <string>
//#include <string>

extern "C"
{
#include "lua51/include/lua.h"
#include "lua51/include/lauxlib.h"
#include "lua51/include/lualib.h"
}

#ifdef _WIN32
#pragma comment(lib, "lua51/lua5.1.lib")
#endif
#define lua_pushfuncsettable(L,n,f) (lua_pushstring(L, n), lua_pushcfunction(L, f),lua_settable(L, -3))
HANDLE ProcHandle;
DWORD oldprotect;

std::string GetProcessName(DWORD processID)
{
	CHAR szProcessName[MAX_PATH] = "<unknown>";
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE, processID);
	if (!hProcess)
	{
		//cout << "Could not open Process:(" << processID << ") at GetProcessName OpenProcess error code: " << GetLastError() << endl;
	}

	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),&cbNeeded))
		{
			if (GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(CHAR)) == 0)
			{
				std::cout << "Error at GetModuleBaseName: " << GetLastError() << std::endl;
			}
			
			CloseHandle(hProcess);
			std::string str(szProcessName);
			return str;
		}
		else {
			std::cout << "Error at EnumProcessModules: " << GetLastError() << std::endl;
		}
		CloseHandle(hProcess);
	}
	
	std::string strr("hi");
	return strr;
}

int Lua_OpenProcess(lua_State *L)
{
	DWORDLONG pid = lua_tonumber(L, -1);
	HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	if (!phandle)
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	else
	{
		ProcHandle = phandle;
		//cout << "Handle valid" << endl;
		lua_pushinteger(L, (int)phandle);
		return 1;
	}
}
int Lua_ReadInt(lua_State *L)
{
	int val = 0;
	DWORDLONG address = lua_tonumber(L, -1);
	ReadProcessMemory(ProcHandle, (void*)address, &val, sizeof(val), 0);
	lua_pushinteger(L, val);
	return 1;
}
int Lua_Unprotect(lua_State *L)
{
	int val = 0;
	DWORDLONG address = lua_tonumber(L, -1);

	SIZE_T size = lua_tonumber(L, -2);
	BOOL fail = VirtualUnlock((LPVOID)address, size);
	if (!fail) {
		DWORDLONG err = GetLastError();
		lua_pushinteger(L, fail);
		lua_pushinteger(L, err);
		return 2;
	}
	lua_pushinteger(L, fail);
	return 1;
}
int Lua_WriteInt(lua_State *L)
{
	int val = lua_tonumber(L, -1);
	DWORDLONG address = lua_tonumber(L, -2);
	WriteProcessMemory(ProcHandle, (void*)address, &val, sizeof(val), 0);
	return 0;
}
int Lua_WriteByte(lua_State *L)
{
	byte val = lua_tonumber(L, -1);
	DWORDLONG address = lua_tonumber(L, -2);
	WriteProcessMemory(ProcHandle, (void*)address, &val, sizeof(val), 0);
	return 0;
}
int Lua_WriteFloat(lua_State *L)
{
	float val = lua_tonumber(L, -1);
	DWORDLONG address = lua_tonumber(L, -2);
	WriteProcessMemory(ProcHandle, (void*)address, &val, sizeof(val), 0);
	return 0;
}
int Lua_ReadFloat(lua_State *L)
{
	float val = 0.f;
	DWORDLONG address = lua_tonumber(L, -1);
	ReadProcessMemory(ProcHandle, (void*)address, &val, sizeof(val), 0);
	lua_pushinteger(L, val);
	return 1;
}
int Lua_ReadByte(lua_State *L)
{
	byte val = 0;
	DWORDLONG address = lua_tonumber(L, -1);
	ReadProcessMemory(ProcHandle, (void*)address, &val, sizeof(val), 0);
	lua_pushinteger(L,val);
	return 1;
}
int Lua_ProtectSelf(lua_State *L) // xd
{
	DWORDLONG address = lua_tonumber(L, -2);
	int lenth = lua_tonumber(L, -1);

	bool fail = VirtualProtect((LPVOID)address, lenth, PAGE_EXECUTE_READWRITE, &oldprotect);
	if (!fail) {
		DWORDLONG err = GetLastError();
		lua_pushinteger(L, fail);
		lua_pushinteger(L, err);
		return 2;
	}
	lua_pushinteger(L, fail);
	return 1;
}
//int Lua_PrintNumber(lua_State *L)
//{
//	DWORDLONG address = lua_tonumber(L, -1);
//	cout << "lpvoid size: " << sizeof(LPVOID) << endl;
//	cout << "void* size: " << sizeof(void*) << endl;
//	cout << "void* size: " << sizeof(double) << endl;
//	cout << "DWORD size: " << sizeof(DWORD) << endl;
//	cout << "LPDWORD size: " << sizeof(LPDWORD) << endl;
//	cout << "lpvoid address: " << (LPVOID)address << endl;
//	cout << "void* address: " << (void*)address << endl;
//	cout << "dword address: " << address << endl;
//	return 0;
//}
int Lua_RevertProtect(lua_State *L) // xddd
{
	DWORDLONG address = lua_tonumber(L, -2);
	int lenth = lua_tonumber(L, -1);
	bool fail = VirtualProtect((LPVOID)address, lenth, oldprotect, &oldprotect);
	if (!fail) {
		DWORDLONG err = GetLastError();
		lua_pushinteger(L, fail);
		lua_pushinteger(L, err);
		return 2;
	}
	lua_pushinteger(L, fail);
	return 1;
}
int Lua_CloseHandle(lua_State *L)
{
	CloseHandle(ProcHandle);
	return 0;
}


int Lua_GetModuleBaseAddress(lua_State *L)
{
	DWORDLONG dwProcID = lua_tonumber(L, -2);
	std::string szModuleName = lua_tostring(L, -1);
	DWORDLONG ModuleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (strcmp(ModuleEntry32.szModule, szModuleName.c_str()) == 0)
				{
					ModuleBaseAddress = (DWORDLONG)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32));
		}
		CloseHandle(hSnapshot);
	}
	lua_pushinteger(L, (INT64)ModuleBaseAddress);
	return 1;
}

int Lua_MemoryGetProcess(lua_State *L)
{
	std::string modsearchname = lua_tostring(L, -1);
	DWORD aProcesses[1024], cProcesses, cbNeeded;
	unsigned int i;
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		return 0;
	}
	cProcesses = cbNeeded / sizeof(DWORD);
	for (i = 0; i < cProcesses; i++)
	{
		if (aProcesses[i] != 0)
		{
			std::string modname = GetProcessName(aProcesses[i]);
			std::size_t found = modname.find(modsearchname);
			//cout << modname << " " << (int)aProcesses[i] << endl;
			if (found != std::string::npos) {

				lua_pushinteger(L,(int)aProcesses[i]);
				return 1;
			}
		}
	}
	return 0;
}

extern "C" int __declspec(dllexport) luaopen_memory_core(lua_State *L)
{
	lua_newtable(L);

	lua_pushfuncsettable(L, "FindProcess", Lua_MemoryGetProcess);
	//lua_pushfuncsettable(L, "ProtectSelf", Lua_ProtectSelf);
	//lua_pushfuncsettable(L, "RevertProtect", Lua_RevertProtect);
	lua_pushfuncsettable(L, "OpenProcess", Lua_OpenProcess);
	//lua_pushfuncsettable(L, "Unprotect", Lua_Unprotect); -- idk if these work
	lua_pushfuncsettable(L, "CloseHandle", Lua_CloseHandle);
	lua_pushfuncsettable(L, "ReadInt", Lua_ReadInt);
	lua_pushfuncsettable(L, "ReadByte", Lua_ReadByte);
	lua_pushfuncsettable(L, "ReadFloat", Lua_ReadFloat);
	lua_pushfuncsettable(L, "WriteInt", Lua_WriteInt);
	lua_pushfuncsettable(L, "WriteFloat", Lua_WriteFloat);
	lua_pushfuncsettable(L, "WriteByte", Lua_WriteByte);
	lua_pushfuncsettable(L, "GetAddress", Lua_GetModuleBaseAddress);

	return 1;
}
