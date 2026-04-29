#include "scene/scene.h"
#include "utils/utils.h"


EngineMaterial random_mat()
{
    EngineMaterial mat;
    mat.albedo = (Vector3){randomVal(), randomVal(), randomVal()};
    mat.emissionStrength = 0.0;
    mat.emissionColor = mat.albedo;
    if (randomVal() * 0.5 + 0.5 > 0.5)
    {
        mat.roughness = 0.2;
    }
    else
    {
        mat.roughness = 1.0;
    }

    mat.ior = randomVal() * 0.5 + 1.6;
    mat.transmittance = randomVal() * 0.5 + 0.5;
    // mat.transmittance = 0.1;
    return mat;
}


void Scene::shaderSetup(Shader& _shader)
{
    shader = &_shader;
    numSpheresLoc = GetShaderLocation(_shader, "numSpheres");

    // RL_DYNAMIC_DRAW tells the GPU we plan to change this data often.
    ssboId = rlLoadShaderBuffer(10000 * sizeof(Sphere), nullptr, RL_DYNAMIC_DRAW);      // space for upto 10000 spheres
}

void Scene::writeToShader()
{
    int size = spheres.size();
    SetShaderValue(*shader, numSpheresLoc, &size, SHADER_UNIFORM_INT);
    
    if (size > 0)
    {
        rlUpdateShaderBuffer(ssboId, spheres.data(), size * sizeof(Sphere), 0);
        rlBindShaderBuffer(ssboId, 0);
    }
}

void Scene::addSpheres()
{
    spheres.emplace_back(Sphere{
        (Vector3){0.0, -502.0, 0.0},
        500.0,
        EngineMaterial{
            .albedo = (Vector3){1.0, 0.8, 0.8},
            .roughness = 1.0,
            .emissionColor = (Vector3){1.0, 1.0, 1.0},
            .emissionStrength = 0.0,
            .transmittance = 0.0
        }
        }
    );
    spheres.emplace_back(Sphere{
        (Vector3){0.0, 200.0, -120.0},
        50.0,
        EngineMaterial{
            .albedo = (Vector3){1.0, 1.0, 1.0},
            .roughness = 1.0,
            .emissionColor = (Vector3){1.0, 1.0, 1.0},
            .emissionStrength = 2.5
        }
        }
    );
    // spheres.emplace_back(Sphere{
    //     (Vector3){2.5, 0.0, 0.0},
    //     1.0,
    //     EngineMaterial{
    //         .albedo = (Vector3){1.0, 1.0, 1.0},
    //         .roughness = 1.0,
    //         .emissionColor = (Vector3){1.0, 1.0, 1.0},
    //         .emissionStrength = 0.0,
    //         .transmittance = 0.5,
    //         .ior = 1.33
    //     }
    //     }
    // );
    // spheres.emplace_back(Sphere{
    //     (Vector3){0.0, 1.0, 0.0},
    //     1.5,
    //     EngineMaterial{
    //         .albedo = (Vector3){1.0, 1.0, 1.0},
    //         .roughness = 0.01,
    //         .emissionColor = (Vector3){1.0, 1.0, 1.0},
    //         .emissionStrength = 0.0,
    //         .transmittance = 0.95,
    //         .ior = 1.0
    //     }
    //     }
    // );
    spheres.emplace_back(Sphere{
        (Vector3){0.0, 1.0, 0.0},
        2.0,
        EngineMaterial{
            .albedo = (Vector3){1.0, 1.0, 1.0},
            .roughness = 0.1,
            .emissionColor = (Vector3){1.0, 1.0, 1.0},
            .emissionStrength = 0.0,
            .transmittance = 0.95,
            .ior = 1.33
        }
        }
    );



    for (int i = 0; i < 20; i++)
    {
        Sphere sph;
        sph.center = (Vector3){randomVal(), 0.0, randomVal()} * 20.0;
        sph.radius = randomVal()* 0.5 + 1.0;

        sph.mat = random_mat();

        spheres.push_back(sph);
    }
}


