#include "auth.h"
#include "imgui.h"
#include "imgui_style.h"
#include "rlImGui.h"
#include <fstream>
#include <string>

using namespace std;

string simple_hash(const string &input) {
  unsigned int hash = 0;

  // Seed value - can be changed to alter hash results
  const unsigned int seed = 131; // 31, 131, 1313, 13131, etc..

  // Process each character in the string
  for (char c : input) {
    hash = (hash * seed) + c;
    // Add some bit rotation
    hash = (hash << 3) | (hash >> 29);
  }

  // Convert to hex string
  char hex_str[9];
  snprintf(hex_str, sizeof(hex_str), "%08x", hash);

  return string(hex_str);
}

bool runAuthentication() {
  // User data
  char username[32] = "";
  char password[32] = "";
  char reg_username[32] = "";
  char reg_password[32] = "";
  char confirm_password[32] = "";
  bool logged_in = false;
  bool show_register = false;
  string error_msg;

  SetCustomImGuiStyle(1.5f);

  // Main authentication loop
  while (!WindowShouldClose() && !logged_in) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    rlImGuiBegin();

    if (!show_register) {
      // Login Window
      ImGui::Begin("Login", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::InputText("Username", username, sizeof(username));
      ImGui::InputText("Password", password, sizeof(password),
                       ImGuiInputTextFlags_Password);

      if (ImGui::Button("Login")) {
        ifstream file("users.txt");
        if (!file) {
          error_msg = "No users file found!";
        } else {
          string line;
          bool found = false;
          string hashed_input = simple_hash(password);
          while (getline(file, line)) {
            size_t colon = line.find(':');
            if (colon != string::npos &&
                line.substr(0, colon) == username &&
                line.substr(colon + 1) == hashed_input) {
              found = true;
              break;
            }
          }

          if (found) {
            logged_in = true;
            error_msg.clear();
          } else {
            error_msg = "Invalid username or password!";
          }
        }
      }

      if (ImGui::Button("Register")) {
        show_register = true;
        error_msg.clear();
      }

      if (!error_msg.empty()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", error_msg.c_str());
      }

      ImGui::End();
    } else {
      // Registration Window
      ImGui::Begin("Register", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::InputText("Username", reg_username, sizeof(reg_username));
      ImGui::InputText("Password", reg_password, sizeof(reg_password),
                       ImGuiInputTextFlags_Password);
      ImGui::InputText("Confirm Password", confirm_password,
                       sizeof(confirm_password), ImGuiInputTextFlags_Password);

      if (ImGui::Button("Create Account")) {
        if (strlen(reg_username) == 0 || strlen(reg_password) == 0) {
          error_msg = "Username and password required!";
        } else if (strcmp(reg_password, confirm_password) != 0) {
          error_msg = "Passwords don't match!";
        } else {
          ofstream file("users.txt", ios::app);
          if (file) {
            string hashed_pw = simple_hash(reg_password);
            file << reg_username << ":" << hashed_pw << "\n";
            error_msg = "Registration successful!";
            // Clear registration fields
            reg_username[0] = '\0';
            reg_password[0] = '\0';
            confirm_password[0] = '\0';
          } else {
            error_msg = "Failed to save user!";
          }
        }
      }

      if (ImGui::Button("Back to Login")) {
        show_register = false;
        error_msg.clear();
      }

      if (!error_msg.empty()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", error_msg.c_str());
      }

      ImGui::End();
    }

    rlImGuiEnd();
    EndDrawing();
  }

  return logged_in;
}
