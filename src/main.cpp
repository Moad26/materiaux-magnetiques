#include <cmath>
#include <raylib.h>
#include <raymath.h>

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
    cube(4, 1, target);

    DrawGrid(40,1);

    EndMode3D();

    DrawText("Use WASD + Mouse to move", 10, 10, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
