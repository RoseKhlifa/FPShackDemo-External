#pragma once
#include <iostream>
#include "Struct.h"
#include "BaseOffset.h"
#include "Functions.h"
#include "OwnImGui.h"
DWORD PlayerCount = 0;// 玩家数量

class CEntity	// 实体类
{
public:
	DWORD EntityAddress = 0x0;		// 实体地址
	DWORD MouseAddress = 0x0;    // 鼠标地址
	Vec3 fPos{ 0,0,0 };				// 实体脚底坐标
	Vec3 hPos{ 0,0,0 };				// 实体头顶坐标
	Vec3 fScPos{ 0,0,0 };			// 屏幕脚底坐标
	Vec3 hScPos{ 0,0,0 };			// 屏幕头顶坐标
	RECINFO Rect;					// 方框数据
	int Health = 0;					// 生命
	const float InitHealth = 100.0; // 初始生命
	float AngleX = 0.0f;			// 鼠标X角度值
	float AngleY = 0.0f;			// 鼠标Y角度值
	float Height = 75;				// 实体高度
	int Camp = -1;					// 阵营
	char Name[32] = { 0 };			// 名字
	float Distance = 0.0f;			// 距离
	float MouseX = 0.0f;         // 鼠标X位置
	float MouseY = 0.0f;         // 鼠标Y位置

	// 更新名字
	std::string ReadEntityName(uintptr_t EntityBase);
	//	更新距离
	bool CalculateDistance3D(const Vec3& LocalPos);
	// 实体数据更新
	bool UpDate(float Matrix[4][4]);
	// 是否激活
	//bool IsActive();
	void Get3DBox(Vec3 out[8]);
	bool CalculateViewAngle(const CEntity& local, const CEntity& entity, float& yaw, float& pitch);
	int GetClosetTarget();
	bool WorldToScreen3D(const Vec3& in, ImVec2& out, float Matrix[4][4])const;
	bool WorldToScreenPoint(const Vec3& pos, float Matrix[4][4], Vec3& out)const;
	void Get3DBoxCorners(Vec3 out[8])const;
private:
	// 转屏幕坐标
	bool WorldToScreen(float Matrix[4][4],int a);
	// 更新脚底坐标
	bool GetfPos();
	// 更新头部坐标
	bool GethPos();
	// 更新血量
	bool GetHealth();
	// 更新阵营
	bool GetCamp();
	// 更新角度数据
	bool GetAngles();
	// 3Dfangkuang
	
	

};

