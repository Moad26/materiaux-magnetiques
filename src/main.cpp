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

SimulationState simState = SimulationState::PAUSED;
float temperature = 2.5f;
float J = 1.0f;
float B = 0.0f;
int stepsPerFrame = 100;
bool showEnergy = false;
Color upColor = RED;
Color downColor = BLUE;

vector<Atome> make_cubic_struc(int x, int y, int z, float distance) {
  vector<Atome> points(x * y * z);
  auto getIndex = [=](int i, int j, int k) { return i * y * z + j * z + k; };

  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        int idx = getIndex(i, j, k);
        points[idx].pos = {i * distance, j * distance, k * distance};
        points[idx].spin = GetRandomValue(0, 1) ? Spin::UP : Spin::DOWN;

        if (i > 0)
          points[idx].neigh.push_back(getIndex(i - 1, j, k));
        if (i < x - 1)
          points[idx].neigh.push_back(getIndex(i + 1, j, k));
        if (j > 0)
          points[idx].neigh.push_back(getIndex(i, j - 1, k));
        if (j < y - 1)
          points[idx].neigh.push_back(getIndex(i, j + 1, k));
        if (k > 0)
          points[idx].neigh.push_back(getIndex(i, j, k - 1));
        if (k < z - 1)
          points[idx].neigh.push_back(getIndex(i, j, k + 1));
      }
    }
  }
  return points;
}

vector<Atome> make_hexagonal_struc(int x, int y, int z, float distance) {
  vector<Atome> points;
  points.reserve(x * y * z);

  float a = distance;
  float c = a * 1.2f; // Ideal c/a ratio for HCP

  for (int layer = 0; layer < z; layer++) {
    // ABAB stacking pattern
    bool isLayerB = (layer % 2 == 1);

    for (int row = 0; row < y; row++) {
      for (int col = 0; col < x; col++) {
        Atome atom;

        // Base position
        atom.pos.x = col * a;
        atom.pos.y = row * (a * sqrt(3.0f) / 2.0f);
        atom.pos.z = layer * c;

        // Apply offset for B layers
        if (isLayerB) {
          atom.pos.x += a / 2.0f;
          atom.pos.y += (a * sqrt(3.0f) / 6.0f);
        }

        // Apply offset for even rows within each layer
        if (row % 2 == 1) {
          atom.pos.x += a / 2.0f;
        }

        points.push_back(atom);
      }
    }
  }

  // Establish neighbor connections
  for (size_t i = 0; i < points.size(); i++) {
    points[i].neigh.clear();

    for (size_t j = 0; j < points.size(); j++) {
      if (i == j)
        continue;

      float dist = Vector3Distance(points[i].pos, points[j].pos);
      // In HCP, the nearest neighbor distance is exactly 'a'
      if (dist <= a * 1.1f) {
        points[i].neigh.push_back(static_cast<int>(j));
      }
    }
  }

  return points;
}

vector<Atome> make_fcc_struc(int x, int y, int z, float distance) {
  vector<Atome> points;

  // In FCC, we want to avoid duplicates at the boundaries
  float a = distance; // lattice constant

  // Create a 3D grid to track atom positions
  const float tolerance = 0.01f * a;
  auto isNearExistingAtom = [&points, tolerance](const Vector3 &pos) {
    for (const auto &atom : points) {
      if (Vector3Distance(atom.pos, pos) < tolerance) {
        return true;
      }
    }
    return false;
  };

  // Generate atoms for each unit cell
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        Vector3 basePos = {i * a, j * a, k * a};

        // Corner atom (0,0,0)
        Vector3 pos = basePos;
        if (!isNearExistingAtom(pos)) {
          Atome atom;
          atom.pos = pos;
          points.push_back(atom);
        }

        // Face centers
        Vector3 facePositions[] = {
            {basePos.x + a / 2, basePos.y + a / 2, basePos.z},
            {basePos.x + a / 2, basePos.y, basePos.z + a / 2},
            {basePos.x, basePos.y + a / 2, basePos.z + a / 2}};

        for (const auto &facePos : facePositions) {
          if (!isNearExistingAtom(facePos)) {
            Atome atom;
            atom.pos = facePos;
            points.push_back(atom);
          }
        }
      }
    }
  }

  // Establish neighbor connections (each atom has 12 nearest neighbors in FCC)
  for (size_t i = 0; i < points.size(); i++) {
    points[i].neigh.clear();

    for (size_t j = 0; j < points.size(); j++) {
      if (i == j)
        continue;

      float dist = Vector3Distance(points[i].pos, points[j].pos);
      // Nearest neighbor distance in FCC is a/√2 ≈ 0.707a
      if (dist <= a * 0.75f) {
        points[i].neigh.push_back(static_cast<int>(j));
      }
    }
  }

  return points;
}

vector<Atome> make_bcc_struc(int x, int y, int z, float distance) {
  vector<Atome> points;

  float a = distance; // lattice constant

  // Create a 3D grid to track atom positions
  const float tolerance = 0.01f * a;
  auto isNearExistingAtom = [&points, tolerance](const Vector3 &pos) {
    for (const auto &atom : points) {
      if (Vector3Distance(atom.pos, pos) < tolerance) {
        return true;
      }
    }
    return false;
  };

  // Generate atoms for each unit cell
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        Vector3 basePos = {i * a, j * a, k * a};

        // Corner atom (0,0,0)
        Vector3 pos = basePos;
        if (!isNearExistingAtom(pos)) {
          Atome atom;
          atom.pos = pos;
          points.push_back(atom);
        }

        // Body center
        Vector3 centerPos = {basePos.x + a / 2, basePos.y + a / 2,
                             basePos.z + a / 2};
        if (!isNearExistingAtom(centerPos)) {
          Atome atom;
          atom.pos = centerPos;
          points.push_back(atom);
        }
      }
    }
  }

  // Establish neighbor connections (each atom has 8 nearest neighbors in BCC)
  for (size_t i = 0; i < points.size(); i++) {
    points[i].neigh.clear();

    for (size_t j = 0; j < points.size(); j++) {
      if (i == j)
        continue;

      float dist = Vector3Distance(points[i].pos, points[j].pos);
      // Nearest neighbor distance in BCC is a*√3/2 ≈ 0.866a
      if (dist <= a * 0.9f) {
        points[i].neigh.push_back(static_cast<int>(j));
      }
    }
  }

  return points;
}

vector<Mesh> CreateChunkedCylinderLines(const vector<Atome> &structure,
                                        float radius = 0.05f, int segments = 8,
                                        int maxCylindersPerChunk = 1000) {
  vector<Mesh> meshChunks;
  vector<vector<pair<Vector3, Vector3>>>
      chunks; // Stores start/end points for each chunk

  // First collect all cylinder pairs
  vector<pair<Vector3, Vector3>> allCylinders;
  for (const auto &atom : structure) {
    for (int neighborIdx : atom.neigh) {
      if (neighborIdx > &atom - &structure[0]) {
        allCylinders.emplace_back(atom.pos, structure[neighborIdx].pos);
      }
    }
  }

  // Split into chunks
  for (size_t i = 0; i < allCylinders.size(); i += maxCylindersPerChunk) {
    auto start = allCylinders.begin() + i;
    auto end = (i + maxCylindersPerChunk) < allCylinders.size()
                   ? start + maxCylindersPerChunk
                   : allCylinders.end();
    chunks.emplace_back(start, end);
  }

  // Create a mesh for each chunk
  for (const auto &chunk : chunks) {
    const int vertsPerCylinder = segments * 2;
    const int trisPerCylinder = segments * 2;

    Mesh mesh = {0};
    mesh.vertexCount = chunk.size() * vertsPerCylinder;
    mesh.triangleCount = chunk.size() * trisPerCylinder;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 *
                                               sizeof(unsigned short));

    int vertexOffset = 0;
    int indexOffset = 0;

    for (const auto &[start, end] : chunk) {
      Vector3 direction = Vector3Normalize(Vector3Subtract(end, start));
      Vector3 perp =
          (fabs(direction.x) < fabs(direction.y))
              ? Vector3Normalize(Vector3CrossProduct(direction, {1, 0, 0}))
              : Vector3Normalize(Vector3CrossProduct(direction, {0, 1, 0}));

      // Generate cylinder vertices
      for (int i = 0; i < segments; i++) {
        float angle = 2 * PI * i / segments;
        Vector3 circleVec = Vector3Scale(
            Vector3Add(Vector3Scale(perp, cosf(angle)),
                       Vector3Scale(Vector3CrossProduct(perp, direction),
                                    sinf(angle))),
            radius);

        // Bottom ring
        mesh.vertices[(vertexOffset + i) * 3 + 0] = start.x + circleVec.x;
        mesh.vertices[(vertexOffset + i) * 3 + 1] = start.y + circleVec.y;
        mesh.vertices[(vertexOffset + i) * 3 + 2] = start.z + circleVec.z;

        // Top ring
        mesh.vertices[(vertexOffset + segments + i) * 3 + 0] =
            end.x + circleVec.x;
        mesh.vertices[(vertexOffset + segments + i) * 3 + 1] =
            end.y + circleVec.y;
        mesh.vertices[(vertexOffset + segments + i) * 3 + 2] =
            end.z + circleVec.z;

        // Normals
        Vector3 normal = Vector3Normalize(circleVec);
        mesh.normals[(vertexOffset + i) * 3 + 0] = normal.x;
        mesh.normals[(vertexOffset + i) * 3 + 1] = normal.y;
        mesh.normals[(vertexOffset + i) * 3 + 2] = normal.z;
        mesh.normals[(vertexOffset + segments + i) * 3 + 0] = normal.x;
        mesh.normals[(vertexOffset + segments + i) * 3 + 1] = normal.y;
        mesh.normals[(vertexOffset + segments + i) * 3 + 2] = normal.z;
      }

      // Generate cylinder indices
      for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;

        // Bottom triangle
        mesh.indices[indexOffset++] = vertexOffset + i;
        mesh.indices[indexOffset++] = vertexOffset + next;
        mesh.indices[indexOffset++] = vertexOffset + segments + i;

        // Top triangle
        mesh.indices[indexOffset++] = vertexOffset + segments + i;
        mesh.indices[indexOffset++] = vertexOffset + next;
        mesh.indices[indexOffset++] = vertexOffset + segments + next;
      }

      vertexOffset += vertsPerCylinder;
    }

    UploadMesh(&mesh, false);
    meshChunks.push_back(mesh);
  }

  return meshChunks;
}

Mesh CreateBakedCylinderLines(const vector<Atome> &structure,
                              float radius = 0.05f, int segments = 8) {
  int cylinderCount = 0;
  for (const auto &atom : structure) {
    for (int neighborIdx : atom.neigh) {
      if (neighborIdx > &atom - &structure[0])
        cylinderCount++;
    }
  }

  const int vertsPerCylinder = segments * 2;
  const int trisPerCylinder = segments * 2;

  Mesh mesh = {0};
  mesh.vertexCount = cylinderCount * vertsPerCylinder;
  mesh.triangleCount = cylinderCount * trisPerCylinder;

  mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
  mesh.normals = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
  mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
  mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 *
                                             sizeof(unsigned short));

  int vertexOffset = 0;
  int indexOffset = 0;

  for (const auto &atom : structure) {
    for (int neighborIdx : atom.neigh) {
      if (neighborIdx > &atom - &structure[0]) {
        Vector3 start = atom.pos;
        Vector3 end = structure[neighborIdx].pos;
        Vector3 direction = Vector3Normalize(Vector3Subtract(end, start));

        Vector3 perp =
            (fabs(direction.x) < fabs(direction.y))
                ? Vector3Normalize(
                      Vector3CrossProduct(direction, (Vector3){1, 0, 0}))
                : Vector3Normalize(
                      Vector3CrossProduct(direction, (Vector3){0, 1, 0}));

        for (int i = 0; i < segments; i++) {
          float angle = 2 * PI * i / segments;
          Vector3 circleVec = Vector3Scale(
              Vector3Add(Vector3Scale(perp, cosf(angle)),
                         Vector3Scale(Vector3CrossProduct(perp, direction),
                                      sinf(angle))),
              radius);

          // Bottom ring
          mesh.vertices[(vertexOffset + i) * 3 + 0] = start.x + circleVec.x;
          mesh.vertices[(vertexOffset + i) * 3 + 1] = start.y + circleVec.y;
          mesh.vertices[(vertexOffset + i) * 3 + 2] = start.z + circleVec.z;

          // Top ring
          mesh.vertices[(vertexOffset + segments + i) * 3 + 0] =
              end.x + circleVec.x;
          mesh.vertices[(vertexOffset + segments + i) * 3 + 1] =
              end.y + circleVec.y;
          mesh.vertices[(vertexOffset + segments + i) * 3 + 2] =
              end.z + circleVec.z;

          // Normals
          Vector3 normal = {circleVec.x / radius, circleVec.y / radius,
                            circleVec.z / radius};
          mesh.normals[(vertexOffset + i) * 3 + 0] = normal.x;
          mesh.normals[(vertexOffset + i) * 3 + 1] = normal.y;
          mesh.normals[(vertexOffset + i) * 3 + 2] = normal.z;
          mesh.normals[(vertexOffset + segments + i) * 3 + 0] = normal.x;
          mesh.normals[(vertexOffset + segments + i) * 3 + 1] = normal.y;
          mesh.normals[(vertexOffset + segments + i) * 3 + 2] = normal.z;
        }

        for (int i = 0; i < segments; i++) {
          int next = (i + 1) % segments;

          // Bottom triangle
          mesh.indices[indexOffset++] = vertexOffset + i;
          mesh.indices[indexOffset++] = vertexOffset + next;
          mesh.indices[indexOffset++] = vertexOffset + segments + i;

          // Top triangle
          mesh.indices[indexOffset++] = vertexOffset + segments + i;
          mesh.indices[indexOffset++] = vertexOffset + next;
          mesh.indices[indexOffset++] = vertexOffset + segments + next;
        }

        vertexOffset += vertsPerCylinder;
      }
    }
  }

  UploadMesh(&mesh, false);
  return mesh;
}

void DrawInstanced(Mesh mesh, Material material, vector<Matrix> &transforms) {
  rlEnableShader(material.shader.id);
  for (Matrix transform : transforms) {
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(transform));
    DrawMesh(mesh, material, MatrixIdentity());
    rlPopMatrix();
  }
  rlDisableShader();
}

float CalculateTotalEnergy(const vector<Atome> &structure) {
  float totalEnergy = 0.0f;
  for (const auto &atom : structure) {
    totalEnergy += atom.energy;
  }
  return totalEnergy / 2.0f; // Divide by 2 to avoid double counting
}

void UpdateEnergies(vector<Atome> &structure, float J, float B) {
  for (auto &atom : structure) {
    float interactionEnergy = 0.0f;
    for (int neighborIdx : atom.neigh) {
      interactionEnergy += static_cast<int>(structure[neighborIdx].spin);
    }
    atom.energy = -J * static_cast<int>(atom.spin) * interactionEnergy -
                  B * static_cast<int>(atom.spin);
  }
}

void MonteCarloStep(vector<Atome> &structure, float temperature, float J,
                    float B) {
  int randomIdx = GetRandomValue(0, structure.size() - 1);
  auto &atom = structure[randomIdx];

  float currentEnergy = 0.0f;
  for (int neighborIdx : atom.neigh) {
    currentEnergy += static_cast<int>(structure[neighborIdx].spin);
  }
  currentEnergy = -J * static_cast<int>(atom.spin) * currentEnergy -
                  B * static_cast<int>(atom.spin);

  Spin newSpin = (atom.spin == Spin::UP) ? Spin::DOWN : Spin::UP;

  float newEnergy = 0.0f;
  for (int neighborIdx : atom.neigh) {
    newEnergy += static_cast<int>(structure[neighborIdx].spin);
  }
  newEnergy = -J * static_cast<int>(newSpin) * newEnergy -
              B * static_cast<int>(newSpin);

  float deltaE = newEnergy - currentEnergy;

  if (deltaE < 0 || (temperature > 0 && GetRandomValue(0, 10000) / 10000.0f <
                                            exp(-deltaE / temperature))) {
    atom.spin = newSpin;
    UpdateEnergies(structure, J, B);
  }
}
int main() {
  int monitor = GetCurrentMonitor();
  int screenWidth = GetMonitorWidth(monitor);
  int screenHeight = GetMonitorHeight(monitor);

  // Borderless window at monitor size
  InitWindow(screenWidth, screenHeight, "3D Ising Model Simulation");
  SetWindowPosition(screenWidth / 2, screenHeight / 2);

  SetTargetFPS(60);
  rlImGuiSetup(true);

  // Camera setup
  Camera3D camera = {0};
  camera.position = {0, 10, 30};
  camera.target = {10, 10, 10};
  camera.up = {0, 1, 0};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  // Simulation parameters
  int N = 10, O = 10, P = 10;
  float distance = 2.0f;
  float sphereRadius = 0.5f;
  float cylinderRadius = 0.05f;
  int segments = 8;
  bool showGrid = true;
  bool needsRebuild = true;
  Vector2 cameraAngle = {0};
  float movementSpeed = 10.0f;
  float cameraSensitivity = 0.3f;

  // Structure type
  StructureType currentStructure = StructureType::CUBIC;
  const char *structureTypes[] = {"Cubic", "Hexagonal", "Face-Centered Cubic",
                                  "Body-Centered Cubic"};
  int currentStructureType = 0;

  // Initialize structure
  vector<Atome> structure;
  vector<Matrix> sphereTransforms;
  vector<Mesh> cylinderMeshes;

  // Create default sphere mesh and material
  Mesh sphereMesh = GenMeshSphere(sphereRadius, 16, 16);
  Material sphereMaterial = LoadMaterialDefault();
  sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RED;

  // Create line material
  cylinderMeshes =
      CreateChunkedCylinderLines(structure, cylinderRadius, segments);
  Material lineMaterial = LoadMaterialDefault();
  lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;

  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowPadding = ImVec2(20, 20);    // Inner window padding
  style.FramePadding = ImVec2(20, 20);     // Button/slider padding
  style.ItemSpacing = ImVec2(15, 15);      // Space between elements
  style.GrabMinSize = 20.0f;               // Slider handle width
  style.WindowMinSize = ImVec2(500, 1000); // Minimum window size

  // Main game loop
  while (!WindowShouldClose()) {
    // Get frame timing for consistent movement speed
    float deltaTime = GetFrameTime();
    float currentSpeed = movementSpeed * deltaTime;

    // Get ImGui IO for mouse capture check
    ImGuiIO &io = ImGui::GetIO();

    // Movement direction in camera space
    Vector3 moveDir = {0};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_Z))
      moveDir.z += 1.0f;
    if (IsKeyDown(KEY_S))
      moveDir.z -= 1.0f;
    if (IsKeyDown(KEY_D))
      moveDir.x += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_Q))
      moveDir.x -= 1.0f;
    if (IsKeyDown(KEY_SPACE))
      moveDir.y += 1.0f;
    if (IsKeyDown(KEY_LEFT_CONTROL))
      moveDir.y -= 1.0f;

    // Normalize if we're moving in multiple directions
    if (Vector3Length(moveDir) > 0.0f) {
      moveDir = Vector3Normalize(moveDir);
    }

    // Calculate camera orientation vectors
    Vector3 forward =
        Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = camera.up;

    // Apply movement relative to camera orientation
    Vector3 movement = Vector3Zero();
    movement =
        Vector3Add(movement, Vector3Scale(forward, moveDir.z * currentSpeed));
    movement =
        Vector3Add(movement, Vector3Scale(right, moveDir.x * currentSpeed));
    movement = Vector3Add(movement, Vector3Scale(up, moveDir.y * currentSpeed));

    // Apply movement to position
    camera.position = Vector3Add(camera.position, movement);
    camera.target = Vector3Add(camera.target, movement);

    // Update camera look direction based on mouse input
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
        (!io.WantCaptureMouse || !ImGui::IsAnyItemActive())) {
      Vector2 mouseDelta = GetMouseDelta();

      cameraAngle.x -= mouseDelta.y * cameraSensitivity * deltaTime * 60.0f;
      cameraAngle.y -= mouseDelta.x * cameraSensitivity * deltaTime * 60.0f;
      cameraAngle.x = Clamp(cameraAngle.x, -89.0f, 89.0f);

      // Calculate the new forward direction
      Vector3 newForward = {
          cosf(DEG2RAD * cameraAngle.y) * cosf(DEG2RAD * cameraAngle.x),
          sinf(DEG2RAD * cameraAngle.x),
          sinf(DEG2RAD * cameraAngle.y) * cosf(DEG2RAD * cameraAngle.x)};

      // Update camera target based on the new direction
      camera.target = Vector3Add(camera.position, newForward);

      HideCursor();
    } else {
      ShowCursor();
    }
    // Rebuild structure if needed
    if (needsRebuild) {
      switch (currentStructure) {
      case StructureType::CUBIC:
        structure = make_cubic_struc(N, O, P, distance);
        break;
      case StructureType::HEXAGONAL:
        structure = make_hexagonal_struc(N, O, P, distance);
        break;
      case StructureType::FCC:
        structure = make_fcc_struc(N, O, P, distance);
        break;
      case StructureType::BCC:
        structure = make_bcc_struc(N, O, P, distance);
        break;
      }

      // Initialize spins and energies
      for (auto &atom : structure) {
        atom.spin = GetRandomValue(0, 1) ? Spin::UP : Spin::DOWN;
      }
      UpdateEnergies(structure, J, B);

      // Recreate transforms
      sphereTransforms.clear();
      for (const auto &atom : structure) {
        sphereTransforms.push_back(
            MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
      }

      // Recreate cylinder meshes
      cylinderMeshes =
          CreateChunkedCylinderLines(structure, cylinderRadius, segments);

      needsRebuild = false;
    }

    // Run simulation
    if (simState == SimulationState::RUNNING ||
        simState == SimulationState::STEP) {
      for (int i = 0; i < stepsPerFrame; i++) {
        MonteCarloStep(structure, temperature, J, B);
      }

      if (simState == SimulationState::STEP) {
        simState = SimulationState::PAUSED;
      }
    }

    // Rendering
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);
    // Draw spheres
    for (size_t i = 0; i < structure.size(); i++) {
      Color color = (structure[i].spin == Spin::UP) ? upColor : downColor;
      if (showEnergy) {
        // Calculate normalized energy (0-1 range)
        float minE = -fabsf(J) * structure[i].neigh.size() - fabsf(B);
        float maxE = fabsf(J) * structure[i].neigh.size() + fabsf(B);
        float normalizedEnergy = (structure[i].energy - minE) / (maxE - minE);
        normalizedEnergy = Clamp(normalizedEnergy, 0.0f, 1.0f);

        color = Color{(unsigned char)(255 * normalizedEnergy), 0,
                      (unsigned char)(255 * (1.0f - normalizedEnergy)), 255};
      }
      sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = color;
      DrawMesh(sphereMesh, sphereMaterial, sphereTransforms[i]);
    }

    // Draw cylinders
    for (const auto &mesh : cylinderMeshes) {
      DrawMesh(mesh, lineMaterial, MatrixIdentity());
    }

    if (showGrid)
      DrawGrid(40, 1);
    EndMode3D();

    // UI
    rlImGuiBegin();
    ImGui::Begin("Controls");

    // Structure controls
    if (ImGui::SliderInt("Grid Size X", &N, 1, 10))
      needsRebuild = true;
    if (ImGui::SliderInt("Grid Size Y", &O, 1, 10))
      needsRebuild = true;
    if (ImGui::SliderInt("Grid Size Z", &P, 1, 10))
      needsRebuild = true;
    if (ImGui::SliderFloat("Atom Distance", &distance, 1.0f, 5.0f))
      needsRebuild = true;

    if (ImGui::Combo("Structure Type", &currentStructureType, structureTypes,
                     IM_ARRAYSIZE(structureTypes))) {
      currentStructure = static_cast<StructureType>(currentStructureType);
      needsRebuild = true;
    }

    ImGui::Separator();
    ImGui::Text("Visual Parameters");
    bool sphereSizeChanged = false;
    if (ImGui::SliderFloat("Sphere Radius", &sphereRadius, 0.1f, 1.0f)) {
      sphereSizeChanged = true;
    }
    bool bondRadiusChanged = false;
    if (ImGui::SliderFloat("Bond Radius", &cylinderRadius, 0.01f, 0.2f)) {
      bondRadiusChanged = true;
    }
    ImGui::Checkbox("Show Grid", &showGrid);

    // Ising Model Controls
    ImGui::Separator();
    ImGui::Text("Ising Model Simulation");
    ImGui::Separator();

    if (ImGui::Button("Start Simulation"))
      simState = SimulationState::RUNNING;
    ImGui::SameLine();
    if (ImGui::Button("Pause Simulation"))
      simState = SimulationState::PAUSED;
    ImGui::SameLine();
    if (ImGui::Button("Single Step"))
      simState = SimulationState::STEP;

    ImGui::SliderFloat("Temperature", &temperature, 0.0f, 5.0f);
    ImGui::SliderFloat("Coupling (J)", &J, -2.0f, 2.0f);
    ImGui::SliderFloat("Magnetic Field (B)", &B, -2.0f, 2.0f);
    ImGui::SliderInt("Steps/Frame", &stepsPerFrame, 1, 1000);
    ImGui::Checkbox("Show Energy", &showEnergy);

    // Color controls
    float upColorArray[3] = {upColor.r / 255.0f, upColor.g / 255.0f,
                             upColor.b / 255.0f};
    if (ImGui::ColorEdit3("Up Spin Color", upColorArray)) {
      upColor = Color{(unsigned char)(upColorArray[0] * 255),
                      (unsigned char)(upColorArray[1] * 255),
                      (unsigned char)(upColorArray[2] * 255), 255};
    }

    float downColorArray[3] = {downColor.r / 255.0f, downColor.g / 255.0f,
                               downColor.b / 255.0f};
    if (ImGui::ColorEdit3("Down Spin Color", downColorArray)) {
      downColor = Color{(unsigned char)(downColorArray[0] * 255),
                        (unsigned char)(downColorArray[1] * 255),
                        (unsigned char)(downColorArray[2] * 255), 255};
    }

    ImGui::Separator();
    ImGui::Text("Camera Settings");
    ImGui::Separator();
    ImGui::SliderFloat("Movement Speed", &movementSpeed, 1.0f, 30.0f);
    ImGui::SliderFloat("Camera Sensitivity", &cameraSensitivity, 0.1f, 1.0f);

    // Simulation stats
    float totalEnergy = CalculateTotalEnergy(structure);
    int upSpins = 0, downSpins = 0;
    for (const auto &atom : structure) {
      if (atom.spin == Spin::UP)
        upSpins++;
      else
        downSpins++;
    }

    ImGui::Text("Total Energy: %.2f", totalEnergy);
    ImGui::Text("Up Spins: %d, Down Spins: %d", upSpins, downSpins);
    ImGui::Text("Magnetization: %.2f",
                (upSpins - downSpins) / (float)structure.size());
    ImGui::Text("FPS: %d", GetFPS());

    ImGui::End();
    rlImGuiEnd();

    if (sphereSizeChanged) {
      UnloadMesh(sphereMesh);
      sphereMesh = GenMeshSphere(sphereRadius, 16, 16);
      sphereSizeChanged = false;
    }
    if (bondRadiusChanged) {
      cylinderMeshes =
          CreateChunkedCylinderLines(structure, cylinderRadius, segments);
    }
    EndDrawing();
  }

  // Cleanup
  rlImGuiShutdown();
  UnloadMesh(sphereMesh);
  for (auto &mesh : cylinderMeshes) {
    UnloadMesh(mesh);
  }
  UnloadMaterial(sphereMaterial);
  UnloadMaterial(lineMaterial);
  CloseWindow();

  return 0;
}
