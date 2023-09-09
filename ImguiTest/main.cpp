// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs
#define NOMINMAX
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#pragma comment(lib, "d3d11.lib")
#include "RenderManager.h"
#include "ImageClass.h"
// user includes
#include <DirectXTex.h>
#include "emoji_slider.h"

#include "Profiler.h"
#include "OutputFormatters.h"

//#########################################################
//################ USER FUNCTIONS #########################
//#########################################################
void DrawMenu();


//#########################################################
//################ MAIN LOOP ##############################
//#########################################################

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// Main code
int main(int, char**)
{
    SCOPED_PROFILER("main");
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);
    RenderManager* manager = RenderManager::GetInstance();
    // Initialize Direct3D
    if (!manager->CreateDeviceD3D(hwnd))
    {
        manager->CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    manager->InitImGui();

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;
        manager->MainRenderLoop(DrawMenu);
    }

    manager->Shutdown();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        RenderManager::GetInstance()->GetResizeWidth() = (UINT)LOWORD(lParam); // Queue resize
        RenderManager::GetInstance()->GetResizeHeight() = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}


//#########################################################
//################ DRAW MENU ##############################
//#########################################################
#include <mutex>
#include <filesystem>
#include <iostream>
#include <wincodec.h>
#include <utility>
#include <Windows.h>




#include <map>
ImGuiImage test;
ImTextureID myIconID;
DirectX::TexMetadata iconMetadata;
std::once_flag flag;


static std::map<ImGuiID, float> padding_anim;
static std::vector<ImVec2> sparkle_positions;
static std::vector<ImVec2> sparkle_sizes;


bool EmojiSliderWithLabel(const char* label, float* value, float min, float max, ImTextureID knobTexture, ImTextureID starTexture, float knobRadius = 20, ImGuiSliderFlags flags = 0)
{
    bool value_changed = false; // Declare and initialize value_changed

    ImGuiIO& io = ImGui::GetIO();
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImGuiID id = window->GetID(label);
    const float w = ImGui::CalcItemWidth();
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImGui::Dummy(ImVec2(0.0f, label_size.y));
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect total_bb(frame_bb.Min - ImVec2(0, label_size.y + style.ItemInnerSpacing.y), frame_bb.Max);

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id, &frame_bb))
        return false;

    // Check if the mouse is hovering over the slider
    bool is_mouse_hovering_slider = ImGui::IsMouseHoveringRect(frame_bb.Min, frame_bb.Max);

    // Tabbing or CTRL-clicking on Slider turns it into an input box
    const bool hovered = ImGui::ItemHoverable(frame_bb, id, 0);
    const bool hovered_plus = ImGui::ItemHoverable(total_bb, id, 0);

    const bool temp_input_allowed = false;
    bool temp_input_is_active = temp_input_allowed && ImGui::TempInputIsActive(id);

    // Process mouse interaction only if the mouse is hovering over the slider
    if (is_mouse_hovering_slider)
    {
        if (!temp_input_is_active)
        {
            const bool focus_requested = temp_input_allowed && ImGui::FocusableItemRegister(window, id);
            const bool clicked = (hovered && g.IO.MouseClicked[0]);
            if (focus_requested || clicked || g.NavActivateId == id || g.NavId == id)
            {
                ImGui::SetActiveID(id, window);
                ImGui::SetFocusID(id, window);
                ImGui::FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
                if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavId == id))
                {
                    temp_input_is_active = true;
                    ImGui::FocusableItemUnregister(window);
                }
            }
        }

        if (temp_input_is_active)
        {
            // Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
            const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
            value_changed = ImGui::TempInputScalar(frame_bb, id, label, ImGuiDataType_Float, value, "%.3f", is_clamp_input ? &min : NULL, is_clamp_input ? &max : NULL);
        }

        // Draw frame
        const ImU32 frame_col = ImGui::GetColorU32(ImGuiCol_FrameBg);
        ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

        // Slider behavior
        ImRect grab_bb;
        value_changed |= ImGui::SliderBehavior(frame_bb, id, ImGuiDataType_Float, value, &min, &max, "%.3f", flags, &grab_bb);
        if (value_changed)
            ImGui::MarkItemEdited(id);

        if (grab_bb.Max.x > grab_bb.Min.x)
            ImGui::RenderFrame(frame_bb.Min, ImVec2(grab_bb.Max.x, frame_bb.Max.y), ImGui::GetColorU32(ImGuiCol_FrameBgActive), true, g.Style.FrameRounding);
    }

    // Calculate the position of the circular knob
    float t = (*value - min) / (max - min);
    ImVec2 knob_pos = ImVec2(frame_bb.Min.x + t * (frame_bb.GetWidth() - knobRadius * 2) + knobRadius, frame_bb.GetCenter().y);
    float knob_radius = knobRadius;

    Rotation::ImRotateStart();
    // Draw the circular knob with the provided emoji image
    ImGui::GetWindowDrawList()->AddImage(knobTexture, knob_pos - ImVec2(knob_radius, knob_radius), knob_pos + ImVec2(knob_radius, knob_radius), ImVec2(0, 0), ImVec2(1, 1));
    Rotation::ImRotateEnd(DirectX::XM_PI);
    // Display value using user-provided display format so the user can add prefix/suffix/decorations to the value.
    char value_buf[64];
    const char* value_buf_end = value_buf + ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), ImGuiDataType_Float, value, "%.3f");

    // Label
    if (label_size.x > 0)
        ImGui::RenderText(ImVec2(frame_bb.Min.x, frame_bb.Min.y - style.ItemInnerSpacing.y - label_size.y), label);

    // Value size
    const ImVec2 value_size = ImGui::CalcTextSize(value_buf, value_buf_end, true);

    auto it_padding = padding_anim.find(id);
    if (it_padding == padding_anim.end())
    {
        padding_anim.insert({ id, {0.f} });
        it_padding = padding_anim.find(id);
    }

    it_padding->second = ImClamp(it_padding->second + (2.5f * ImGui::GetIO().DeltaTime * (hovered_plus || ImGui::GetActiveID() == id ? 1.f : -1.f)), 0.f, 1.f);

    // Value
    if (value_size.x > 0.0f && it_padding->second > 0.f)
    {
        auto value_col = ImGui::GetColorU32(ImGuiCol_FrameBg, it_padding->second);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, it_padding->second);

        char window_name[16];
        ImFormatString(window_name, IM_ARRAYSIZE(window_name), "##tp_%s", label);

        ImGui::SetNextWindowPos(frame_bb.Max - ImVec2(frame_bb.Max.x - frame_bb.Min.x, 0) / 2 - ImVec2(value_size.x / 2 + 3.f, 0.f));
        ImGui::SetNextWindowSize((frame_bb.Max - ImVec2(frame_bb.Max.x - frame_bb.Min.x, 0) / 2 + ImVec2(value_size.x / 2 + 3.f, value_size.y + 6)) - (frame_bb.Max - ImVec2(frame_bb.Max.x - frame_bb.Min.x, 0) / 2 - ImVec2(value_size.x / 2 + 3.f, 0.f)));
        ImGui::SetNextWindowBgAlpha(it_padding->second);

        ImGuiWindowFlags flags = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration;

        ImGui::Begin(window_name, NULL, flags);

        ImGui::RenderFrame(frame_bb.Min + ImVec2(0.f, (frame_bb.Max.y - frame_bb.Min.y)), frame_bb.Min + ImVec2((frame_bb.Max.x - frame_bb.Min.x), (frame_bb.Max.y - frame_bb.Min.y) + value_size.y + 6), ImGui::GetColorU32(ImGuiCol_FrameBg), ImGui::GetStyle().FrameRounding);
        ImGui::RenderTextClipped(frame_bb.Min + ImVec2(0.f, (frame_bb.Max.y - frame_bb.Min.y)), frame_bb.Min + ImVec2((frame_bb.Max.x - frame_bb.Min.x), (frame_bb.Max.y - frame_bb.Min.y) + value_size.y + 6), value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

        ImGui::End();

        ImGui::PopStyleVar();
    }

    // Add sparkle effect when the value is changing
    if (value_changed)
    {
        // Generate sparkles at the knob position
        sparkle_positions.push_back(knob_pos);
        sparkle_sizes.emplace_back(ImVec2(knobRadius *1.5, knobRadius *1.5));
    }

    // Update and render sparkles
    for (int i = 0; i < sparkle_positions.size(); ++i)
    {
        ImVec2& sparkle_pos = sparkle_positions[i];
        ImVec2& sparkle_size = sparkle_sizes[i];

        // Move the sparkle upward
        sparkle_pos.y -= ImGui::GetIO().DeltaTime * 50.0f;

        // Shrink the sparkle
        sparkle_size -= ImVec2(ImGui::GetIO().DeltaTime * 10.0f, ImGui::GetIO().DeltaTime * 10.0f);

        if (sparkle_size.x <= 0.0f && sparkle_size.y <= 0.0f)
        {
            // Remove expired sparkles
            sparkle_positions.erase(sparkle_positions.begin() + i);
            sparkle_sizes.erase(sparkle_sizes.begin() + i);
            --i;
            continue;
        }

        // Draw the sparkle
        const float sparkle_alpha = it_padding->second * 0.5f; // Adjust sparkle alpha based on padding animation
        //ImGui::GetWindowDrawList()->AddCircleFilled(sparkle_pos, sparkle_size, IM_COL32(255, 255, 0, static_cast<int>(255 * sparkle_alpha)));
        ImGui::GetWindowDrawList()->AddImage(starTexture, ImVec2(sparkle_pos.x - sparkle_size.x / 2, sparkle_pos.y - sparkle_size.y / 2), ImVec2(sparkle_pos.x + sparkle_size.x / 2, sparkle_pos.y + sparkle_size.y / 2));
    }

    return value_changed;
}

enum class Edge
{
    Top,
    Right,
    Bottom,
    Left
};




void MoveEmojiAlongBorder(float& xPos, float& yPos, ImTextureID emoji, Edge& currentEdge)
{
    const char* text = "";
    ImGui::Text(text);
    float speed = 500.0f;
    float deltaTime = ImGui::GetIO().DeltaTime;

    float windowWidth = ImGui::GetWindowSize().x;
    float windowHeight = ImGui::GetWindowSize().y;
    float windowPosX = ImGui::GetWindowPos().x;
    float windowPosY = ImGui::GetWindowPos().y;



    ImVec2 rectSize = ImVec2(50, 50);


    switch (currentEdge)
    {
    case Edge::Top:
        text = "Top";
        yPos -= speed * deltaTime;
        if (yPos < windowPosY)
        {
            yPos = windowPosY;
            currentEdge = Edge::Right;
        }
        break;

    case Edge::Right:
        text = "Right";
        xPos += speed * deltaTime;
        if (xPos + rectSize.x > windowPosX + windowWidth)
        {
            xPos = windowPosX + windowWidth - rectSize.x;
            currentEdge = Edge::Bottom;
        }
        break;

    case Edge::Bottom:
        text = "Bottom";
        yPos += speed * deltaTime;
        if (yPos + rectSize.y > windowPosY + windowHeight)
        {
            yPos = windowPosY + windowHeight - 50;
            currentEdge = Edge::Left;
        }
        break;

    case Edge::Left:
        text = "Left";
        xPos -= speed * deltaTime;
        if (xPos < windowPosX)
        {
            xPos = windowPosX;
            currentEdge = Edge::Top;
        }
        break;
    }

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec2 rectMin = ImVec2(xPos, yPos);
    ImVec2 rectMax = ImVec2(xPos + 50, yPos + 50);
    window->DrawList->AddImage(emoji, rectMin, rectMax);
}


ImGuiImage star;
float test_float;
int knob_radius;
ImVec2 pos_rect;
Edge CurrentEdge;
void DrawMenu()
{
    
    SCOPED_PROFILER("DrawMenu")
    ImGui::ShowStyleEditor();
    ImGui::Begin("Hello");
    
    std::call_once(flag, []() {
        SCOPED_PROFILER("LambdaFunction");
        
        ID3D11Device* g_pd3dDevice = RenderManager::GetInstance()->GetDevice();
        test = ImGuiImage(L"icon.png");
        star = ImGuiImage(L"star.png");

        test.Resize(64, 64);
        DirectX::TexMetadata metadata;
        DirectX::ScratchImage scratchImage;

        HRESULT hr = DirectX::LoadFromWICFile(L"icon.png", DirectX::WIC_FLAGS_NONE, &metadata, scratchImage);

        if (SUCCEEDED(hr))
        { 

            DirectX::ScratchImage resizedImage;

            hr = DirectX::Resize(
                scratchImage.GetImages(),
                scratchImage.GetImageCount(),
                scratchImage.GetMetadata(),
                64,                         // Destination width
                64,                         // Destination height
                DirectX::TEX_FILTER_DEFAULT,
                resizedImage
            );

            
            using namespace DirectX;
            ScratchImage out;
            ID3D11ShaderResourceView* srv;
            hr = DirectX::CreateShaderResourceView(g_pd3dDevice, resizedImage.GetImages(), resizedImage.GetImageCount(), resizedImage.GetMetadata(), &srv);
            if (SUCCEEDED(hr))
            {
                myIconID = (ImTextureID)srv;
                iconMetadata = resizedImage.GetMetadata();
            }
        }

        pos_rect = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
        CurrentEdge = Edge::Right;

        
        });
    if (myIconID)
    {
        
        ImGui::Image(myIconID, ImVec2(iconMetadata.width, iconMetadata.height));
        
    }
    test.Draw();
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        test.Reset();
    }
    ImGui::SameLine();
    if (ImGui::Button("Resize"))
    {
        test.Resize(64, 64);
    }
    ImGui::SameLine();
    if (ImGui::Button("Rotate"))
    {
        test.Rotation() += 45;
    }
    ImGui::SliderInt("Radius", &knob_radius, 12, 100, "%d");
    EmojiSliderWithLabel("test", &test_float, 0, 100, test.GetTextureID(), star.GetTextureID(), knob_radius);
    MoveEmojiAlongBorder(pos_rect.x , pos_rect.y, star.GetTextureID(), CurrentEdge);
    DrawFunnySquares();

    ImGui::End();
    DUMP_TO_IMGUI();
}
