#include "scene/scene.h"

void draw_quad()
{
    // Draw a custom fullscreen quad with explicit UVs
    rlBegin(RL_QUADS);
        // Top-Left
        rlTexCoord2f(0.0f, 0.0f); 
        rlVertex2f(0.0f, 0.0f);
        
        // Bottom-Left
        rlTexCoord2f(0.0f, 1.0f); 
        rlVertex2f(0.0f, (float)GetScreenHeight());
        
        // Bottom-Right
        rlTexCoord2f(1.0f, 1.0f); 
        rlVertex2f((float)GetScreenWidth(), (float)GetScreenHeight());
        
        // Top-Right
        rlTexCoord2f(1.0f, 0.0f); 
        rlVertex2f((float)GetScreenWidth(), 0.0f);
    rlEnd();
}

RenderTexture2D LoadRenderTextureFloat(int width, int height)
{
RenderTexture2D target = { 0 };
    target.id = rlLoadFramebuffer();   
    
    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);
        target.texture.id = rlLoadTexture(nullptr, width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        target.texture.mipmaps = 1;

        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

        rlDisableFramebuffer();
    }

    return target;
}


int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 800, "path tracer");
    
    Shader shader = LoadShader(
        "src/shaders/fullscreen.vs",
        "src/shaders/pathtrace.fs"
    );

    int shadertTimeLoc = GetShaderLocation(shader, "time");
    int resolutionLoc = GetShaderLocation(shader, "resolution");
    int camPosLoc = GetShaderLocation(shader, "camPos");
    int camTargetLoc = GetShaderLocation(shader, "camTarget");

    int frameCountLoc = GetShaderLocation(shader, "frameCount");
    int previousFrameLoc = GetShaderLocation(shader, "previousFrame");
    int skyboxLoc = GetShaderLocation(shader, "skybox");

    Image img = LoadImage("skyboxes/skybox.png");
    Texture2D skyTex = LoadTextureFromImage(img);
    SetShaderValueTexture(shader, skyboxLoc, skyTex);
    UnloadImage(img);


    RenderTexture2D targetA = LoadRenderTextureFloat(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D targetB = LoadRenderTextureFloat(GetScreenWidth(), GetScreenHeight());

    RenderTexture2D* currentTarget = &targetA;
    RenderTexture2D* previousTarget = &targetB;

    int frameCount = 1;

    float time = 0.0;

    Scene scene;
    scene.shaderSetup(shader);
    scene.addSpheres();
    scene.writeToShader();

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, -10.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;  

    DisableCursor(); 
    SetTargetFPS(60);
    float sensitivity = 5.0;
    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        float resolution[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
        time = GetTime();



        UpdateCameraPro(&camera,
            (Vector3){
                (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))*sensitivity*delta -      // Move forward-backward
                (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))*sensitivity*delta,
                (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))*sensitivity*delta -   // Move right-left
                (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))*sensitivity*delta,
                (IsKeyDown(KEY_E)) * sensitivity * delta - (IsKeyDown(KEY_Q) * sensitivity * delta)      // Move up-down
            },
            (Vector3){
                GetMouseDelta().x * delta * 2.f,                            // Rotation: yaw
                GetMouseDelta().y * delta * 2.f,                            // Rotation: pitch
                0.0f                                                        // Rotation: roll
            },
            GetMouseWheelMove()*2.0f * delta                                // for mouse wheel zoom control
        );

        Vector2 mouseDelta = GetMouseDelta();
        bool cameraMoved = (
            IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D) ||
            IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) ||
            IsKeyDown(KEY_E) || IsKeyDown(KEY_Q) ||
            mouseDelta.x != 0.0f || mouseDelta.y != 0.0f ||
            GetMouseWheelMove() != 0.0f
        );

        // If we moved, clear the buffer by resetting the counter!
        if (cameraMoved) {
            frameCount = 1;
        }

        BeginTextureMode(*currentTarget);
            ClearBackground(BLACK);
            BeginShaderMode(shader);
                SetShaderValue(shader, shadertTimeLoc, &time, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);
                SetShaderValue(shader, camPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, camTargetLoc, &camera.target, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, frameCountLoc, &frameCount, SHADER_UNIFORM_INT);
                SetShaderValueTexture(shader, skyboxLoc, skyTex);
                SetShaderValueTexture(shader, previousFrameLoc, previousTarget->texture);
                draw_quad();
            EndShaderMode();
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTextureRec(
                currentTarget->texture, 
                (Rectangle){ 0, 0, (float)currentTarget->texture.width, (float)-currentTarget->texture.height }, 
                (Vector2){ 0, 0 }, 
                WHITE
            );
        EndDrawing();

        RenderTexture2D* temp = currentTarget;
        currentTarget = previousTarget;
        previousTarget = temp;

        frameCount++;
    }

    UnloadShader(shader);
    CloseWindow();
}



