#ifndef SIMULATION_H
#define SIMULATION_H

#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "rlgl.h"
#include <vector>

using namespace std;

enum class Spin : int {
  UP = 1,
  DOWN = -1,
};

enum class StructureType {
  CUBIC,
  HEXAGONAL,
  FCC,
  BCC,
};

enum class SimulationState { PAUSED, RUNNING, STEP };

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

// Structure generation functions
vector<Atome> make_cubic_struc(int x, int y, int z, float distance);
vector<Atome> make_hexagonal_struc(int x, int y, int z, float distance);
vector<Atome> make_fcc_struc(int x, int y, int z, float distance);
vector<Atome> make_bcc_struc(int x, int y, int z, float distance);

// Visualization functions
vector<Mesh> CreateChunkedCylinderLines(const vector<Atome> &structure,
                                        float radius = 0.05f, int segments = 8,
                                        int maxCylindersPerChunk = 1000);
Mesh CreateBakedCylinderLines(const vector<Atome> &structure,
                              float radius = 0.05f, int segments = 8);
void DrawInstanced(Mesh mesh, Material material, vector<Matrix> &transforms);

// Simulation functions
float CalculateTotalEnergy(const vector<Atome> &structure);
void UpdateEnergies(vector<Atome> &structure, float J, float B);
void MonteCarloStep(vector<Atome> &structure, float temperature, float J,
                    float B);

#endif // SIMULATION_H
