#pragma once
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <atlconv.h>
#include "GameData.h"

namespace Function {
	DWORD GetProcessID(const char* ProcessName);
	HMODULE GetProcessModuleHandle(DWORD Pid, const char* ModuleName);
	std::string GetTimeHMSS();// 获取当前时间，格式为时:分:秒
	template <typename Type>
	Type Read(DWORD Address)
	{
		Type Temp;
		if (!ReadProcessMemory(Game::hProcess, reinterpret_cast<LPCVOID>(Address), &Temp, sizeof(Type), 0))
			return 0;
		return Temp;
	}
	template <typename Type>
	bool Write(DWORD Address, const Type& Value)
	{
		return WriteProcessMemory(Game::hProcess,reinterpret_cast<LPVOID>(Address),&Value,sizeof(Type),nullptr);
	}
	bool IsKeyDown(int vk)
	{
		return (GetAsyncKeyState(vk) & 0x8000);
	}
}