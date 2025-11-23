#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"
#include "Struct.h"
#include "Singleton.h"
#include <d3d11.h>
#include <iostream>
#include <tchar.h>
#include <dwmapi.h>
#include <thread>
#include <atlstr.h>
#include <algorithm>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma warning(disable:4996)
class CEntity;
// Datas
static ID3D11Device* g_pd3dDevice;
static ID3D11DeviceContext* g_pd3dDeviceContext;
static IDXGISwapChain* g_pSwapChain;
static ID3D11RenderTargetView* g_mainRenderTargetView;
typedef void(*Draw)();
bool MenuActivation = true;	// 菜单激活
bool clean = false;			// 是否清理
WNDCLASSEX W_class;

namespace WindowData {
	HWND hToWindow;		// 目标窗口句柄
	HWND hMyWindow;		// 绘制窗口句柄
	RECT ToWindowRect;	// 目标窗口数据
	char ToWindowName[256];
	char ToWindowClassName[256];
	int  width, height; // 目标窗口大小
}

class OwnImGui : public Singleton<OwnImGui>
{
private:
	void Initialize();
	bool CreateOwnWindow(HWND ToWindow);
	void WindowMessageCircle(Draw MainFunction);
	// 监视目标窗口
	void MonitorToWindowState();
	// 窗口回调
	static LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	// 初始字体大小
	size_t c_FontSize = 15;
	// 结束
	bool done = false;
public:
	OwnImGui(const char* MenuName_);
	OwnImGui() { strcpy(MenuName, "RoseKhlifa"); }
	~OwnImGui();
	// 菜单名字
	char MenuName[256]{};
	// 菜单状态
	bool Open = true;
	// 绘制主函数
	void Start(const char* ToWindowClassName, const char* ToWindowName, Draw MainFunction);
	// 退出绘制
	void End() { done = true; }
	// 设置字体大小
	void ReSetFontSize(size_t FontSize);
	// 个人主题
	void MyStyle();
	// 选择框
	void CheckboxEx(const char* str_id, bool* v);
	// 文本
	void Text(const char* Text, Vec2 Pos,int color);
	// 矩形
	void Rectangle(Vec2 Pos, int width, int height, int color, int thickness);
	// 线
	void Line(Vec2 From, Vec2 To,int color,int thickness);
	// 圆
	void Circle(Vec2 Center, float radius, int color, float thickness);
	// 填充圆
	void CircleFilled(Vec2 Center, float radius, int color, int num = 360);
	// 填充矩形
	void RectangleFilled(Vec2 Pos, int width, int height, int color);
	// 描边文本
	void StrokeText(const char* Text, Vec2 Pos, int color_b, int color);
	// 连接点
	void ConnectPoint(std::vector<Vec2> Points, int color, float thickness);
	// 颜色转换
	int  Color(float* Color_);
	// 线条样式选择
	int LineStyle = 1; // 选择 0~3 对应 4 种线条
	void DrawLineStyleSelector(); // 绘制选择框
	// 雷达样式选择
	int RadarStyle = 2; // 选择 0~1 对应 2 种雷达
	void DrawRadarStyleSelector();
	//方框样式选择
	int BoxStyle = 1; // 选择 0~2 对应 3 种方框
	void DrawBoxStyleSelector();
	// 不同样式线条
	void DrawRainbowSweepLine(const ImVec2& start, const ImVec2& end, float thickness = 2.0f);//霓虹流动线
	void DrawRainbowStaticGradientLine(const ImVec2& start, const ImVec2& end, float thickness = 2.0f);//静态渐变线
	void DrawRainbowPulseLine(const ImVec2& start, const ImVec2& end, float thickness = 2.0f);//呼吸 RGB
	void DrawRainbowWaveLine(const ImVec2& start, const ImVec2& end, float thickness = 2.0f);//波浪颜色
	// 血条
	void DrawHealthBar(const Vec2& rectPos, float rectHeight, int health, int maxHealth);
	// 距离
	void DrawDistance(const Vec2& pos, float distance, ImU32 color = IM_COL32(255, 255, 255, 255));
	// 距离颜色
	ImU32 DistanceColor(float distance);
	// 方向箭头
	void DrawDirectionArrow(float ex, float ey, ImU32 color, float size);
	// 圆角矩形
	void RoundRect(const Vec2& pos, float w, float h, float r, ImU32 color, float thickness = 2.0f);
	// 填充圆角矩形
	void RoundRectFilled(const Vec2& pos, float w, float h, float r, ImU32 color);
	// 雷达
	void DrawRadarBackground(float size);
	// 雷达点
	void DrawRadarPoint(const CEntity& ent, const CEntity& local, float size, ImU32 color);
	// 综合雷达
	void DrawRadar(const CEntity& local, const std::vector<CEntity>& ents);
	// 3D盒子
	void Draw3DBox(const CEntity& ent, float Matrix[4][4], ImU32 color);
	// 雷达综合
	void DrawRadar1(const CEntity& local, const std::vector<CEntity>& ents);
	void DrawRadar2(const CEntity& local, const std::vector<CEntity>& ents, bool rightTop);
	// 自瞄风格选择
	int AimbotStyle = 2; // 选择 0~2 对应 3 种自瞄
	void DrawAimbotStyleSelector();
	// 自瞄视野
	void DrawFOV(float radius, ImU32 color = IM_COL32(255, 255, 0, 80), float thickness = 2.0f);
	float AimbotFOV = 100.0f;// 自瞄视野大小
	
	

	void DrawRadar2(const CEntity& local, const std::vector<CEntity>& ents);


};
// ==== Sample3 雷达 ====
struct RadarPoint {
	float x, y;
	bool isVisible;
};

class RadarSystem {
public:
	float radarSize;
	float radarRange;
	float centerX, centerY;

	RadarSystem(float size, float range, float cx, float cy)
		: radarSize(size), radarRange(range), centerX(cx), centerY(cy) {
	}

	float degToRad(float deg);
	RadarPoint calculateRadarPosition(float localYaw, float* local_xyz, float* entity_xyz);
	RadarPoint calculateRadarPositionSafe(float localYaw, float* local_xyz, float* entity_xyz);
};