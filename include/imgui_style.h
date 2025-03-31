// imgui_style.h
#ifndef IMGUI_STYLE_H
#define IMGUI_STYLE_H
#include "imgui.h"

inline void SetCustomImGuiStyle(float scaling = 1.5f) {
  ImGuiStyle &style = ImGui::GetStyle();
  ImGuiIO &io = ImGui::GetIO();

  // Clear existing fonts
  io.Fonts->Clear();

  // Configure font settings for better rendering
  ImFontConfig fontConfig;
  fontConfig.OversampleH = 2;   // Horizontal oversampling
  fontConfig.OversampleV = 2;   // Vertical oversampling
  fontConfig.PixelSnapH = true; // Snap to pixel boundaries

  // Load font with improved settings
  io.Fonts->AddFontFromFileTTF("/home/moad/desktop/cpp/crist-project/assets/"
                               "JetBrainsMonoNLNerdFont-Regular.ttf",
                               18.0f * scaling, // Scale the base font size
                               &fontConfig);

  // Basic scaling (don't use FontGlobalScale when scaling the font directly)
  style.ScaleAllSizes(scaling);
  // io.FontGlobalScale = 1.0f; // Uncomment if you prefer using FontGlobalScale
  // instead

  // Colors (dark theme with custom accents)
  style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.44f, 0.89f, 1.00f);

  // Rounding and padding
  style.WindowRounding = 10.0f;
  style.FrameRounding = 5.0f;
  style.GrabRounding = 5.0f;
  style.WindowPadding = ImVec2(15, 15);
  style.FramePadding = ImVec2(20, 15);
  style.ItemSpacing = ImVec2(15, 15);
  style.ItemInnerSpacing = ImVec2(10, 10);

  // Borders
  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;
  style.PopupBorderSize = 1.0f;

  // Make title bar match our theme
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.10f, 0.11f, 1.00f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);

  // Rebuild font atlas
  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  // rlImGui will handle the texture upload automatically
}

#endif // IMGUI_STYLE_H
