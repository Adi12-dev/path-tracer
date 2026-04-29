#include "raylib.h"
#include "raymath.h"

#include "rlgl.h"

#include <vector>



struct alignas(16) EngineMaterial{
    Vector3 albedo;
    float roughness;
    Vector3 emissionColor;
    float emissionStrength;
    float transmittance;
    float ior;      // index of refraction
    float padding1;
    float padding2;
};

struct alignas(16) Sphere {
    Vector3 center;
    float radius;
    EngineMaterial mat;
};


class Scene
{
public:
    int numSpheresLoc;
    unsigned int ssboId;
    std::vector<Sphere> spheres;
    

    Scene() {}

    void shaderSetup(Shader& _shader);
    void writeToShader();
    void addSpheres();

private:
    Shader* shader = nullptr;
};