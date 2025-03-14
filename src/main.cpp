#include <cmath>
#include <raylib.h>

int cube() {
  Vector3 points[8] = {
      {0, 0, 0}, {4, 0, 0}, {4, 0, 4}, {0, 0, 4},
      {0, 4, 0}, {4, 4, 0}, {4, 4, 4}, {0, 4, 4},
  };
  for (int i = 0; i < 8; i++) {
    DrawSphere((Vector3)points[i], 1.0f, RED);
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
  InitWindow(600, 400, "Simulating Structures");
  SetTargetFPS(60);

  Vector3 target = {2, 2, 2};

  // Define the 3D camera
  Camera3D camera = {{0, 2, 20}, // Position
                     target,     // Target (where it's looking)
                     {0, 1, 0},  // Up direction
                     60.0f,      // Field of view
                     CAMERA_PERSPECTIVE};

  while (!WindowShouldClose()) {
    // Handle camera movement
    UpdateCamera(&camera,
                 CAMERA_FIRST_PERSON); // Allows movement with WASD & mouse

    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);
    cube();

    EndMode3D();

    DrawText("Use WASD + Mouse to move", 10, 10, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
