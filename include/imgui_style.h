// imgui_style.h
#ifndef IMGUI_STYLE_H
#define IMGUI_STYLE_H
#include "imgui.h"
#include <iostream>

inline void SetCustomImGuiStyle(float scaling = 1.5f) {
  ImGuiStyle &style = ImGui::GetStyle();
  ImGuiIO &io = ImGui::GetIO();

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
  // Rosé Pine color palette
  const ImVec4 base = ImVec4(0.11f, 0.09f, 0.13f, 1.00f);    // #191724
  const ImVec4 surface = ImVec4(0.16f, 0.14f, 0.18f, 1.00f); // #1f1d2e
  const ImVec4 overlay = ImVec4(0.22f, 0.20f, 0.25f, 1.00f); // #26233a
  const ImVec4 muted = ImVec4(0.42f, 0.39f, 0.49f, 1.00f);   // #6e6a86
  const ImVec4 subtle = ImVec4(0.58f, 0.55f, 0.65f, 1.00f);  // #908caa
  const ImVec4 text = ImVec4(0.89f, 0.85f, 0.83f, 1.00f);    // #e0def4
  const ImVec4 love = ImVec4(0.86f, 0.37f, 0.51f, 1.00f);    // #eb6f92
  const ImVec4 gold = ImVec4(0.95f, 0.67f, 0.29f, 1.00f);    // #f6c177
  const ImVec4 rose = ImVec4(0.94f, 0.60f, 0.71f, 1.00f);    // #ebbcba
  const ImVec4 pine = ImVec4(0.06f, 0.58f, 0.50f, 1.00f);    // #31748f
  const ImVec4 foam = ImVec4(0.45f, 0.78f, 0.76f, 1.00f);    // #9ccfd8
  const ImVec4 iris = ImVec4(0.55f, 0.62f, 0.88f, 1.00f);    // #c4a7e7

  // Apply the theme
  style.Colors[ImGuiCol_Text] = text;
  style.Colors[ImGuiCol_TextDisabled] = muted;
  style.Colors[ImGuiCol_WindowBg] = surface;
  style.Colors[ImGuiCol_ChildBg] = base;
  style.Colors[ImGuiCol_PopupBg] = surface;
  style.Colors[ImGuiCol_Border] = overlay;
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_FrameBg] = overlay;
  style.Colors[ImGuiCol_FrameBgHovered] =
      ImVec4(overlay.x + 0.05f, overlay.y + 0.05f, overlay.z + 0.05f, 1.00f);
  style.Colors[ImGuiCol_FrameBgActive] =
      ImVec4(overlay.x + 0.10f, overlay.y + 0.10f, overlay.z + 0.10f, 1.00f);
  style.Colors[ImGuiCol_TitleBg] = base;
  style.Colors[ImGuiCol_TitleBgActive] = overlay;
  style.Colors[ImGuiCol_TitleBgCollapsed] = base;
  style.Colors[ImGuiCol_MenuBarBg] = base;
  style.Colors[ImGuiCol_ScrollbarBg] = base;
  style.Colors[ImGuiCol_ScrollbarGrab] = overlay;
  style.Colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(overlay.x + 0.10f, overlay.y + 0.10f, overlay.z + 0.10f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] =
      ImVec4(overlay.x + 0.15f, overlay.y + 0.15f, overlay.z + 0.15f, 1.00f);
  style.Colors[ImGuiCol_CheckMark] = rose;
  style.Colors[ImGuiCol_SliderGrab] = rose;
  style.Colors[ImGuiCol_SliderGrabActive] = love;
  style.Colors[ImGuiCol_Button] = overlay;
  style.Colors[ImGuiCol_ButtonHovered] =
      ImVec4(overlay.x + 0.10f, overlay.y + 0.10f, overlay.z + 0.10f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive] =
      ImVec4(overlay.x + 0.15f, overlay.y + 0.15f, overlay.z + 0.15f, 1.00f);
  style.Colors[ImGuiCol_Header] = overlay;
  style.Colors[ImGuiCol_HeaderHovered] =
      ImVec4(overlay.x + 0.10f, overlay.y + 0.10f, overlay.z + 0.10f, 1.00f);
  style.Colors[ImGuiCol_HeaderActive] =
      ImVec4(overlay.x + 0.15f, overlay.y + 0.15f, overlay.z + 0.15f, 1.00f);
  style.Colors[ImGuiCol_Separator] = overlay;
  style.Colors[ImGuiCol_SeparatorHovered] =
      ImVec4(overlay.x + 0.10f, overlay.y + 0.10f, overlay.z + 0.10f, 1.00f);
  style.Colors[ImGuiCol_SeparatorActive] =
      ImVec4(overlay.x + 0.15f, overlay.y + 0.15f, overlay.z + 0.15f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = overlay;
  style.Colors[ImGuiCol_ResizeGripHovered] =
      ImVec4(overlay.x + 0.10f, overlay.y + 0.10f, overlay.z + 0.10f, 1.00f);
  style.Colors[ImGuiCol_ResizeGripActive] =
      ImVec4(overlay.x + 0.15f, overlay.y + 0.15f, overlay.z + 0.15f, 1.00f);
  style.Colors[ImGuiCol_Tab] = base;
  style.Colors[ImGuiCol_TabHovered] = overlay;
  style.Colors[ImGuiCol_TabActive] = overlay;
  style.Colors[ImGuiCol_TabUnfocused] = base;
  style.Colors[ImGuiCol_TabUnfocusedActive] = base;
  style.Colors[ImGuiCol_PlotLines] = foam;
  style.Colors[ImGuiCol_PlotLinesHovered] = love;
  style.Colors[ImGuiCol_PlotHistogram] = gold;
  style.Colors[ImGuiCol_PlotHistogramHovered] = love;
  style.Colors[ImGuiCol_TableHeaderBg] = base;
  style.Colors[ImGuiCol_TableBorderStrong] = overlay;
  style.Colors[ImGuiCol_TableBorderLight] =
      ImVec4(overlay.x, overlay.y, overlay.z, 0.50f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(iris.x, iris.y, iris.z, 0.35f);
  style.Colors[ImGuiCol_DragDropTarget] = ImVec4(iris.x, iris.y, iris.z, 0.95f);
  style.Colors[ImGuiCol_NavHighlight] = ImVec4(iris.x, iris.y, iris.z, 0.80f);
  style.Colors[ImGuiCol_NavWindowingHighlight] =
      ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
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
