//
// Created by rowan on 26/12/2024.
//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "string.h"
#include "geometry.h"
#include "animationData.h"
#ifndef LAB4_CITY_H
#define LAB4_CITY_H
class Tile{
public:
    int tileType;
    std::vector<Geometry*> geometries;
    glm::vec3 position;
    Tile(glm::vec3 position, int tileType);
    void setupPlane();
    void setupRoad(glm::vec3 rotation);
    void setupTway(glm::vec3 rotation);
    void setupCorner(glm::vec3 rotation);
    void render();
    void lightRender(Light * light);


};


class City{
public:
    glm::vec3 position;
    std::vector<std::vector<Tile*>> tiles;
    City(glm::vec3 position, std::vector<std::vector<Tile*>> tiles);
    void render();
    void lightRender(Light * light);
};


class Scene{
public:
     std::vector<City*> cities;
     std::unordered_map<std::string, GLuint> textures;
     std::vector<Entity*> entities;
     std::vector<Light*> lights;

    Scene();
     GLuint getTexture(const std::string& filepath);
     void render();
     void lightRender();
};
#endif //LAB4_CITY_H
