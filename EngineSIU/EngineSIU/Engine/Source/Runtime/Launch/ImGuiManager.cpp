#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGuiManager.h"
#include "Font/RawFonts.h"
#include "Font/IconDefs.h"
#include "ImGui/ImGuizmo.h"

void UImGuiManager::Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(device, deviceContext);
    io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\malgun.ttf)", 20.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

    ImFontConfig FeatherFontConfig;
    FeatherFontConfig.PixelSnapH = true;
    FeatherFontConfig.FontDataOwnedByAtlas = false;
    FeatherFontConfig.GlyphOffset = ImVec2(0, 0);
    
    constexpr const ImWchar feather_icon_min = 0xE900; // 실제 Feather 아이콘 시작 유니코드
    constexpr ImWchar feather_icon_max = 0xEA1E; // 실제 Feather 아이콘 끝 유니코드 (또는 사용하는 아이콘 중 가장 큰 값)

    static constexpr ImWchar IconRanges[] = {
        feather_icon_min, feather_icon_max, // 로드할 아이콘의 전체 범위
        0 // NULL 종료
    };

    io.Fonts->AddFontFromMemoryTTF(FeatherRawData, FontSizeOfFeather, 22.0f, &FeatherFontConfig, IconRanges);
    PreferenceStyle();
}

void UImGuiManager::BeginFrame() const
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void UImGuiManager::EndFrame() const
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

/* GUI Style Preference */
void UImGuiManager::PreferenceStyle() const
{
    // Window
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.9f);
    // Title
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4{ 0.02f, 0.02f, 0.02f, 1.0f };
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.02f, 0.02f, 0.02f, 1.0f };
    ImGui::GetStyle().WindowRounding = 5.0f;
    ImGui::GetStyle().PopupRounding = 3.0f;
    ImGui::GetStyle().FrameRounding = 3.0f;

    // Sep
    ImGui::GetStyle().Colors[ImGuiCol_Separator] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);

    // Popup
    ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.9f);
    
    // Frame
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Button
    ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0, 0.0f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.105f, 0.105f, 0.105f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.00f, 0.85f, 1.0f);

    // Header
    ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0.203f, 0.203f, 0.203f, 0.6f);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImVec4(0.105f, 0.105f, 0.105f, 0.6f);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.85f, 0.85f);

    // Text
    ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 0.9f);

    // Tabs
    ImGui::GetStyle().Colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
    ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
    ImGui::GetStyle().Colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    ImGui::GetStyle().Colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
}

void UImGuiManager::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

