#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Constants
constexpr int MAX_STRUCTURE_SIZE = 20;
constexpr float MAX_HUE = 240.0f;
constexpr float MIN_ATOM_SIZE = 0.3f;
constexpr float MAX_ATOM_SIZE = 2.0f;

// Enum for different display modes
enum DisplayMode {
    NORMAL,
    WIREFRAME,
    HIGHLIGHT_ENERGY
};

// Enum for spin values
enum class Spin : int {
    UP = 1,
    DOWN = -1,
};

// Atom structure with necessary properties
struct Atom {
    Vector3 pos;
    Spin spin = Spin::UP;
    vector<int> neighborIndices;
    float energy = 0.0f;
    float radius = 0.5f;
    Color color = RED;
};

// UI theme colors
const Color BACKGROUND_COLOR = { 245, 245, 245, 255 };
const Color PRIMARY_COLOR = { 41, 128, 185, 255 };
const Color SECONDARY_COLOR = { 52, 152, 219, 255 };
const Color TEXT_COLOR = { 44, 62, 80, 255 };
const Color HIGHLIGHT_COLOR = { 231, 76, 60, 255 };
const Color ATOM_UP_COLOR = { 52, 152, 219, 255 };
const Color ATOM_DOWN_COLOR = { 231, 76, 60, 255 };
const Color WIRE_COLOR = { 44, 62, 80, 180 };

// Simulation parameters
struct SimulationParams {
    int N = 10;
    float distance = 2.0f;
    float couplingConstant = 1.0f;
};

vector<Atom> makeStructure(const SimulationParams& params) {
    vector<Atom> atoms;
    atoms.reserve(params.N * params.N * params.N);
    
    for (int i = 0; i < params.N; i++) {
        for (int j = 0; j < params.N; j++) {
            for (int k = 0; k < params.N; k++) {
                Atom atom;
                atom.pos = {i * params.distance, j * params.distance, k * params.distance};
                atom.spin = (GetRandomValue(0, 1) == 0) ? Spin::UP : Spin::DOWN;
                atom.color = (atom.spin == Spin::UP) ? ATOM_UP_COLOR : ATOM_DOWN_COLOR;
                
                int currentIdx = i + j * params.N + k * params.N * params.N;
                
                if (i > 0) atom.neighborIndices.push_back((i-1) + j*params.N + k*params.N*params.N);
                if (i < params.N-1) atom.neighborIndices.push_back((i+1) + j*params.N + k*params.N*params.N);
                if (j > 0) atom.neighborIndices.push_back(i + (j-1)*params.N + k*params.N*params.N);
                if (j < params.N-1) atom.neighborIndices.push_back(i + (j+1)*params.N + k*params.N*params.N);
                if (k > 0) atom.neighborIndices.push_back(i + j*params.N + (k-1)*params.N*params.N);
                if (k < params.N-1) atom.neighborIndices.push_back(i + j*params.N + (k+1)*params.N*params.N);
                
                atoms.push_back(atom);
            }
        }
    }
    
    for (auto& atom : atoms) {
        atom.energy = 0.0f;
        for (int neighborIdx : atom.neighborIndices) {
            atom.energy -= params.couplingConstant * static_cast<int>(atom.spin) * 
                          static_cast<int>(atoms[neighborIdx].spin);
        }
    }
    
    return atoms;
}

bool DrawCustomButton(Rectangle bounds, const char *text, Color color) {
    bool pressed = false;
    int state = 0;
    Vector2 mousePoint = GetMousePosition();
    
    if (CheckCollisionPointRec(mousePoint, bounds)) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = 2;
        else state = 1;
        
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
    }
    
    Color buttonColor = color;
    if (state == 1) buttonColor = ColorBrightness(color, 0.2f);
    if (state == 2) buttonColor = ColorBrightness(color, -0.2f);
    
    DrawRectangleRec(bounds, buttonColor);
    DrawRectangleLinesEx(bounds, 1, ColorBrightness(buttonColor, -0.3f));
    
    int textWidth = MeasureText(text, 20);
    DrawText(text, bounds.x + bounds.width/2 - textWidth/2, bounds.y + bounds.height/2 - 10, 20, WHITE);
    
    return pressed;
}

void DrawHelpPanel() {
    DrawRectangle(10, 40, 300, 210, Fade(BACKGROUND_COLOR, 0.8f));
    DrawRectangleLines(10, 40, 300, 210, PRIMARY_COLOR);
    
    DrawText("Controls:", 20, 50, 20, TEXT_COLOR);
    DrawText("- WASD: Move camera", 20, 80, 16, TEXT_COLOR);
    DrawText("- Mouse: Look around", 20, 105, 16, TEXT_COLOR);
    DrawText("- E/Q: Move up/down", 20, 130, 16, TEXT_COLOR);
    DrawText("- R: Reset camera", 20, 155, 16, TEXT_COLOR);
    DrawText("- H: Toggle help", 20, 180, 16, TEXT_COLOR);
    DrawText("- Click: Select atom", 20, 205, 16, TEXT_COLOR);
}

void DrawLegend(DisplayMode mode) {
    DrawRectangle(10, 260, 200, 120, Fade(BACKGROUND_COLOR, 0.8f));
    DrawRectangleLines(10, 260, 200, 120, PRIMARY_COLOR);
    
    DrawText("Legend:", 20, 270, 20, TEXT_COLOR);
    
    if (mode == NORMAL || mode == WIREFRAME) {
        DrawText("Spin Up", 40, 300, 16, ATOM_UP_COLOR);
        DrawText("Spin Down", 40, 325, 16, ATOM_DOWN_COLOR);
    } else {
        DrawText("Low Energy", 40, 300, 16, ColorFromHSV(MAX_HUE, 0.8f, 0.9f));
        DrawText("High Energy", 40, 325, 16, ColorFromHSV(0, 0.8f, 0.9f));
    }
}

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 780;
    
    InitWindow(screenWidth, screenHeight, "3D Atomic Structure Simulator");
    SetTargetFPS(60);
    
    Camera3D camera = {
        {0, 10, 30},
        {10, 10, 10},
        {0, 1, 0},
        60.0f,
        CAMERA_PERSPECTIVE
    };
    Vector3 initialCameraPos = camera.position;
    
    SimulationParams params;
    auto atoms = makeStructure(params);
    
    bool showHelp = true;
    bool showLegend = true;
    DisplayMode displayMode = NORMAL;
    float atomSizeMultiplier = 1.0f;
    bool autoRotate = false;
    float rotationAngle = 0.0f;
    int selectedAtom = -1;
    bool paused = false;
    
    while (!WindowShouldClose()) {
        if (!autoRotate && !paused) {
            UpdateCamera(&camera, CAMERA_FIRST_PERSON);
            
            if (IsKeyDown(KEY_E)) camera.position.y += 0.2f;
            if (IsKeyDown(KEY_Q)) camera.position.y -= 0.2f;
            
            if (IsKeyPressed(KEY_R)) {
                camera.position = initialCameraPos;
                camera.target = {10, 10, 10};
            }
        } else if (autoRotate) {
            rotationAngle += 0.5f;
            camera.position.x = initialCameraPos.z * sin(rotationAngle * DEG2RAD);
            camera.position.z = initialCameraPos.z * cos(rotationAngle * DEG2RAD);
            camera.target = {10, 10, 10};
        }
        
        if (IsKeyPressed(KEY_H)) showHelp = !showHelp;
        if (IsKeyPressed(KEY_P)) paused = !paused;
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !IsKeyDown(KEY_LEFT_ALT)) {
            Ray ray = GetMouseRay(GetMousePosition(), camera);
            float closestDistance = INFINITY;
            selectedAtom = -1;
            
            for (size_t i = 0; i < atoms.size(); i++) {
                RayCollision collision = GetRayCollisionSphere(ray, atoms[i].pos, atoms[i].radius * atomSizeMultiplier);
                if (collision.hit && collision.distance < closestDistance) {
                    closestDistance = collision.distance;
                    selectedAtom = i;
                }
            }
        }
        
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        
        DrawRectangle(0, 0, screenWidth, 30, PRIMARY_COLOR);
        DrawText("3D Atomic Structure Simulator", 10, 5, 20, WHITE);
        DrawFPS(screenWidth - 80, 5);
        
        BeginMode3D(camera);
        
        float minEnergy = INFINITY, maxEnergy = -INFINITY;
        if (displayMode == HIGHLIGHT_ENERGY) {
            for (const auto& atom : atoms) {
                minEnergy = min(minEnergy, atom.energy);
                maxEnergy = max(maxEnergy, atom.energy);
            }
        }
        
        for (size_t i = 0; i < atoms.size(); i++) {
            Color atomColor;
            
            if (displayMode == HIGHLIGHT_ENERGY) {
                float normalizedEnergy = (atoms[i].energy - minEnergy) / (maxEnergy - minEnergy + 0.0001f);
                atomColor = ColorFromHSV(MAX_HUE * (1.0f - normalizedEnergy), 0.8f, 0.9f);
            } else {
                atomColor = (atoms[i].spin == Spin::UP) ? ATOM_UP_COLOR : ATOM_DOWN_COLOR;
            }
            
            if (i == selectedAtom) {
                atomColor = ColorBrightness(atomColor, 0.3f);
                DrawSphereWires(atoms[i].pos, atoms[i].radius * atomSizeMultiplier * 1.2f, 16, 16, WHITE);
            }
            
            DrawSphere(atoms[i].pos, atoms[i].radius * atomSizeMultiplier, atomColor);
            
            if (displayMode == WIREFRAME || (displayMode == NORMAL && IsKeyDown(KEY_LEFT_ALT))) {
                for (int neighborIdx : atoms[i].neighborIndices) {
                    DrawLine3D(atoms[i].pos, atoms[neighborIdx].pos, WIRE_COLOR);
                }
            }
        }
        
        DrawGrid(40, 1.0f);
        
        EndMode3D();
        
        DrawRectangle(990, 50, 200, 250, Fade(BACKGROUND_COLOR, 0.8f));
        DrawRectangleLines(990, 50, 200, 250, PRIMARY_COLOR);
        
        if (DrawCustomButton({1000, 60, 180, 30}, "Toggle Wireframe", SECONDARY_COLOR)) {
            displayMode = (displayMode == WIREFRAME) ? NORMAL : WIREFRAME;
        }
        
        if (DrawCustomButton({1000, 100, 180, 30}, "Color by Spin", SECONDARY_COLOR)) {
            displayMode = NORMAL;
        }
        
        if (DrawCustomButton({1000, 140, 180, 30}, "Color by Energy", SECONDARY_COLOR)) {
            displayMode = HIGHLIGHT_ENERGY;
        }
        
        if (DrawCustomButton({1000, 180, 180, 30}, "Toggle Auto-Rotate", SECONDARY_COLOR)) {
            autoRotate = !autoRotate;
        }
        
        if (DrawCustomButton({1000, 220, 85, 30}, "Atoms -", SECONDARY_COLOR)) {
            atomSizeMultiplier = max(MIN_ATOM_SIZE, atomSizeMultiplier - 0.1f);
        }
        
        if (DrawCustomButton({1095, 220, 85, 30}, "Atoms +", SECONDARY_COLOR)) {
            atomSizeMultiplier = min(MAX_ATOM_SIZE, atomSizeMultiplier + 0.1f);
        }
        
        if (DrawCustomButton({1000, 260, 180, 30}, paused ? "Resume" : "Pause", SECONDARY_COLOR)) {
            paused = !paused;
        }
        
        char sizeText[30];
        sprintf(sizeText, "Atom Size: %.1f", atomSizeMultiplier);
        DrawText(sizeText, 1000, 300, 20, TEXT_COLOR);
        
        string modeText = "Mode: ";
        modeText += (displayMode == NORMAL) ? "Spin Colors" : 
                   (displayMode == WIREFRAME) ? "Wireframe" : "Energy Heat Map";
        DrawText(modeText.c_str(), 10, screenHeight - 30, 20, TEXT_COLOR);
        
        DrawText(autoRotate ? "Auto-Rotate: ON" : "Auto-Rotate: OFF", 250, screenHeight - 30, 20, TEXT_COLOR);
        DrawText(paused ? "Status: PAUSED" : "Status: RUNNING", 450, screenHeight - 30, 20, TEXT_COLOR);
        
        if (showHelp) {
            DrawHelpPanel();
        } else {
            DrawText("Press H for Help", 10, 40, 20, TEXT_COLOR);
        }
        
        if (showLegend) {
            DrawLegend(displayMode);
        }
        
        if (selectedAtom != -1) {
            char infoText[256];
            sprintf(infoText, "Selected Atom:\nPos: (%.1f, %.1f, %.1f)\nSpin: %s\nEnergy: %.2f",
                    atoms[selectedAtom].pos.x,
                    atoms[selectedAtom].pos.y,
                    atoms[selectedAtom].pos.z,
                    (atoms[selectedAtom].spin == Spin::UP) ? "Up" : "Down",
                    atoms[selectedAtom].energy);
            
            DrawRectangle(10, screenHeight - 120, 250, 110, Fade(BACKGROUND_COLOR, 0.8f));
            DrawRectangleLines(10, screenHeight - 120, 250, 110, PRIMARY_COLOR);
            DrawText(infoText, 20, screenHeight - 110, 16, TEXT_COLOR);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
