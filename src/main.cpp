#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>
#include <vector>
using namespace std;

// we create the values of spin
enum class Spin : int {
  UP = 1,
  DOWN = -1,
};

// we create atom with needed specs
struct atome {
  Vector3 pos;
  Spin spin = Spin::UP;
  vector<int> neigh;
  float energy = 0.0f;
  float radius = 0.5f;
};

// now we draw a whole structure
vector<atome> make_struc(const int x, const int y, const int z,
                         const float distance) {
  vector<atome> points(x * y * z);
  auto getIndex = [=](int i, int j, int k) { return i * y * z + j * z + k; };

  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        int idx = getIndex(i, j, k);
        points[idx].pos = {i * distance, j * distance, k * distance};

        // Store indices instead of full Vector3 neighbors
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

void DrawInstanced(Mesh mesh, Material material, vector<Matrix> &transfroms) {
  rlEnableShader(material.shader.id);
  for (Matrix transform : transfroms) {
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(transform));
    DrawMesh(mesh, material, MatrixIdentity());
    rlPopMatrix();
  }
  rlDisableShader();
}

int main() {
  InitWindow(1200, 780, "Using GPU instensing");
  // SetTargetFPS(60);

  Mesh sphereMesh = GenMeshSphere(0.5f, 10, 10);
  Material sphereMaterial = LoadMaterialDefault();
  sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RED;

  Mesh lineMesh = GenMeshCylinder(0.05f, 1.0f, 8);
  Material lineMaterial = LoadMaterialDefault();
  lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;

  Vector3 target = {10, 10, 10};

  // Define the 3D camera
  Camera3D camera = {{0, 10, 30}, // Position
                     target,      // Target (where it's looking)
                     {0, 1, 0},   // Up direction
                     60.0f,       // Field of view
                     CAMERA_PERSPECTIVE};
  int N = 10;
  float distance = 2.0f;
  auto structure = make_struc(N, N, N, distance);

  vector<Matrix> sphereTransforms;
  for (const auto &atom : structure) {
    sphereTransforms.push_back(
        MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
  }

  while (!WindowShouldClose()) {
    // Handle camera movement
    UpdateCamera(&camera,
                 CAMERA_FIRST_PERSON); // Allows movement with WASD & mouse

    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawFPS(1000, 10);
    BeginMode3D(camera);

    DrawInstanced(sphereMesh, sphereMaterial, sphereTransforms);

    for (const auto &atom : structure) {
      for (int neighborIdx : atom.neigh) {
        Vector3 posA = atom.pos;
        Vector3 posB = structure[neighborIdx].pos;
        Vector3 midPoint = Vector3Scale(Vector3Add(posA, posB), 0.5f);

        Vector3 dir = Vector3Subtract(posB, posA);
        float length = Vector3Length(dir);
        Vector3 up = {0.0f, 1.0f, 0.0f};
        Vector3 axis = Vector3CrossProduct(up, dir);
        float angle = acos(Vector3DotProduct(up, Vector3Normalize(dir)));

        Matrix rotation = MatrixRotate(axis, angle);
        Matrix linkTransform = MatrixMultiply(
            MatrixMultiply(MatrixScale(1.0f, length / 2.0f, 1.0f), rotation),
            MatrixTranslate(midPoint.x, midPoint.y, midPoint.z));

        DrawMesh(lineMesh, lineMaterial, linkTransform);
      }
    }
    DrawGrid(40, 1);

    EndMode3D();

    DrawText("Use WASD + Mouse to move", 10, 10, 20, DARKGRAY);
    EndDrawing();
  }
  UnloadMesh(sphereMesh);
  UnloadMesh(lineMesh);
  UnloadMaterial(sphereMaterial);
  UnloadMaterial(lineMaterial);

  CloseWindow();
  return 0;
}
