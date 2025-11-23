#pragma once
#include <Windows.h>

namespace Base 
{
	const DWORD EntityList = 0x00191FCC;	// 实体列表
	const DWORD LocalEntity = 0x0017E0A8;	// 本地实体
	const DWORD Matrix = 0x17DFD0;		// 矩阵
	const DWORD Count = 0x0018AC0C;		// 玩家数量
	const DWORD Mouse = 0x0018AC00; // 鼠标地址
	struct EntityData_	// 实体数据
	{
		const DWORD fPos = 0x28;	// 脚底坐标 
		const DWORD hPos = 0x4;	// 头部坐标 
		//const DWORD Active = 0xED;	// 激活
		const DWORD Camp = 0x30C;	// 阵营
		const DWORD Health = 0xEC;	// 生命
		const DWORD AngleX = 0x4;	// 鼠标X
		const DWORD AngleY = 0x8; // 鼠标Y
		const DWORD Name = 0x205; // 名字
	}EntityData;
}