#include "OwnImGui.h"
#include "Entity.h"
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void UpdateWindowState();

const float radarSize = 140.0f;     // 雷达圆半径
const float radarScale = 0.04f;     // 世界单位 → 雷达缩放（越小越敏感）
void OwnImGui::Start(const char* ToWindowClassName,const char* ToWindowName , Draw MainFunction)
{
    int flag1 = -1, flag2 = -1;
    flag1 = strcmp(ToWindowName, "");
    flag2 = strcmp(ToWindowClassName, "");

    if (flag1 == 0 && flag2 == 0)
    {
        std::cout << "[error]: WindowInfo can't be empty." << std::endl;
        return;
    }
    if (reinterpret_cast<int>(MainFunction) <= 0)
    {
        std::cout << "[error]: MainFunction can't be null." << std::endl;
        return;
    }

    if (flag1)
        strcpy_s(WindowData::ToWindowName, strlen(ToWindowName) + 1, ToWindowName);
    if (flag2)
        strcpy_s(WindowData::ToWindowClassName, strlen(ToWindowClassName) + 1, ToWindowClassName);

    if (flag1)
    {
        if (flag2)
            WindowData::hToWindow = FindWindowA(WindowData::ToWindowClassName, WindowData::ToWindowName);
        else
            WindowData::hToWindow = FindWindowA(NULL, WindowData::ToWindowName);
    }
    else if (flag2)
    {
        WindowData::hToWindow = FindWindowA(WindowData::ToWindowClassName, NULL);
    }


    if (reinterpret_cast<int>(WindowData::hToWindow) <= 0)
    {
        std::cout << "[error]: Window isn't exist."<<std::endl;
        clean = true;
        return;
    }
    if (!CreateOwnWindow(WindowData::hToWindow))
    {
        clean = true;
        return;
    }
    Initialize();
    WindowMessageCircle(MainFunction);
}

OwnImGui::OwnImGui(const char* MenuName_)
{
    strcpy_s(MenuName, strlen(MenuName_) + 1, MenuName_);
}

OwnImGui::~OwnImGui()
{
    if (!clean)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(WindowData::hMyWindow);
        ::UnregisterClass(W_class.lpszClassName, W_class.hInstance);
    }
}

void OwnImGui::WindowMessageCircle(Draw MainFunction)
{
    MSG msg;
    while (true)
    {
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                done = true;
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        if (done)
            break;       

        UpdateWindowState();

        // Start ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        //  Main
        {
            if (MenuName == "")
            {
                Open = ImGui::Begin("Menu");
            }
            else
            {
                Open = ImGui::Begin(MenuName);
            }
            MainFunction();
            ImGui::End();
        }
        // Rendering
        ImGui::Render();

        const float clear_color_with_alpha[4] = { 0,0,0,0 };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(0, 0);
    }
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(WindowData::hMyWindow);
    ::UnregisterClass(W_class.lpszClassName, W_class.hInstance);
    clean = true;
}

void OwnImGui::Initialize()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();(void)io;

    // Setup ImGui style
    //ImGui::StyleColorsDark();
    this->MyStyle();
    ImGui::GetStyle().WindowRounding = 4;
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg]= ImVec4(0.24f, 0.23f, 0.23f, 0.5f);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", c_FontSize,0,io.Fonts->GetGlyphRangesChineseFull());

    // Setup backends
    ImGui_ImplWin32_Init(WindowData::hMyWindow);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}

bool OwnImGui::CreateOwnWindow(HWND g_hToWindow)
{
	// Create window
    USES_CONVERSION;
    WNDCLASSEX
	W_class = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW , (WNDPROC)WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, (HBRUSH)RGB(0, 0, 0), NULL, _T("ImGui"), NULL };
	RegisterClassEx(&W_class);
    WindowData::hMyWindow = CreateWindowExA(WS_EX_TRANSPARENT, W2A(W_class.lpszClassName), "DrawWindow", WS_POPUP, 1, 1, 100, 100, 0, 0, 0, 0);
    //SetLayeredWindowAttributes(g_hMyWindow, 0, 255, LWA_ALPHA);
    UpdateWindow(WindowData::hMyWindow);
    ShowWindow(WindowData::hMyWindow, SW_SHOWDEFAULT);

    // Get ToWindow size
    GetWindowRect(g_hToWindow, &WindowData::ToWindowRect);
    WindowData::width = WindowData::ToWindowRect.right - WindowData::ToWindowRect.left;
    WindowData::height = WindowData::ToWindowRect.bottom - WindowData::ToWindowRect.top;

	// Initialize d3d
	if (!CreateDeviceD3D(WindowData::hMyWindow))
	{
		CleanupDeviceD3D();
		::UnregisterClass(W_class.lpszClassName, W_class.hInstance);
        std::cout << "[error]: Create D3D Device failed." << std::endl;
        return false;
	}

	// Show the window
	::ShowWindow(WindowData::hMyWindow, SW_SHOWDEFAULT);
	::UpdateWindow(WindowData::hMyWindow);
    return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall OwnImGui::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    if (MenuActivation)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;
    }

    switch (msg)
    {
    case WM_CREATE:
        {
            MARGINS     Margin = { -1 };
            DwmExtendFrameIntoClientArea(hWnd, &Margin);
            break;
        }
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void OwnImGui::Text(const char* Text, Vec2 Pos, int color)
{
    ImGui::GetForegroundDrawList()->AddText(ImVec2(Pos.x, Pos.y), color, Text);
}

void OwnImGui::Rectangle(Vec2 Pos, int width, int height,int color, int thickness)
{
    ImGui::GetForegroundDrawList()->AddRect(ImVec2(Pos.x, Pos.y), ImVec2(Pos.x+width, Pos.y+height), color, 0, 15, thickness);
}

void OwnImGui::Line(Vec2 From, Vec2 To, int color, int thickness)
{
    ImGui::GetForegroundDrawList()->AddLine(ImVec2(From.x,From.y), ImVec2(To.x,To.y), color, thickness);
}

void OwnImGui::Circle(Vec2 Center,float radius,int color,float thickness)
{

    ImGui::GetForegroundDrawList()->AddCircle(ImVec2(Center.x,Center.y), radius, color, max(radius, 50), thickness);
}

void OwnImGui::CircleFilled(Vec2 Center, float radius, int color, int num)
{
    ImGui::GetForegroundDrawList()->AddCircleFilled(ImVec2(Center.x, Center.y), radius, color, num);
}

void OwnImGui::RectangleFilled(Vec2 Pos, int width, int height, int color)
{
    ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(Pos.x, Pos.y), ImVec2(Pos.x + width, Pos.y + height), color, 0, 0);
}

void OwnImGui::StrokeText(const char* Text, Vec2 Pos, int color_b, int color)
{
    this->Text(Text, Vec2(Pos.x - 1, Pos.y + 1), color_b);
    this->Text(Text, Vec2(Pos.x - 1, Pos.y - 1), color_b);
    this->Text(Text, Vec2(Pos.x + 1, Pos.y + 1), color_b);
    this->Text(Text, Vec2(Pos.x + 1, Pos.y - 1), color_b);
    this->Text(Text, Pos, color);
}

void UpdateWindowState()
{
    ImGuiIO& io = ImGui::GetIO();

    // Update draw window pos
    POINT TempPoint{ 0,0 };
    RECT TempRect;
    GetClientRect(WindowData::hToWindow, &TempRect);

    ClientToScreen(WindowData::hToWindow, &TempPoint);
    WindowData::ToWindowRect.left = TempPoint.x;
    WindowData::ToWindowRect.right = WindowData::ToWindowRect.left + TempRect.right;
    WindowData::ToWindowRect.top = TempPoint.y;
    WindowData::ToWindowRect.bottom = WindowData::ToWindowRect.top + TempRect.bottom;

    WindowData::width = WindowData::ToWindowRect.right - WindowData::ToWindowRect.left;
    WindowData::height = WindowData::ToWindowRect.bottom - WindowData::ToWindowRect.top;
    SetWindowPos(WindowData::hMyWindow, HWND_TOPMOST, WindowData::ToWindowRect.left, WindowData::ToWindowRect.top, WindowData::width, WindowData::height, SWP_SHOWWINDOW);

    // Check mouse status
    if (io.WantCaptureMouse)
    {
        SetWindowLong(WindowData::hMyWindow, GWL_EXSTYLE, GetWindowLong(WindowData::hMyWindow, GWL_EXSTYLE) & (~WS_EX_LAYERED));
        MenuActivation = true;
    }
    else
    {
        SetWindowLong(WindowData::hMyWindow, GWL_EXSTYLE, GetWindowLong(WindowData::hMyWindow, GWL_EXSTYLE)|WS_EX_LAYERED);
        MenuActivation = false;
    }
}

void OwnImGui::MonitorToWindowState()
{
    if (reinterpret_cast<int>(FindWindowA(NULL, WindowData::ToWindowName)) <= 0)
    {
        SendMessageA(WindowData::hMyWindow, WM_DESTROY, 0, 0);
    }
    Sleep(100);
}

void OwnImGui::ReSetFontSize(size_t FontSize)
{
    c_FontSize = FontSize;
}

void OwnImGui::ConnectPoint(std::vector<Vec2> Points, int color, float thickness)
{
    if (Points.size() <= 0)
        return;
    for (int i = 0; i < Points.size() - 1; i++)
    {
        Line(Points[i], Points[i + 1], color, thickness);
        if (i == Points.size() - 2)
            Line(Points[i + 1], Points[0], color, thickness);
    }
}

void OwnImGui::CheckboxEx(const char* str_id, bool* v)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;

    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg;
    if (ImGui::IsItemHovered())
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.86f, 0.07f, 0.23f, 1.0f), t));
    else
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.78f, 0.07f, 0.15f, 1.0f), t));

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
    ImGui::SameLine();
    ImGui::Text(str_id);
}

void OwnImGui::MyStyle()
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.84f, 0.84f, 0.84f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.70f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.15f, 0.19f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.17f, 0.21f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.17f, 0.19f, 0.23f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.22f, 0.22f, 0.28f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.19f, 0.21f, 0.28f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.30f, 0.38f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.33f, 0.41f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.23f, 0.27f, 0.34f, 0.54f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.17f, 0.21f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.14f, 0.17f, 0.21f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.22f, 0.24f, 0.27f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.17f, 0.19f, 0.21f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.43f, 0.50f, 0.57f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.33f, 0.39f, 0.46f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.32f, 0.36f, 0.42f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.30f, 0.39f, 0.49f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.32f, 0.41f, 0.51f, 0.40f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.33f, 0.41f, 0.40f);
    colors[ImGuiCol_Header] = ImVec4(0.34f, 0.39f, 0.46f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.44f, 0.50f, 0.31f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.29f, 0.34f, 0.40f, 0.31f);
    colors[ImGuiCol_Separator] = ImVec4(0.29f, 0.32f, 0.38f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.32f, 0.36f, 0.42f, 0.50f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.29f, 0.34f, 0.50f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.17f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.23f, 0.29f, 0.36f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.14f, 0.18f, 0.23f, 0.20f);
    colors[ImGuiCol_Tab] = ImVec4(0.19f, 0.21f, 0.23f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.29f, 0.35f, 0.86f);
    colors[ImGuiCol_TabActive] = ImVec4(0.26f, 0.31f, 0.38f, 0.86f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

int  OwnImGui::Color(float* Color_)
{
    return IM_COL32(Color_[0] * 255, Color_[1] * 255, Color_[2] * 255, Color_[3] * 255);
}
void OwnImGui::DrawRainbowSweepLine(const ImVec2& start, const ImVec2& end, float thickness)// 彩虹线条
{
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    static float t = 0.0f;
    t += 0.02f; // 动画速度（越大越快）

    const int segments = 40; // 分段数量（越大越细）

    for (int i = 0; i < segments; i++)
    {
        float p1 = (float)i / segments;
        float p2 = (float)(i + 1) / segments;

        ImVec2 s = {
            start.x + (end.x - start.x) * p1,
            start.y + (end.y - start.y) * p1
        };

        ImVec2 e = {
            start.x + (end.x - start.x) * p2,
            start.y + (end.y - start.y) * p2
        };

        float phase = p1 + t;

        ImColor col(
            sinf(phase * 6.283f) * 0.5f + 0.5f,        // R
            sinf((phase + 0.33f) * 6.283f) * 0.5f + 0.5f, // G
            sinf((phase + 0.66f) * 6.283f) * 0.5f + 0.5f, // B
            1.0f
        );

        draw->AddLine(s, e, col, thickness);
    }
}
void OwnImGui::DrawRainbowStaticGradientLine(const ImVec2& start, const ImVec2& end, float thickness)
{
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const int segments = 40;

    ImColor cStart(255, 0, 0, 255);
    ImColor cEnd(0, 120, 255, 255);

    for (int i = 0; i < segments; i++)
    {
        float p1 = (float)i / segments;
        float p2 = (float)(i + 1) / segments;

        ImVec2 s = { start.x + (end.x - start.x) * p1,
                     start.y + (end.y - start.y) * p1 };
        ImVec2 e = { start.x + (end.x - start.x) * p2,
                     start.y + (end.y - start.y) * p2 };

        ImColor col(
            cStart.Value.x * (1 - p1) + cEnd.Value.x * p1,
            cStart.Value.y * (1 - p1) + cEnd.Value.y * p1,
            cStart.Value.z * (1 - p1) + cEnd.Value.z * p1,
            1.0f
        );

        dl->AddLine(s, e, col, thickness);
    }
}
void OwnImGui::DrawRainbowPulseLine(const ImVec2& start, const ImVec2& end, float thickness)
{
    ImDrawList* dl = ImGui::GetForegroundDrawList();

    static float t = 0;
    t += 0.03f;

    float r = sinf(t + 0.0f) * 0.5f + 0.5f;
    float g = sinf(t + 2.0f) * 0.5f + 0.5f;
    float b = sinf(t + 4.0f) * 0.5f + 0.5f;

    ImColor col(r, g, b, 1.0f);

    dl->AddLine(start, end, col, thickness);
}
void OwnImGui::DrawRainbowWaveLine(const ImVec2& start, const ImVec2& end, float thickness)
{
    ImDrawList* dl = ImGui::GetForegroundDrawList();

    static float t = 0;
    t += 0.03f;

    const int segments = 40;

    for (int i = 0; i < segments; i++)
    {
        float p1 = (float)i / segments;
        float p2 = (float)(i + 1) / segments;

        ImVec2 s = { start.x + (end.x - start.x) * p1,
                     start.y + (end.y - start.y) * p1 };
        ImVec2 e = { start.x + (end.x - start.x) * p2,
                     start.y + (end.y - start.y) * p2 };

        float wave = sinf((p1 * 8.0f) + t); // 8 波数
        ImColor col(
            (wave * 0.5f + 0.5f),
            (sinf(wave + 2.0f) * 0.5f + 0.5f),
            (sinf(wave + 4.0f) * 0.5f + 0.5f),
            1.0f
        );

        dl->AddLine(s, e, col, thickness);
    }
}
void OwnImGui::DrawLineStyleSelector()
{
    const char* items[] = {
        "Rainbow Sweep",
        "Static Gradient",
        "Pulse RGB",
        "Wave RGB"
    };

    ImGui::Combo("Line Style", &LineStyle, items, IM_ARRAYSIZE(items));
}
void OwnImGui::DrawRadarStyleSelector()
{
    const char* items[] = {
        "Sample 1",
        "Sample 2",
		"Sample 3",
    };

    ImGui::Combo("Radar Style", &RadarStyle, items, IM_ARRAYSIZE(items));
}
void OwnImGui::DrawAimbotStyleSelector()
{
    const char* items[] = {
        "by Distance",
        "by Crosshair",
        "by Fov",
    };
	ImGui::Combo("Aimbot Style", &AimbotStyle, items, IM_ARRAYSIZE(items));
}
void OwnImGui::DrawBoxStyleSelector()
{
    const char* items[] = {
        "Full Box",
        "Corner Box",
        "3D Box",
    };
	ImGui::Combo("Box Style", &BoxStyle, items, IM_ARRAYSIZE(items));
}
void OwnImGui::DrawHealthBar(const Vec2& rectPos, float rectHeight, int health, int maxHealth)
{
    if (health <= 0 || maxHealth <= 0)
        return;

    float healthPercent = static_cast<float>(health) / maxHealth;

    int green = static_cast<int>(healthPercent * 255);
    int red = 255 - green;
    int healthColor = IM_COL32(red, green, 0, 255);

    float filledHeight = rectHeight * healthPercent;

    // 血条从底部往上绘制
    float startY = rectPos.y + rectHeight - filledHeight;

    ImGui::GetForegroundDrawList()->AddRectFilled(
        ImVec2(rectPos.x - 6, startY),
        ImVec2(rectPos.x - 6 + 4, startY + filledHeight),
        healthColor
    );
}
void OwnImGui::DrawDistance(const Vec2& pos, float distance, ImU32 color)
{
    // 距离小于 0，不绘制
    if (distance < 0.0f)
        return;

    char buffer[32];
    // 格式化为米（保留 1 位小数）
    sprintf_s(buffer, "%.1fm", distance);

    ImGui::GetForegroundDrawList()->AddText(ImVec2(pos.x, pos.y - 15),color,buffer);
}
ImU32 OwnImGui::DistanceColor(float distance)
{
    // 最大距离：你可以改
    const float MaxDist = 200.f;

    float t = std::clamp(distance / MaxDist, 0.0f, 1.0f);

    // 近 = 红, 中 = 黄, 远 = 绿
    int r = (int)(255 * (1.0f - t));
    int g = (int)(255 * t);
    int b = 0;

    return IM_COL32(r, g, b, 255);
}
// 优化版：增加黑色描边，更清晰
void OwnImGui::DrawDirectionArrow(float ex, float ey, ImU32 color, float size)
{
    if (size <= 0) size = 30.0f;
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    float cx = WindowData::width * 0.5f;
    float cy = WindowData::height * 0.5f;

    float dx = ex - cx;
    float dy = ey - cy;

    float angle = atan2f(dy, dx);

    // 箭头中心位置（贴边）
    float radius = min(WindowData::width, WindowData::height) * 0.5f - size - 10;
    float px = cx + cos(angle) * radius;
    float py = cy + sin(angle) * radius;

    // 三角形三个点
    ImVec2 tip(px + cos(angle) * size,        // 尖端
        py + sin(angle) * size);

    ImVec2 left(px + cos(angle + 2.5f) * size * 0.6f,
        py + sin(angle + 2.5f) * size * 0.6f);

    ImVec2 right(px + cos(angle - 2.5f) * size * 0.6f,
        py + sin(angle - 2.5f) * size * 0.6f);

    // 黑色描边（更清晰）
    draw->AddTriangleFilled(tip, left, right, IM_COL32(0, 0, 0, 200));
    // 内部三角颜色
    draw->AddTriangleFilled({ tip.x, tip.y },{ left.x, left.y },{ right.x, right.y },color);
}
void OwnImGui::RoundRect(const Vec2& pos, float w, float h, float r, ImU32 color, float thickness)
{
    ImGui::GetForegroundDrawList()->AddRect(
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + w, pos.y + h),
        color,
        r,                        // 圆角大小
        0,
        thickness
    );
}

void OwnImGui::RoundRectFilled(const Vec2& pos, float w, float h, float r, ImU32 color)
{
    ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(pos.x, pos.y),ImVec2(pos.x + w, pos.y + h), color,r);
}
void OwnImGui::DrawRadarBackground(float size)
{
    float x = WindowData::width - size - 20;
    float y = 20;

    ImGui::GetForegroundDrawList()->AddRectFilled(
        ImVec2(x, y),
        ImVec2(x + size, y + size),
        IM_COL32(20, 20, 20, 180),
        5.0f
    );

    ImGui::GetForegroundDrawList()->AddRect(
        ImVec2(x, y),
        ImVec2(x + size, y + size),
        IM_COL32(255, 255, 255, 200),
        5.0f,
        0,
        2.0f
    );
}

void OwnImGui::DrawRadarPoint(const CEntity& ent, const CEntity& local, float size, ImU32 color)
{
    float radarX = WindowData::width - size - 20;
    float radarY = 20;
    float half = size * 0.5f;

    Vec3 delta = {
        ent.fPos.x - local.fPos.x,
        ent.fPos.y - local.fPos.y,
        ent.fPos.z - local.fPos.z
    };

    float scale = 0.15f;

    float px = radarX + half + delta.x * scale;
    float py = radarY + half + delta.y * scale;

    px = std::clamp(px, radarX, radarX + size);
    py = std::clamp(py, radarY, radarY + size);

    ImGui::GetForegroundDrawList()->AddCircleFilled(
        ImVec2(px, py),
        4.0f,
        color
    );
}

void OwnImGui::DrawRadar(const CEntity& local, const std::vector<CEntity>& ents)
{
    float size = 160.0f;

    DrawRadarBackground(size);

    for (auto& e : ents)
    {
        if (e.Health <= 0 || local.Camp == e.Camp) continue;

        DrawRadarPoint(e, local, size, IM_COL32(255, 0, 0, 255));
    }
}
void OwnImGui::DrawRadar1(const CEntity& local, const std::vector<CEntity>& ents)
{
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    // 雷达中心（你可放到屏幕任意位置）
    float cx = 150.0f;
    float cy = 150.0f;

    float radius = 120.0f;   // 雷达半径
    float scale = 3.0f;    // World → Radar 缩放（越大敌人越靠近中心）

    // --- 雷达背景 ---
    draw->AddCircleFilled({ cx, cy }, radius, IM_COL32(15, 15, 15, 180)); // 灰色背景
    draw->AddCircle({ cx, cy }, radius, IM_COL32(255, 255, 255, 80), 64, 1.5f); // 外圈

    // ---- 中间十字线（可选）----
    draw->AddLine({ cx - radius, cy }, { cx + radius, cy }, IM_COL32(255, 255, 255, 50));
    draw->AddLine({ cx, cy - radius }, { cx, cy + radius }, IM_COL32(255, 255, 255, 50));

    // --- 玩家箭头（三角形）---
    float yawRad = local.AngleX * (3.1415926f / 180.f);

    ImVec2 p1 = { cx + cos(yawRad) * 12, cy + sin(yawRad) * 12 };
    ImVec2 p2 = { cx + cos(yawRad + 2.5f) * 8, cy + sin(yawRad + 2.5f) * 8 };
    ImVec2 p3 = { cx + cos(yawRad - 2.5f) * 8, cy + sin(yawRad - 2.5f) * 8 };

    draw->AddTriangleFilled(p1, p2, p3, IM_COL32(0, 255, 255, 255));

    // --- 旋转敌人点位（让雷达随玩家旋转）---
    float rotate = -yawRad;

    for (auto& e : ents)
    {
        if (e.Health <= 0 || e.Health > 100)
            continue;
        if (e.Camp == local.Camp)
            continue;

        // 取得2D平面差值（忽略高度）
        float dx = e.fPos.x - local.fPos.x;
        float dy = e.fPos.y - local.fPos.y;

        // 旋转（重要：CS2雷达随着你旋转而转）
        float rx = dx * cos(rotate) - dy * sin(rotate);
        float ry = dx * sin(rotate) + dy * cos(rotate);

        // 缩放
        rx *= scale;
        ry *= scale;

        // 限制敌人点不能超出雷达边界（CS2 同款）
        float dist = sqrt(rx * rx + ry * ry);
        if (dist > radius - 8)
        {
            rx = rx / dist * (radius - 8);
            ry = ry / dist * (radius - 8);
        }

        ImVec2 pos = { cx + rx, cy + ry };

        // 画敌人点
        draw->AddCircleFilled(pos, 4, IM_COL32(255, 60, 60, 255));  // 红点
        draw->AddCircle(pos, 4, IM_COL32(0, 0, 0, 255));            // 黑色描边
    }
}
void OwnImGui::DrawFOV(float radius, ImU32 color, float thickness)
{
    float cx = WindowData::width / 2.0f;
    float cy = WindowData::height / 2.0f;

    ImDrawList* draw = ImGui::GetForegroundDrawList();
    draw->AddCircle(ImVec2(cx, cy), radius, color, 64, thickness);
}
float RadarSystem::degToRad(float deg)
{
    return deg * 3.1415926535f / 180.0f;
}

RadarPoint RadarSystem::calculateRadarPosition(
    float localYaw,
    float* local_xyz,
    float* entity_xyz
)
{
    RadarPoint result;

    float deltaX = entity_xyz[0] - local_xyz[0];
    float deltaY = entity_xyz[1] - local_xyz[1];
    float distance = sqrt(deltaX * deltaX + deltaY * deltaY);

    if (distance > radarRange) {
        result.isVisible = false;
        return result;
    }

    float angle = atan2(deltaY, deltaX);
    float adjustedAngle = angle - degToRad(localYaw);

    float radarDistance = (distance / radarRange) * (radarSize / 2.0f);

    result.x = centerX + radarDistance * cos(adjustedAngle);
    result.y = centerY + radarDistance * sin(adjustedAngle);
    result.isVisible = true;

    return result;
}

RadarPoint RadarSystem::calculateRadarPositionSafe(
    float localYaw,
    float* local_xyz,
    float* entity_xyz
)
{
    RadarPoint result = calculateRadarPosition(localYaw, local_xyz, entity_xyz);
    if (!result.isVisible)
        return result;

    float half = radarSize / 2.0f;

    result.x = std::clamp(result.x, centerX - half, centerX + half);
    result.y = std::clamp(result.y, centerY - half, centerY + half);

    return result;
}




void OwnImGui::DrawRadar2(const CEntity& local, const std::vector<CEntity>& ents)
{
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    float size = 160.0f;     // 雷达直径
    float range = 50.0f;     // 50 世界单位

    float cx = WindowData::width - size - 20 + size / 2.0f;
    float cy = 20 + size / 2.0f;

    // === 雷达背景 ===
    draw->AddCircleFilled({ cx, cy }, size / 2.0f, IM_COL32(20, 20, 20, 200));
    draw->AddCircle({ cx, cy }, size / 2.0f, IM_COL32(255, 255, 255, 180), 64, 2.0f);

    // 十字线
    draw->AddLine({ cx - size / 2, cy }, { cx + size / 2, cy }, IM_COL32(255, 255, 255, 40));
    draw->AddLine({ cx, cy - size / 2 }, { cx, cy + size / 2 }, IM_COL32(255, 255, 255, 40));

    // 本地玩家点（青色）
    draw->AddCircleFilled({ cx, cy }, 4.0f, IM_COL32(0, 255, 255, 255));

    // 创建雷达系统
    RadarSystem radar(size, range, cx, cy);

    float local_xyz[3] = { local.fPos.x, local.fPos.y, local.fPos.z };

    for (auto& e : ents)
    {
        if (e.Health <= 0 || e.Health > 100) continue;
        if (e.Camp == local.Camp) continue;

        float entity_xyz[3] = { e.fPos.x, e.fPos.y, e.fPos.z };

        RadarPoint pt = radar.calculateRadarPosition(local.AngleX, local_xyz, entity_xyz);
        if (!pt.isVisible) continue;

        // 根据血量变化颜色
        ImU32 c = IM_COL32(255, 60, 60, 255);
        if (e.Health > 75) c = IM_COL32(0, 255, 0, 255);
        else if (e.Health > 25) c = IM_COL32(255, 255, 0, 255);

        draw->AddCircleFilled(ImVec2(pt.x, pt.y), 4.0f, c);
        draw->AddCircle(ImVec2(pt.x, pt.y), 4.0f, IM_COL32(0, 0, 0, 255));
    }
}
void OwnImGui::Draw3DBox(const CEntity& ent, float Matrix[4][4], ImU32 color)
{
    Vec3 corners3D[8];
    ent.Get3DBoxCorners(corners3D);

    Vec3 corners2D[8];
    for (int i = 0; i < 8; i++)
    {
        if (!ent.WorldToScreenPoint(corners3D[i], Matrix, corners2D[i]))
            return;
    }

    ImDrawList* dl = ImGui::GetForegroundDrawList();

    int edges[12][2] =
    {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    for (auto& e : edges)
    {
        dl->AddLine(
            ImVec2(corners2D[e[0]].x, corners2D[e[0]].y),
            ImVec2(corners2D[e[1]].x, corners2D[e[1]].y),
            color,
            2.0f
        );
    }
}