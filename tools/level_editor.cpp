#include "../game.hpp"

static constexpr int ZOOM = 14;

static constexpr int WW = 128 * ZOOM;
static constexpr int WH = 64 * ZOOM;

#define IMGUI_IMPLEMENTATION
#include <misc/single_file/imgui_single_file.h>

// Dear ImGui: standalone example application for DirectX 9
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include <d3d9.h>
#include <tchar.h>

#pragma comment(lib, "d3d9.lib")

#include <vector>
#include <string>
#include <cmath>

#define FMT_HEADER_ONLY
#include <fmt/format.h>


static std::vector<phys_box> editor_boxes[IM_ARRAYSIZE(LEVELS)];
static level_info editor_levels[IM_ARRAYSIZE(LEVELS)];
static int level_index = 0;

static ImVec2 dvec2imvec(dvec2 v)
{
    return { float(v.x) / (128 * 16) * WW, float(v.y) / (64 * 16) * WH };
}

static ImVec2 dvec2imvec(dvec3 v)
{
    return dvec2imvec(dvec2{ v.x, v.y });
}

static void input_coord(char const* label, int16_t& v)
{
    double fv = float(v) / 256;
    ImGui::InputDouble(label, &fv, 1.0, 1.0);
    fv = tclamp<double>(fv, -125, 125);
    v = (int16_t)round(fv * 256);
}

static void input_yaw(char const* label, uint8_t& v)
{
    int iv = v;
    ImGui::InputInt(label, &iv, 1, 4);
    v = uint8_t(iv);
}

static void input_pitch(char const* label, int8_t& v)
{
    int iv = v;
    ImGui::InputInt(label, &iv, 1, 4);
    iv = tclamp<int>(iv, -64, 64);
    v = int8_t(iv);
}

static double c2f(int16_t x)
{
    return double(x) / 256;
}

static int editor_boxi = 0;

static std::string editor_get_string()
{
    std::string r;
    auto const& boxes = editor_boxes[level_index];

    r += "#pragma once\n\n";
    r += fmt::format(
        "static constexpr phys_box LEVEL_{:02d}_BOXES[{}] PROGMEM =\n{{\n",
        level_index, (int)boxes.size());
    for(auto const& box : boxes)
    {
        r += fmt::format(
            "    {{ {{ {}, {}, {} }}, {{ {}, {}, {} }}, {}, {} }},\n",
            box.size.x, box.size.y, box.size.z,
            box.pos.x, box.pos.y, box.pos.z,
            box.yaw, box.pitch);
    }
    r += "};\n";

    return r;
}

static void editor_gui()
{
    using namespace ImGui;
    SetNextWindowSize({ 500, 600 }, ImGuiCond_FirstUseEver);
    Begin("Editor");
    InputInt("Level Index", &level_index);
    if(Button("Copy level info to clipboard"))
    {
        std::string s = editor_get_string();
        SetClipboardText(s.c_str());
    }
    level_index = tclamp(level_index, 0, (int)IM_ARRAYSIZE(LEVELS) - 1);
    current_level = &editor_levels[level_index];
    auto& level = editor_levels[level_index];
    int num_boxes = (int)editor_boxes[level_index].size();

    std::vector<std::string> boxstrs;
    std::vector<char const*> boxstrs2;
    for(int i = 0; i < num_boxes; ++i)
    {
        phys_box const& box = editor_boxes[level_index][i];
        boxstrs.push_back(fmt::format(
            "Box {:02d}: {:4.1f} {:4.1f} {:4.1f}   {}/{}",
            i,
            //c2f(box.pos.x), c2f(box.pos.y), c2f(box.pos.z),
            c2f(box.size.x), c2f(box.size.y), c2f(box.size.z),
            box.yaw, box.pitch));
    }
    for(size_t i = 0; i < boxstrs.size(); ++i)
        boxstrs2.push_back(boxstrs[i].c_str());
    auto& boxes = editor_boxes[level_index];
    if(Button("Add Box"))
        boxes.push_back({});
    SameLine();
    if(Button("Remove Box"))
    {
        boxes.erase(boxes.begin() + editor_boxi);
        editor_boxi = tclamp(editor_boxi, 0, (int)boxes.size() - 1);
    }
    ListBox("##Boxes", &editor_boxi, boxstrs2.data(), (int)boxstrs2.size());
    //SameLine();
    //BeginGroup();
    if(editor_boxi >= 0 && editor_boxi < num_boxes)
    {
        phys_box& box = boxes[editor_boxi];
        input_coord("x pos", box.pos.x);
        input_coord("y pos", box.pos.y);
        input_coord("z pos", box.pos.z);
        input_coord("x size", box.size.x);
        input_coord("y size", box.size.y);
        input_coord("z size", box.size.z);
        input_yaw  ("x yaw", box.yaw);
        input_pitch("x pitch", box.pitch);
    }
    //EndGroup();
    End();
}

static void editor_draw_boxes()
{
    auto* draw = ImGui::GetBackgroundDrawList();

    static int const CORNERS[8 * 3] =
    {
        -1, -1, -1,
        -1, -1, +1,
        -1, +1, -1,
        -1, +1, +1,
        +1, -1, -1,
        +1, -1, +1,
        +1, +1, -1,
        +1, +1, +1,
    };

    auto const& boxes = editor_boxes[level_index];
    for(int j, i = 0; i < (int)boxes.size(); ++i)
    {
        auto const& box = boxes[i];
        dvec3 p[8];
        dvec3 size = box.size;
        mat3 m;
        rotation_phys(m, box.yaw, box.pitch);
        for(j = 0; j < 8; ++j)
        {
            p[j] = box.pos;
            dvec3 t = box.size;
            t.x *= CORNERS[j * 3 + 0];
            t.y *= CORNERS[j * 3 + 1];
            t.z *= CORNERS[j * 3 + 2];
            t = matvec(m, t);
            p[j].x += t.x;
            p[j].y += t.y;
            p[j].z += t.z;
            p[j] = transform_point(p[j]);
            if(p[j].z < 128) break;
        }
        if(j < 8) continue;

        ImU32 col;
        if(i == editor_boxi)
            col = ImGui::ColorConvertFloat4ToU32(ImVec4(1, 0, 0, 1));
        else
            col = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 1, 1));

        draw->AddLine(dvec2imvec(p[0]), dvec2imvec(p[1]), col, 3.f);
        draw->AddLine(dvec2imvec(p[0]), dvec2imvec(p[2]), col, 3.f);
        draw->AddLine(dvec2imvec(p[1]), dvec2imvec(p[3]), col, 3.f);
        draw->AddLine(dvec2imvec(p[2]), dvec2imvec(p[3]), col, 3.f);

        draw->AddLine(dvec2imvec(p[4]), dvec2imvec(p[5]), col, 3.f);
        draw->AddLine(dvec2imvec(p[4]), dvec2imvec(p[6]), col, 3.f);
        draw->AddLine(dvec2imvec(p[5]), dvec2imvec(p[7]), col, 3.f);
        draw->AddLine(dvec2imvec(p[6]), dvec2imvec(p[7]), col, 3.f);

        draw->AddLine(dvec2imvec(p[0]), dvec2imvec(p[4]), col, 3.f);
        draw->AddLine(dvec2imvec(p[1]), dvec2imvec(p[5]), col, 3.f);
        draw->AddLine(dvec2imvec(p[2]), dvec2imvec(p[6]), col, 3.f);
        draw->AddLine(dvec2imvec(p[3]), dvec2imvec(p[7]), col, 3.f);
    }
}

uint8_t poll_btns() { return 0; }

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Arduboy Minigolf Editor"), WS_OVERLAPPEDWINDOW, 100, 100, WW, WH, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if(!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    memcpy(editor_levels, LEVELS, sizeof(LEVELS));
    for(int i = 0; i < (int)IM_ARRAYSIZE(LEVELS); ++i)
    {
        editor_boxes[i].resize(LEVELS[i].num_boxes);
        memcpy(editor_boxes[i].data(), LEVELS[i].boxes,
            editor_boxes[i].size() * sizeof(phys_box));
        editor_levels[i].boxes = editor_boxes[i].data();
    }

    current_level = &editor_levels[level_index];
    yaw = pitch = 0;
    cam = { 0, 8 * 256, 24 * 256 };
    ball = { 256 * 16, 256 * 2, 256 * 1 };

    // Main loop
    bool done = false;
    while(!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        MSG msg;
        while(::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if(msg.message == WM_QUIT)
                done = true;
        }
        if(done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        int look_speed = 128;
        int move_speed = 64;

        if(ImGui::IsKeyDown(ImGuiKey_A))
        {
            if(ImGui::IsKeyDown(ImGuiKey_UpArrow   )) look_up(look_speed);
            if(ImGui::IsKeyDown(ImGuiKey_DownArrow )) look_up(-look_speed);
            if(ImGui::IsKeyDown(ImGuiKey_LeftArrow )) look_right(-look_speed);
            if(ImGui::IsKeyDown(ImGuiKey_RightArrow)) look_right(look_speed);
        }
        else if(ImGui::IsKeyDown(ImGuiKey_B))
        {
            if(ImGui::IsKeyDown(ImGuiKey_UpArrow  )) move_up(move_speed);
            if(ImGui::IsKeyDown(ImGuiKey_DownArrow)) move_up(-move_speed);
        }
        else
        {
            if(ImGui::IsKeyDown(ImGuiKey_UpArrow   )) move_forward(move_speed);
            if(ImGui::IsKeyDown(ImGuiKey_DownArrow )) move_forward(-move_speed);
            if(ImGui::IsKeyDown(ImGuiKey_LeftArrow )) move_right(-move_speed);
            if(ImGui::IsKeyDown(ImGuiKey_RightArrow)) move_right(move_speed);
        }

        ImGui::ShowDemoWindow(nullptr);

        editor_gui();

        {
            auto const* faces = current_level->faces;
            uint8_t nf = render_scene();
            auto* draw = ImGui::GetBackgroundDrawList();
            for(uint8_t i = 0; i < nf; ++i)
            {
                uint8_t t = face_order[i];
                uint8_t i0, i1, i2, pt;
                uint16_t j = uint16_t(t) * 4;
                if(t < MAX_FACES)
                {
                    // normal face
                    i0 = pgm_read_byte(&faces[j + 0]);
                    i1 = pgm_read_byte(&faces[j + 1]);
                    i2 = pgm_read_byte(&faces[j + 2]);
                    pt = pgm_read_byte(&faces[j + 3]);
                }
                else if(t == 255)
                {
                    // ball
                    //if(ball_valid)
                    //{
                    //    dvec2 c = vs[balli0];
                    //    ballr = uint16_t(vs[balli1].x - c.x);
                    //    draw_ball_filled(vs[balli0], ballr);
                    //}
                    continue;
                }
                else
                {
                    // clip face
                    i0 = clip_faces[j - MAX_FACES * 4 + 0];
                    i1 = clip_faces[j - MAX_FACES * 4 + 1];
                    i2 = clip_faces[j - MAX_FACES * 4 + 2];
                    pt = clip_faces[j - MAX_FACES * 4 + 3];
                }
                static ImU32 const COLORS[5] =
                {
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.00f, 0.00f, 0.00f, 1.f)),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.25f, 0.25f, 0.25f, 1.f)),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.50f, 0.50f, 0.50f, 1.f)),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.75f, 0.75f, 0.75f, 1.f)),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(1.00f, 1.00f, 1.00f, 1.f)),
                };
                draw->AddTriangleFilled(
                    dvec2imvec(vs[i0]),
                    dvec2imvec(vs[i1]),
                    dvec2imvec(vs[i2]),
                    COLORS[pt]);
            }

            editor_draw_boxes();

            // ball
            {
                auto c = transform_point(ball);
                if(c.z >= 128)
                {
                    draw->AddCircleFilled(
                        dvec2imvec(c),
                        16.f,
                        ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.f, 0.5f, 0.5f)));
                }
            }
        }

#if 0
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if(show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if(ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if(show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if(ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
#endif

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if(g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if(result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if(g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if(g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if(hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch(msg)
    {
    case WM_SIZE:
        if(g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}