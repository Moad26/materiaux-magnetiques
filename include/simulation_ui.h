#pragma once
#include "simulation.h"  // Pour SimulationParams et les structures de données

/**
 * @brief Lance la simulation principale avec paramètres configurables
 * @param initialParams Paramètres initiaux de la simulation
 * @return Code de sortie (0 si succès)
 * 
 * Cette fonction gère :
 * - L'initialisation de la fenêtre et de la caméra
 * - La boucle principale de rendu
 * - L'interface utilisateur ImGui
 * - La gestion des entrées utilisateur
 */
int runSimulation(const SimulationParams& initialParams);

// ------------------------------------------
// Variables globales pour l'interface utilisateur
// (Déclarées extern car définies dans simulation_ui.cpp)
// ------------------------------------------

/**
 * @brief État actuel de la simulation (défini dans l'UI)
 */
extern SimulationState simState;

/**
 * @brief Couleur des spins "up" (modifiable via l'UI)
 */
extern Color upColor;

/**
 * @brief Couleur des spins "down" (modifiable via l'UI)
 */
extern Color downColor;

/**
 * @brief Active/désactive la visualisation des énergies
 */
extern bool showEnergy;

/**
 * @brief Active/désactive la grille de référence 3D
 */
extern bool showGrid;
