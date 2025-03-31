#include "simulation_ui.h"
#include "imgui_style.h"
#include <algorithm>

// Variables globales pour l'état visuel
Color upColor = RED;
Color downColor = BLUE;
bool showEnergy = false;
bool showGrid = true;
SimulationState simState = SimulationState::PAUSED;

int runSimulation(const SimulationParams& initialParams) {
    // Initialisation des paramètres
    SimulationParams params = initialParams;
    const float J = 1.0f;  // Couplage constant

    // Configuration de la fenêtre
    int monitor = GetCurrentMonitor();
    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);
    InitWindow(screenWidth, screenHeight, "3D Ising Model Simulation");
    SetWindowPosition(0, 0);
    SetTargetFPS(60);
    rlImGuiSetup(true);

    // Configuration de la caméra
    Camera3D camera = {0};
    camera.position = {10, 15, 20};
    camera.target = {5, 5, 5};
    camera.up = {0, 1, 0};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Paramètres du système
    int N = 5, O = 5, P = 5;  // Taille du réseau
    float distance = 2.0f;     // Distance interatomique
    StructureType currentStructure = StructureType::CUBIC;
    const char* structureTypes[] = {"Cubique", "Hexagonal", "FCC", "BCC"};
    int currentStructureType = 0;

    // Initialisation des structures
    std::vector<Atome> structure;
    std::vector<Matrix> sphereTransforms;
    std::vector<Mesh> cylinderMeshes;
    bool needsRebuild = true;

    // Création des meshs de base
    float sphereRadius = 0.5f;
    float cylinderRadius = 0.1f;
    int segments = 8;
    Mesh sphereMesh = GenMeshSphere(sphereRadius, 16, 16);
    Material sphereMaterial = LoadMaterialDefault();
    Material lineMaterial = LoadMaterialDefault();
    lineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;

    // Configuration de l'interface
    SetCustomImGuiStyle(1.5f);
    Vector2 cameraAngle = {25, -45};
    float movementSpeed = 10.0f;
    float cameraSensitivity = 0.3f;

    // Boucle principale
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Contrôles de la caméra --------------------------------------------
        if (!ImGui::GetIO().WantCaptureMouse) {
            // Rotation
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                Vector2 mouseDelta = GetMouseDelta();
                cameraAngle.x -= mouseDelta.y * cameraSensitivity;
                cameraAngle.y -= mouseDelta.x * cameraSensitivity;
                cameraAngle.x = std::clamp(cameraAngle.x, -89.0f, 89.0f);
                HideCursor();
            } else {
                ShowCursor();
            }

            // Translation
            Vector3 moveDir = {0};
            if (IsKeyDown(KEY_W)) moveDir.z = 1;
            if (IsKeyDown(KEY_S)) moveDir.z = -1;
            if (IsKeyDown(KEY_D)) moveDir.x = 1;
            if (IsKeyDown(KEY_A)) moveDir.x = -1;
            if (IsKeyDown(KEY_SPACE)) moveDir.y = 1;
            if (IsKeyDown(KEY_LEFT_CONTROL)) moveDir.y = -1;

            if (Vector3Length(moveDir) > 0) {
                moveDir = Vector3Normalize(moveDir);
                float speed = movementSpeed * deltaTime;
                Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
                Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
                Vector3 up = camera.up;

                camera.position = Vector3Add(camera.position, Vector3Scale(forward, moveDir.z * speed));
                camera.position = Vector3Add(camera.position, Vector3Scale(right, moveDir.x * speed));
                camera.position = Vector3Add(camera.position, Vector3Scale(up, moveDir.y * speed));
                camera.target = Vector3Add(camera.target, Vector3Scale(forward, moveDir.z * speed));
                camera.target = Vector3Add(camera.target, Vector3Scale(right, moveDir.x * speed));
                camera.target = Vector3Add(camera.target, Vector3Scale(up, moveDir.y * speed));
            }
        }

        // Mise à jour de la cible de la caméra
        Vector3 newForward = {
            cosf(DEG2RAD * cameraAngle.y) * cosf(DEG2RAD * cameraAngle.x),
            sinf(DEG2RAD * cameraAngle.x),
            sinf(DEG2RAD * cameraAngle.y) * cosf(DEG2RAD * cameraAngle.x)
        };
        camera.target = Vector3Add(camera.position, newForward);

        // Reconstruction du système si nécessaire ---------------------------
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

            // Initialisation des spins et énergies
            for (auto& atom : structure) {
                atom.spin = GetRandomValue(0, 1) ? Spin::UP : Spin::DOWN;
            }
            UpdateEnergies(structure, params);

            // Préparation des transformations
            sphereTransforms.clear();
            for (const auto& atom : structure) {
                sphereTransforms.push_back(MatrixTranslate(atom.pos.x, atom.pos.y, atom.pos.z));
            }

            // Recréation des cylindres de liaison
            cylinderMeshes = CreateChunkedCylinderLines(structure, cylinderRadius, segments);
            needsRebuild = false;
        }

        // Simulation Monte Carlo --------------------------------------------
        if (simState == SimulationState::RUNNING || simState == SimulationState::STEP) {
            for (int i = 0; i < params.stepsPerFrame; i++) {
                MonteCarloStep(structure, params);
            }
            UpdateEnergies(structure, params);
            
            if (simState == SimulationState::STEP) {
                simState = SimulationState::PAUSED;
            }
        }

        // Rendu ------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Rendu 3D
        BeginMode3D(camera);
        {
            // Dessin des atomes
            for (size_t i = 0; i < structure.size(); i++) {
                Color color = (structure[i].spin == Spin::UP) ? upColor : downColor;
                
                if (showEnergy) {
                    // Gradient de couleur selon l'énergie
                    float normalizedEnergy = /* ... calcul énergie normalisée ... */;
                    color = ColorFromHSV(normalizedEnergy * 240, 0.9f, 0.9f);
                }
                
                sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = color;
                DrawMesh(sphereMesh, sphereMaterial, sphereTransforms[i]);
            }

            // Dessin des liaisons
            for (const auto& mesh : cylinderMeshes) {
                DrawMesh(mesh, lineMaterial, MatrixIdentity());
            }

            if (showGrid) DrawGrid(20, 1.0f);
        }
        EndMode3D();

        // Interface utilisateur --------------------------------------------
        rlImGuiBegin();
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10));
            ImGui::SetNextWindowSize(ImVec2(350, screenHeight - 20));
            
            ImGui::Begin("Contrôles", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            // Contrôles du réseau
            if (ImGui::CollapsingHeader("Structure", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::Combo("Type", &currentStructureType, structureTypes, 4)) {
                    currentStructure = static_cast<StructureType>(currentStructureType);
                    needsRebuild = true;
                }
                
                if (ImGui::SliderInt("Taille X", &N, 1, 20)) needsRebuild = true;
                if (ImGui::SliderInt("Taille Y", &O, 1, 20)) needsRebuild = true;
                if (ImGui::SliderInt("Taille Z", &P, 1, 20)) needsRebuild = true;
                if (ImGui::SliderFloat("Distance", &distance, 1.0f, 5.0f)) needsRebuild = true;
            }

            // Contrôles de simulation
            if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat("Température", &params.temperature, 0.0f, 5.0f);
                ImGui::SliderFloat("Champ B", &params.B, -2.0f, 2.0f);
                ImGui::SliderInt("Pas MC/frame", &params.stepsPerFrame, 1, 1000);
                
                if (ImGui::Button("Démarrer")) simState = SimulationState::RUNNING;
                ImGui::SameLine();
                if (ImGui::Button("Pause")) simState = SimulationState::PAUSED;
                ImGui::SameLine();
                if (ImGui::Button("Un pas")) simState = SimulationState::STEP;
                
                ImGui::Checkbox("Afficher énergie", &showEnergy);
            }

            // Affichage des statistiques
            if (ImGui::CollapsingHeader("Statistiques")) {
                float totalEnergy = CalculateTotalEnergy(structure);
                int upSpins = std::count_if(structure.begin(), structure.end(), 
                    [](const Atome& a) { return a.spin == Spin::UP; });
                
                ImGui::Text("Énergie totale: %.2f", totalEnergy);
                ImGui::Text("Spins up: %d (%.1f%%)", upSpins, 100.0f * upSpins / structure.size());
                ImGui::Text("Magnétisation: %.2f", (2.0f * upSpins - structure.size()) / structure.size());
                ImGui::Text("FPS: %d", GetFPS());
            }

            ImGui::End();
        }
        rlImGuiEnd();

        EndDrawing();
    }

    // Nettoyage
    UnloadMesh(sphereMesh);
    for (auto& mesh : cylinderMeshes) UnloadMesh(mesh);
    UnloadMaterial(sphereMaterial);
    UnloadMaterial(lineMaterial);
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
