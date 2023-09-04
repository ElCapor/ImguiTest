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

// user includes
#include <DirectXTex.h>
#include "emoji_slider.h"

//#########################################################
//################ USER FUNCTIONS #########################
//#########################################################
void DrawMenu();




//#########################################################
//################ MAIN LOOP ##############################
//#########################################################

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float valutest = 0;

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

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DrawMenu();
       

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

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
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
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
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
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
namespace fs = std::filesystem;
    

class ImGuiImage {
public:
    ImGuiImage()
    {
        std::cout << "Cant do nothing with an empty constructor lol" << std::endl;

    }
    ImGuiImage(const wchar_t* path)
    {
        if (fs::exists(path))
        {
            original_path = path;

            DirectX::ScratchImage tempImage; // the temp image to store from the file
            DirectX::TexMetadata tempMetadata;
            if (LoadFromFile(path, tempImage))
            {
                HRESULT hr;
                std::vector<uint8_t> tempBytes;
                if (ImageToBytes(tempBytes, tempImage))
                {
                    bytes = std::make_unique<std::vector<uint8_t>>(tempBytes);
                    image_info.height = tempImage.GetMetadata().height;
                    image_info.width = tempImage.GetMetadata().width;
                    image_info.imageSize = tempImage.GetMetadata().arraySize;
                    std::cout << "Bytes set successfully" << std::endl;

                    // we could use the tempImage that we made , but i did this to make sure that the bytes we set are correct

                    if (!bytes->empty()) // we perform check , cuz something bad could happen idk
                    {
                        // we do everything again to make sure it's correct
                        DirectX::ScratchImage anotherImage;
                        if (BytesToImage(*bytes, anotherImage))
                        {
                            ID3D11ShaderResourceView* srv;
                            hr = DirectX::CreateShaderResourceView(g_pd3dDevice, anotherImage.GetImages(), anotherImage.GetImageCount(), anotherImage.GetMetadata(), &srv);
                            if (SUCCEEDED(hr))
                            {
                                this->m_ImageID = (ImTextureID)srv;
                                std::cout << "Finished" << std::endl;
                            }
                            else {
                                std::cout << "Failed to create the shader resource view" << std::endl;
                            }
                        }
                        else {
                            std::cout << "Failed to load memory from the bytes , perhaps they are empty ?" << std::endl;
                        }
                    }
                    else {
                        std::cout << "Bytes are empty , this is a fatal error" << std::endl;
                    }
                }
                else {
                    std::cout << "ImageToBytes failed" << std::endl;
                }
            }
            else {
                std::cout << "Loading image from system failed , please check your path" << std::endl;
            }
        }
        else {
            std::cout << "Image doesnt exist , make sure the path is correct" << std::endl;
        }
       
    }

    ImTextureID GetTextureID()
    {
        return this->m_ImageID;
    }

    ImVec2 GetSize()
    {
        return ImVec2(image_info.width, image_info.height);
    }


    // lets add a function to support resizing on the fly
    bool Resize(float newHeight, float newWidth)
    {
        // make sure we're loaded
        if (!bytes->empty() && m_ImageID != NULL)
        {
            if (newHeight == image_info.height && newWidth == image_info.width)
            {
                std::cout << "[WARNING] | " << __FUNCTION__ << " | image wasnt resized because it has the same dimensions" << std::endl;
                return true; // no need to resize if same size
            }
            HRESULT hr;
            DirectX::ScratchImage tempImage;
            if (BytesToImage(*bytes, tempImage))
            {
                DirectX::ScratchImage newImage;
                DirectX::TexMetadata newMetadata;
                hr = DirectX::Resize(
                    tempImage.GetImages(),
                    tempImage.GetImageCount(),
                    tempImage.GetMetadata(),
                    newWidth,
                    newHeight,
                    DirectX::TEX_FILTER_DEFAULT,
                    newImage
                );
                if (SUCCEEDED(hr))
                {
                    DirectX::Blob data;
                    hr = DirectX::SaveToWICMemory(
                        newImage.GetImages(),
                        newImage.GetImageCount(),
                        DirectX::WIC_FLAGS_NONE,
                        GUID_ContainerFormatPng,
                        data
                    );
                    if (SUCCEEDED(hr))
                    {
                        std::vector<uint8_t> tempBytes;
                        if (BlobToBytes(data, tempBytes))
                        {
                            bytes = std::make_unique<std::vector<uint8_t>>(tempBytes);

                            image_info.height = newImage.GetMetadata().height;
                            image_info.width = newImage.GetMetadata().width;
                            image_info.imageSize = newImage.GetMetadata().arraySize;

                            std::cout << "Bytes set successfully" << std::endl;
                            ID3D11ShaderResourceView* srv;
                            hr = DirectX::CreateShaderResourceView(g_pd3dDevice, newImage.GetImages(), newImage.GetImageCount(), newImage.GetMetadata(), &srv);
                            if (SUCCEEDED(hr))
                            {
                                this->m_ImageID = (ImTextureID)srv;
                                std::cout << "Finished" << std::endl;
                                return true;
                            }
                            else {
                                std::cout << "Failed to create the shader resource view" << std::endl;
                                return false;
                            }
                        }
                        else {
                            std::cout << "Failed to convert Blob to bytes" << std::endl;
                            return false;
                        }
                        
                        
                    }
                    else {
                        std::cout << "[ERROR] | " << __FUNCTION__ << " | Failed to write bytes into our own image" << std::endl;
                        return false;
                    }
                }
                else {
                    std::cout << "[ERROR] | " << __FUNCTION__ << " | Failed to resize the image " << std::endl;
                    return false;
                }
            }
            else {
                std::cout << "[ERROR] | " << __FUNCTION__ << " | Failed to load image from bytes" << std::endl;
                return false;
            }
        }
        
        else {
            std::cout << "[ERROR] You're trying to resize an uninitialized image (maybe try using .Reset()) " << std::endl;
            return false;
        }
    }

    
    void Reset()
    {
        //new object (= original cuz we only dealin with bytes so no changes to original file
        ImGuiImage tempImage(original_path);

        //swap members , cuz you cant assign the current objects
        Swap(tempImage);
    }

    void Draw()
    {
        // perform checks cuz u shouldnt be drawin an image with no data lol
        if (!bytes->empty() && m_ImageID != NULL)
        {
            ImGui::Image(GetTextureID(), GetSize());
        }
    }

private:
    ImTextureID m_ImageID;
    std::unique_ptr<std::vector<uint8_t>> bytes; // image bytes

    const wchar_t* original_path; // keep this in case , i might write a reset function that will revert it to it's old state maybe
    // perhaps i should consider storing everything as new bytes but i'm not sure yet
    // or maybe not saving edits but idk yet
    struct {
        float width;
        float height;
        size_t imageSize;
    } image_info;


    

// internal functions to make my life easier
private:
    void Swap(ImGuiImage& other)
    {
        std::swap(m_ImageID, other.m_ImageID);
        std::swap(bytes, other.bytes);
        std::swap(image_info, other.image_info);
    }

    bool BlobToBytes(DirectX::Blob& blob, std::vector<uint8_t>& bytes) const
    {
        if (blob.GetBufferPointer() != nullptr || blob.GetBufferSize() > 0)
        {
            const uint8_t* dataPtr = static_cast<const uint8_t*>(blob.GetBufferPointer());
            size_t dataSize = blob.GetBufferSize();
            std::vector<uint8_t> imageBytes(dataPtr, dataPtr + dataSize);
            bytes = std::move(imageBytes);
            return true;
        }
        return false;
        

    }

    bool ImageToBytes(std::vector<uint8_t>& bytelist, DirectX::ScratchImage& image) const
    {
        HRESULT hr;
        DirectX::Blob data;
        hr = DirectX::SaveToWICMemory(image.GetImages(), image.GetImageCount(), DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, data);
        if (SUCCEEDED(hr))
        {
            if (BlobToBytes(data, bytelist))
            {
                return true;
            }
            else {
                std::cout << "Blob was empty" << std::endl;
                return false;
            }
        }
        else {
            std::cout << "Failed to parse image bytes" << std::endl;
            return false;
        }
    }

    bool BytesToImage(std::vector<uint8_t> bytes, DirectX::ScratchImage& image) const
    {
        if (!bytes.empty())
        {
            HRESULT hr;
            DirectX::ScratchImage tempImage;
            DirectX::TexMetadata tempMeta;
            hr = DirectX::LoadFromWICMemory(
                bytes.data(),
                bytes.size(),
                DirectX::WIC_FLAGS_NONE,
                &tempMeta,
                tempImage
            );
            if (SUCCEEDED(hr))
            {
                image = std::move(tempImage);
                return true;
            }
            else {
                std::cout << "Failed to parse image bytes" << std::endl;
                return false;
            }
        }
        else {
            std::cout << "[ERROR] " << __FUNCTION__ << " Can't get an image from empty bytes" << std::endl;
            return false;
        }
        
    }

    bool LoadFromFile(const wchar_t* path, DirectX::ScratchImage& image)
    {
        if (fs::exists(path))
        {
            HRESULT hr;
                DirectX::ScratchImage tempImage; // the temp image to store from the file
                DirectX::TexMetadata tempMetadata;

                // loading the image from the file
                hr = DirectX::LoadFromWICFile(path, DirectX::WIC_FLAGS_NONE, &tempMetadata, tempImage);
                if (SUCCEEDED(hr))
                {
                    image = std::move(tempImage);
                    return true;
                }
                else {
                    std::cout << "Loading image from system failed , please check your path" << std::endl;
                    return false;
                }
        }
        else {
            std::cout << "Image doesnt exist , make sure the path is correct" << std::endl;
            return false;
        }
    }
};
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

    // Draw the circular knob with the provided emoji image
    ImGui::GetWindowDrawList()->AddImage(knobTexture, knob_pos - ImVec2(knob_radius, knob_radius), knob_pos + ImVec2(knob_radius, knob_radius), ImVec2(0, 0), ImVec2(1, 1));

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
        sparkle_sizes.push_back(ImVec2(knobRadius *1.5, knobRadius *1.5));
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
    ImGui::ShowStyleEditor();
    ImGui::Begin("Hello");
    
    std::call_once(flag, []() {

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
    ImGui::SliderInt("Radius", &knob_radius, 12, 100, "%d");
    EmojiSliderWithLabel("test", &test_float, 0, 100, test.GetTextureID(), star.GetTextureID(), knob_radius);
    MoveEmojiAlongBorder(pos_rect.x , pos_rect.y, star.GetTextureID(), CurrentEdge);


    ImGui::End();
}
