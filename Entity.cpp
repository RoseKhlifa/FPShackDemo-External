#include "Entity.h"

bool CEntity::WorldToScreen(float Matrix[4][4],int a)
{
	float View, SightX = WindowData::width / 2, SightY = WindowData::height / 2;//sightX 视角宽 sightY 视角高
	if (a == 1)//脚
	{
		if (fPos == 0)
			return false;
		View = Matrix[0][3] * this->fPos.x + Matrix[1][3] * this->fPos.y + Matrix[2][3] * this->fPos.z + Matrix[3][3];
		if (View <= 0.01)
			return false;
		View = 1 / View;
		this->fScPos.x = SightX + (Matrix[0][0] * this->fPos.x + Matrix[1][0] * this->fPos.y + Matrix[2][0] * this->fPos.z + Matrix[3][0]) * View * SightX;
		this->fScPos.y = SightY - (Matrix[0][1] * this->fPos.x + Matrix[1][1] * this->fPos.y + Matrix[2][1] * this->fPos.z + Matrix[3][1]) * View * SightY;
		this->fScPos.z = SightY - (Matrix[0][1] * this->fPos.x + Matrix[1][1] * this->fPos.y + Matrix[2][1] * this->fPos.z + Matrix[3][1]) * View * SightY;
		return true;
	}
	if (a == 2)//头
	{
		if (hPos == 0)
			return false;
		View = Matrix[0][3] * this->hPos.x + Matrix[1][3] * this->hPos.y + Matrix[2][3] * this->hPos.z + Matrix[3][3];
		if (View <= 0.01)
			return false;
		View = 1 / View;
		this->hScPos.x = SightX + (Matrix[0][0] * this->hPos.x + Matrix[1][0] * this->hPos.y + Matrix[2][0] * this->hPos.z + Matrix[3][0]) * View * SightX;
		this->hScPos.y = SightY - (Matrix[0][1] * this->hPos.x + Matrix[1][1] * this->hPos.y + Matrix[2][1] * (this->hPos.z+0.5) + Matrix[3][1]) * View * SightY;
		this->hScPos.z = SightY - (Matrix[0][1] * this->hPos.x + Matrix[1][1] * this->hPos.y + Matrix[2][1] * this->hPos.z + Matrix[3][1]) * View * SightY;
		return true;
	}
	
}
bool CEntity::WorldToScreenPoint(const Vec3& pos, float Matrix[4][4], Vec3& out)const
{
	float w =
		Matrix[0][3] * pos.x +
		Matrix[1][3] * pos.y +
		Matrix[2][3] * pos.z +
		Matrix[3][3];

	if (w < 0.001f)
		return false;

	float inv = 1.0f / w;

	float SightX = WindowData::width * 0.5f;
	float SightY = WindowData::height * 0.5f;

	out.x = SightX + (
		Matrix[0][0] * pos.x +
		Matrix[1][0] * pos.y +
		Matrix[2][0] * pos.z +
		Matrix[3][0]
		) * inv * SightX;

	out.y = SightY - (
		Matrix[0][1] * pos.x +
		Matrix[1][1] * pos.y +
		Matrix[2][1] * pos.z +
		Matrix[3][1]
		) * inv * SightY;

	return true;
}
void CEntity::Get3DBoxCorners(Vec3 out[8])const
{
	float width = 1.0f;   // 根据模型调节
	float height = 5.0f;  // 人物高度
	float depth = 1.2f;

	// 脚底位置
	Vec3 base = this->fPos;

	// 头位置
	Vec3 top = { fPos.x, fPos.y, fPos.z + height };

	// 8 corners
	out[0] = { base.x - width, base.y - depth, base.z };
	out[1] = { base.x + width, base.y - depth, base.z };
	out[2] = { base.x + width, base.y + depth, base.z };
	out[3] = { base.x - width, base.y + depth, base.z };

	out[4] = { top.x - width, top.y - depth, top.z };
	out[5] = { top.x + width, top.y - depth, top.z };
	out[6] = { top.x + width, top.y + depth, top.z };
	out[7] = { top.x - width, top.y + depth, top.z };
}

bool CEntity::UpDate(float Matrix[4][4])
{
	if (this->EntityAddress <= 0)
		return false;
	if (!this->GetfPos())		// 更新坐标
		return false;			
	if (!this->GethPos())		// 更新坐标
		return false;
	
	if (!this->GetHealth())		// 更新血量
		return false;
	if (!this->GetCamp())		// 更新阵营
		return false;			
	if (!this->GetAngles())		// 更新角度
		return false;			
	if (!WorldToScreen(Matrix,1))	// 更新屏幕脚底坐标
		return false;
	if (!WorldToScreen(Matrix,2))	// 更新屏幕头部坐标
		return false;
	// 方框数据计算
	this->Rect.height = this->fScPos.z - this->hScPos.z+20;
	this->Rect.width = this->Rect.height * 0.53;
	this->Rect.point.x = this->fScPos.x - this->Rect.width / 2;
	this->Rect.point.y = this->hScPos.y;
	return true;
}

//bool CEntity::IsActive()
//{
//	if (this->EntityAddress <= 0)
//		return false;
//	int TempNum;
//	TempNum = Function::Read<int>(this->EntityAddress + Base::EntityData.Active);
//	if (TempNum != 0)
//		return false;
//	return true;
//}

bool CEntity::GetfPos()
{
	if (this->EntityAddress <= 0)
		return false;
	this->fPos.x = Function::Read<float>(this->EntityAddress + Base::EntityData.fPos);
	this->fPos.y = Function::Read<float>(this->EntityAddress + Base::EntityData.fPos + 0x4);
	this->fPos.z = Function::Read<float>(this->EntityAddress + Base::EntityData.fPos + 0x8);
	return true;
}
bool CEntity::GethPos()
{
	if (this->EntityAddress <= 0)
		return false;
	this->hPos.x = Function::Read<float>(this->EntityAddress + Base::EntityData.hPos);
	this->hPos.y = Function::Read<float>(this->EntityAddress + Base::EntityData.hPos + 0x4);
	this->hPos.z = Function::Read<float>(this->EntityAddress + Base::EntityData.hPos + 0x8);
	return true;
}

bool CEntity::GetHealth()
{
	if (this->EntityAddress <= 0)
		return false;
	this->Health = Function::Read<int>(this->EntityAddress + Base::EntityData.Health);
	return true;
}

bool CEntity::GetCamp()
{
	if (this->EntityAddress <= 0)
		return false;
	this->Camp = Function::Read<int>(this->EntityAddress + Base::EntityData.Camp);
	return true;
}

bool CEntity::GetAngles()
{
	if (this->EntityAddress <= 0)
		return false;
	this->AngleX= Function::Read<float>(this->EntityAddress + Base::EntityData.AngleX);
	this->AngleY = Function::Read<float>(this->EntityAddress + Base::EntityData.AngleY);
	return true;
}

std::string CEntity::ReadEntityName(uintptr_t EntityBase)
{
	const uintptr_t NameOffset = 0x205;     // 你确认的偏移
	const size_t NameLength = 20;           // CE 显示的长度

	char buffer[NameLength + 1] = { 0 };

	ReadProcessMemory(
		Game::hProcess,
		(LPCVOID)(EntityBase + NameOffset),
		buffer,
		NameLength,
		nullptr
	);

	buffer[NameLength] = '\0'; // 终止符

	return std::string(buffer);
}
bool CEntity::CalculateDistance3D(const Vec3& LocalPos)
{
	float dx = this->fPos.x - LocalPos.x;
	float dy = this->fPos.y - LocalPos.y;
	float dz = this->fPos.z - LocalPos.z;

	this->Distance = sqrtf(dx * dx + dy * dy + dz * dz);
	return true;
}
void CEntity::Get3DBox(Vec3 out[8])
{
	float w = 0.4f;   // 半宽
	float h = Height; // 高度
	float d = 0.4f;   // 半深度

	out[0] = { fPos.x - w, fPos.y - d, fPos.z };
	out[1] = { fPos.x + w, fPos.y - d, fPos.z };
	out[2] = { fPos.x + w, fPos.y + d, fPos.z };
	out[3] = { fPos.x - w, fPos.y + d, fPos.z };

	out[4] = { fPos.x - w, fPos.y - d, fPos.z + h };
	out[5] = { fPos.x + w, fPos.y - d, fPos.z + h };
	out[6] = { fPos.x + w, fPos.y + d, fPos.z + h };
	out[7] = { fPos.x - w, fPos.y + d, fPos.z + h };
}
bool CEntity::CalculateViewAngle(const CEntity& local, const CEntity& entity,float&yaw,float&pitch)
{
	float dx = local.hPos.x - entity.hPos.x;
	float dy = local.hPos.y - entity.hPos.y;
	float high = entity.hPos.z - entity.fPos.z;
	float dz = (entity.hPos.z) - (local.hPos.z); // 64视为头部高度补偿

	float _yaw = atan2f(dy, dx) * (180.0f / 3.1415926f);
	_yaw -= 90.0f; // 坐标系调整
	if (_yaw < 0.0f)
		_yaw += 360.0f;
	if (_yaw > 360.0f)
		_yaw -= 360.0f;
	float _pitch = atan2f(dz, sqrtf(dx * dx + dy * dy)) * (180.0f / 3.1415926f);
	if (_pitch > 89.f)_pitch = 89.f;
	if (_pitch < -89.f)_pitch = -89.f;
	yaw = _yaw;
	pitch = _pitch;
	//std::cout << "Calculating View Angle:" << std::endl;
	//std::cout << "Yaw: " << yaw << " Pitch: " << pitch << std::endl;
	return true;
	
}
//int CEntity::GetClosetTarget(const CEntity& local,)
//{
//	
//}