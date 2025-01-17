#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "math.h"
#include "stdio.h"
#include "./functions/3dfunctions.c"

#define RCAMERA_IMPLEMENTATION

//Define types and structures
//------------------------------------------------------------------------------------
typedef enum GameScreen { TITLE, LEVELSELECT, EPISODE1 } GameScreen;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 450, "microwave simulator");
    
    SetTargetFPS(60);
    
    InitAudioDevice();
    
    DisableCursor();
    
    //First variable declarations
    //--------------------------------------------------------------------------------------
    int currentmonitor = 0;
    
    float playerspeed;
    float playersprint;
    Vector3 playermove = {0, 0, 0};
    Vector3 playerrotate = {0, 0, 0};
    
    bool fullscreen = false;
    bool moving = false;
    bool Tutorial_Interact_Completed = false;
    bool levelstarted = false;
    
    Vector3 microwaveposition = { 0.0f, 1.2f, 0.0f };
    Vector3 microwavesize = {4.5f, 4.5f, 4.5f};
    Vector2 mapPosition = {-2, -2};
      
    Sound microwavesfx = LoadSound("sounds/microwave.ogg");
    
    GameScreen currentscreen = TITLE;

    Ray InteractRay = { 0 };
    
    Quaternion Vector3RotateRight = QuaternionFromAxisAngle((Vector3){ 0.0f, 1.0f, 0.0f }, DEG2RAD * 90.0f);
    Quaternion Vector3RotateLeft = QuaternionFromAxisAngle((Vector3){ 0.0f, 1.0f, 0.0f }, DEG2RAD * -90.0f);
    
    //Define Camera
    //--------------------------------------------------------------------------------------
    Camera camera = { 0 };
    camera.position = (Vector3){0.0f, 2.0f, 4.0f};
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f};
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    int cameraMode = CAMERA_ORBITAL;
    
    //Load Models/Textures
    //--------------------------------------------------------------------------------------   
    Model microwave = LoadModel("models/kitchen-microwave-appliance/microwave.glb");
    //microwave.transform = microwave.transform * MatrixScale(4.5f, 4.5f, 4.5f);
    BoundingBox microwave_collision;
    
    Model kitchencounter = LoadModel("models/kitchen counter/counter.glb");   
    
    Image testimage = LoadImage("textures/testimage.png");      // Load cubicmap image (RAM)
    Texture2D cubicmap = LoadTextureFromImage(testimage);       // Convert image to texture to display (VRAM)
    Color *mapPixels = LoadImageColors(testimage);
    UnloadImage(testimage);

    Mesh map01_mesh = GenMeshCubicmapEx(testimage, (Vector3){ 3.0f, 3.0f, 3.0f });
    Model map01 = LoadModelFromMesh(map01_mesh);
    BoundingBox map01_collision = GetModelBoundingBox(map01);
    
    
    Texture2D map01_diffuse = LoadTexture("textures/map_atlas.png");
    map01.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = map01_diffuse;

    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector3 oldcameraposition = camera.position;
        Vector3 cameraforward = GetCameraForward(&camera);
        
        //int playerCellX = camera.position.x - mapPosition.x;
        //int playerCellY = camera.position.y - mapPosition.y;
        
        float playerspeed = 6.0f*GetFrameTime();
        float playersprint = 12.0f*GetFrameTime();
        

        RayCollision InteractRayCollision = { 0 };
        InteractRayCollision.hit = false;        
        InteractRay = GetMouseRay((Vector2){(GetScreenWidth()/2), (GetScreenHeight()/2)}, camera);        
        RayCollision MicrowaveHitInfo = GetRayCollisionBox(InteractRay, microwave_collision);
        
        
        switch(currentscreen)
        {
            case TITLE:
            {
                cameraMode = CAMERA_ORBITAL;
                bool sfxplaying = IsSoundPlaying(microwavesfx);
                if(!sfxplaying) PlaySound(microwavesfx);
                
                if(IsKeyPressed(KEY_SPACE)) currentscreen = LEVELSELECT;
            
                UpdateCamera(&camera, cameraMode);
            } break;
            
            case LEVELSELECT: 
            {
                StopSound(microwavesfx);
                cameraMode = CAMERA_FIRST_PERSON;
                UpdateCamera(&camera, cameraMode); 
                
                if (IsKeyPressed(KEY_E) && MicrowaveHitInfo.hit)
                {
                    currentscreen = EPISODE1;
                    Tutorial_Interact_Completed = true;
                }
            } break;
            
            case EPISODE1: 
            {
                if (!levelstarted) 
                {
                    camera.position = (Vector3){0.0f, 2.0f, 4.0f};
                    levelstarted = true;
                }
                
                cameraMode = CAMERA_FIRST_PERSON;
                UpdateCamera(&camera, cameraMode);
            } break;
        }


        camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
        
        Vector3 movevector = Vector3Multiply(Vector3Normalize(cameraforward), (Vector3){GetFrameTime()*playerspeed, 0, GetFrameTime()*playerspeed});
        
        //Key Presses
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_F11))
        {
            fullscreen = !fullscreen;            
            if(fullscreen){MaximizeWindow();}
            else{RestoreWindow();}            
            ToggleBorderlessWindowed();
        }
        if (currentscreen == EPISODE1) 
        {
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D))
            {
                moving = true;
            }
            else
            {
                moving = false;
            }
            
            if(IsKeyDown(KEY_W)) 
            {
                if (playermove.x < playerspeed) playermove.x += 0.01f;
                else playermove.x = playerspeed;
                if(IsKeyDown(KEY_LEFT_SHIFT)) playermove.x = playersprint;                
            }
            
            if(IsKeyDown(KEY_A)) 
            {
                playermove.y = -playerspeed;   
                if(IsKeyDown(KEY_LEFT_SHIFT)) playermove.y = -playersprint;
            }
            
            if (IsKeyDown(KEY_S)) 
            {
                playermove.x = -playerspeed;
            }
            
            if(IsKeyDown(KEY_D)) 
            {
                playermove.y = playerspeed;
                if(IsKeyDown(KEY_LEFT_SHIFT)) playermove.y = playersprint;                
            }
            
            
            if (!IsKeyDown(KEY_W) && !IsKeyDown(KEY_S)) playermove.x = 0;
            if (!IsKeyDown(KEY_A) && !IsKeyDown(KEY_D)) playermove.y = 0;
            
            if (IsKeyPressed(KEY_LEFT_CONTROL)) 
            {
                bool crouching = !crouching; 
                if (crouching)
                {
                    camera.position.y = camera.position.y - 1;
                    camera.target.y = camera.target.y - 1;
                    playerspeed = playerspeed/10;
                }
                else
                {
                    camera.position.y = camera.position.y + 1;
                    camera.target.y = camera.target.y + 1;
                    playerspeed = playerspeed*10;
                }
            }
            
            Vector3ClampValue(playermove, 0, playersprint);
            UpdateCameraPro(&camera, playermove, playerrotate, 0);
        }
            
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------        
        

        
        microwave_collision = GetMeshBoundingBox(microwave.meshes[0]);
        microwave_collision.min = Vector3Scale(microwave_collision.min, microwavesize.x);
        microwave_collision.max = Vector3Scale(microwave_collision.max, microwavesize.x);
        microwave_collision.min = Vector3Add(microwave_collision.min, microwaveposition);
        microwave_collision.max = Vector3Add(microwave_collision.max, microwaveposition);
                
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(BLACK);                  
            
            switch(currentscreen)
            {
                case TITLE:
                {
                    DrawText("welcome to microwave game", 10, 30, 20, WHITE);
                    DrawText("press space to start", 10, 50, 20, WHITE);

                    
                    BeginMode3D(camera); // begin 3d drawing
                    
                    DrawModelEx(microwave, microwaveposition, (Vector3){0, 1, 0}, 0.0f, microwavesize, WHITE);
                    DrawModelEx(kitchencounter, (Vector3){0, 0, 0}, (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, WHITE);
                
                    DrawGrid(10, 1.0f);
                    
                    EndMode3D(); // end 3d drawing
                    
                    DrawFPS(10, 10);                   
                } break;
                
                
                case LEVELSELECT:
                {
                    BeginMode3D(camera);
                    
                    DrawModelEx(microwave, microwaveposition, (Vector3){0, 1, 0}, 0.0f, microwavesize, WHITE);
                    DrawModelEx(kitchencounter, (Vector3){0, 0, 0}, (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, WHITE);
                    
                    EndMode3D();
                    
                    if(!Tutorial_Interact_Completed)
                    { 
                        float temp_01_float = GetScreenWidth()/2-MeasureText("Press 'E' to interact.", 20)/2;
                        float temp_02_float = GetScreenHeight()-GetScreenHeight()*0.2f;
                        DrawText("Press 'E' to interact.", temp_01_float, temp_02_float, 20, RED);
                    }
                } break;
                
                case EPISODE1:
                {                    
                    BeginMode3D(camera);
                    
                    DrawModel(map01, (Vector3){-2, 0, -2}, 1, WHITE);
                    
                    EndMode3D();   
                    
                    //DrawText(playerCellX, 10, 20, 20, WHITE);
                    //DrawText(playerCellY, 10, 40, 20, WHITE);
                    DrawText(TextFormat("Cam Pos: %f, %f", camera.position.x, camera.position.y), 10, 60, 20, WHITE);
                    DrawText(TexFormat("%d", mapPosition), 10, 70, 100, WHITE);
                    
                    DrawFPS(10, 10); 
                } break;
            }     
 
        EndDrawing();
        //----------------------------------------------------------------------------------
    
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}