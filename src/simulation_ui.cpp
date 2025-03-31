#include "simulation_ui.h"
#include "imgui_style.h"

int runSimulation() {
  int monitor = GetCurrentMonitor();
  int screenWidth = GetMonitorWidth(monitor);
  int screenHeight = GetMonitorHeight(monitor);

  // Borderless window at monitor size
  InitWindow(screenWidth, screenHeight, "3D Ising Model Simulation");
  SetWindowPosition(screenWidth / 2, screenHeight / 2);

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
  int N = 10, O = 10, P = 10;
  float distance = 2.0f;
  float sphereRadius = 0.5f;
  float cylinderRadius = 0.05f;
  int segments = 8;
  bool showGrid = true;
  bool needsRebuild = true;
  Vector2 cameraAngle = {0};
  float movementSpeed = 10.0f;
  float cameraSensitivity = 0.3f;

  // Structure type
  StructureType currentStructure = StructureType::CUBIC;
  const char *structureTypes[] = {"Cubic", "Hexagonal", "Face-Centered Cubic",
                                  "Body-Centered Cubic"};
  int currentStructureType = 0;

  // Initialize structure
  vector<Atome> structure;
  vector<Matrix> sphereTransforms;
  vector<Mesh> cylinderMeshes;

  // Create default sphere mesh and material
  Mesh sphereMesh = GenMeshSphere(sphereRadius, 16, 16);
  Material sphereMaterial = LoadMaterialDefault();
  sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RED;

  // Create line material
  cylinderMeshes =
      CreateChunkedCylinderLines(structure, cylinderRadius, segments);
  Material lineMaterial = LoadMaterialDefault();
  lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;

  SetCustomImGuiStyle(1.5f);
  // Main game loop
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
    // Rebuild structure if needed
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

      // Initialize spins and energies
      for (auto &atom : structure) {
        atom.spin = GetRandomValue(0, 1) ? Spin::UP : Spin::DOWN;
      }
      UpdateEnergies(structure, J, B);

      // Recreate transforms
      sphereTransforms.clear();
      for (const auto &atom : structure) {
        sphereTransforms.push_back(
            MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
      }

      // Recreate cylinder meshes
      cylinderMeshes =
          CreateChunkedCylinderLines(structure, cylinderRadius, segments);

      needsRebuild = false;
    }

    // Run simulation
    if (simState == SimulationState::RUNNING ||
        simState == SimulationState::STEP) {
      for (int i = 0; i < stepsPerFrame; i++) {
        MonteCarloStep(structure, temperature, J, B);
      }

      if (simState == SimulationState::STEP) {
        simState = SimulationState::PAUSED;
      }
    }

    // Rendering
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);
    // Draw spheres
    for (size_t i = 0; i < structure.size(); i++) {
      Color color = (structure[i].spin == Spin::UP) ? upColor : downColor;
      if (showEnergy) {
        // Calculate normalized energy (0-1 range)
        float minE = -fabsf(J) * structure[i].neigh.size() - fabsf(B);
        float maxE = fabsf(J) * structure[i].neigh.size() + fabsf(B);
        float normalizedEnergy = (structure[i].energy - minE) / (maxE - minE);
        normalizedEnergy = Clamp(normalizedEnergy, 0.0f, 1.0f);

        if (normalizedEnergy < 0.25f) {
          float t = normalizedEnergy / 0.25f;
          color.r = (unsigned char)(72 * t + 68 * (1 - t));
          color.g = (unsigned char)(33 * t + 1 * (1 - t));
          color.b = (unsigned char)(116 * t + 84 * (1 - t));
        } else if (normalizedEnergy < 0.5f) {
          float t = (normalizedEnergy - 0.25f) / 0.25f;
          color.r = (unsigned char)(32 * t + 72 * (1 - t));
          color.g = (unsigned char)(144 * t + 33 * (1 - t));
          color.b = (unsigned char)(140 * t + 116 * (1 - t));
        } else if (normalizedEnergy < 0.75f) {
          float t = (normalizedEnergy - 0.5f) / 0.25f;
          color.r = (unsigned char)(253 * t + 32 * (1 - t));
          color.g = (unsigned char)(231 * t + 144 * (1 - t));
          color.b = (unsigned char)(37 * t + 140 * (1 - t));
        } else {
          float t = (normalizedEnergy - 0.75f) / 0.25f;
          color.r = (unsigned char)(253 * t + 253 * (1 - t));
          color.g = (unsigned char)(231 * t + 231 * (1 - t));
          color.b = (unsigned char)(37 * t + 37 * (1 - t));
        }
        color.a = 255;
      }
      sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = color;
      DrawMesh(sphereMesh, sphereMaterial, sphereTransforms[i]);
    }

    // Draw cylinders
    for (const auto &mesh : cylinderMeshes) {
      DrawMesh(mesh, lineMaterial, MatrixIdentity());
    }

    if (showGrid)
      DrawGrid(40, 1);
    EndMode3D();

    // UI
    rlImGuiBegin();
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Set drawer width
    float drawerWidth = (int)(screenWidth / 3);

    // Configure the window to snap to the right edge and be full height
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(drawerWidth, (float)screenHeight));

    // Remove window decorations for a cleaner drawer look
    ImGuiWindowFlags drawerFlags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Controls", nullptr, drawerFlags);

    // Structure controls
    if (ImGui::SliderInt("Grid Size X", &N, 1, 10))
      needsRebuild = true;
    if (ImGui::SliderInt("Grid Size Y", &O, 1, 10))
      needsRebuild = true;
    if (ImGui::SliderInt("Grid Size Z", &P, 1, 10))
      needsRebuild = true;
    if (ImGui::SliderFloat("Atom Distance", &distance, 1.0f, 5.0f))
      needsRebuild = true;

    if (ImGui::Combo("Structure Type", &currentStructureType, structureTypes,
                     IM_ARRAYSIZE(structureTypes))) {
      currentStructure = static_cast<StructureType>(currentStructureType);
      needsRebuild = true;
    }

    ImGui::Separator();
    ImGui::Text("Visual Parameters");
    bool sphereSizeChanged = false;
    if (ImGui::SliderFloat("Sphere Radius", &sphereRadius, 0.1f, 1.0f)) {
      sphereSizeChanged = true;
    }
    bool bondRadiusChanged = false;
    if (ImGui::SliderFloat("Bond Radius", &cylinderRadius, 0.01f, 0.2f)) {
      bondRadiusChanged = true;
    }
    ImGui::Checkbox("Show Grid", &showGrid);

    // Ising Model Controls
    ImGui::Separator();
    ImGui::Text("Ising Model Simulation");
    ImGui::Separator();

    if (ImGui::Button("Start Simulation"))
      simState = SimulationState::RUNNING;
    ImGui::SameLine();
    if (ImGui::Button("Pause Simulation"))
      simState = SimulationState::PAUSED;
    ImGui::SameLine();
    if (ImGui::Button("Single Step"))
      simState = SimulationState::STEP;

    ImGui::SliderFloat("Temperature", &temperature, 0.0f, 5.0f);
    ImGui::SliderFloat("Coupling (J)", &J, -2.0f, 2.0f);
    ImGui::SliderFloat("Magnetic Field (B)", &B, -2.0f, 2.0f);
    ImGui::SliderInt("Steps/Frame", &stepsPerFrame, 1, 1000);
    ImGui::Checkbox("Show Energy", &showEnergy);

    // Color controls
    float upColorArray[3] = {upColor.r / 255.0f, upColor.g / 255.0f,
                             upColor.b / 255.0f};
    if (ImGui::ColorEdit3("Up Spin Color", upColorArray)) {
      upColor = Color{(unsigned char)(upColorArray[0] * 255),
                      (unsigned char)(upColorArray[1] * 255),
                      (unsigned char)(upColorArray[2] * 255), 255};
    }

    float downColorArray[3] = {downColor.r / 255.0f, downColor.g / 255.0f,
                               downColor.b / 255.0f};
    if (ImGui::ColorEdit3("Down Spin Color", downColorArray)) {
      downColor = Color{(unsigned char)(downColorArray[0] * 255),
                        (unsigned char)(downColorArray[1] * 255),
                        (unsigned char)(downColorArray[2] * 255), 255};
    }

    ImGui::Separator();
    ImGui::Text("Camera Settings");
    ImGui::Separator();
    ImGui::SliderFloat("Movement Speed", &movementSpeed, 1.0f, 30.0f);
    ImGui::SliderFloat("Camera Sensitivity", &cameraSensitivity, 0.1f, 1.0f);

    // Simulation stats
    float totalEnergy = CalculateTotalEnergy(structure);
    int upSpins = 0, downSpins = 0;
    for (const auto &atom : structure) {
      if (atom.spin == Spin::UP)
        upSpins++;
      else
        downSpins++;
    }

    ImGui::Text("Total Energy: %.2f", totalEnergy);
    ImGui::Text("Up Spins: %d, Down Spins: %d", upSpins, downSpins);
    ImGui::Text("Magnetization: %.2f",
                (upSpins - downSpins) / (float)structure.size());
    ImGui::Text("FPS: %d", GetFPS());

    ImGui::End();
    rlImGuiEnd();

    if (sphereSizeChanged) {
      UnloadMesh(sphereMesh);
      sphereMesh = GenMeshSphere(sphereRadius, 16, 16);
      sphereSizeChanged = false;
    }
    if (bondRadiusChanged) {
      cylinderMeshes =
          CreateChunkedCylinderLines(structure, cylinderRadius, segments);
    }
    EndDrawing();
  }

  // Cleanup
  rlImGuiShutdown();
  UnloadMesh(sphereMesh);
  for (auto &mesh : cylinderMeshes) {
    UnloadMesh(mesh);
  }
  UnloadMaterial(sphereMaterial);
  UnloadMaterial(lineMaterial);
  CloseWindow();

  return 0;
}
