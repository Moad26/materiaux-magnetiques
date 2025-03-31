#pragma once
#include <vector>
#include "raylib.h"


// ÉNUMÉRATIONS ET STRUCTURES DE DONNÉES

/// État possible de la simulation
enum class SimulationState {
    RUNNING,    // Simulation en cours
    PAUSED,     // Simulation en pause
    STEP        // Un seul pas de simulation
};

/// Type de structure cristalline supportée
enum class StructureType {
    CUBIC,      // Cubique simple
    HEXAGONAL,  // Hexagonal compact (HCP)
    FCC,        // Cubique à faces centrées
    BCC         // Cubique centré
};

/// Orientation possible du spin atomique
enum class Spin { 
    UP = 1,     // Spin orienté vers le haut
    DOWN = -1   // Spin orienté vers le bas
};

/// Paramètres configurables de la simulation
struct SimulationParams {
    float temperature = 2.5f;   // Température réduite (kₙT/J)
    float B = 0.0f;            // Champ magnétique externe réduit (B/J)
    int stepsPerFrame = 100;    // Nombre de pas Monte Carlo par frame
};

/// Représente un atome dans le réseau
struct Atome {
    Vector3 pos;            // Position 3D (x,y,z)
    Spin spin;              // Orientation du spin
    float energy = 0;       // Énergie locale de l'atome
    std::vector<int> neigh; // Indices des atomes voisins
};


// FONCTIONS DE CONSTRUCTION DES RÉSEAUX

std::vector<Atome> make_cubic_struc(int x, int y, int z, float distance);
/**
 * Crée un réseau cubique simple
 * @param x,y,z Dimensions du réseau en nombre d'atomes
 * @param distance Distance interatomique
 * @return Vecteur contenant tous les atomes positionnés
 */


std::vector<Atome> make_hexagonal_struc(int x, int y, int z, float distance);
/**
 * Crée un réseau hexagonal compact (HCP)
 * @param x,y,z Dimensions du réseau
 * @param distance Distance entre atomes voisins
 * @return Vecteur des atomes avec empilement ABAB
 */


std::vector<Atome> make_fcc_struc(int x, int y, int z, float distance);
/**
 * Crée un réseau cubique à faces centrées (FCC)
 * @param x,y,z Dimensions du réseau
 * @param distance Paramètre de maille
 * @return Vecteur des atomes avec leurs 12 voisins
 */


std::vector<Atome> make_bcc_struc(int x, int y, int z, float distance);
/**
 * Crée un réseau cubique centré (BCC)
 * @param x,y,z Dimensions du réseau
 * @param distance Paramètre de maille
 * @return Vecteur des atomes avec leurs 8 voisins
 */


// FONCTIONS DE SIMULATION

void MonteCarloStep(std::vector<Atome>& structure, const SimulationParams& params);
/**
 * Effectue un pas Monte Carlo (algorithme de Metropolis)
 * @param structure Référence au vecteur d'atomes
 * @param params Paramètres de simulation actuels
 */


void UpdateEnergies(std::vector<Atome>& structure, const SimulationParams& params);
/**
 * Met à jour les énergies de tous les atomes
 * @param structure Vecteur d'atomes à mettre à jour
 * @param params Paramètres courants (B, J implicite)
 */


float CalculateTotalEnergy(const std::vector<Atome>& structure);
/**
 * Calcule l'énergie totale du système
 * @param structure Vecteur d'atomes
 * @return Énergie totale (divisée par 2 pour éviter double comptage)
 */



// FONCTIONS DE VISUALISATION

std::vector<Mesh> CreateChunkedCylinderLines(const std::vector<Atome>& structure,
                                           float radius, int segments,
                                           int maxCylindersPerChunk = 100);
/**
 * Crée des cylindres pour les liaisons entre atomes (version optimisée par chu>
 * @param structure Vecteur d'atomes
 * @param radius Rayon des cylindres
 * @param segments Nombre de segments par cylindre
 * @param maxCylindersPerChunk Nombre max de cylindres par mesh
 * @return Vecteur de meshs pour le rendu
 */


Mesh CreateBakedCylinderLines(const std::vector<Atome>& structure,
                             float radius, int segments);
/**
 * Crée des cylindres pour les liaisons (version tout-en-un)
 * @param structure Vecteur d'atomes
 * @param radius Rayon des cylindres
 * @param segments Nombre de segments
 * @return Mesh unique contenant toutes les liaisons
 */


void DrawInstanced(Mesh mesh, Material material, std::vector<Matrix>& transforms);
/**
 * Dessine un mesh avec plusieurs transformations (instancing)
 * @param mesh Mesh à dessiner
 * @param material Matériau à appliquer
 * @param transforms Matrices de transformation
 */
