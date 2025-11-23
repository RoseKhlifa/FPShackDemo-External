#include <iostream>
#include <Windows.h>
#include <string>
#include "Entity.h"
#include "Functions.h"
#include "OwnImGui.h"
#include "GameData.h"

static bool IsBoxOpened = true;
static bool IsHealthOpened = true;
static bool IsNameOpened = true;
static bool IsLineOpened = true;
static bool IsDistanceOpened = true;
static bool IsWarnOpened = true;
static bool IsTriAngleWarnOpened = true;
static bool IsAimbotOpened = true;
static bool IsRadarOpened = true;
int g_SelectedLineStyle = 1;
const int AIM_HOTKEY = VK_RBUTTON;






// Author: RoseKhlifa , email: 2221542777@qq.com

void Menu()
{
	ImGui::Checkbox("Box", &IsBoxOpened);
    if (IsBoxOpened)
    {
        OwnImGui::get().DrawBoxStyleSelector();
    }
	ImGui::Checkbox("Health", &IsHealthOpened);
	ImGui::Checkbox("Name", &IsNameOpened);
	ImGui::Checkbox("Distance", &IsDistanceOpened);
	ImGui::Checkbox("Line", &IsLineOpened);
    if (IsLineOpened)
    {
        OwnImGui::get().DrawLineStyleSelector();
    }
	ImGui::Checkbox("Warning", &IsWarnOpened);
    if (IsWarnOpened)
    {
        ImGui::Checkbox("TriAngleWarning", &IsTriAngleWarnOpened);
    }
    ImGui::Checkbox("Aimbot", &IsAimbotOpened);
    if (IsAimbotOpened)
    {
        OwnImGui::get().DrawAimbotStyleSelector();
        if(OwnImGui::get().AimbotStyle==2)
        {
            ImGui::SliderFloat("Aimbot Fov", &OwnImGui::get().AimbotFOV, 1.0f, 200.0f);
		}
    }
    ImGui::Checkbox("Radar", &IsRadarOpened);
    if (IsRadarOpened)
    {
        OwnImGui::get().DrawRadarStyleSelector();
    }
	
}

void DrawMain()
{
	Menu();
	float Matrix[4][4];	// 矩阵
	CEntity LocalEntity;// 本地实体

	ReadProcessMemory(Game::hProcess, (LPCVOID)(Game::ModuleAddress + Base::Matrix), &Matrix, 64, 0);

	LocalEntity.EntityAddress = Function::Read<DWORD>(Game::ModuleAddress + Base::LocalEntity);
	PlayerCount = Function::Read<DWORD>(Game::ModuleAddress + Base::Count);
	LocalEntity.MouseAddress = Function::Read<DWORD>(Game::ModuleAddress + Base::Mouse);
	LocalEntity.UpDate(Matrix);
	//std::cout << "--------------------" << std::endl;
	int count = 0;// 近距离人数统计
    bool haveTarget = false;
    float bestDist = FLT_MAX;
    float bestYaw = 0.0f;
    float bestPitch = 0.0f;
    float bestCrosshair = FLT_MAX;
    float bestTargetScreenX = 0.0f;
    float bestTargetScreenY = 0.0f;
    std::vector<CEntity> ents;
    ents.reserve(PlayerCount);  // 性能更好
    DWORD entityListBase = Function::Read<DWORD>(Game::ModuleAddress + Base::EntityList);
	for (int i = 0; i < PlayerCount - 1; i++)//如果实体列表有本人的话 就加一 对本人进行过滤
    {
        CEntity Entity;
        DWORD entPtr = Function::Read<DWORD>(entityListBase + 0x4 * i);
        if (entPtr == 0)
            continue;

        Entity.EntityAddress = entPtr;

        bool ok = Entity.UpDate(Matrix);
        Entity.CalculateDistance3D(LocalEntity.fPos);
        // 过滤本人
        // if (LocalEntity.EntityAddress == Entity.EntityAddress)
        //     continue;
        
        // 过滤队友
        if (LocalEntity.Camp == Entity.Camp)
            continue;

        // 过滤异常血量
        if (Entity.Health <= 0 || Entity.Health > 100)
            continue;
        ents.push_back(Entity);      // ⭐ 必须先保存实体，雷达才能显示
        // 统计近距离敌人数量
        if (Entity.Distance <= 50.0f)
            count++;

        bool onScreen = (Entity.fScPos.x >= 0 && Entity.fScPos.x <= WindowData::width && Entity.fScPos.y >= 0 && Entity.fScPos.y <= WindowData::height);

        // W2S 失败就不画 ESP，但上面统计不受影响
        if (!ok)
            continue;


        // ----- 以下是绘制部分 -----

        std::string hpText = "HP: " + std::to_string(Entity.Health);
        std::string name = Entity.ReadEntityName(Entity.EntityAddress);

        if (IsBoxOpened)
            switch (OwnImGui::get().BoxStyle)
            {
			    case 0: OwnImGui::get().Rectangle(Entity.Rect.point, Entity.Rect.width, Entity.Rect.height, OwnImGui::get().DistanceColor(Entity.Distance), 1.5); break;
			    case 1: OwnImGui::get().RoundRect(Entity.Rect.point, Entity.Rect.width, Entity.Rect.height, 6.0f, OwnImGui::get().DistanceColor(Entity.Distance), 2.0f); break;
                case 2: OwnImGui::get().Draw3DBox(Entity, Matrix, OwnImGui::get().DistanceColor(Entity.Distance));
            }
            //OwnImGui::get().Rectangle(Entity.Rect.point, Entity.Rect.width, Entity.Rect.height, IM_COL32(255, 255, 0, 255), 1.5);
            //OwnImGui::get().RoundRect(Entity.Rect.point, Entity.Rect.width, Entity.Rect.height, 6.0f, IM_COL32(255, 255, 0, 255), 2.0f);
            
        if (IsHealthOpened)
            OwnImGui::get().DrawHealthBar(Entity.Rect.point, Entity.Rect.height, Entity.Health, LocalEntity.InitHealth);

        if (IsNameOpened)
            OwnImGui::get().Text(name.c_str(), { Entity.Rect.point.x, Entity.Rect.point.y - 20 }, IM_COL32(255, 255, 255, 255));

        if (IsDistanceOpened)
        {
            ImU32 distColor = OwnImGui::get().DistanceColor(Entity.Distance);
            OwnImGui::get().DrawDistance({ Entity.Rect.point.x, Entity.Rect.point.y - 15 }, Entity.Distance/2, distColor);
        }
        if (IsTriAngleWarnOpened)
        {
            if (!onScreen)
            {
                OwnImGui::get().DrawDirectionArrow(Entity.hScPos.x,Entity.hScPos.y,OwnImGui::get().DistanceColor(Entity.Distance),35.0f);
            }
        }
        if (IsLineOpened)
        {
            ImVec2 start = { WindowData::width / 2.0f, 0.0f };
            ImVec2 end = { Entity.hScPos.x, Entity.hScPos.y };

            switch (OwnImGui::get().LineStyle)
            {
            case 0: OwnImGui::get().DrawRainbowSweepLine(start, end, 2.0f); break;
            case 1: OwnImGui::get().DrawRainbowStaticGradientLine(start, end, 2.0f); break;
            case 2: OwnImGui::get().DrawRainbowPulseLine(start, end, 2.0f); break;
            case 3: OwnImGui::get().DrawRainbowWaveLine(start, end, 2.0f); break;
            }
        }

        if (IsAimbotOpened)
        {
            float yaw = 0.0f; float pitch = 0.0f;
            Entity.CalculateViewAngle(LocalEntity, Entity, yaw, pitch);
            switch (OwnImGui::get().AimbotStyle)
            {
            case 0: //按照距离最近
            {
                if (Entity.Distance < bestDist)
                {
                    bestDist = Entity.Distance;
                    bestYaw = yaw;
                    bestPitch = pitch;
                    haveTarget = true;
                    // ⭐ 保存屏幕坐标，用于画线
                    bestTargetScreenX = Entity.hScPos.x;
                    bestTargetScreenY = Entity.hScPos.y;
                }
                break;
			}// case 0 结束
            case 1: //按照离准星最近
            {
                if (!onScreen) break;   // 离准星最近必须是屏幕内的目标

                float cx = WindowData::width / 2.0f;
                float cy = WindowData::height / 2.0f;

                // 敌人投影到屏幕的坐标
                float dx = Entity.fScPos.x - cx;
                float dy = Entity.fScPos.y - cy;

                float crossDist = sqrt(dx * dx + dy * dy); // ⭐ 离准星的像素距离

                if (crossDist < bestCrosshair)
                {
                    bestCrosshair = crossDist;
                    bestYaw = yaw;
                    bestPitch = pitch;
                    haveTarget = true;
                    // ⭐ 保存屏幕坐标，用于画线
                    bestTargetScreenX = Entity.hScPos.x;
                    bestTargetScreenY = Entity.hScPos.y;
                }
                break;
			}//  case 1 结束

            case 2:
            {

                OwnImGui::get().DrawFOV(OwnImGui::get().AimbotFOV);
                if (!onScreen) break;
                
                float cx = WindowData::width / 2.0f;
                float cy = WindowData::height / 2.0f;

                // 敌人屏幕坐标
                float ex = Entity.hScPos.x;
                float ey = Entity.hScPos.y;

                // 与准星距离
                float dx = ex - cx;
                float dy = ey - cy;
                float distToCross = sqrt(dx * dx + dy * dy);


                // 如果敌人在 FOV 圈外 → 不考虑
                if (distToCross > OwnImGui::get().AimbotFOV) break;

                // FOV模式：谁距离准星最近，锁谁
                if (distToCross < bestCrosshair)
                {
                    bestCrosshair = distToCross;
                    bestYaw = yaw;
                    bestPitch = pitch;
                    haveTarget = true;
                    // ⭐ 保存屏幕坐标，用于画线
                    bestTargetScreenX = Entity.hScPos.x;
                    bestTargetScreenY = Entity.hScPos.y;
                }
			}// case 2 结束

			}// switch AimbotStyle 结束
                
		}// IsAimbotOpened 结束
        
	}// for 循环结束

    // 循环结束后再画 Warning
    if (IsWarnOpened)
    {
        static float wtime = 0.0f;
        wtime += 0.1f;
        // 0~255 闪烁
        int alpha = (int)(150 + 105 * sin(wtime));
        ImU32 flashColor = IM_COL32(255, 0, 0, alpha);
        std::string warningText ="WARNING!There are " + std::to_string(count) + " enemy(s) within 50 units!";
        OwnImGui::get().Text(warningText.c_str(),{ WindowData::width / 2.0f - 30.0f, 50.0f },flashColor);
    }
    if (IsRadarOpened)
    {
        switch (OwnImGui::get().RadarStyle)
        {
		    case 0: OwnImGui::get().DrawRadar(LocalEntity, ents); break;
		    case 1: OwnImGui::get().DrawRadar1(LocalEntity, ents); break;
            case 2: OwnImGui::get().DrawRadar2(LocalEntity, ents); break;
        }
    }
    if(IsAimbotOpened && haveTarget)
    {
        float cx = WindowData::width * 0.5f;
        float cy = WindowData::height * 0.5f;

        // enemyX/Y 是自瞄目标的屏幕坐标
        float enemyX = bestTargetScreenX;
        float enemyY = bestTargetScreenY;

        OwnImGui::get().Line({ cx, cy },{ enemyX, enemyY },IM_COL32(255, 255, 255, 255),2.0f);
		SHORT state = GetAsyncKeyState(AIM_HOTKEY);
        if (state && 0x8000)
        {
            //std::cout << "Aimbot Active! Yaw:" << bestYaw << " Pitch:" << bestPitch << std::endl;
            Function::Write<float>(LocalEntity.MouseAddress + 0x34, bestYaw);
            Function::Write<float>(LocalEntity.MouseAddress + 0x38, bestPitch);
        }
		
	}
	
	//std::cout << "--------------------" << std::endl;
}

int main()
{
	Game::Pid = Function::GetProcessID("ac_client.exe");
	if (Game::Pid <= 0)
	{
		std::cout << "[+]["<< Function::GetTimeHMSS()<<"]Get \"ac_client.exe\" process-id failed." << std::endl;
		system("pause");
		return 0;
	}
	Game::hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_CREATE_THREAD, TRUE, Game::Pid);
	Game::ModuleAddress = reinterpret_cast<DWORD>(Function::GetProcessModuleHandle(Game::Pid, "ac_client.exe"));
	//std::string time = GetTimeHMSS();
	std::cout << "[+]["<< Function::GetTimeHMSS() <<"]Pid:" << Game::Pid << std::endl;
	std::cout <<"[+]["<< Function::GetTimeHMSS()<<"]ModuleAddress:0x" << std::hex <<Game::ModuleAddress << std::endl;
	std::cout << "[+]["<< Function::GetTimeHMSS()<<"]Runing..." << std::endl;
	OwnImGui::get().Start("SDL_app", "AssaultCube", DrawMain);//DrawMain为回调函数
	return 0;
}
