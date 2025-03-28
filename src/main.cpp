#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>
// Add ImGui includes
#include "imgui.h"
#include "rlImGui.h"

using namespace std;

enum class Spin : int {
  UP = 1,
  DOWN = -1,
};

// Atom struct with reduced memory footprint
struct Atome {
  Vector3 pos;
  Spin spin = Spin::UP;
  vector<int> neigh; // Stores indices instead of full positions
  float energy = 0.0f;
  float radius = 0.5f;
};

// 3D Grid Structure
vector<Atome> make_struc(int x, int y, int z, float distance) {
  vector<Atome> points(x * y * z);
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

Mesh CreateBakedCylinderLines(const vector<Atome>& structure, float radius = 0.05f, int segments = 8) {
    // Calculate total needed vertices and triangles
    int cylinderCount = 0;
    for (const auto& atom : structure) {
        for (int neighborIdx : atom.neigh) {
            if (neighborIdx > &atom - &structure[0]) {
                cylinderCount++;
            }
        }
    }

    const int vertsPerCylinder = segments * 2; // 2 rings (top and bottom)
    const int trisPerCylinder = segments * 2;   // 2 triangles per segment

    Mesh mesh = {0};
    mesh.vertexCount = cylinderCount * vertsPerCylinder;
    mesh.triangleCount = cylinderCount * trisPerCylinder;

    // Allocate memory
    mesh.vertices = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    int vertexOffset = 0;
    int indexOffset = 0;

    for (const auto& atom : structure) {
        for (int neighborIdx : atom.neigh) {
            if (neighborIdx > &atom - &structure[0]) {
                Vector3 start = atom.pos;
                Vector3 end = structure[neighborIdx].pos;
                Vector3 direction = Vector3Normalize(Vector3Subtract(end, start));
                float length = Vector3Distance(start, end);

                // Find an arbitrary perpendicular vector
                Vector3 perp;
                if (fabs(direction.x) < fabs(direction.y)) {
                    perp = Vector3Normalize(Vector3CrossProduct(direction, (Vector3){1,0,0}));
                } else {
                    perp = Vector3Normalize(Vector3CrossProduct(direction, (Vector3){0,1,0}));
                }

                // Generate cylinder vertices
                for (int i = 0; i < segments; i++) {
                    float angle = 2*PI*i/segments;
                    Vector3 circleVec = Vector3Scale(Vector3Add(
                        Vector3Scale(perp, cosf(angle)),
                        Vector3Scale(Vector3CrossProduct(perp, direction), sinf(angle))
                    ), radius);

                    // Bottom ring
                    mesh.vertices[(vertexOffset + i)*3 + 0] = start.x + circleVec.x;
                    mesh.vertices[(vertexOffset + i)*3 + 1] = start.y + circleVec.y;
                    mesh.vertices[(vertexOffset + i)*3 + 2] = start.z + circleVec.z;
                    
                    // Top ring
                    mesh.vertices[(vertexOffset + segments + i)*3 + 0] = end.x + circleVec.x;
                    mesh.vertices[(vertexOffset + segments + i)*3 + 1] = end.y + circleVec.y;
                    mesh.vertices[(vertexOffset + segments + i)*3 + 2] = end.z + circleVec.z;

                    // Normals (point outward from center)
                    mesh.normals[(vertexOffset + i)*3 + 0] = circleVec.x/radius;
                    mesh.normals[(vertexOffset + i)*3 + 1] = circleVec.y/radius;
                    mesh.normals[(vertexOffset + i)*3 + 2] = circleVec.z/radius;
                    
                    mesh.normals[(vertexOffset + segments + i)*3 + 0] = circleVec.x/radius;
                    mesh.normals[(vertexOffset + segments + i)*3 + 1] = circleVec.y/radius;
                    mesh.normals[(vertexOffset + segments + i)*3 + 2] = circleVec.z/radius;
                }

                // Generate indices
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

                vertexOffset += vertsPerCylinder; // Move to the next cylinder's vertices
            }
        }
    }

    UploadMesh(&mesh, false);
    return mesh;
}

// Draw multiple instances using instanced rendering
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
  // Initialize window
  InitWindow(1200, 780, "Atom Structure Visualization with ImGui");

  // Camera setup
  Camera3D camera = {
      {0, 10, 30}, {10, 10, 10}, {0, 1, 0}, 60.0f, CAMERA_PERSPECTIVE};

  // Initialize ImGui
  rlImGuiSetup(true);

  // Grid Setup - make these configurable via ImGui
  int N = 10;
  float distance = 2.0f;
  float sphereRadius = 0.5f;
  float cylinderRadius = 0.05f;
  int segments = 8;
  Color sphereColor = RED;
  Color cylinderColor = BLACK;
  bool showGrid = true;
  
  // Create initial structure
  auto structure = make_struc(N, N, N, distance);

  // Precompute transformations for spheres
  vector<Matrix> sphereTransforms;
  for (const auto &atom : structure) {
    sphereTransforms.push_back(
        MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
  }

  // Sphere & Cylinder Meshes
  Mesh sphereMesh = GenMeshSphere(sphereRadius, 10, 10);
  Material sphereMaterial = LoadMaterialDefault();
  sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = sphereColor;

  Mesh bakedLineMesh = CreateBakedCylinderLines(structure, cylinderRadius, segments);
  Material lineMaterial = LoadMaterialDefault();
  lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = cylinderColor;

  // Variables for structure rebuilding
  bool needsRebuild = false;
  
  // Main loop
  while (!WindowShouldClose()) {
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);

    // Begin drawing
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // ImGui UI
    rlImGuiBegin();
    
    ImGui::Begin("Atom Structure Controls");
    
    // Structure parameters
    bool structureChanged = false;
    
    ImGui::Text("Structure Parameters");
    ImGui::Separator();
    
    if (ImGui::SliderInt("Grid Size", &N, 1, 20)) {
        structureChanged = true;
    }
    
    if (ImGui::SliderFloat("Atom Distance", &distance, 1.0f, 5.0f)) {
        structureChanged = true;
    }
    
    ImGui::Separator();
    ImGui::Text("Visual Parameters");
    ImGui::Separator();
    
    // Visual parameters
    ImGui::SliderFloat("Sphere Radius", &sphereRadius, 0.1f, 1.0f);
    ImGui::SliderFloat("Bond Radius", &cylinderRadius, 0.01f, 0.2f);
    ImGui::SliderInt("Bond Segments", &segments, 3, 16);
    
    // Colors
    float sphereColorArray[3] = {
        sphereColor.r / 255.0f, 
        sphereColor.g / 255.0f, 
        sphereColor.b / 255.0f
    };
    
    float cylinderColorArray[3] = {
        cylinderColor.r / 255.0f, 
        cylinderColor.g / 255.0f, 
        cylinderColor.b / 255.0f
    };
    
    if (ImGui::ColorEdit3("Sphere Color", sphereColorArray)) {
        sphereColor = (Color){
            (unsigned char)(sphereColorArray[0] * 255),
            (unsigned char)(sphereColorArray[1] * 255),
            (unsigned char)(sphereColorArray[2] * 255),
            255
        };
        sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = sphereColor;
    }
    
    if (ImGui::ColorEdit3("Bond Color", cylinderColorArray)) {
        cylinderColor = (Color){
            (unsigned char)(cylinderColorArray[0] * 255),
            (unsigned char)(cylinderColorArray[1] * 255),
            (unsigned char)(cylinderColorArray[2] * 255),
            255
        };
        lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = cylinderColor;
    }
    
    ImGui::Checkbox("Show Grid", &showGrid);
    
    if (ImGui::Button("Rebuild Structure") || structureChanged) {
        needsRebuild = true;
    }
    
    ImGui::Separator();
    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", 
                camera.position.x, camera.position.y, camera.position.z);
    ImGui::Text("FPS: %d", GetFPS());
    
    ImGui::End();
    
    // End ImGui frame
    rlImGuiEnd();
    
    // Rebuild structure if needed
    if (needsRebuild) {
        // Rebuild structure
        structure = make_struc(N, N, N, distance);
        
        // Update sphere transforms
        sphereTransforms.clear();
        for (const auto &atom : structure) {
            sphereTransforms.push_back(
                MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
        }
        
        // Update sphere mesh
        UnloadMesh(sphereMesh);
        sphereMesh = GenMeshSphere(sphereRadius, 10, 10);
        
        // Update bond mesh
        UnloadMesh(bakedLineMesh);
        bakedLineMesh = CreateBakedCylinderLines(structure, cylinderRadius, segments);
        
        needsRebuild = false;
    }
    
    // 3D Rendering
    BeginMode3D(camera);
    
    // Draw atoms and bonds
    DrawInstanced(sphereMesh, sphereMaterial, sphereTransforms);
    DrawMesh(bakedLineMesh, lineMaterial, MatrixIdentity());
    
    // Draw grid if enabled
    if (showGrid) {
        DrawGrid(40, 1);
    }
    
    EndMode3D();
    
    // Instructions
    DrawText("Use WASD + Mouse to move", 10, 10, 20, DARKGRAY);
    
    EndDrawing();
  }

  // Cleanup
  rlImGuiShutdown();
  UnloadMesh(sphereMesh);
  UnloadMesh(bakedLineMesh);
  UnloadMaterial(sphereMaterial);
  UnloadMaterial(lineMaterial);
  CloseWindow();

  return 0;
}
