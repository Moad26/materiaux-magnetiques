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

struct Atome {
  Vector3 pos;
  Spin spin = Spin::UP;
  vector<int> neigh;
  float energy = 0.0f;
  float radius = 0.5f;
};

vector<Atome> make_struc(int x, int y, int z, float distance) {
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

Mesh CreateBakedCylinderLines(const vector<Atome> &structure,
                              float radius = 0.05f, int segments = 8) {
  int cylinderCount = 0;
  for (const auto &atom : structure) {
    for (int neighborIdx : atom.neigh) {
      if (neighborIdx > &atom - &structure[0]) {
        cylinderCount++;
      }
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
        float length = Vector3Distance(start, end);

        Vector3 perp;
        if (fabs(direction.x) < fabs(direction.y)) {
          perp = Vector3Normalize(
              Vector3CrossProduct(direction, (Vector3){1, 0, 0}));
        } else {
          perp = Vector3Normalize(
              Vector3CrossProduct(direction, (Vector3){0, 1, 0}));
        }

        for (int i = 0; i < segments; i++) {
          float angle = 2 * PI * i / segments;
          Vector3 circleVec = Vector3Scale(
              Vector3Add(Vector3Scale(perp, cosf(angle)),
                         Vector3Scale(Vector3CrossProduct(perp, direction),
                                      sinf(angle))),
              radius);

          mesh.vertices[(vertexOffset + i) * 3 + 0] = start.x + circleVec.x;
          mesh.vertices[(vertexOffset + i) * 3 + 1] = start.y + circleVec.y;
          mesh.vertices[(vertexOffset + i) * 3 + 2] = start.z + circleVec.z;

          mesh.vertices[(vertexOffset + segments + i) * 3 + 0] =
              end.x + circleVec.x;
          mesh.vertices[(vertexOffset + segments + i) * 3 + 1] =
              end.y + circleVec.y;
          mesh.vertices[(vertexOffset + segments + i) * 3 + 2] =
              end.z + circleVec.z;

          mesh.normals[(vertexOffset + i) * 3 + 0] = circleVec.x / radius;
          mesh.normals[(vertexOffset + i) * 3 + 1] = circleVec.y / radius;
          mesh.normals[(vertexOffset + i) * 3 + 2] = circleVec.z / radius;

          mesh.normals[(vertexOffset + segments + i) * 3 + 0] =
              circleVec.x / radius;
          mesh.normals[(vertexOffset + segments + i) * 3 + 1] =
              circleVec.y / radius;
          mesh.normals[(vertexOffset + segments + i) * 3 + 2] =
              circleVec.z / radius;
        }

        for (int i = 0; i < segments; i++) {
          int next = (i + 1) % segments;

          mesh.indices[indexOffset++] = vertexOffset + i;
          mesh.indices[indexOffset++] = vertexOffset + next;
          mesh.indices[indexOffset++] = vertexOffset + segments + i;

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
  const int screenWidth = 1920;
  const int screenHeight = 1080;

  InitWindow(screenWidth, screenHeight, "Visualisation de structure cubique");
  rlImGuiSetup(true);
  HideCursor();

  // Camera setup
  Camera3D camera = {0};
  camera.position = {0, 10, 30};
  camera.target = {10, 10, 10};
  camera.up = {0, 1, 0};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  // Simulation parameters
  int N = 5;
  int O = 5;
  int P = 5;
  float distance = 2.0f;
  float sphereRadius = 0.5f;
  float cylinderRadius = 0.05f;
  int segments = 8;
  Color sphereColor = RED;
  Color cylinderColor = BLACK;
  bool showGrid = true;
  bool needsRebuild = false;

  // Camera control variables
  Vector2 cameraAngle = {0};
  float cameraSpeed = 0.1f;
  float cameraSensitivity = 0.3f;

  // Initialize structure
  auto structure = make_struc(N, O, P, distance);
  vector<Matrix> sphereTransforms;
  for (const auto &atom : structure) {
    sphereTransforms.push_back(
        MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
  }

  // Create meshes
  Mesh sphereMesh = GenMeshSphere(sphereRadius, 10, 10);
  Material sphereMaterial = LoadMaterialDefault();
  sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = sphereColor;

  Mesh bakedLineMesh =
      CreateBakedCylinderLines(structure, cylinderRadius, segments);
  Material lineMaterial = LoadMaterialDefault();
  lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = cylinderColor;
  while (!WindowShouldClose()) {
    // Handle input
    Vector2 mouseDelta = GetMouseDelta();
    cameraAngle.x -= mouseDelta.y * cameraSensitivity;
    cameraAngle.y -= mouseDelta.x * cameraSensitivity;
    cameraAngle.x = Clamp(cameraAngle.x, -89.0f, 89.0f);

    // Calculate camera direction
    Vector3 direction = {
        cosf(DEG2RAD * cameraAngle.y) * cosf(DEG2RAD * cameraAngle.x),
        sinf(DEG2RAD * cameraAngle.x),
        sinf(DEG2RAD * cameraAngle.y) * cosf(DEG2RAD * cameraAngle.x)};
    direction = Vector3Normalize(direction);

    // Movement input
    Vector3 moveDir = {0};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_Z))
      moveDir.z = 1;
    if (IsKeyDown(KEY_S))
      moveDir.z = -1;
    if (IsKeyDown(KEY_D))
      moveDir.x = 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_Q))
      moveDir.x = -1;

    // Normalize movement
    if (Vector3Length(moveDir) > 0) {
      moveDir = Vector3Normalize(moveDir);
    }

    // Apply movement
    Vector3 forward = {direction.x, 0, direction.z};
    forward = Vector3Normalize(forward);
    Vector3 right = Vector3CrossProduct(forward, camera.up);

    camera.position = Vector3Add(
        camera.position, Vector3Scale(forward, moveDir.z * cameraSpeed));
    camera.position = Vector3Add(camera.position,
                                 Vector3Scale(right, moveDir.x * cameraSpeed));

    // Update camera target
    camera.target = Vector3Add(camera.position, direction);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // 1. Render 3D content FIRST
    BeginMode3D(camera);
    DrawInstanced(sphereMesh, sphereMaterial, sphereTransforms);
    DrawMesh(bakedLineMesh, lineMaterial, MatrixIdentity());
    if (showGrid)
      DrawGrid(40, 1);
    EndMode3D();

    // 2. THEN render UI on top
    rlImGuiBegin();
    ImGui::Begin("Atom Structure Controls");
    ImGui::Text("Structure Parameters");
    ImGui::Separator();

    bool structureChanged = false;
    if (ImGui::SliderInt("Grid Size", &N, 1, 20))
      structureChanged = true;
    if (ImGui::SliderInt("Grid Size Y", &O, 1, 20))
      structureChanged = true;
    if (ImGui::SliderInt("Grid Size Z", &P, 1, 20))
      structureChanged = true;
    if (ImGui::SliderFloat("Atom Distance", &distance, 1.0f, 5.0f))
      structureChanged = true;

    ImGui::Separator();
    ImGui::Text("Visual Parameters");
    ImGui::Separator();

    ImGui::SliderFloat("Sphere Radius", &sphereRadius, 0.1f, 1.0f);
    ImGui::SliderFloat("Bond Radius", &cylinderRadius, 0.01f, 0.2f);
    ImGui::SliderInt("Bond Segments", &segments, 3, 16);

    float sphereColorArray[3] = {sphereColor.r / 255.0f, sphereColor.g / 255.0f,
                                 sphereColor.b / 255.0f};
    float cylinderColorArray[3] = {cylinderColor.r / 255.0f,
                                   cylinderColor.g / 255.0f,
                                   cylinderColor.b / 255.0f};

    if (ImGui::ColorEdit3("Sphere Color", sphereColorArray)) {
      sphereColor = (Color){(unsigned char)(sphereColorArray[0] * 255),
                            (unsigned char)(sphereColorArray[1] * 255),
                            (unsigned char)(sphereColorArray[2] * 255), 255};
      sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = sphereColor;
    }

    if (ImGui::ColorEdit3("Bond Color", cylinderColorArray)) {
      cylinderColor =
          (Color){(unsigned char)(cylinderColorArray[0] * 255),
                  (unsigned char)(cylinderColorArray[1] * 255),
                  (unsigned char)(cylinderColorArray[2] * 255), 255};
      lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = cylinderColor;
    }

    ImGui::Checkbox("Show Grid", &showGrid);

    if (ImGui::Button("Rebuild Structure") || structureChanged) {
      needsRebuild = true;
    }

    ImGui::Separator();
    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f", camera.position.x,
                camera.position.y, camera.position.z);
    ImGui::Text("FPS: %d", GetFPS());
    ImGui::End();
    rlImGuiEnd();

    DrawText("Use WASD + Mouse to move", 10, 10, 20, DARKGRAY);

    if (needsRebuild) {
      structure = make_struc(N, N, N, distance);

      sphereTransforms.clear();
      for (const auto &atom : structure) {
        sphereTransforms.push_back(
            MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
      }

      UnloadMesh(sphereMesh);
      sphereMesh = GenMeshSphere(sphereRadius, 10, 10);

      UnloadMesh(bakedLineMesh);
      bakedLineMesh =
          CreateBakedCylinderLines(structure, cylinderRadius, segments);

      needsRebuild = false;
    }

    EndDrawing();
  }

  rlImGuiShutdown();
  UnloadMesh(sphereMesh);
  UnloadMesh(bakedLineMesh);
  UnloadMaterial(sphereMaterial);
  UnloadMaterial(lineMaterial);
  CloseWindow();

  return 0;
}
