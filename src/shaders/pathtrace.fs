#version 430

in vec2 fragTexCoord;
out vec4 finalColor;

uniform float time;
uniform vec2 resolution;
uniform vec3 camPos;
uniform vec3 camTarget;

uniform int numSpheres;

uniform int frameCount;
uniform sampler2D skybox;
uniform sampler2D previousFrame;


struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Material{
    vec3 albedo;
    float roughness;
    vec3 emissionColor;
    float emissionStrength;
    float transmittance;
    float ior;
};

struct Sphere {
    vec3 center;
    float radius;
    Material mat;
};

struct HitInfo {
    bool didHit;
    float dist;
    vec3 normal;
    Material mat;
};

// This is an SSBO! It can hold megabytes of data.
// binding = 0 means it connects to "Buffer Slot 0" on the GPU
layout(std430, binding = 0) buffer SphereBuffer {
    Sphere spheres[]; 
};



uint pcg_hash(inout uint seed) 
{
    seed = seed * 747796405u + 2891336453u;
    uint word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    return (word >> 22u) ^ word;
}

float randomFloat(inout uint seed) 
{
    return float(pcg_hash(seed)) / 4294967295.0;
}

vec3 randomDirection(inout uint seed) 
{
    // Math trick to pick a random point on a 3D sphere
    float z = randomFloat(seed) * 2.0 - 1.0;
    float a = randomFloat(seed) * 6.2831853; // 2 * PI
    float r = sqrt(1.0 - z * z);
    return vec3(r * cos(a), r * sin(a), z);
}


Ray getCameraRay(vec2 uv, vec3 lookFrom, vec3 lookAt, float zoom, inout uint seed)
{
    lookAt += randomDirection(seed) * 0.01;     // for antialiasing
    vec3 forward = normalize(lookAt - lookFrom);
    vec3 globalUp = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(forward, globalUp));
    vec3 up = cross(forward, right);
    vec3 rayDir = normalize(forward * zoom + uv.x * right + uv.y * up);
    
    return Ray(lookFrom, rayDir);
}

HitInfo intersect(Ray ray, Sphere sph)
{
    vec3 sph2tail = ray.origin - sph.center;
    float b = dot(ray.direction, sph2tail);
    float c = dot(sph2tail, sph2tail) - sph.radius * sph.radius;
    float disc = b * b - c;

    HitInfo hitInfo;
    hitInfo.didHit = false;

    if (disc >= 0.0)
    {
        float root_disc = sqrt(disc);
        float dist_near = -b - root_disc;
        float dist_far = -b + root_disc;
        if (dist_near > 0.0 || dist_far > 0.0)
        {
            hitInfo.didHit = true;
            hitInfo.mat = sph.mat;

            if (dist_near > 0.0)
            {
                hitInfo.dist = dist_near;
            }
            else if (dist_far > 0.0)
            {
                hitInfo.dist = dist_far;
            }
            hitInfo.normal = normalize(hitInfo.dist * ray.direction + ray.origin - sph.center);
        }
    }
    return hitInfo;
}


HitInfo traceScene(Ray ray)
{
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.dist = 999999.0;

    for (int i = 0; i < numSpheres; i++)
    {
        HitInfo hitInfo = intersect(ray, spheres[i]);
        if (hitInfo.didHit)
        {
            if (hitInfo.dist < closestHit.dist)
            {
                closestHit = hitInfo;
            }
        }
    }
    return closestHit;
}

void main()
{
    uint seed = uint(gl_FragCoord.x) * 1973u + 
                uint(gl_FragCoord.y) * 9277u + 
                uint(time * 1000.0) * 26699u;

    vec2 uv = fragTexCoord * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    Ray ray = getCameraRay(uv, camPos, camTarget, 1.0, seed);

    vec3 color = vec3(0.0);
    vec3 throughput = vec3(1.0);    // energy of light;

    int maxBounce = 10;
    
    for (int bounce = 0; bounce < maxBounce; bounce++)
    {
        HitInfo hitInfo = traceScene(ray);
        if (hitInfo.didHit)
        { 
            color += throughput * (hitInfo.mat.emissionColor * hitInfo.mat.emissionStrength);
            

            vec3 hitPoint = ray.direction * hitInfo.dist + ray.origin;

            vec3 N = hitInfo.normal;
            float cosi = dot(ray.direction, hitInfo.normal);
            float eta;

            if (cosi < 0.0) // entering material
            {
                eta = 1.0 / hitInfo.mat.ior;
            }
            else        // exiting material
            {
                N = -N;
                eta = hitInfo.mat.ior / 1.0;
            }


            if (hitInfo.mat.transmittance > randomFloat(seed))    // refraction
            {
                float cosX = clamp(dot(-ray.direction, N), 0.0, 1.0);

                float R0 = pow((1.0 - hitInfo.mat.ior) / (1.0 + hitInfo.mat.ior), 2.0);
                float fresnel = R0 + (1.0 - R0) * pow(1.0 - cosX, 5.0);

                if (randomFloat(seed) < fresnel)    // Fresnel reflection
                {
                    ray.direction = reflect(ray.direction, N);
                    ray.origin = hitPoint + N * 0.001;
                    // throughput *= 1.0 / (fresnel + 0.001);
                }
                else
                {
                    vec3 refractedDir = refract(ray.direction, N, eta);
                    if (refractedDir == vec3(0.0))     // TIR
                    {
                        ray.direction = reflect(ray.direction, N);
                        ray.origin = hitPoint + N * 0.001;
                    }
                    else
                    {
                        ray.direction = refractedDir;
                        ray.origin = hitPoint - N * 0.001;
                    }
                    throughput *= 1.0 / (1.0 - fresnel + 0.001);
                }

                
            }
            else        // reflection
            {
                ray.origin = hitPoint + N * 0.001;
                if (randomFloat(seed) > hitInfo.mat.roughness)     // specular reflection
                {
                    ray.direction = reflect(ray.direction, N);
                }
                else        // diffuse reflection
                {
                    ray.direction = normalize(N + randomDirection(seed));
                    throughput *= hitInfo.mat.albedo;
                }   
            }

        }
        else
        {
            vec3 dir = normalize(ray.direction);

            float u = atan(dir.x, -dir.z) / (2.0 * 3.14159265) + 0.5;
            float v = acos(clamp(dir.y, -1.0, 1.0)) / 3.14159265;

            vec3 skyColor = texture(skybox, vec2(u, v)).rgb;
            color += throughput * skyColor;

            break;
        }
    }

    vec2 texCoord = vec2(fragTexCoord.x, 1.0 - fragTexCoord.y); 
    vec3 prevColor = texture(previousFrame, texCoord).rgb;

    vec3 accumulatedColor = mix(prevColor, color, 1.0 / float(frameCount));

    finalColor = vec4(accumulatedColor, 1.0); 
    
}


