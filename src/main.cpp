#include "auth.h"
#include "raylib.h"
#include "rlImGui.h"
#include "simulation_ui.h"

int main() {
  // Initialisation fenêtre en plein écran
  int monitor = GetCurrentMonitor();
  int screenWidth = GetMonitorWidth(monitor);
  int screenHeight = GetMonitorHeight(monitor);

  // Configuration initiale des paramètres de simulation
  SimulationParams simParams;
  simParams.temperature = 2.5f;   // Température par défaut
  simParams.B = 0.1f;            // Petit champ magnétique initial
  simParams.stepsPerFrame = 100; // Nombre raisonnable de pas MC

  // Création fenêtre borderless
  InitWindow(screenWidth, screenHeight, "3D Ising Model Simulation");
  SetWindowPosition(0, 0);  // Position en haut à gauche
  SetTargetFPS(60);         // Synchronisation verticale
  rlImGuiSetup(true);       // Initialisation ImGui

  // Authentification utilisateur
  bool logged_in = runAuthentication();

  // Nettoyage après authentification
  rlImGuiShutdown();
  CloseWindow();

  // Si authentification réussie, lancer la simulation avec les paramètres
  if (logged_in) {
    runSimulation(simParams);  // Passage des paramètres initiaux
  }

  return 0;
}
