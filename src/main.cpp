#include "auth.h"
#include "raylib.h"
#include "rlImGui.h"
#include "simulation_ui.h"

int main() {
  // Initialize window
  int monitor = GetCurrentMonitor();
  int screenWidth = GetMonitorWidth(monitor);
  int screenHeight = GetMonitorHeight(monitor);

  // Borderless window at monitor size
  InitWindow(screenWidth, screenHeight, "3D Ising Model Simulation");
  SetWindowPosition(screenWidth / 2, screenHeight / 2);
  SetTargetFPS(60);
  rlImGuiSetup(true);

  // User data
  bool logged_in = runAuthentication();
  // Clean up ImGui and close window before starting simulation
  rlImGuiShutdown();
  CloseWindow();

  // Run simulation only if logged in
  if (logged_in) {
    runSimulation();
  }

  return 0;
}
