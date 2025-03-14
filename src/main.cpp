#include <cmath>
#include <raylib.h>

int main() {
  InitWindow(600, 400, "Simulating Structures");
  SetTargetFPS(60);

  Vector3 target = {2, 2, 2};
  Vector3 pos1 = {0, 0, 0};
  Vector3 pos2 = {4, 0, 0};
  Vector3 pos3 = {4, 0, 4};
  Vector3 pos4 = {0, 0, 4};
  Vector3 pos5 = {0, 4, 0};
  Vector3 pos6 = {4, 4, 0};
  Vector3 pos7 = {4, 4, 4};
  Vector3 pos8 = {0, 4, 4};

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
    DrawSphere((Vector3)pos1, 1.0f, RED); // Draw a sphere at the origin
    DrawSphere((Vector3)pos2, 1.0f, RED);
    DrawSphere((Vector3)pos3, 1.0f, RED);
    DrawSphere((Vector3)pos4, 1.0f, RED);
    DrawSphere((Vector3)pos5, 1.0f, RED);
    DrawSphere((Vector3)pos6, 1.0f, RED);
    DrawSphere((Vector3)pos7, 1.0f, RED);
    DrawSphere((Vector3)pos8, 1.0f, RED);
    DrawLine3D((Vector3)pos1, (Vector3)pos5, BLACK);
    DrawLine3D((Vector3)pos2, (Vector3)pos6, BLACK);
    DrawLine3D((Vector3)pos3, (Vector3)pos7, BLACK);
    DrawLine3D((Vector3)pos4, (Vector3)pos8, BLACK);
    DrawLine3D((Vector3)pos5, (Vector3)pos6, BLACK);
    DrawLine3D((Vector3)pos6, (Vector3)pos7, BLACK);
    DrawLine3D((Vector3)pos7, (Vector3)pos8, BLACK);
    DrawLine3D((Vector3)pos8, (Vector3)pos5, BLACK);
    DrawLine3D((Vector3)pos1, (Vector3)pos2, BLACK);
    DrawLine3D((Vector3)pos2, (Vector3)pos3, BLACK);
    DrawLine3D((Vector3)pos3, (Vector3)pos4, BLACK);
    DrawLine3D((Vector3)pos4, (Vector3)pos1, BLACK);

    DrawGrid(10, 1); // Adds a grid to visualize space
    EndMode3D();

    DrawText("Use WASD + Mouse to move", 10, 10, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
