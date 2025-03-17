#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <vector>

using namespace std;

int cube(float distance, float radius, Vector3 start) {
  Vector3 base[8] = {
      {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1},
      {0, 1, 0}, {1, 1, 0}, {1, 1, 1}, {0, 1, 1},
  };
  Vector3 points[8];
  Vector3 v;
  for (int i = 0; i < 8; i++) {
    v = Vector3Scale(base[i], distance);
    v = Vector3Add(v, start);
    points[i] = v;
  }
  for (int i = 0; i < 8; i++) {
    DrawSphere((Vector3)points[i], radius, RED);
  };
  for (int i = 0; i < 4; i++) {
    DrawLine3D((Vector3)points[i], (Vector3)points[(i + 1) % 4], BLACK);

    DrawLine3D((Vector3)points[i + 4], (Vector3)points[((i + 1) % 4) + 4],
               BLACK);

    DrawLine3D((Vector3)points[i], (Vector3)points[i + 4], BLACK);
  }
  return 0;
}

enum class Spin : int {
  UP = 1,
  DOWN = -1,
};

struct atome {
  Vector3 pos;
  Spin spin = Spin::UP;
  vector<Vector3> neigh;
  float energy = 0.0;
};

vector<vector<vector<atome>>> make_struc(const int x, const int y, const int z,
                                         const float distance) {
  vector<vector<vector<atome>>> points;
  float pos_x;
  float pos_y;
  float pos_z;
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        pos_x = i * distance;
        pos_y = i * distance;
        pos_z = i * distance;
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
      }
    }
  }

  return points;
}

int main() {
  InitWindow(1200, 780, "Simulating Structures");
  SetTargetFPS(60);

  Vector3 target = {10, 10, 10};

  // Define the 3D camera
  Camera3D camera = {{0, 10, 30}, // Position
                     target,      // Target (where it's looking)
                     {0, 1, 0},   // Up direction
                     60.0f,       // Field of view
                     CAMERA_PERSPECTIVE};

  while (!WindowShouldClose()) {
    // Handle camera movement
    UpdateCamera(&camera,
                 CAMERA_FIRST_PERSON); // Allows movement with WASD & mouse

    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    DrawGrid(40, 1);

    EndMode3D();

    DrawText("Use WASD + Mouse to move", 10, 10, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
