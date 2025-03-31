#include "simulation.h"

// Variables globales (conservées car partagées avec l'UI)
SimulationState simState = SimulationState::PAUSED;
float J = 1.0f;                // Couplage entre spins (reste global car souvent constant)
Color upColor = RED;            // Couleur spin up
Color downColor = BLUE;         // Couleur spin down
bool showEnergy = false;        // Visualisation énergie

/**
 * @brief Crée un réseau cubique simple
 * @param x,y,z Dimensions du réseau
 * @param distance Distance interatomique
 * @return Vecteur des atomes positionnés
 */

vector<Atome> make_cubic_struc(int x, int y, int z, float distance) {
    vector<Atome> points(x * y * z);
    auto getIndex = [=](int i, int j, int k) { return i * y * z + j * z + k; };

    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                int idx = getIndex(i, j, k);
                points[idx].pos = {i * distance, j * distance, k * distance};
                points[idx].spin = GetRandomValue(0, 1) ? Spin::UP : Spin::DOWN;

                // Connexions avec les 6 voisins
                if (i > 0) points[idx].neigh.push_back(getIndex(i - 1, j, k));
                if (i < x - 1) points[idx].neigh.push_back(getIndex(i + 1, j, k));
                if (j > 0) points[idx].neigh.push_back(getIndex(i, j - 1, k));
                if (j < y - 1) points[idx].neigh.push_back(getIndex(i, j + 1, k));
                if (k > 0) points[idx].neigh.push_back(getIndex(i, j, k - 1));
                if (k < z - 1) points[idx].neigh.push_back(getIndex(i, j, k + 1));
            }
        }
    }
    return points;
}


vector<Atome> make_hexagonal_struc(int x, int y, int z, float distance) {
  vector<Atome> points;
  points.reserve(x * y * z);

  float a = distance;
  float c = a * 1.2f; // Ideal c/a ratio for HCP

  for (int layer = 0; layer < z; layer++) {
    // ABAB stacking pattern
    bool isLayerB = (layer % 2 == 1);

    for (int row = 0; row < y; row++) {
      for (int col = 0; col < x; col++) {
        Atome atom;

        // Base position
        atom.pos.x = col * a;
        atom.pos.y = row * (a * sqrt(3.0f) / 2.0f);
        atom.pos.z = layer * c;

        // Apply offset for B layers
        if (isLayerB) {
          atom.pos.x += a / 2.0f;
          atom.pos.y += (a * sqrt(3.0f) / 6.0f);
        }

        // Apply offset for even rows within each layer
        if (row % 2 == 1) {
          atom.pos.x += a / 2.0f;
        }

        points.push_back(atom);
      }
    }
  }

  // Establish neighbor connections
  for (size_t i = 0; i < points.size(); i++) {
    points[i].neigh.clear();

    for (size_t j = 0; j < points.size(); j++) {
      if (i == j)
        continue;

      float dist = Vector3Distance(points[i].pos, points[j].pos);
      // In HCP, the nearest neighbor distance is exactly 'a'
      if (dist <= a * 1.1f) {
        points[i].neigh.push_back(static_cast<int>(j));
      }
    }
  }

  return points;
}

vector<Atome> make_fcc_struc(int x, int y, int z, float distance) {
  vector<Atome> points;

  // In FCC, we want to avoid duplicates at the boundaries
  float a = distance; // lattice constant

  // Create a 3D grid to track atom positions
  const float tolerance = 0.01f * a;
  auto isNearExistingAtom = [&points, tolerance](const Vector3 &pos) {
    for (const auto &atom : points) {
      if (Vector3Distance(atom.pos, pos) < tolerance) {
        return true;
      }
    }
    return false;
  };

  // Generate atoms for each unit cell
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        Vector3 basePos = {i * a, j * a, k * a};

        // Corner atom (0,0,0)
        Vector3 pos = basePos;
        if (!isNearExistingAtom(pos)) {
          Atome atom;
          atom.pos = pos;
          points.push_back(atom);
        }

        // Face centers
        Vector3 facePositions[] = {
            {basePos.x + a / 2, basePos.y + a / 2, basePos.z},
            {basePos.x + a / 2, basePos.y, basePos.z + a / 2},
            {basePos.x, basePos.y + a / 2, basePos.z + a / 2}};

        for (const auto &facePos : facePositions) {
          if (!isNearExistingAtom(facePos)) {
            Atome atom;
            atom.pos = facePos;
            points.push_back(atom);
          }
        }
      }
    }
  }

  // Establish neighbor connections (each atom has 12 nearest neighbors in FCC)
  for (size_t i = 0; i < points.size(); i++) {
    points[i].neigh.clear();

    for (size_t j = 0; j < points.size(); j++) {
      if (i == j)
        continue;

      float dist = Vector3Distance(points[i].pos, points[j].pos);
      // Nearest neighbor distance in FCC is a/√2 ≈ 0.707a
      if (dist <= a * 0.75f) {
        points[i].neigh.push_back(static_cast<int>(j));
      }
    }
  }

  return points;
}

vector<Atome> make_bcc_struc(int x, int y, int z, float distance) {
  vector<Atome> points;

  float a = distance; // lattice constant

  // Create a 3D grid to track atom positions
  const float tolerance = 0.01f * a;
  auto isNearExistingAtom = [&points, tolerance](const Vector3 &pos) {
    for (const auto &atom : points) {
      if (Vector3Distance(atom.pos, pos) < tolerance) {
        return true;
      }
    }
    return false;
  };

  // Generate atoms for each unit cell
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      for (int k = 0; k < z; k++) {
        Vector3 basePos = {i * a, j * a, k * a};

        // Corner atom (0,0,0)
        Vector3 pos = basePos;
        if (!isNearExistingAtom(pos)) {
          Atome atom;
          atom.pos = pos;
          points.push_back(atom);
        }

        // Body center
        Vector3 centerPos = {basePos.x + a / 2, basePos.y + a / 2,
                             basePos.z + a / 2};
        if (!isNearExistingAtom(centerPos)) {
          Atome atom;
          atom.pos = centerPos;
          points.push_back(atom);
        }
      }
    }
  }

  // Establish neighbor connections (each atom has 8 nearest neighbors in BCC)
  for (size_t i = 0; i < points.size(); i++) {
    points[i].neigh.clear();

    for (size_t j = 0; j < points.size(); j++) {
      if (i == j)
        continue;

      float dist = Vector3Distance(points[i].pos, points[j].pos);
      // Nearest neighbor distance in BCC is a*√3/2 ≈ 0.866a
      if (dist <= a * 0.9f) {
        points[i].neigh.push_back(static_cast<int>(j));
      }
    }
  }

  return points;
}

vector<Mesh> CreateChunkedCylinderLines(const vector<Atome> &structure,
                                        float radius, int segments,
                                        int maxCylindersPerChunk) {
  vector<Mesh> meshChunks;
  vector<vector<pair<Vector3, Vector3>>>
      chunks; // Stores start/end points for each chunk

  // First collect all cylinder pairs
  vector<pair<Vector3, Vector3>> allCylinders;
  for (const auto &atom : structure) {
    for (int neighborIdx : atom.neigh) {
      if (neighborIdx > &atom - &structure[0]) {
        allCylinders.emplace_back(atom.pos, structure[neighborIdx].pos);
      }
    }
  }

  // Split into chunks
  for (size_t i = 0; i < allCylinders.size(); i += maxCylindersPerChunk) {
    auto start = allCylinders.begin() + i;
    auto end = (i + maxCylindersPerChunk) < allCylinders.size()
                   ? start + maxCylindersPerChunk
                   : allCylinders.end();
    chunks.emplace_back(start, end);
  }

  // Create a mesh for each chunk
  for (const auto &chunk : chunks) {
    const int vertsPerCylinder = segments * 2;
    const int trisPerCylinder = segments * 2;

    Mesh mesh = {0};
    mesh.vertexCount = chunk.size() * vertsPerCylinder;
    mesh.triangleCount = chunk.size() * trisPerCylinder;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 *
                                               sizeof(unsigned short));

    int vertexOffset = 0;
    int indexOffset = 0;

    for (const auto &[start, end] : chunk) {
      Vector3 direction = Vector3Normalize(Vector3Subtract(end, start));
      Vector3 perp =
          (fabs(direction.x) < fabs(direction.y))
              ? Vector3Normalize(Vector3CrossProduct(direction, {1, 0, 0}))
              : Vector3Normalize(Vector3CrossProduct(direction, {0, 1, 0}));

      // Generate cylinder vertices
      for (int i = 0; i < segments; i++) {
        float angle = 2 * PI * i / segments;
        Vector3 circleVec = Vector3Scale(
            Vector3Add(Vector3Scale(perp, cosf(angle)),
                       Vector3Scale(Vector3CrossProduct(perp, direction),
                                    sinf(angle))),
            radius);

        // Bottom ring
        mesh.vertices[(vertexOffset + i) * 3 + 0] = start.x + circleVec.x;
        mesh.vertices[(vertexOffset + i) * 3 + 1] = start.y + circleVec.y;
        mesh.vertices[(vertexOffset + i) * 3 + 2] = start.z + circleVec.z;

        // Top ring
        mesh.vertices[(vertexOffset + segments + i) * 3 + 0] =
            end.x + circleVec.x;
        mesh.vertices[(vertexOffset + segments + i) * 3 + 1] =
            end.y + circleVec.y;
        mesh.vertices[(vertexOffset + segments + i) * 3 + 2] =
            end.z + circleVec.z;

        // Normals
        Vector3 normal = Vector3Normalize(circleVec);
        mesh.normals[(vertexOffset + i) * 3 + 0] = normal.x;
        mesh.normals[(vertexOffset + i) * 3 + 1] = normal.y;
        mesh.normals[(vertexOffset + i) * 3 + 2] = normal.z;
        mesh.normals[(vertexOffset + segments + i) * 3 + 0] = normal.x;
        mesh.normals[(vertexOffset + segments + i) * 3 + 1] = normal.y;
        mesh.normals[(vertexOffset + segments + i) * 3 + 2] = normal.z;
      }

      // Generate cylinder indices
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

      vertexOffset += vertsPerCylinder;
    }

    UploadMesh(&mesh, false);
    meshChunks.push_back(mesh);
  }

  return meshChunks;
}

Mesh CreateBakedCylinderLines(const vector<Atome> &structure, float radius,
                              int segments) {
  int cylinderCount = 0;
  for (const auto &atom : structure) {
    for (int neighborIdx : atom.neigh) {
      if (neighborIdx > &atom - &structure[0])
        cylinderCount++;
    }
  }

  const int vertsPerCylinder = segments * 2;
  const int trisPerCylinder = segments * 2;

  Mesh mesh = {0};
  mesh.vertexCount = cylinderCount * vertsPerCylinder;
  mesh.triangleCount = cylinderCount * trisPerCylinder;

  mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
  mesh.normals = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
  mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
  mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 *
                                             sizeof(unsigned short));

  int vertexOffset = 0;
  int indexOffset = 0;

  for (const auto &atom : structure) {
    for (int neighborIdx : atom.neigh) {
      if (neighborIdx > &atom - &structure[0]) {
        Vector3 start = atom.pos;
        Vector3 end = structure[neighborIdx].pos;
        Vector3 direction = Vector3Normalize(Vector3Subtract(end, start));

        Vector3 perp =
            (fabs(direction.x) < fabs(direction.y))
                ? Vector3Normalize(
                      Vector3CrossProduct(direction, (Vector3){1, 0, 0}))
                : Vector3Normalize(
                      Vector3CrossProduct(direction, (Vector3){0, 1, 0}));

        for (int i = 0; i < segments; i++) {
          float angle = 2 * PI * i / segments;
          Vector3 circleVec = Vector3Scale(
              Vector3Add(Vector3Scale(perp, cosf(angle)),
                         Vector3Scale(Vector3CrossProduct(perp, direction),
                                      sinf(angle))),
              radius);

          // Bottom ring
          mesh.vertices[(vertexOffset + i) * 3 + 0] = start.x + circleVec.x;
          mesh.vertices[(vertexOffset + i) * 3 + 1] = start.y + circleVec.y;
          mesh.vertices[(vertexOffset + i) * 3 + 2] = start.z + circleVec.z;

          // Top ring
          mesh.vertices[(vertexOffset + segments + i) * 3 + 0] =
              end.x + circleVec.x;
          mesh.vertices[(vertexOffset + segments + i) * 3 + 1] =
              end.y + circleVec.y;
          mesh.vertices[(vertexOffset + segments + i) * 3 + 2] =
              end.z + circleVec.z;

          // Normals
          Vector3 normal = {circleVec.x / radius, circleVec.y / radius,
                            circleVec.z / radius};
          mesh.normals[(vertexOffset + i) * 3 + 0] = normal.x;
          mesh.normals[(vertexOffset + i) * 3 + 1] = normal.y;
          mesh.normals[(vertexOffset + i) * 3 + 2] = normal.z;
          mesh.normals[(vertexOffset + segments + i) * 3 + 0] = normal.x;
          mesh.normals[(vertexOffset + segments + i) * 3 + 1] = normal.y;
          mesh.normals[(vertexOffset + segments + i) * 3 + 2] = normal.z;
        }

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

        vertexOffset += vertsPerCylinder;
      }
    }
  }

  UploadMesh(&mesh, false);
  return mesh;
}

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

float CalculateTotalEnergy(const vector<Atome> &structure) {
  float totalEnergy = 0.0f;
  for (const auto &atom : structure) {
    totalEnergy += atom.energy;
  }
  return totalEnergy / 2.0f; // Divide by 2 to avoid double counting
}

void UpdateEnergies(vector<Atome> &structure, float J, float B) {
  for (auto &atom : structure) {
    float interactionEnergy = 0.0f;
    for (int neighborIdx : atom.neigh) {
      interactionEnergy += static_cast<int>(structure[neighborIdx].spin);
    }
    atom.energy = -J * static_cast<int>(atom.spin) * interactionEnergy -
                  B * static_cast<int>(atom.spin);
  }
}


/**
 * @brief Met à jour les énergies de tous les atomes
 * @param structure Vecteur des atomes
 * @param params Paramètres de simulation
 */
void UpdateEnergies(vector<Atome> &structure, const SimulationParams &params) {
    for (auto &atom : structure) {
        float interactionEnergy = 0.0f;
        for (int neighborIdx : atom.neigh) {
            interactionEnergy += static_cast<int>(structure[neighborIdx].spin);
        }
        atom.energy = -J * static_cast<int>(atom.spin) * interactionEnergy 
                       - params.B * static_cast<int>(atom.spin);
    }
}

/**
 * @brief Calcule l'énergie totale du système
 * @param structure Vecteur des atomes
 * @return Énergie totale (divisée par 2 pour éviter double comptage)
 */
float CalculateTotalEnergy(const vector<Atome> &structure) {
    float totalEnergy = 0.0f;
    for (const auto &atom : structure) {
        totalEnergy += atom.energy;
    }
    return totalEnergy / 2.0f;
}


// Simulation MONTE CARLO //

/**
 * @brief Effectue un pas Monte Carlo
 * @param structure Référence vers les atomes
 * @param params Paramètres de simulation (température, champ B, etc.)
 */
void MonteCarloStep(vector<Atome> &structure, const SimulationParams &params) {
    int randomIdx = GetRandomValue(0, structure.size() - 1);
    auto &atom = structure[randomIdx];

    // Calcul énergie actuelle
    float currentEnergy = 0.0f;
    for (int neighborIdx : atom.neigh) {
        currentEnergy += static_cast<int>(structure[neighborIdx].spin);
    }
    currentEnergy = -J * static_cast<int>(atom.spin) * currentEnergy 
                    - params.B * static_cast<int>(atom.spin);

    // Test d'inversion de spin
    Spin newSpin = (atom.spin == Spin::UP) ? Spin::DOWN : Spin::UP;
    
    float newEnergy = 0.0f;
    for (int neighborIdx : atom.neigh) {
        newEnergy += static_cast<int>(structure[neighborIdx].spin);
    }
    newEnergy = -J * static_cast<int>(newSpin) * newEnergy 
                - params.B * static_cast<int>(newSpin);

    float deltaE = newEnergy - currentEnergy;
    // Critère de Metropolis
    if (deltaE < 0 || (params.temperature > 0 && 
        GetRandomValue(0, 10000)/10000.0f < exp(-deltaE/params.temperature))) {
        atom.spin = newSpin;
    }
}



