#ifndef SIMULATION_APP_H
#define SIMULATION_APP_H
#include "imgui_style.h"
#include "simulation.h"
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
int runSimulation();

#endif
