#include "raylib.h"
#include "raymath.h"
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
  vector<Vector3> neigh;
  float energy = 0.0f;
  float radius = 0.5f;
};

// now we draw a whole structure
vector<vector<vector<atome>>> make_struc(const int x, const int y, const int z,
                                         const float distance) {
  vector<vector<vector<atome>>> points(
      x, vector<vector<atome>>(y, vector<atome>(z)));
  float pos_x;
  float pos_y;
  float pos_z;
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        pos_x = i * distance;
        pos_y = j * distance;
        pos_z = k * distance;
        points[i][j][k].pos = {pos_x, pos_y, pos_z};

        vector<Vector3> neighbors;

        if (i > 0)
          neighbors.push_back({(i - 1) * distance, pos_y, pos_z});
        if (i < x - 1)
          neighbors.push_back({(i + 1) * distance, pos_y, pos_z});
        if (j > 0)
          neighbors.push_back({pos_x, (j - 1) * distance, pos_z});
        if (j < y - 1)
          neighbors.push_back({pos_x, (j + 1) * distance, pos_z});
        if (k > 0)
          neighbors.push_back({pos_x, pos_y, (k - 1) * distance});
        if (k < z - 1)
          neighbors.push_back({pos_x, pos_y, (k + 1) * distance});

        points[i][j][k].neigh = neighbors;
      }
    }
  }

  return points;
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

  while (!WindowShouldClose()) {
    // Handle camera movement
    UpdateCamera(&camera,
                 CAMERA_FIRST_PERSON); // Allows movement with WASD & mouse

    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawFPS(1000, 10);
    BeginMode3D(camera);
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        for (int k = 0; k < N; k++) {
          Matrix transform = MatrixTranslate(structure[i][j][k].pos.x,
                                             structure[i][j][k].pos.y,
                                             structure[i][j][k].pos.z);
          DrawMesh(sphereMesh, sphereMaterial, transform);
          Vector3 posA = structure[i][j][k].pos;
          for (Vector3 posB : structure[i][j][k].neigh) {
            Vector3 midPoint = Vector3Scale(Vector3Add(posA, posB), 0.5f);

            Vector3 dir = Vector3Subtract(posB, posA);
            float length = Vector3Length(dir);

            Vector3 up = {0.0f, 1.0f, 0.0f};
            Vector3 axis = Vector3CrossProduct(up, dir);
            float angle = acos(Vector3DotProduct(up, Vector3Normalize(dir)));

            Matrix rotation = MatrixRotate(axis, angle);
            Matrix linkTransform = MatrixMultiply(
                MatrixMultiply(MatrixScale(1.0f, length / 2.0f, 1.0f),
                               rotation),
                MatrixTranslate(midPoint.x, midPoint.y, midPoint.z));
            DrawMesh(lineMesh, lineMaterial, linkTransform);
          }
        }
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
