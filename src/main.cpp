#include "auth.h"
#include "raylib.h"
#include "rlImGui.h"
#include "simulation_ui.h"

int main() {
  // Initialisation fenêtre en plein écran
  int monitor = GetCurrentMonitor();
  int screenWidth = GetMonitorWidth(monitor);
  int screenHeight = GetMonitorHeight(monitor);

  // Création fenêtre borderless
  InitWindow(screenWidth, screenHeight, "3D Ising Model Simulation");
  SetWindowPosition(screenWidth / 2, screenHeight / 2);
  SetTargetFPS(60);
  rlImGuiSetup(true);

  // Authentification utilisateur
  bool logged_in = runAuthentication();
  // Nettoyage après authentification
  rlImGuiShutdown();
  CloseWindow();

  // Si authentification réussie, lancer la simulation avec les paramètres
  if (logged_in) {
    runSimulation();
  }

  return 0;
}
