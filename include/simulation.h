#ifndef SIMULATION_H
#define SIMULATION_H
#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "rlgl.h"
#include <deque>
#include <vector>

using namespace std;

enum class Spin : int {
  UP = 1,    // Spin orienté vers le haut
  DOWN = -1, // Spin orienté vers le bas
};

/// Type de structure cristalline supportée
enum class StructureType {
  CUBIC,     // Cubique simple
  HEXAGONAL, // Hexagonal compact (HCP)
  FCC,       // Cubique à faces centrées
  BCC,       // Cubique centré
};

// ÉNUMÉRATIONS ET STRUCTURES DE DONNÉES

/// État possible de la simulation

enum class SimulationState {
  PAUSED,  // Simulation en cours
  RUNNING, // Simulation en pause
  STEP     // Un seul pas de simulation
};

/// Représente un atome dans le réseau
struct Atome {
  Vector3 pos;
  Spin spin = Spin::UP;
  vector<int> neigh;
  float energy = 0.0f;
  float radius = 0.5f;
};

// Global variables
extern SimulationState simState;
extern float temperature;
extern float J;
extern float B;
extern int stepsPerFrame;
extern bool showEnergy;
extern Color upColor;
extern Color downColor;

// FONCTIONS DE CONSTRUCTION DES RÉSEAUX
vector<Atome> make_cubic_struc(int x, int y, int z, float distance);
/**
 * Crée un réseau cubique simple
 * @param x,y,z Dimensions du réseau en nombre d'atomes
 * @param distance Distance interatomique
 * @return Vecteur contenant tous les atomes positionnés
 */

vector<Atome> make_hexagonal_struc(int x, int y, int z, float distance);
/**
 * Crée un réseau hexagonal compact (HCP)
 * @param x,y,z Dimensions du réseau
 * @param distance Distance entre atomes voisins
 * @return Vecteur des atomes avec empilement ABAB
 */

vector<Atome> make_fcc_struc(int x, int y, int z, float distance);
/**
 * Crée un réseau cubique à faces centrées (FCC)
 * @param x,y,z Dimensions du réseau
 * @param distance Paramètre de maille
 * @return Vecteur des atomes avec leurs 12 voisins
 */

vector<Atome> make_bcc_struc(int x, int y, int z, float distance);
/**
 * Crée un réseau cubique centré (BCC)
 * @param x,y,z Dimensions du réseau
 * @param distance Paramètre de maille
 * @return Vecteur des atomes avec leurs 8 voisins
 */

// FONCTIONS DE VISUALISATION
vector<Mesh> CreateChunkedCylinderLines(const vector<Atome> &structure,
                                        float radius = 0.05f, int segments = 8,
                                        int maxCylindersPerChunk = 1000);
/**
 * Crée des cylindres pour les liaisons entre atomes
 * (version optimisée par la methode de chunked baking)
 * @param structure Vecteur d'atomes
 * @param radius Rayon des cylindres
 * @param segments Nombre de segments par cylindre
 * @param maxCylindersPerChunk Nombre max de cylindres par mesh
 * @return Vecteur de meshs pour le rendu
 */

Mesh CreateBakedCylinderLines(const vector<Atome> &structure,
                              float radius = 0.05f, int segments = 8);
/**
 * Crée des cylindres pour les liaisons
 * (version optimisée par la methode de baking)
 * @param structure Vecteur d'atomes
 * @param radius Rayon des cylindres
 * @param segments Nombre de segments
 * @return Mesh unique contenant toutes les liaisons
 */

void DrawInstanced(Mesh mesh, Material material, vector<Matrix> &transforms);
/**
 * Dessine un mesh avec plusieurs transformations (instancing)
 * @param mesh Mesh à dessiner
 * @param material Matériau à appliquer
 * @param transforms Matrices de transformation
 */

// FONCTIONS DE SIMULATION
float CalculateTotalEnergy(const vector<Atome> &structure);
/**
 * Calcule l'énergie totale du système
 * @param structure Vecteur d'atomes
 * @return Énergie totale (divisée par 2 pour éviter double comptage)
 */

void UpdateEnergies(vector<Atome> &structure, float J, float B);
/**
 * Met à jour les énergies de tous les atomes
 * @param structure Vecteur d'atomes à mettre à jour
 * @param params Paramètres courants (B, J implicite)
 */

void MonteCarloStep(vector<Atome> &structure, float temperature, float J,
                    float B);
/**
 * Effectue un pas Monte Carlo (algorithme de Metropolis)
 * @param structure Référence au vecteur d'atomes
 * @param params Paramètres de simulation actuels
 */

void UpdateEnergyHistory(deque<float> &energyHistory, float currentEnergy,
                         size_t maxHistoryPoints);
#endif // SIMULATION_H
