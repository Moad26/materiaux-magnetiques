#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <vector>

using namespace std;

// first test of 3d structure in raylib and could be desposed of
int cube(float distance, float radius, Vector3 start) {
  // initialise a base array
  Vector3 base[8] = {
      {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1},
      {0, 1, 0}, {1, 1, 0}, {1, 1, 1}, {0, 1, 1},
  };

  Vector3 points[8];
  Vector3 v;
  // here we create a array of points with proper distance and starting point
  for (int i = 0; i < 8; i++) {
    v = Vector3Scale(base[i], distance);
    v = Vector3Add(v, start);
    points[i] = v;
  }
  // we draw the spheres in pos of points
  for (int i = 0; i < 8; i++) {
    DrawSphere((Vector3)points[i], radius, RED);
  };
  // now we draw links between those shperes
  for (int i = 0; i < 4; i++) {
    DrawLine3D((Vector3)points[i], (Vector3)points[(i + 1) % 4], BLACK);

    DrawLine3D((Vector3)points[i + 4], (Vector3)points[((i + 1) % 4) + 4],
               BLACK);

    DrawLine3D((Vector3)points[i], (Vector3)points[i + 4], BLACK);
  }
  return 0;
}

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
  InitWindow(1200, 780, "Simulating Structures");
  SetTargetFPS(60);

  Vector3 target = {10, 10, 10};

  // Define the 3D camera
  Camera3D camera = {{0, 10, 30}, // Position
                     target,      // Target (where it's looking)
                     {0, 1, 0},   // Up direction
                     60.0f,       // Field of view
                     CAMERA_PERSPECTIVE};
  int N = 5;
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
          DrawSphere(structure[i][j][k].pos, structure[i][j][k].radius, RED);

          for(const auto& neighbor:structure[i][j][k].neigh){
            DrawLine3D(structure[i][j][k].pos, neighbor, BLACK);
          }
        }
      }
    }

    DrawGrid(40, 1);

    EndMode3D();

    DrawText("Use WASD + Mouse to move", 10, 10, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
