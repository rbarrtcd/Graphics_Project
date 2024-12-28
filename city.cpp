#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <render/shader.h>
#include "animationData.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "glm/gtc/type_ptr.hpp"

#include "entity.h"
#include "light.h"
#include "utilities.h"
#include "geometry.h"
#include "primitives.h"
#include "skybox.h"
#include "deferredShader.h"
#include "city.h"

#include <vector>
#include <iostream>
#include <thread>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <functional>
#include <iomanip>
#include <xutility>
#include <fstream>
#include <sstream>
#include <random>

static GLFWwindow *window;
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// OpenGL camera view parameters

const int MAX_LIGHTS = 16;
const float TILE_SIZE = 640.0;

// View control
static float viewAzimuth = 0.f;
static float viewPolar = 0.f;
static float viewDistance = 800.0f;

static glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);

static glm::vec3 cameraSpeed = glm::vec3(250, 250, 250);
float yaw = 180.0f;
float pitch = -45.0f;
float mouseSensitivity = 0.05f;
float lastX, lastY;


const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
static glm::vec3 lightIntensity = 1.0f * ((8.0f * wave500) + (15.6f * wave600) + (18.4f * wave700));
static glm::vec3 lightPosition(-275.0f, 500.0f, -275.0f);


int skyboxSetting;
static Scene scene;



GLuint geometry_programID = 0;
GLuint lightingPass_shader = 0;
GLuint shadowMap_shader = 0;
GLuint skybox_shader = 0;
GLuint entity_shader = 0;
GLuint animation_shader = 0;
GLuint entityLight_shader = 0;
GLuint sunGeo_shader = 0;
GLuint sunEntity_shader = 0;
GLuint sunlighting_shader = 0;

const std::string BUILDTEXTPATH = "../assets/textures/buildings";
const std::vector<std::string> BUILDTEXTS = {
        "/lab2/facade0.jpg",
        "/lab2/facade1.jpg",
        "/lab2/facade3.jpg",
        "/lab2/facade4.jpg",
        "/facades/Facades_01_basecolor.jpg",
        "/facades/Facades_03v2_basecolor.jpg",
        "/facades/Facades_04_basecolor.jpg",
        "/facades/Facades_05_basecolor.jpg",
        "/facades/Facades_06_basecolor.jpg",
};

struct Camera {
    // Static members
    static glm::vec3 position;      // Camera position
    static glm::vec3 lookAt;        // Look-at target
    static glm::mat4 viewMatrix;    // View matrix
    static glm::mat4 projectionMatrix; // Projection matrix
    static glm::mat4 VP;      // View-Projection matrix

    // Static method to update the View matrix
    static void updateViewMatrix() {
        viewMatrix = glm::lookAt(position, lookAt, glm::vec3(0.0f, 1.0f, 0.0f)); // Up vector as Y-axis
        updateVP();
    }

    // Static method to update the Projection matrix
    static void updateProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane) {
        projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
        updateVP();
    }

    // Static method to update the View-Projection matrix
    static void updateVP() {
        VP = projectionMatrix * viewMatrix;
    }
};
glm::vec3 Camera::position = glm::vec3(0.0f, 0.0f, 3.0f);  // Default position
glm::vec3 Camera::lookAt = glm::vec3(0.0f, 0.0f, 0.0f);    // Default look-at target
glm::mat4 Camera::viewMatrix = glm::mat4(1.0f);            // Identity matrix as default
glm::mat4 Camera::projectionMatrix = glm::mat4(1.0f);      // Identity matrix as default
glm::mat4 Camera::VP = glm::mat4(1.0f);


Tile::Tile(glm::vec3 setPosition, int tileType){
    position = setPosition;
    if (tileType == 0){
        setupBuilding();
    } else if (tileType == 1){
        setupRoad(glm::vec3(0,0,0));
    } else if (tileType == 2){
        setupRoad(glm::vec3(0,90,0));
    } else if (tileType >= 3 && tileType <= 6){
        setupTway(glm::vec3(0,90*abs(tileType-3),0));
    } else if (tileType >= 7 && tileType <= 10){
        setupCorner(glm::vec3(0,90*abs(tileType-7),0));
    }
}
void Tile::setupPlane(){
    Geometry * groundPlane = new Geometry(glm::vec3(position.x+(TILE_SIZE/2), position.y, position.z +(TILE_SIZE/2)),
                                          glm::vec3(TILE_SIZE, 1, TILE_SIZE), glm::vec3(0,0,0), geometry_programID);
    groundPlane->initialize(meshData_plane,scene.getTexture("../assets/textures/concrete/gravel_concrete_03_diff_4k.jpg"));
    geometries.push_back(groundPlane);
}

void Tile::setupRoad(glm::vec3 rotation){
    glm::vec3 centerPos = glm::vec3(position.x+(TILE_SIZE/2), position.y, position.z +(TILE_SIZE/2));
    float offset = (TILE_SIZE/2)-10;
    float radians = glm::radians(rotation.y);

    // Compute the offset based on rotation
    float xOffset = offset * cos(radians);  // Positive/negative X offset
    float zOffset = offset * sin(radians);  // Positive/negative Z offset

    // Add the calculated offset to centerPos
    glm::vec3 streetLightPos1 = centerPos + glm::vec3(-xOffset, 0, zOffset);

    Geometry * streetLight = new Geometry(streetLightPos1,
                                          glm::vec3(100, 100, 100), rotation+glm::vec3(-90,0,0), geometry_programID);


    MeshData * streetLampMesh = scene.getModel("../assets/models/streetlight/StreetLamp.fbx");

    streetLight->initialize(*streetLampMesh, scene.getTexture("../assets/models/streetlight/StreetLampDiffuse.png"));
    geometries.push_back(streetLight);
    glm::vec3 streetLightPos2 = centerPos + glm::vec3(xOffset, 0, -zOffset);
    Geometry * streetLight2 = new Geometry(streetLightPos2,glm::vec3(100, 100, 100), rotation+glm::vec3(-90,180,0), geometry_programID);
    streetLight2->initialize(*streetLampMesh, scene.getTexture("../assets/models/streetlight/StreetLampDiffuse.png"));
    geometries.push_back(streetLight2);
    Geometry * groundPlane = new Geometry(glm::vec3(position.x+(TILE_SIZE/2), position.y, position.z +(TILE_SIZE/2)),
                                          glm::vec3(TILE_SIZE, 1, TILE_SIZE), rotation, geometry_programID);

    const float offsetFactor = 0.8;
    streetLightPos1 = centerPos + glm::vec3(-xOffset*offsetFactor, 0, zOffset*offsetFactor);
    Light * light1 = new Light(streetLightPos1+glm::vec3(0,325,0), glm::vec3(255, 164.65, 38.43), 100.0, 256, 800, POINT_LIGHT);
    scene.lights.push_back(light1);

    streetLightPos2 = centerPos + glm::vec3(xOffset*offsetFactor, 0, -zOffset*offsetFactor);
    Light * light2 = new Light(streetLightPos2+glm::vec3(0,325,0), glm::vec3(255, 164.65, 38.43), 100.0, 256, 800, POINT_LIGHT);
    scene.lights.push_back(light2);


    groundPlane->initialize(meshData_road,scene.getTexture("../assets/textures/road/road_texture.png"));
    geometries.push_back(groundPlane);
}

void Tile::setupTway(glm::vec3 rotation){
    Geometry * groundPlane = new Geometry(glm::vec3(position.x+(TILE_SIZE/2), position.y, position.z +(TILE_SIZE/2)),
                                          glm::vec3(TILE_SIZE, 1, TILE_SIZE), rotation, geometry_programID);
    groundPlane->initialize(meshData_tway,scene.getTexture("../assets/textures/road/tway_texture.png"));
    geometries.push_back(groundPlane);
}

void Tile::setupCorner(glm::vec3 rotation){
    Geometry * groundPlane = new Geometry(glm::vec3(position.x+(TILE_SIZE/2), position.y, position.z +(TILE_SIZE/2)),
                                          glm::vec3(TILE_SIZE, 1, TILE_SIZE), rotation, geometry_programID);
    groundPlane->initialize(meshData_corner, scene.getTexture("../assets/textures/road/tway_texture.png"));
    geometries.push_back(groundPlane);
}

void Tile::setupBuilding() {
    // Create a random number generator
    static std::random_device rd;  // Seed generator
    static std::mt19937 rng(rd()); // Mersenne Twister PRNG

    // Add ground plane geometry
    Geometry* groundPlane = new Geometry(
            glm::vec3(position.x + (TILE_SIZE / 2), position.y, position.z + (TILE_SIZE / 2)),
            glm::vec3(TILE_SIZE, 1, TILE_SIZE),
            glm::vec3(0, 0, 0),
            geometry_programID
    );
    groundPlane->initialize(meshData_plane, scene.getTexture("../assets/textures/concrete/gravel_concrete_03_diff_4k.jpg"));
    geometries.push_back(groundPlane);

    // Generate a random height scale for the building
    std::uniform_real_distribution<float> scaleDist(0.2f, 7.2f); // 0.1 to 8.9 to avoid exact zero
    float rand_y_scale = scaleDist(rng);

    // Add building geometry
    Geometry* building = new Geometry(
            glm::vec3(position.x + (TILE_SIZE / 2), position.y +(TILE_SIZE * rand_y_scale/2), position.z + (TILE_SIZE / 2)),
            glm::vec3(TILE_SIZE * 0.75f, TILE_SIZE * rand_y_scale, TILE_SIZE * 0.75f),
            glm::vec3(0, 0.0f, 0.0f),
            geometry_programID
    );

    // Create and modify a deep copy of the mesh data
    MeshData buildingData = deepCopyMeshData(meshData_building);
    for (int i = 0; i < (buildingData.numVertices) * 2; i += 2) {
        buildingData.uv_buffer_data[i] *= 4;
        buildingData.uv_buffer_data[i + 1] *= (TILE_SIZE * rand_y_scale) / 128.0f;
    }

    // Select a random texture
    std::uniform_int_distribution<int> textureDist(0, static_cast<int>(BUILDTEXTS.size() - 1));
    int randomIndex = textureDist(rng);
    std::string randomTexturePath = BUILDTEXTPATH + BUILDTEXTS[randomIndex];

    // Initialize the building with the modified data and random texture
    building->initialize(buildingData, scene.getTexture(randomTexturePath));
    geometries.push_back(building);
}

void Tile::render(){
    for (Geometry * g:geometries) {
        g->render(Camera::VP);
    }
}

void Tile::lightRender(Light * light){
    for (Geometry * g:geometries) {
        if (scene.isDaytime){
            g->lightRender(sunGeo_shader, *light);
        } else {
            g->lightRender(shadowMap_shader, *light);
        }
    }
}




City::City(glm::vec3 setPosition, std::vector<std::vector<Tile*>> theTiles){
    position = setPosition;
    tiles = theTiles;
}

void City::render(){
    for (std::vector<Tile *> tileRow : tiles){
        for (Tile * tile : tileRow){
            tile->render();
        }
    }
}
void City::lightRender(Light * light){
    if (light->lightType == POINT_LIGHT){
        int gridX = (int)light->position.x/(int)TILE_SIZE;
        int gridZ = (int)light->position.z/(int)TILE_SIZE;
        for (int i = -1; i < 2; ++i) {
            int newX = gridX+i;
            for (int j = -1; j < 2; ++j) {
                int newZ = gridZ+j;
                if (newX >= 0 && newX < 10 && newZ >= 0 && newZ < 10){
                    tiles[newX][newZ]->lightRender(light);
                }
            }
        }
    } else {
        for (std::vector<Tile *> tileRow: tiles) {
            for (Tile *tile: tileRow) {
                tile->lightRender(light);
            }
        }
    }
}



 GLuint Scene::getTexture(const std::string& filepath) {
    // Check if the texture is already loaded
    auto it = textures.find(filepath);
    if (it != textures.end()) {
        return it->second; // Return the existing texture ID
    }

    // Load the texture and store it in the map
    GLuint textureID = LoadTextureTileBox(filepath.c_str());
    textures[filepath] = textureID; // Store the key-value pair
    return textureID;
}

MeshData* Scene::getModel(const std::string& filepath) {
    // Check if the model is already loaded
    auto it = models.find(filepath);
    if (it != models.end()) {
        return it->second.get(); // Return the existing MeshData pointer
    }

    // Prepare storage for model data
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> uvs;
    std::vector<GLuint> indices;
    std::vector<GLfloat> colors;

    // Load the model
    if (!loadFbx(filepath, vertices, normals, uvs, indices, colors)) {
        throw std::runtime_error("Failed to load model: " + filepath);
    }

    // Create a MeshData instance
    MeshData originalMeshData(
            static_cast<int>(vertices.size() / 3),
            static_cast<int>(indices.size()),
            vertices.data(),
            normals.data(),
            colors.data(),
            indices.data(),
            uvs.data()
    );

    // Deep copy the MeshData instance
    MeshData* copiedMeshData = new MeshData(deepCopyMeshData(originalMeshData));

    // Store the copied model in the unordered_map
    models[filepath] = std::unique_ptr<MeshData>(copiedMeshData);

    return copiedMeshData; // Return the newly loaded and copied model
}

AnimationData* Scene::getAnimation(const std::string& filepath) {
    // Check if the animation is already loaded
    auto it = animations.find(filepath);
    if (it != animations.end()) {
        return it->second.get(); // Return the existing AnimationData pointer
    }

    // Load the animation
    AnimationData animData = AnimationData();
    animData = loadFBXAnimation(filepath);

    AnimationData* copiedMeshData = new AnimationData(deepCopyAnimData(animData));
    // Store the animation in the unordered_map
    animations[filepath] = std::unique_ptr<AnimationData>(copiedMeshData);

    return copiedMeshData; // Return the newly loaded animation
}


void Scene::render(){
    for (City * city : cities){
        city->render();
    }
    for (Ped * ped : peds){
        ped->render();
    }
}


void Scene::lightRender() {
    if (isDaytime){
        glBindFramebuffer(GL_FRAMEBUFFER, theSun->shadowFBO);
        glViewport(0, 0, theSun->shadowMapSize, theSun->shadowMapSize);
        glClear(GL_DEPTH_BUFFER_BIT);
        //for (City *city: cities) {
        //    city->lightRender(theSun);
        //}
        cities[0]->lightRender(theSun);
        for (Ped *ped: peds) {
            ped->lightRender(theSun);
        }
    }else{

// Calculate distances and store them alongside light pointers
        std::vector<std::pair<float, Light*>> distances;
        closeLights.clear();
        for (Light *light : lights) {
            float distance = glm::distance(light->position, Camera::position);
            distances.emplace_back(distance, light);
        }

// Sort the lights by distance
        std::sort(distances.begin(), distances.end(), [](const auto &a, const auto &b) {
            return a.first < b.first; // Sort by distance (ascending)
        });

// Select the closest MAX_LIGHTS lights
        for (size_t i = 0; i < std::min(MAX_LIGHTS, static_cast<int>(distances.size())); ++i) {
            closeLights.push_back(distances[i].second);
        }

        for (Light *light: closeLights) {
            glBindFramebuffer(GL_FRAMEBUFFER, light->shadowFBO);
            glViewport(0, 0, light->shadowMapSize, light->shadowMapSize);
            glClear(GL_DEPTH_BUFFER_BIT);

            glm::vec3 oldPosition = light->position;
            light->position.x = fmod(light->position.x, (10*TILE_SIZE));
            if (light->position.x < 0){
                light->position.x += (10*TILE_SIZE);
            }
            light->position.z = fmod(light->position.z, (10*TILE_SIZE));
            if (light->position.z < 0){
                light->position.z += (10*TILE_SIZE);
            }

            light->update();
            //for (City *city: cities) {
            //    city->lightRender(light);
            //}
            cities[0]->lightRender(light);
            light->position = oldPosition;
            light->update();
            for (Ped *ped: peds) {
                if (glm::length(light->position - ped->entity->position) < light->lightRange*1.2) {
                    ped->lightRender(light);
                }
            }
        }
    }
}

//Pedestriants
void Ped::initialize() {
    std::vector<GLfloat> vertices, normals, uvs, colours;
    std::vector<GLuint> indices;
    MeshData * pedMesh = scene.getModel("../assets/models/lowPolyHuman/ManUnity.fbx");
    extractMeshDataToVectors(*pedMesh, vertices, normals, colours, indices, uvs);
    int randomValueX = (rand() % 15) - 7;
    int randomValueY = (rand() % 15) - 7;
    float posX = TILE_SIZE * randomValueX;
    float posY = TILE_SIZE * randomValueY;
    Entity * theEntity = new Entity(glm::vec3(posX,0,posY), glm::vec3(10, 10, 10), glm::vec3(-90,0,0), entity_shader, animation_shader);
    entity = theEntity;
    int maxUnits;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);
    entity->setTexture(scene.getTexture("../assets/models/lowPolyHuman/ManColors.png"));
    entity->loadModelData(vertices, normals, colours, indices, uvs);
    AnimationData testAnimData = *scene.getAnimation("../assets/models/lowPolyHuman/ManUnity.fbx");
    entity->loadAnimationData(testAnimData);
    entity->playAnimation(2);
    lastTime = glfwGetTime();
    state=0;
    idleTime = 1.0;
}

Scene::Scene(){
    lights = std::vector<Light *>();
    textures = std::unordered_map<std::string, GLuint>();
    cities = std::vector<City *>();
}

Ped::Ped(){

}

void Ped::render() {
    float deltaTime = glfwGetTime() - lastTime;

    if (state == 0) { // Idle state
        idleTime -= deltaTime;
        if (idleTime < 0) {
            // Transition to walking
            state = 1;
            entity->playAnimation(0);

            // Randomly choose a direction: 0 (forward), 90 (right), or -90 (left)
            static std::random_device rd;
            static std::mt19937 rng(rd());
            std::uniform_int_distribution<int> directionDist(0, 2); // 0, 1, or 2
            int direction = directionDist(rng);

            float angle = 0.0f;
            if (direction == 0) angle = 0.0f;      // Forward
            else if (direction == 1) angle = 90.0f; // Right
            else if (direction == 2) angle = -90.0f; // Left

            // Update the entity's rotation
            entity->rotation.y += angle;

            // Calculate the target position based on the new direction
            glm::vec3 forwardVector = glm::vec3(
                    glm::sin(glm::radians(entity->rotation.y)),
                    0.0f,
                    glm::cos(glm::radians(entity->rotation.y))
            );
            targetPos = entity->position + forwardVector * 640.0f;
        }
    } else { // Walking state
        if (glm::length(targetPos - entity->position) < (pedSpeed * deltaTime)) {
            // Arrive at the target
            entity->position = targetPos;
            state = 0;
            entity->playAnimation(2);
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(0.0f, 7.0f);
            idleTime = 3.0f + dis(gen);
        } else {
            // Move toward the target
            entity->position += glm::normalize(targetPos - entity->position) * deltaTime * pedSpeed;
        }
    }

    lastTime = glfwGetTime();
    entity->update();
    entity->render(Camera::VP);
}


void Ped::lightRender(Light *light) {
    if (scene.isDaytime){
        entity->lightRender(sunGeo_shader, sunEntity_shader, *light);
    } else {
        entity->lightRender(shadowMap_shader, entityLight_shader, *light);
    }
}






float clamp(float x, float min, float max){
    return std::min(std::max(x, max), min);
}





void camera_update(float deltaTime){
    glm::vec3 forward = glm::normalize(Camera::lookAt - Camera::position);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    glm::vec3 current_speed = cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        current_speed *= glm::vec3(3.0);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        Camera::position += forward * current_speed*deltaTime;
        Camera::lookAt += forward * current_speed*deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        Camera::position -= forward * current_speed*deltaTime;
        Camera::lookAt -= forward * current_speed*deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        Camera::position -= right * current_speed*deltaTime;
        Camera::lookAt -= right * current_speed*deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        Camera::position += right * current_speed*deltaTime;
        Camera::lookAt += right * current_speed*deltaTime;
    }
    float newPos = 0;
    if (Camera::position.x < 0) {
        newPos = (TILE_SIZE * scene.cities[0]->tiles.size());
        Camera::position.x += newPos;
        Camera::lookAt.x += newPos;
        for (Ped *ped : scene.peds) {
            ped->targetPos.x += newPos;
            ped->entity->position.x += newPos;
            if (ped->targetPos.x > (TILE_SIZE*10*2)){
                ped->targetPos.x -= (TILE_SIZE*10*3);
                ped->entity->position.x -= (TILE_SIZE*10*3);
            }
        }
    }

    if (Camera::position.x > (TILE_SIZE * scene.cities[0]->tiles.size())) {
        newPos = TILE_SIZE * scene.cities[0]->tiles.size();
        Camera::position.x -= newPos;
        Camera::lookAt.x -= newPos;
        for (Ped *ped : scene.peds) {
            ped->targetPos.x -= newPos;
            ped->entity->position.x -= newPos;
            if (ped->targetPos.x < -(TILE_SIZE*10*2)){
                ped->targetPos.x += (TILE_SIZE*10*3);
                ped->entity->position.x += (TILE_SIZE*10*3);
            }
        }
    }

    if (Camera::position.z < 0) {
        newPos = TILE_SIZE * scene.cities[0]->tiles.size();
        Camera::position.z += newPos;
        Camera::lookAt.z += newPos;
        for (Ped *ped : scene.peds) {
            ped->targetPos.z += newPos;
            ped->entity->position.z += newPos;
            if (ped->targetPos.z > (TILE_SIZE*10*2)){
                ped->targetPos.z -= (TILE_SIZE*10*3);
                ped->entity->position.z -= (TILE_SIZE*10*3);
            }
        }
    }

    if (Camera::position.z > (TILE_SIZE * scene.cities[0]->tiles.size())) {
        newPos = TILE_SIZE * scene.cities[0]->tiles.size();
        Camera::position.z -= newPos;
        Camera::lookAt.z -= newPos;
        for (Ped *ped : scene.peds) {
            ped->targetPos.z -= newPos;
            ped->entity->position.z -= newPos;
            if (ped->targetPos.z < -(TILE_SIZE*10*2)){
                ped->targetPos.z += (TILE_SIZE*10*3);
                ped->entity->position.z += (TILE_SIZE*10*3);
            }
        }
    }
    Camera::updateViewMatrix();
}


bool pause = false;

std::vector<std::vector<int>> loadTileData(const std::string& filePath) {
    std::vector<std::vector<int>> grid;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<int> row;
        std::stringstream ss(line);
        std::string value;

        while (std::getline(ss, value, ',')) {
            row.push_back(std::stoi(value));
        }

        grid.push_back(row);
    }

    file.close();
    return grid;
}

int main(void)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int const screenWidth = 1920;
    int const screenHeight = 1080;

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(screenWidth, screenHeight, "Graphics Lab", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to open a GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -1;
    }

    geometry_programID = LoadShadersFromFile("../shaders/geometry.vert", "../shaders/geometry.frag");
    if (geometry_programID == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    lightingPass_shader = LoadShadersFromFile("../shaders/lighting.vert", "../shaders/lighting.frag");
    if (lightingPass_shader == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    shadowMap_shader = LoadShadersFromFile("../shaders/shadowMap.vert", "../shaders/shadowMap.geom", "../shaders/shadowMap.frag");
    if (shadowMap_shader == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    skybox_shader = LoadShadersFromFile("../shaders/skybox.vert", "../shaders/skybox.frag");
    if (skybox_shader == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    entity_shader = LoadShadersFromFile("../shaders/entity.vert", "../shaders/entity.frag");
    if (entity_shader == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    animation_shader = LoadShadersFromFile("../shaders/entityAnimated.vert", "../shaders/entityAnimated.frag");
    if (animation_shader == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    entityLight_shader = LoadShadersFromFile("../shaders/shadowEntity.vert", "../shaders/shadowEntity.geom", "../shaders/shadowEntity.frag");
    if (entityLight_shader == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    sunGeo_shader = LoadShadersFromFile("../shaders/sunShadowMap.vert", "../shaders/shadowMap.frag");
    if (sunGeo_shader == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    sunEntity_shader = LoadShadersFromFile("../shaders/sunShadowEntity.vert", "../shaders/shadowEntity.frag");
    if (sunEntity_shader == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    sunlighting_shader = LoadShadersFromFile("../shaders/lighting.vert", "../shaders/sunlighting.frag");
    if (sunlighting_shader == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    GLuint gBuffer, gColour, gPosition, gNormal, rboDepth, gEmit;


    // Create and configure the G-buffer
    createGBuffer(&gBuffer, &gColour, &gPosition, &gNormal, &rboDepth, &gEmit, screenWidth, screenHeight);


    // Background
    glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);


    DeferredShader * deferredShader = new DeferredShader(lightingPass_shader);
    deferredShader->initialize();

    //const char* concrete_texture = "../assets/textures/concrete/gravel_concrete_03_diff_4k.jpg";

    std::vector<std::vector<int>> cityTilesTypes = loadTileData("../city_layout.txt");

    //City * rootCity = new City(glm::vec3((cityDec*(TILE_SIZE*10))-(i*(TILE_SIZE*10)), 0.0, (cityDec*(TILE_SIZE*10))-(j*(TILE_SIZE*10))), cityTiles);
    //scene.cities.push_back(rootCity);

    std::vector<std::vector<Tile*>> baseCopy;
    for (int ti = 0; ti < cityTilesTypes.size(); ++ti) {
        std::vector<Tile*> newList;
        baseCopy.push_back(newList);
        float xPos = 0+(TILE_SIZE*ti);
        for (int tj = 0; tj < cityTilesTypes[ti].size(); ++tj) {
            float yPos = 0+(tj*TILE_SIZE);
            Tile * newTile = new Tile(glm::vec3(xPos, 0.0, yPos),cityTilesTypes[ti][tj]);
            if (cityTilesTypes[ti][tj] == 1 || cityTilesTypes[ti][tj] == 2){
                scene.lights.pop_back();
                scene.lights.pop_back();
            }
            baseCopy[ti].push_back(newTile);
        }

    }

    const int citySize = 5;
    const int cityDec = citySize/2;
    for (int i = 0; i < citySize; ++i) {
        float cityPosX = ((cityDec*(TILE_SIZE*10))-(i*(TILE_SIZE*10)));
        for (int j = 0; j < citySize; ++j) {
            std::vector<std::vector<Tile*>> cityTiles;
            float cityPosZ = ((cityDec*(TILE_SIZE*10))-(j*TILE_SIZE*10));
            for (int ti = 0; ti < cityTilesTypes.size(); ++ti) {
                std::vector<Tile*> newList;
                cityTiles.push_back(newList);
                float xPos = cityPosX+(TILE_SIZE*ti);
                for (int tj = 0; tj < cityTilesTypes[ti].size(); ++tj) {
                    float zPos = cityPosZ+(tj*TILE_SIZE);
                    Tile * newTile = new Tile(glm::vec3(xPos, 0.0, zPos),cityTilesTypes[ti][tj]);
                    Tile * baseTile = baseCopy[ti][tj];
                    if (cityTilesTypes[ti][tj] == 0) {
                        for (int k = 0; k < newTile->geometries.size(); k++) {
                            if (baseTile->geometries[k]->scale.y > 5) {
                                newTile->geometries[k]->textureID = baseTile->geometries[k]->textureID;
                                newTile->geometries[k]->position.y = baseTile->geometries[k]->position.y;
                                newTile->geometries[k]->scale.y = baseTile->geometries[k]->scale.y;
                                newTile->geometries[k]->modelMatrix = computeModelMatrix(newTile->geometries[k]->position,
                                                                                         newTile->geometries[k]->rotation,
                                                                                         newTile->geometries[k]->scale);

                                memcpy(newTile->geometries[k]->uv_buffer_data, baseTile->geometries[k]->uv_buffer_data,
                                       sizeof(GLfloat) * (baseTile->geometries[k]->numVertices * 2));
                                newTile->geometries[k]->setupBuffers();
                            }
                        }
                    }

                    cityTiles[ti].push_back(newTile);
                }

            }
            City * newCity = new City(glm::vec3(cityPosX, 0.0, cityPosZ), cityTiles);
            scene.cities.push_back(newCity);


        }
    }
    std::swap(scene.cities[0], scene.cities[(citySize*citySize)/2]);
    //City * rootCity = new City(glm::vec3(0.0, 0.0, 0.0), cityTiles);
    //scene.cities.push_back(rootCity);

    int numberOfPeds = 70;
    for (int i = 0; i < numberOfPeds; ++i) {
        Ped * newPed = new Ped();
        newPed->initialize();
        scene.peds.push_back(newPed);
    }

    // Camera setup
    Camera::position.y = viewDistance * cos(viewPolar);
    Camera::position.x = viewDistance * cos(viewAzimuth);
    Camera::position.z = viewDistance * sin(viewAzimuth);

    glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 70;
    glm::float32 zNear = 0.4f;
    glm::float32 zFar = (TILE_SIZE*10*3);
    //projectionMatrix = glm::perspective(glm::radians(FoV), (float) screenWidth / (float) screenHeight, zNear, zFar);
    Camera::updateProjectionMatrix(glm::radians(FoV), (float) screenWidth / (float) screenHeight, zNear, zFar);

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        Light * newLight = new Light(glm::vec3(0, -5000, 0), glm::vec3(0, 0, 0), 0.0, 1.0, 1, POINT_LIGHT);
        scene.lights.push_back(newLight);
    }

    Light * theSun = new Light(glm::vec3(0, 20000, 0), glm::vec3(244.0/255.0, 233.0/255.0, 155.0/255.0), 1.0, 4096, 40000.0, SPOT_LIGHT);
    scene.theSun = theSun;
    theSun->setDir(glm::normalize( glm::vec3(5*TILE_SIZE, 0, 5*TILE_SIZE)-theSun->position));

    Skybox * theSkybox = new Skybox();
    const std::vector<std::string> dayFilePaths = {"../assets/skybox/daytime/px.png",
                                                   "../assets/skybox/daytime/nx.png",
                                                   "../assets/skybox/daytime/py.png",
                                                   "../assets/skybox/daytime/ny.png",
                                                   "../assets/skybox/daytime/pz.png",
                                                   "../assets/skybox/daytime/nz.png"};

    const std::vector<std::string> nightFilePaths = {"../assets/skybox/night/px.png",
                                                     "../assets/skybox/night/nx.png",
                                                     "../assets/skybox/night/py.png",
                                                     "../assets/skybox/night/ny.png",
                                                     "../assets/skybox/night/pz.png",
                                                     "../assets/skybox/night/nz.png"};

    theSkybox->initialise(skybox_shader);
    int skyboxDay = theSkybox->addSkybox(dayFilePaths);
    int skyboxNight = theSkybox->addSkybox(nightFilePaths);
    scene.isDaytime = true;

    static double lastTime = glfwGetTime();
    float time = 0.0f;			// Animation time
    float fTime = 0.0f;			// Time for measuring fps
    unsigned long frames = 0;
    float lastFrameTime = 0.0f;


// Timing variables for sections
    double renderTime = 0.0;
    double lightRenderTime = 0.0;
    double deferredRenderTime = 0.0;

    do {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            scene.isDaytime = true;
            skyboxSetting = skyboxDay;
            theSkybox->skyboxIndex = skyboxSetting;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            scene.isDaytime = false;
            skyboxSetting = skyboxNight;
            theSkybox->skyboxIndex = skyboxSetting;
        }

        float currentFrameTime = glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        double currentTime = glfwGetTime();
        lastTime = currentTime;
        time += deltaTime;

        frames++;
        fTime += deltaTime;
        if (fTime > 2.0f) {
            float fps = frames / fTime;


            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << "City | FPS: " << fps
                   << " | Render: " << (float)(renderTime * 1000)/float(frames) << "ms"
                   << " | Light: " << (float)(lightRenderTime * 1000)/float(frames) << "ms"
                   << " | Deferred: " << (float)(deferredRenderTime * 1000)/float(frames) << "ms";
            glfwSetWindowTitle(window, stream.str().c_str());

            // Reset timing accumulators
            renderTime = 0.0;
            lightRenderTime = 0.0;
            deferredRenderTime = 0.0;
            frames = 0;
            fTime = 0;
        }

        camera_update(deltaTime);

        // Render to buffer
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Measure scene.render time
        double startRenderTime = glfwGetTime();
        theSkybox->render(Camera::viewMatrix, Camera::projectionMatrix);
        scene.render();
        renderTime += glfwGetTime() - startRenderTime;




        //scene.lightRender();
        // Measure light render time
        double startLightRenderTime = glfwGetTime();
        scene.lightRender();
        lightRenderTime += glfwGetTime() - startLightRenderTime;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            saveGBufferTextures(gBuffer, gColour, gPosition, gNormal, rboDepth, gEmit, 1920, 1080);
            //saveDepthTexture(theSun->shadowFBO, "sunDepth2.png");
        }

        // Render to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, screenWidth, screenHeight);

        // Measure deferred render time
        double startDeferredRenderTime = glfwGetTime();
        if (scene.isDaytime){
            deferredShader->sunRender(gBuffer, gColour, gPosition, gNormal, gEmit, *theSun, sunlighting_shader);
        } else {
            deferredShader->render(gBuffer, gColour, gPosition, gNormal, gEmit, scene.closeLights);
        }
        deferredRenderTime += glfwGetTime() - startDeferredRenderTime;

        glfwSwapBuffers(window);
    } while (!glfwWindowShouldClose(window));





    glDeleteFramebuffers(1, &gBuffer);
    glDeleteTextures(1, &gPosition);
    glDeleteTextures(1, &gNormal);
    glDeleteTextures(1, &gColour);
    glDeleteRenderbuffers(1, &rboDepth);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;

    // Initialize the lastX and lastY values if this is the first mouse movement
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Calculate offset from last mouse position
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    // Apply sensitivity to the offsets
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    // Update yaw and pitch
    yaw += xOffset;
    pitch += yOffset;

    // Constrain pitch to prevent flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Calculate the new `lookat` direction
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    Camera::lookAt = glm::normalize(direction) + Camera::position; // Update `lookat` based on camera position
    Camera::updateViewMatrix();
}



// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        viewAzimuth = 0.f;
        viewPolar = 0.f;
        Camera::position.y = viewDistance * cos(viewPolar);
        Camera::position.x = viewDistance * cos(viewAzimuth);
        Camera::position.z = viewDistance * sin(viewAzimuth);
        Camera::updateViewMatrix();
        std::cout << "Reset." << std::endl;
    }

    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        lightPosition.x = lightPosition.x + 8;

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        lightPosition.x = lightPosition.x - 8;

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        lightPosition.z = lightPosition.z + 8;
    }

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        lightPosition.z = lightPosition.z - 8;

    if (key == GLFW_KEY_I && action == GLFW_PRESS)
        lightPosition.y = lightPosition.y + 8;

    if (key == GLFW_KEY_K && action == GLFW_PRESS)
        lightPosition.y = lightPosition.y - 8;


    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
