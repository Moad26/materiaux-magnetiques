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

struct Atome {
  Vector3 pos;
  Spin spin = Spin::UP;
  vector<int> neigh;
  float energy = 0.0f;
  float radius = 0.5f;
};

vector<Atome> make_cubic_struc(int x, int y, int z, float distance) {
  vector<Atome> points(x * y * z);
  auto getIndex = [=](int i, int j, int k) { return i * y * z + j * z + k; };

  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        int idx = getIndex(i, j, k);
        points[idx].pos = {i * distance, j * distance, k * distance};

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
  float c = a * sqrt(6.0f) / 3.0f;

  for (int layer = 0; layer < x; layer++) {
    float offset = (layer % 2) * (a / 2.0f);

    for (int row = 0; row < y; row++) {
      float rowOffset = (row % 2) * (a / 2.0f);

      for (int col = 0; col < z; col++) {
        Atome atom;
        atom.pos.x = col * a + offset;
        atom.pos.y = row * (a * sqrt(3.0f) / 2.0f);
        atom.pos.z = layer * c;
        points.push_back(atom);
      }
    }
  }

  for (size_t i = 0; i < points.size(); i++) {
    points[i].neigh.clear();

    for (size_t j = 0; j < points.size(); j++) {
      if (i == j)
        continue;

      float dist = Vector3Distance(points[i].pos, points[j].pos);
      if (dist <= a * 1.1f) {
        points[i].neigh.push_back(static_cast<int>(j));
      }
    }
  }

  return points;
}

vector<Atome> make_fcc_struc(int x, int y, int z, float distance) {
  vector<Atome> points;
  points.reserve(4 * x * y * z); // FCC has 4 atoms per unit cell

  float a = distance; // lattice constant

  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        // Base positions for FCC unit cell
        Vector3 basePos = {i * a, j * a, k * a};

        // Add the 4 atoms of the FCC unit cell
        points.push_back({basePos}); // Corner (0,0,0)
        points.push_back(
            {{basePos.x + a / 2, basePos.y + a / 2, basePos.z}}); // Face center
        points.push_back(
            {{basePos.x + a / 2, basePos.y, basePos.z + a / 2}}); // Face center
        points.push_back(
            {{basePos.x, basePos.y + a / 2, basePos.z + a / 2}}); // Face center
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
      if (dist <=
          a * 0.71f) { // nearest neighbor distance in FCC is a/sqrt(2) ≈ 0.707a
        points[i].neigh.push_back(static_cast<int>(j));
      }
    }
  }

  return points;
}

vector<Atome> make_bcc_struc(int x, int y, int z, float distance) {
  vector<Atome> points;
  points.reserve(2 * x * y * z); // BCC has 2 atoms per unit cell

  float a = distance; // lattice constant

  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        // Base positions for BCC unit cell
        Vector3 basePos = {i * a, j * a, k * a};

        // Add the 2 atoms of the BCC unit cell
        points.push_back({basePos}); // Corner (0,0,0)
        points.push_back({{basePos.x + a / 2, basePos.y + a / 2,
                           basePos.z + a / 2}}); // Body center
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
      if (dist <= a * 0.87f) { // nearest neighbor distance in BCC is
                               // a*sqrt(3)/2 ≈ 0.866a
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

int main() {
  // Window setup
  InitWindow(1920, 1080, "Atom Structure Visualization");
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
  int N = 10;
  int O = 10;
  int P = 10;
  float distance = 2.0f;
  float sphereRadius = 0.5f;
  float cylinderRadius = 0.05f;
  int segments = 8;
  Color sphereColor = RED;
  Color cylinderColor = BLACK;
  bool showGrid = true;
  bool needsRebuild = false;

  // Camera control parameters
  Vector2 cameraAngle = {0};
  float movementSpeed = 10.0f; // Units per second
  float cameraSensitivity = 0.3f;

  StructureType currentStructure = StructureType ::CUBIC;
  const char *structureTypes[] = {"Cubic", "Hexagonal", "Face-Centered Cubic",
                                  "Body-Centered Cubic"};
  int currentStructureType = 0;

  // Initialize structure
  auto structure = make_cubic_struc(N, O, P, distance);
  vector<Matrix> sphereTransforms;
  for (const auto &atom : structure) {
    sphereTransforms.push_back(
        MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
  }

  // Create meshes
  Mesh sphereMesh = GenMeshSphere(sphereRadius, 16, 16);
  Material sphereMaterial = LoadMaterialDefault();
  sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = sphereColor;

  vector<Mesh> cylinderMeshes =
      CreateChunkedCylinderLines(structure, cylinderRadius, segments);
  Material lineMaterial = LoadMaterialDefault();
  lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = cylinderColor;

  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowPadding = ImVec2(20, 20);    // Inner window padding
  style.FramePadding = ImVec2(20, 20);     // Button/slider padding
  style.ItemSpacing = ImVec2(15, 15);      // Space between elements
  style.GrabMinSize = 20.0f;               // Slider handle width
  style.WindowMinSize = ImVec2(500, 1000); // Minimum window size
  // Main loop
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

    // Rendering
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);
    DrawInstanced(sphereMesh, sphereMaterial, sphereTransforms);
    for (const auto &mesh : cylinderMeshes) {
      DrawMesh(mesh, lineMaterial, MatrixIdentity());
    }
    if (showGrid)
      DrawGrid(40, 1);
    EndMode3D();

    // UI
    rlImGuiBegin();
    ImGui::Begin("Controls");
    ImGui::Text("Structure Parameters");
    ImGui::Separator();

    bool structureChanged = false;
    if (ImGui::SliderInt("Grid Size X", &N, 1, 20))
      structureChanged = true;
    if (ImGui::SliderInt("Grid Size Y", &O, 1, 20))
      structureChanged = true;
    if (ImGui::SliderInt("Grid Size Z", &P, 1, 20))
      structureChanged = true;
    if (ImGui::SliderFloat("Atom Distance", &distance, 1.0f, 5.0f))
      structureChanged = true;
    ImGui::Combo("Structure Type", &currentStructureType, structureTypes,
                 IM_ARRAYSIZE(structureTypes));
    currentStructure = static_cast<StructureType>(currentStructureType);
    ImGui::Separator();
    ImGui::Text("Visual Parameters");
    ImGui::Separator();

    ImGui::SliderFloat("Sphere Radius", &sphereRadius, 0.1f, 1.0f);
    ImGui::SliderFloat("Bond Radius", &cylinderRadius, 0.01f, 0.2f);
    ImGui::SliderInt("Bond Segments", &segments, 3, 16);

    float sphereColorArray[3] = {sphereColor.r / 255.0f, sphereColor.g / 255.0f,
                                 sphereColor.b / 255.0f};
    if (ImGui::ColorEdit3("Sphere Color", sphereColorArray)) {
      sphereColor = (Color){(unsigned char)(sphereColorArray[0] * 255),
                            (unsigned char)(sphereColorArray[1] * 255),
                            (unsigned char)(sphereColorArray[2] * 255), 255};
      sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = sphereColor;
    }

    float cylinderColorArray[3] = {cylinderColor.r / 255.0f,
                                   cylinderColor.g / 255.0f,
                                   cylinderColor.b / 255.0f};
    if (ImGui::ColorEdit3("Bond Color", cylinderColorArray)) {
      cylinderColor =
          (Color){(unsigned char)(cylinderColorArray[0] * 255),
                  (unsigned char)(cylinderColorArray[1] * 255),
                  (unsigned char)(cylinderColorArray[2] * 255), 255};
      lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = cylinderColor;
    }

    ImGui::Checkbox("Show Grid", &showGrid);

    ImGui::Separator();
    ImGui::Text("Camera Settings");
    ImGui::Separator();
    ImGui::SliderFloat("Movement Speed", &movementSpeed, 1.0f, 30.0f);
    ImGui::SliderFloat("Camera Sensitivity", &cameraSensitivity, 0.1f, 1.0f);

    if (ImGui::Button("Rebuild Structure") || structureChanged) {
      needsRebuild = true;
    }

    ImGui::Separator();
    ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", camera.position.x,
                camera.position.y, camera.position.z);
    ImGui::Text("FPS: %d", GetFPS());
    ImGui::End();
    rlImGuiEnd();

    // Rebuild if needed
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
      sphereTransforms.clear();
      for (const auto &atom : structure) {
        sphereTransforms.push_back(
            MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
      }
      UnloadMesh(sphereMesh);
      sphereMesh = GenMeshSphere(sphereRadius, 16, 16);

      cylinderMeshes =
          CreateChunkedCylinderLines(structure, cylinderRadius, segments);

      needsRebuild = false;
    }
    EndDrawing();
  }

  // Cleanup
  rlImGuiShutdown();
  for (auto &mesh : cylinderMeshes) {
    UnloadMesh(mesh);
  }
  UnloadMesh(sphereMesh);
  UnloadMaterial(sphereMaterial);
  UnloadMaterial(lineMaterial);
  CloseWindow();

  return 0;
}
