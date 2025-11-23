#include "Functions.h"
#include <chrono>
#include <ctime>
DWORD Function::GetProcessID(const char* ProcessName)
{
	PROCESSENTRY32 ProcessInfoPE;
	ProcessInfoPE.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(15, 0);
	Process32First(hSnapshot, &ProcessInfoPE);
	USES_CONVERSION;
	do{
		if (strcmp(W2A(ProcessInfoPE.szExeFile), ProcessName) == 0)
		{
			CloseHandle(hSnapshot);
			return ProcessInfoPE.th32ProcessID;
		}
	} while (Process32Next(hSnapshot, &ProcessInfoPE));
	CloseHandle(hSnapshot);
	return 0;
}

HMODULE Function::GetProcessModuleHandle(DWORD Pid, const char* ModuleName)
{
	MODULEENTRY32 ModuleInfoPE;
	ModuleInfoPE.dwSize = sizeof(MODULEENTRY32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, Pid);
	Module32First(hSnapshot, &ModuleInfoPE);
	USES_CONVERSION;
	do {
		if (strcmp(W2A(ModuleInfoPE.szModule), ModuleName) == 0)
		{
			CloseHandle(hSnapshot);
			return ModuleInfoPE.hModule;
		}
	} while (Module32Next(hSnapshot, &ModuleInfoPE));
	CloseHandle(hSnapshot);
	return 0;
}
std::string Function::GetTimeHMSS()
{
	// 获取当前系统时间点
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);

	// localtime_s 安全版本
	std::tm localTm{};
	localtime_s(&localTm, &t);

	char buffer[16];
	std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &localTm);

	return buffer;
}
