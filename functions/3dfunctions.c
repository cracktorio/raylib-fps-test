#include "raylib.h"         // Declares module functions

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"     // Defines module configuration flags
#endif

#include "utils.h"          // Required for: TRACELOG(), LoadFileData(), LoadFileText(), SaveFileText()
#include "rlgl.h"           // OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2
#include "raymath.h"        // Required for: Vector3, Quaternion and Matrix functionality

#include <stdio.h>          // Required for: sprintf()
#include <stdlib.h>         // Required for: malloc(), free()
#include <string.h>         // Required for: memcmp(), strlen()
#include <math.h>           // Required for: sinf(), cosf(), sqrtf(), fabsf()

// Generate a cubes mesh from pixel data
// NOTE: Vertex data is uploaded to GPU
Mesh GenMeshCubicmapEx(Image cubicmap, Vector3 cubeSize)
{
    #define COLOR_EQUAL(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))
    #define GRAY_131  (Color){ 131, 131, 131, 255 }
    #define DARKGRAY_81  (Color){ 81, 81, 81, 255 }


    Mesh mesh = { 0 };

    Color *pixels = LoadImageColors(cubicmap);

    // NOTE: Max possible number of triangles numCubes*(12 triangles by cube)
    int maxTriangles = cubicmap.width * cubicmap.height * 12;

    int vCounter = 0;       // Used to count vertices
    int tcCounter = 0;      // Used to count texcoords
    int nCounter = 0;       // Used to count normals

    float w = cubeSize.x;
    float h = cubeSize.z;
    float h2 = cubeSize.y;

    Vector3 *mapVertices = (Vector3 *)RL_MALLOC(maxTriangles*3*sizeof(Vector3));
    Vector2 *mapTexcoords = (Vector2 *)RL_MALLOC(maxTriangles*3*sizeof(Vector2));
    Vector3 *mapNormals = (Vector3 *)RL_MALLOC(maxTriangles*3*sizeof(Vector3));

    // Define the 6 normals of the cube, we will combine them accordingly later...
    Vector3 n1 = { 1.0f, 0.0f, 0.0f };
    Vector3 n2 = { -1.0f, 0.0f, 0.0f };
    Vector3 n3 = { 0.0f, 1.0f, 0.0f };
    Vector3 n4 = { 0.0f, -1.0f, 0.0f };
    Vector3 n5 = { 0.0f, 0.0f, -1.0f };
    Vector3 n6 = { 0.0f, 0.0f, 1.0f };

    Vector3 n1half = { 0.5f, 0.0f, 0.0f };
    Vector3 n2half = { -0.5f, 0.0f, 0.0f };
    Vector3 n3half = { 0.0f, 0.5f, 0.0f };
    Vector3 n4half = { 0.0f, -0.5f, 0.0f };
    Vector3 n5half = { 0.0f, 0.0f, -0.5f };
    Vector3 n6half = { 0.0f, 0.0f, 0.5f };

    // NOTE: We use texture rectangles to define different textures for top-bottom-front-back-right-left (6)
    typedef struct RectangleF {
        float x;
        float y;
        float width;
        float height;
    } RectangleF;

    RectangleF rightTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
    RectangleF leftTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
    RectangleF frontTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
    RectangleF backTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
    RectangleF topTexUV = { 0.0f, 0.5f, 0.5f, 0.5f };
    RectangleF bottomTexUV = { 0.5f, 0.5f, 0.5f, 0.5f };

    for (int z = 0; z < cubicmap.height; ++z)
    {
        for (int x = 0; x < cubicmap.width; ++x)
        {
            // Define the 8 vertex of the cube, we will combine them accordingly later...
            Vector3 v1 = { w*(x - 0.5f), h2, h*(z - 0.5f) };
            Vector3 v2 = { w*(x - 0.5f), h2, h*(z + 0.5f) };
            Vector3 v3 = { w*(x + 0.5f), h2, h*(z + 0.5f) };
            Vector3 v4 = { w*(x + 0.5f), h2, h*(z - 0.5f) };
            Vector3 v5 = { w*(x + 0.5f), 0, h*(z - 0.5f) };
            Vector3 v6 = { w*(x - 0.5f), 0, h*(z - 0.5f) };
            Vector3 v7 = { w*(x - 0.5f), 0, h*(z + 0.5f) };
            Vector3 v8 = { w*(x + 0.5f), 0, h*(z + 0.5f) };
            
            Vector3 v1half = { w*(x - 0.5f), h2/2, h*(z - 0.5f) };
            Vector3 v2half = { w*(x - 0.5f), h2/2, h*(z + 0.5f) };
            Vector3 v3half = { w*(x + 0.5f), h2/2, h*(z + 0.5f) };
            Vector3 v4half = { w*(x + 0.5f), h2/2, h*(z - 0.5f) };
            Vector3 v5half = { w*(x + 0.5f), 0, h*(z - 0.5f) };
            Vector3 v6half = { w*(x - 0.5f), 0, h*(z - 0.5f) };
            Vector3 v7half = { w*(x - 0.5f), 0, h*(z + 0.5f) };
            Vector3 v8half = { w*(x + 0.5f), 0, h*(z + 0.5f) };
            Vector3 v5floorhalf = { w*(x + 0.5f), 1.5f, h*(z - 0.5f) };
            Vector3 v6floorhalf = { w*(x - 0.5f), 1.5f, h*(z - 0.5f) };
            Vector3 v7floorhalf = { w*(x - 0.5f), 1.5f, h*(z + 0.5f) };
            Vector3 v8floorhalf = { w*(x + 0.5f), 1.5f, h*(z + 0.5f) };
            
            Vector3 v1quarter = { w*(x - 0.5f), h2/4, h*(z - 0.5f) };
            Vector3 v2quarter = { w*(x - 0.5f), h2/4, h*(z + 0.5f) };
            Vector3 v3quarter = { w*(x + 0.5f), h2/4, h*(z + 0.5f) };
            Vector3 v4quarter = { w*(x + 0.5f), h2/4, h*(z - 0.5f) };
            Vector3 v5quarter = { w*(x + 0.5f), 0, h*(z - 0.5f) };
            Vector3 v6quarter = { w*(x - 0.5f), 0, h*(z - 0.5f) };
            Vector3 v7quarter = { w*(x - 0.5f), 0, h*(z + 0.5f) };
            Vector3 v8quarter = { w*(x + 0.5f), 0, h*(z + 0.5f) };
            Vector3 v5floorquarter = { w*(x + 0.5f), 0.75f, h*(z - 0.5f) };
            Vector3 v6floorquarter = { w*(x - 0.5f), 0.75f, h*(z - 0.5f) };
            Vector3 v7floorquarter = { w*(x - 0.5f), 0.75f, h*(z + 0.5f) };
            Vector3 v8floorquarter = { w*(x + 0.5f), 0.75f, h*(z + 0.5f) };
            
			Vector3 v1halfinverted = { w*(x - 0.5f), h2, h*(z - 0.5f) };
            Vector3 v2halfinverted = { w*(x - 0.5f), h2, h*(z + 0.5f) };
            Vector3 v3halfinverted = { w*(x + 0.5f), h2, h*(z + 0.5f) };
            Vector3 v4halfinverted = { w*(x + 0.5f), h2, h*(z - 0.5f) };
            Vector3 v5halfinverted = { w*(x + 0.5f), h2/2, h*(z - 0.5f) };
            Vector3 v6halfinverted = { w*(x - 0.5f), h2/2, h*(z - 0.5f) };
            Vector3 v7halfinverted = { w*(x - 0.5f), h2/2, h*(z + 0.5f) };
            Vector3 v8halfinverted = { w*(x + 0.5f), h2/2, h*(z + 0.5f) };
            Vector3 v1ceilinghalf = { w*(x - 0.5f), 1.5f, h*(z - 0.5f) };
            Vector3 v2ceilinghalf = { w*(x - 0.5f), 1.5f, h*(z + 0.5f) };
            Vector3 v3ceilinghalf = { w*(x + 0.5f), 1.5f, h*(z + 0.5f) };
            Vector3 v4ceilinghalf = { w*(x + 0.5f), 1.5f, h*(z - 0.5f) };
            Vector3 v5ceilinghalf = { w*(x + 0.5f), 1.5f, h*(z - 0.5f) };
            Vector3 v6ceilinghalf = { w*(x - 0.5f), 1.5f, h*(z - 0.5f) };
            Vector3 v7ceilinghalf = { w*(x - 0.5f), 1.5f, h*(z + 0.5f) };
            Vector3 v8ceilinghalf = { w*(x + 0.5f), 1.5f, h*(z + 0.5f) };
			
			Vector3 v1quarterinverted = { w*(x - 0.5f), h2, h*(z - 0.5f) };
            Vector3 v2quarterinverted = { w*(x - 0.5f), h2, h*(z + 0.5f) };
            Vector3 v3quarterinverted = { w*(x + 0.5f), h2, h*(z + 0.5f) };
            Vector3 v4quarterinverted = { w*(x + 0.5f), h2, h*(z - 0.5f) };
            Vector3 v5quarterinverted = { w*(x + 0.5f), h2/4, h*(z - 0.5f) };
            Vector3 v6quarterinverted = { w*(x - 0.5f), h2/4, h*(z - 0.5f) };
            Vector3 v7quarterinverted = { w*(x - 0.5f), h2/4, h*(z + 0.5f) };
            Vector3 v8quarterinverted = { w*(x + 0.5f), h2/4, h*(z + 0.5f) };
            Vector3 v1ceilingquarter = { w*(x - 0.5f), 0.75f, h*(z - 0.5f) };
            Vector3 v2ceilingquarter = { w*(x - 0.5f), 0.75f, h*(z + 0.5f) };
            Vector3 v3ceilingquarter = { w*(x + 0.5f), 0.75f, h*(z + 0.5f) };
            Vector3 v4ceilingquarter = { w*(x + 0.5f), 0.75f, h*(z - 0.5f) };
            Vector3 v5ceilingquarter = { w*(x + 0.5f), 0.75f, h*(z - 0.5f) };
            Vector3 v6ceilingquarter = { w*(x - 0.5f), 0.75f, h*(z - 0.5f) };
            Vector3 v7ceilingquarter = { w*(x - 0.5f), 0.75f, h*(z + 0.5f) };
            Vector3 v8ceilingquarter = { w*(x + 0.5f), 0.75f, h*(z + 0.5f) };

            // We check pixel color to be WHITE -> draw full cube

            if (COLOR_EQUAL(pixels[z*cubicmap.width + x], WHITE))
            {
                // Define triangles and checking collateral cubes
                //------------------------------------------------

                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                // WARNING: Not required for a WHITE cubes, created to allow seeing the map from outside
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v2;
                mapVertices[vCounter + 2] = v3;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v3;
                mapVertices[vCounter + 5] = v4;
                vCounter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v8;
                mapVertices[vCounter + 2] = v7;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v5;
                mapVertices[vCounter + 5] = v8;
                vCounter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                tcCounter += 6;
            
                // Checking cube on bottom of current cube
                //if (((z < cubicmap.height - 1) && COLOR_EQUAL(pixels[(z + 1)*cubicmap.width + x], BLACK)) || (z == cubicmap.height - 1))
                //{
                    // Define front triangles (2 tris, 6 vertex) --> v2 v7 v3, v3 v7 v8
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v2;
                    mapVertices[vCounter + 1] = v7;
                    mapVertices[vCounter + 2] = v3;
                    mapVertices[vCounter + 3] = v3;
                    mapVertices[vCounter + 4] = v7;
                    mapVertices[vCounter + 5] = v8;
                    vCounter += 6;

                    mapNormals[nCounter] = n6;
                    mapNormals[nCounter + 1] = n6;
                    mapNormals[nCounter + 2] = n6;
                    mapNormals[nCounter + 3] = n6;
                    mapNormals[nCounter + 4] = n6;
                    mapNormals[nCounter + 5] = n6;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ frontTexUV.x, frontTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y + frontTexUV.height };
                    tcCounter += 6;
                //}
                // Checking cube on top of current cube
                //if (((z > 0) && COLOR_EQUAL(pixels[(z - 1)*cubicmap.width + x], BLACK)) || (z == 0))
                //{
                    // Define back triangles (2 tris, 6 vertex) --> v1 v5 v6, v1 v4 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1;
                    mapVertices[vCounter + 1] = v5;
                    mapVertices[vCounter + 2] = v6;
                    mapVertices[vCounter + 3] = v1;
                    mapVertices[vCounter + 4] = v4;
                    mapVertices[vCounter + 5] = v5;
                    vCounter += 6;

                    mapNormals[nCounter] = n5;
                    mapNormals[nCounter + 1] = n5;
                    mapNormals[nCounter + 2] = n5;
                    mapNormals[nCounter + 3] = n5;
                    mapNormals[nCounter + 4] = n5;
                    mapNormals[nCounter + 5] = n5;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 3] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ backTexUV.x, backTexUV.y };
                    mapTexcoords[tcCounter + 5] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    tcCounter += 6;
                //}

                // Checking cube on right of current cube
                //if (((x < cubicmap.width - 1) && COLOR_EQUAL(pixels[z*cubicmap.width + (x + 1)], BLACK)) || (x == cubicmap.width - 1))
                //{
                    // Define right triangles (2 tris, 6 vertex) --> v3 v8 v4, v4 v8 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v3;
                    mapVertices[vCounter + 1] = v8;
                    mapVertices[vCounter + 2] = v4;
                    mapVertices[vCounter + 3] = v4;
                    mapVertices[vCounter + 4] = v8;
                    mapVertices[vCounter + 5] = v5;
                    vCounter += 6;

                    mapNormals[nCounter] = n1;
                    mapNormals[nCounter + 1] = n1;
                    mapNormals[nCounter + 2] = n1;
                    mapNormals[nCounter + 3] = n1;
                    mapNormals[nCounter + 4] = n1;
                    mapNormals[nCounter + 5] = n1;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ rightTexUV.x, rightTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y + rightTexUV.height };
                    tcCounter += 6;
                //}

                // Checking cube on left of current cube
                //if (((x > 0) && COLOR_EQUAL(pixels[z*cubicmap.width + (x - 1)], BLACK)) || (x == 0))
                //{
                    // Define left triangles (2 tris, 6 vertex) --> v1 v7 v2, v1 v6 v7
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1;
                    mapVertices[vCounter + 1] = v7;
                    mapVertices[vCounter + 2] = v2;
                    mapVertices[vCounter + 3] = v1;
                    mapVertices[vCounter + 4] = v6;
                    mapVertices[vCounter + 5] = v7;
                    vCounter += 6;

                    mapNormals[nCounter] = n2;
                    mapNormals[nCounter + 1] = n2;
                    mapNormals[nCounter + 2] = n2;
                    mapNormals[nCounter + 3] = n2;
                    mapNormals[nCounter + 4] = n2;
                    mapNormals[nCounter + 5] = n2;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ leftTexUV.x, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    tcCounter += 6;
                //}
            }
            
            if (COLOR_EQUAL(pixels[z*cubicmap.width + x], GRAY))
            {
                // Define triangles and checking collateral cubes
                //------------------------------------------------

                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                // WARNING: Not required for a WHITE cubes, created to allow seeing the map from outside
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v2;
                mapVertices[vCounter + 2] = v3;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v3;
                mapVertices[vCounter + 5] = v4;
                vCounter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v8;
                mapVertices[vCounter + 2] = v7;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v5;
                mapVertices[vCounter + 5] = v8;
                vCounter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                tcCounter += 6;
            
                // Checking cube on bottom of current cube
                //if (((z < cubicmap.height - 1) && COLOR_EQUAL(pixels[(z + 1)*cubicmap.width + x], BLACK)) || (z == cubicmap.height - 1))
                //{
                    // Define front triangles (2 tris, 6 vertex) --> v2 v7 v3, v3 v7 v8
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v2half;
                    mapVertices[vCounter + 1] = v7half;
                    mapVertices[vCounter + 2] = v3half;
                    mapVertices[vCounter + 3] = v3half;
                    mapVertices[vCounter + 4] = v7half;
                    mapVertices[vCounter + 5] = v8half;
                    vCounter += 6;

                    mapNormals[nCounter] = n6;
                    mapNormals[nCounter + 1] = n6half;
                    mapNormals[nCounter + 2] = n6half;
                    mapNormals[nCounter + 3] = n6half;
                    mapNormals[nCounter + 4] = n6half;
                    mapNormals[nCounter + 5] = n6half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ frontTexUV.x, frontTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y + frontTexUV.height };
                    tcCounter += 6;
                //}

                // Checking cube on top of current cube
                //if (((z > 0) && COLOR_EQUAL(pixels[(z - 1)*cubicmap.width + x], BLACK)) || (z == 0))
                //{
                    // Define back triangles (2 tris, 6 vertex) --> v1 v5 v6, v1 v4 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1half;
                    mapVertices[vCounter + 1] = v5half;
                    mapVertices[vCounter + 2] = v6half;
                    mapVertices[vCounter + 3] = v1half;
                    mapVertices[vCounter + 4] = v4half;
                    mapVertices[vCounter + 5] = v5half;
                    vCounter += 6;

                    mapNormals[nCounter] = n5half;
                    mapNormals[nCounter + 1] = n5half;
                    mapNormals[nCounter + 2] = n5half;
                    mapNormals[nCounter + 3] = n5half;
                    mapNormals[nCounter + 4] = n5half;
                    mapNormals[nCounter + 5] = n5half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 3] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ backTexUV.x, backTexUV.y };
                    mapTexcoords[tcCounter + 5] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    tcCounter += 6;
                //}

                // Checking cube on right of current cube
                //if (((x < cubicmap.width - 1) && COLOR_EQUAL(pixels[z*cubicmap.width + (x + 1)], BLACK)) || (x == cubicmap.width - 1))
                //{
                    // Define right triangles (2 tris, 6 vertex) --> v3 v8 v4, v4 v8 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v3half;
                    mapVertices[vCounter + 1] = v8half;
                    mapVertices[vCounter + 2] = v4half;
                    mapVertices[vCounter + 3] = v4half;
                    mapVertices[vCounter + 4] = v8half;
                    mapVertices[vCounter + 5] = v5half;
                    vCounter += 6;

                    mapNormals[nCounter] = n1half;
                    mapNormals[nCounter + 1] = n1half;
                    mapNormals[nCounter + 2] = n1half;
                    mapNormals[nCounter + 3] = n1half;
                    mapNormals[nCounter + 4] = n1half;
                    mapNormals[nCounter + 5] = n1half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ rightTexUV.x, rightTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y + rightTexUV.height };
                    tcCounter += 6;
                //}

                // Checking cube on left of current cube
                //if (((x > 0) && COLOR_EQUAL(pixels[z*cubicmap.width + (x - 1)], BLACK)) || (x == 0))
                //{
                    // Define left triangles (2 tris, 6 vertex) --> v1 v7 v2, v1 v6 v7
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1half;
                    mapVertices[vCounter + 1] = v7half;
                    mapVertices[vCounter + 2] = v2half;
                    mapVertices[vCounter + 3] = v1half;
                    mapVertices[vCounter + 4] = v6half;
                    mapVertices[vCounter + 5] = v7half;
                    vCounter += 6;

                    mapNormals[nCounter] = n2half;
                    mapNormals[nCounter + 1] = n2half;
                    mapNormals[nCounter + 2] = n2half;
                    mapNormals[nCounter + 3] = n2half;
                    mapNormals[nCounter + 4] = n2half;
                    mapNormals[nCounter + 5] = n2half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ leftTexUV.x, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    tcCounter += 6;
                //}

            // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)            
            mapVertices[vCounter] = v1;
            mapVertices[vCounter + 1] = v3;
            mapVertices[vCounter + 2] = v2;
            mapVertices[vCounter + 3] = v1;
            mapVertices[vCounter + 4] = v4;
            mapVertices[vCounter + 5] = v3;
            vCounter += 6;

            mapNormals[nCounter] = n4;
            mapNormals[nCounter + 1] = n4;
            mapNormals[nCounter + 2] = n4;
            mapNormals[nCounter + 3] = n4;
            mapNormals[nCounter + 4] = n4;
            mapNormals[nCounter + 5] = n4;
            nCounter += 6;

            mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
            mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
            mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
            mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
            mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
            mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
            tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
            mapVertices[vCounter] = v6floorhalf;
            mapVertices[vCounter + 1] = v7floorhalf;
            mapVertices[vCounter + 2] = v8floorhalf;
            mapVertices[vCounter + 3] = v6floorhalf;
            mapVertices[vCounter + 4] = v8floorhalf;
            mapVertices[vCounter + 5] = v5floorhalf;
            vCounter += 6;

            mapNormals[nCounter] = n3half;
            mapNormals[nCounter + 1] = n3half;
            mapNormals[nCounter + 2] = n3half;
            mapNormals[nCounter + 3] = n3half;
            mapNormals[nCounter + 4] = n3half;
            mapNormals[nCounter + 5] = n3half;
            nCounter += 6;

            mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
            mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
            mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
            tcCounter += 6;
                
            }
            
            if (COLOR_EQUAL(pixels[z*cubicmap.width + x], DARKGRAY))
            {
                // Define triangles and checking collateral cubes
                //------------------------------------------------

                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                // WARNING: Not required for a WHITE cubes, created to allow seeing the map from outside
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v2;
                mapVertices[vCounter + 2] = v3;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v3;
                mapVertices[vCounter + 5] = v4;
                vCounter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v8;
                mapVertices[vCounter + 2] = v7;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v5;
                mapVertices[vCounter + 5] = v8;
                vCounter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                tcCounter += 6;
            
                // Checking cube on bottom of current cube
                //if (((z < cubicmap.height - 1) && COLOR_EQUAL(pixels[(z + 1)*cubicmap.width + x], BLACK)) || (z == cubicmap.height - 1))
                //{
                    // Define front triangles (2 tris, 6 vertex) --> v2 v7 v3, v3 v7 v8
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v2quarter;
                    mapVertices[vCounter + 1] = v7quarter;
                    mapVertices[vCounter + 2] = v3quarter;
                    mapVertices[vCounter + 3] = v3quarter;
                    mapVertices[vCounter + 4] = v7quarter;
                    mapVertices[vCounter + 5] = v8quarter;
                    vCounter += 6;

                    mapNormals[nCounter] = n6;
                    mapNormals[nCounter + 1] = n6half;
                    mapNormals[nCounter + 2] = n6half;
                    mapNormals[nCounter + 3] = n6half;
                    mapNormals[nCounter + 4] = n6half;
                    mapNormals[nCounter + 5] = n6half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ frontTexUV.x, frontTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y + frontTexUV.height };
                    tcCounter += 6;
                //}
              
                // Checking cube on top of current cube
                    // Define back triangles (2 tris, 6 vertex) --> v1 v5 v6, v1 v4 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1quarter;
                    mapVertices[vCounter + 1] = v5quarter;
                    mapVertices[vCounter + 2] = v6quarter;
                    mapVertices[vCounter + 3] = v1quarter;
                    mapVertices[vCounter + 4] = v4quarter;
                    mapVertices[vCounter + 5] = v5quarter;
                    vCounter += 6;

                    mapNormals[nCounter] = n5half;
                    mapNormals[nCounter + 1] = n5half;
                    mapNormals[nCounter + 2] = n5half;
                    mapNormals[nCounter + 3] = n5half;
                    mapNormals[nCounter + 4] = n5half;
                    mapNormals[nCounter + 5] = n5half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 3] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ backTexUV.x, backTexUV.y };
                    mapTexcoords[tcCounter + 5] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    tcCounter += 6;

                // Checking cube on right of current cube

                    // Define right triangles (2 tris, 6 vertex) --> v3 v8 v4, v4 v8 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v3quarter;
                    mapVertices[vCounter + 1] = v8quarter;
                    mapVertices[vCounter + 2] = v4quarter;
                    mapVertices[vCounter + 3] = v4quarter;
                    mapVertices[vCounter + 4] = v8quarter;
                    mapVertices[vCounter + 5] = v5quarter;
                    vCounter += 6;

                    mapNormals[nCounter] = n1half;
                    mapNormals[nCounter + 1] = n1half;
                    mapNormals[nCounter + 2] = n1half;
                    mapNormals[nCounter + 3] = n1half;
                    mapNormals[nCounter + 4] = n1half;
                    mapNormals[nCounter + 5] = n1half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ rightTexUV.x, rightTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y + rightTexUV.height };
                    tcCounter += 6;
               

                // Checking cube on left of current cube
                //if (((x > 0) && COLOR_EQUAL(pixels[z*cubicmap.width + (x - 1)], BLACK)) || (x == 0))
                //{
                    // Define left triangles (2 tris, 6 vertex) --> v1 v7 v2, v1 v6 v7
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1quarter;
                    mapVertices[vCounter + 1] = v7quarter;
                    mapVertices[vCounter + 2] = v2quarter;
                    mapVertices[vCounter + 3] = v1quarter;
                    mapVertices[vCounter + 4] = v6quarter;
                    mapVertices[vCounter + 5] = v7quarter;
                    vCounter += 6;

                    mapNormals[nCounter] = n2half;
                    mapNormals[nCounter + 1] = n2half;
                    mapNormals[nCounter + 2] = n2half;
                    mapNormals[nCounter + 3] = n2half;
                    mapNormals[nCounter + 4] = n2half;
                    mapNormals[nCounter + 5] = n2half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ leftTexUV.x, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    tcCounter += 6;
                //}
             

            // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)            
            mapVertices[vCounter] = v1;
            mapVertices[vCounter + 1] = v3;
            mapVertices[vCounter + 2] = v2;
            mapVertices[vCounter + 3] = v1;
            mapVertices[vCounter + 4] = v4;
            mapVertices[vCounter + 5] = v3;
            vCounter += 6;

            mapNormals[nCounter] = n4;
            mapNormals[nCounter + 1] = n4;
            mapNormals[nCounter + 2] = n4;
            mapNormals[nCounter + 3] = n4;
            mapNormals[nCounter + 4] = n4;
            mapNormals[nCounter + 5] = n4;
            nCounter += 6;

            mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
            mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
            mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
            mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
            mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
            mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
            tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
            mapVertices[vCounter] = v6floorquarter;
            mapVertices[vCounter + 1] = v7floorquarter;
            mapVertices[vCounter + 2] = v8floorquarter;
            mapVertices[vCounter + 3] = v6floorquarter;
            mapVertices[vCounter + 4] = v8floorquarter;
            mapVertices[vCounter + 5] = v5floorquarter;
            vCounter += 6;

            mapNormals[nCounter] = n3half;
            mapNormals[nCounter + 1] = n3half;
            mapNormals[nCounter + 2] = n3half;
            mapNormals[nCounter + 3] = n3half;
            mapNormals[nCounter + 4] = n3half;
            mapNormals[nCounter + 5] = n3half;
            nCounter += 6;

            mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
            mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
            mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
            tcCounter += 6;
                
            }

            if (COLOR_EQUAL(pixels[z*cubicmap.width + x], GRAY_131))
            {
                // Define triangles and checking collateral cubes
                //------------------------------------------------

                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                // WARNING: Not required for a WHITE cubes, created to allow seeing the map from outside
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v2;
                mapVertices[vCounter + 2] = v3;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v3;
                mapVertices[vCounter + 5] = v4;
                vCounter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v8;
                mapVertices[vCounter + 2] = v7;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v5;
                mapVertices[vCounter + 5] = v8;
                vCounter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                tcCounter += 6;
            
                // Checking cube on bottom of current cube
                //if (((z < cubicmap.height - 1) && COLOR_EQUAL(pixels[(z + 1)*cubicmap.width + x], BLACK)) || (z == cubicmap.height - 1))
                //{
                    // Define front triangles (2 tris, 6 vertex) --> v2 v7 v3, v3 v7 v8
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v2halfinverted;
                    mapVertices[vCounter + 1] = v7halfinverted;
                    mapVertices[vCounter + 2] = v3halfinverted;
                    mapVertices[vCounter + 3] = v3halfinverted;
                    mapVertices[vCounter + 4] = v7halfinverted;
                    mapVertices[vCounter + 5] = v8halfinverted;
                    vCounter += 6;

                    mapNormals[nCounter] = n6;
                    mapNormals[nCounter + 1] = n6half;
                    mapNormals[nCounter + 2] = n6half;
                    mapNormals[nCounter + 3] = n6half;
                    mapNormals[nCounter + 4] = n6half;
                    mapNormals[nCounter + 5] = n6half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ frontTexUV.x, frontTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y + frontTexUV.height };
                    tcCounter += 6;
                //}

                // Checking cube on top of current cube
                //if (((z > 0) && COLOR_EQUAL(pixels[(z - 1)*cubicmap.width + x], BLACK)) || (z == 0))
                //{
                    // Define back triangles (2 tris, 6 vertex) --> v1 v5 v6, v1 v4 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1halfinverted;
                    mapVertices[vCounter + 1] = v5halfinverted;
                    mapVertices[vCounter + 2] = v6halfinverted;
                    mapVertices[vCounter + 3] = v1halfinverted;
                    mapVertices[vCounter + 4] = v4halfinverted;
                    mapVertices[vCounter + 5] = v5halfinverted;
                    vCounter += 6;

                    mapNormals[nCounter] = n5half;
                    mapNormals[nCounter + 1] = n5half;
                    mapNormals[nCounter + 2] = n5half;
                    mapNormals[nCounter + 3] = n5half;
                    mapNormals[nCounter + 4] = n5half;
                    mapNormals[nCounter + 5] = n5half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 3] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ backTexUV.x, backTexUV.y };
                    mapTexcoords[tcCounter + 5] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    tcCounter += 6;
                //}

                // Checking cube on right of current cube
                //if (((x < cubicmap.width - 1) && COLOR_EQUAL(pixels[z*cubicmap.width + (x + 1)], BLACK)) || (x == cubicmap.width - 1))
                //{
                    // Define right triangles (2 tris, 6 vertex) --> v3 v8 v4, v4 v8 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v3halfinverted;
                    mapVertices[vCounter + 1] = v8halfinverted;
                    mapVertices[vCounter + 2] = v4halfinverted;
                    mapVertices[vCounter + 3] = v4halfinverted;
                    mapVertices[vCounter + 4] = v8halfinverted;
                    mapVertices[vCounter + 5] = v5halfinverted;
                    vCounter += 6;

                    mapNormals[nCounter] = n1half;
                    mapNormals[nCounter + 1] = n1half;
                    mapNormals[nCounter + 2] = n1half;
                    mapNormals[nCounter + 3] = n1half;
                    mapNormals[nCounter + 4] = n1half;
                    mapNormals[nCounter + 5] = n1half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ rightTexUV.x, rightTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y + rightTexUV.height };
                    tcCounter += 6;
                //}

                // Checking cube on left of current cube
                //if (((x > 0) && COLOR_EQUAL(pixels[z*cubicmap.width + (x - 1)], BLACK)) || (x == 0))
                //{
                    // Define left triangles (2 tris, 6 vertex) --> v1 v7 v2, v1 v6 v7
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1halfinverted;
                    mapVertices[vCounter + 1] = v7halfinverted;
                    mapVertices[vCounter + 2] = v2halfinverted;
                    mapVertices[vCounter + 3] = v1halfinverted;
                    mapVertices[vCounter + 4] = v6halfinverted;
                    mapVertices[vCounter + 5] = v7halfinverted;
                    vCounter += 6;

                    mapNormals[nCounter] = n2half;
                    mapNormals[nCounter + 1] = n2half;
                    mapNormals[nCounter + 2] = n2half;
                    mapNormals[nCounter + 3] = n2half;
                    mapNormals[nCounter + 4] = n2half;
                    mapNormals[nCounter + 5] = n2half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ leftTexUV.x, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    tcCounter += 6;
                //}

            // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)            
            mapVertices[vCounter] = v1ceilinghalf;
            mapVertices[vCounter + 1] = v3ceilinghalf;
            mapVertices[vCounter + 2] = v2ceilinghalf;
            mapVertices[vCounter + 3] = v1ceilinghalf;
            mapVertices[vCounter + 4] = v4ceilinghalf;
            mapVertices[vCounter + 5] = v3ceilinghalf;
            vCounter += 6;

            mapNormals[nCounter] = n4;
            mapNormals[nCounter + 1] = n4;
            mapNormals[nCounter + 2] = n4;
            mapNormals[nCounter + 3] = n4;
            mapNormals[nCounter + 4] = n4;
            mapNormals[nCounter + 5] = n4;
            nCounter += 6;

            mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
            mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
            mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
            mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
            mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
            mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
            tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
            mapVertices[vCounter] = v6;
            mapVertices[vCounter + 1] = v7;
            mapVertices[vCounter + 2] = v8;
            mapVertices[vCounter + 3] = v6;
            mapVertices[vCounter + 4] = v8;
            mapVertices[vCounter + 5] = v5;
            vCounter += 6;

            mapNormals[nCounter] = n3half;
            mapNormals[nCounter + 1] = n3half;
            mapNormals[nCounter + 2] = n3half;
            mapNormals[nCounter + 3] = n3half;
            mapNormals[nCounter + 4] = n3half;
            mapNormals[nCounter + 5] = n3half;
            nCounter += 6;

            mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
            mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
            mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
            tcCounter += 6;
                
            }
            
            if (COLOR_EQUAL(pixels[z*cubicmap.width + x], DARKGRAY_81))
            {
                // Define triangles and checking collateral cubes
                //------------------------------------------------

                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                // WARNING: Not required for a WHITE cubes, created to allow seeing the map from outside
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v2;
                mapVertices[vCounter + 2] = v3;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v3;
                mapVertices[vCounter + 5] = v4;
                vCounter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v8;
                mapVertices[vCounter + 2] = v7;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v5;
                mapVertices[vCounter + 5] = v8;
                vCounter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                tcCounter += 6;
            
                // Checking cube on bottom of current cube
                //if (((z < cubicmap.height - 1) && COLOR_EQUAL(pixels[(z + 1)*cubicmap.width + x], BLACK)) || (z == cubicmap.height - 1))
                //{
                    // Define front triangles (2 tris, 6 vertex) --> v2 v7 v3, v3 v7 v8
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v2quarter;
                    mapVertices[vCounter + 1] = v7quarter;
                    mapVertices[vCounter + 2] = v3quarter;
                    mapVertices[vCounter + 3] = v3quarter;
                    mapVertices[vCounter + 4] = v7quarter;
                    mapVertices[vCounter + 5] = v8quarter;
                    vCounter += 6;

                    mapNormals[nCounter] = n6;
                    mapNormals[nCounter + 1] = n6half;
                    mapNormals[nCounter + 2] = n6half;
                    mapNormals[nCounter + 3] = n6half;
                    mapNormals[nCounter + 4] = n6half;
                    mapNormals[nCounter + 5] = n6half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ frontTexUV.x, frontTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y + frontTexUV.height };
                    tcCounter += 6;
                //}
              
                // Checking cube on top of current cube
                    // Define back triangles (2 tris, 6 vertex) --> v1 v5 v6, v1 v4 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1quarter;
                    mapVertices[vCounter + 1] = v5quarter;
                    mapVertices[vCounter + 2] = v6quarter;
                    mapVertices[vCounter + 3] = v1quarter;
                    mapVertices[vCounter + 4] = v4quarter;
                    mapVertices[vCounter + 5] = v5quarter;
                    vCounter += 6;

                    mapNormals[nCounter] = n5half;
                    mapNormals[nCounter + 1] = n5half;
                    mapNormals[nCounter + 2] = n5half;
                    mapNormals[nCounter + 3] = n5half;
                    mapNormals[nCounter + 4] = n5half;
                    mapNormals[nCounter + 5] = n5half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 3] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ backTexUV.x, backTexUV.y };
                    mapTexcoords[tcCounter + 5] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    tcCounter += 6;

                // Checking cube on right of current cube

                    // Define right triangles (2 tris, 6 vertex) --> v3 v8 v4, v4 v8 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v3quarterinverted;
                    mapVertices[vCounter + 1] = v8quarterinverted;
                    mapVertices[vCounter + 2] = v4quarterinverted;
                    mapVertices[vCounter + 3] = v4quarterinverted;
                    mapVertices[vCounter + 4] = v8quarterinverted;
                    mapVertices[vCounter + 5] = v5quarterinverted;
                    vCounter += 6;

                    mapNormals[nCounter] = n1half;
                    mapNormals[nCounter + 1] = n1half;
                    mapNormals[nCounter + 2] = n1half;
                    mapNormals[nCounter + 3] = n1half;
                    mapNormals[nCounter + 4] = n1half;
                    mapNormals[nCounter + 5] = n1half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ rightTexUV.x, rightTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y + rightTexUV.height };
                    tcCounter += 6;
               

                // Checking cube on left of current cube
                //if (((x > 0) && COLOR_EQUAL(pixels[z*cubicmap.width + (x - 1)], BLACK)) || (x == 0))
                //{
                    // Define left triangles (2 tris, 6 vertex) --> v1 v7 v2, v1 v6 v7
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1quarterinverted;
                    mapVertices[vCounter + 1] = v7quarterinverted;
                    mapVertices[vCounter + 2] = v2quarterinverted;
                    mapVertices[vCounter + 3] = v1quarterinverted;
                    mapVertices[vCounter + 4] = v6quarterinverted;
                    mapVertices[vCounter + 5] = v7quarterinverted;
                    vCounter += 6;

                    mapNormals[nCounter] = n2half;
                    mapNormals[nCounter + 1] = n2half;
                    mapNormals[nCounter + 2] = n2half;
                    mapNormals[nCounter + 3] = n2half;
                    mapNormals[nCounter + 4] = n2half;
                    mapNormals[nCounter + 5] = n2half;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ leftTexUV.x, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    tcCounter += 6;
                //}
             

            // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)            
            mapVertices[vCounter] = v1ceilingquarter;
            mapVertices[vCounter + 1] = v3ceilingquarter;
            mapVertices[vCounter + 2] = v2ceilingquarter;
            mapVertices[vCounter + 3] = v1ceilingquarter;
            mapVertices[vCounter + 4] = v4ceilingquarter;
            mapVertices[vCounter + 5] = v3ceilingquarter;
            vCounter += 6;

            mapNormals[nCounter] = n4;
            mapNormals[nCounter + 1] = n4;
            mapNormals[nCounter + 2] = n4;
            mapNormals[nCounter + 3] = n4;
            mapNormals[nCounter + 4] = n4;
            mapNormals[nCounter + 5] = n4;
            nCounter += 6;

            mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
            mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
            mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
            mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
            mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
            mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
            tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
            mapVertices[vCounter] = v6ceilingquarter;
            mapVertices[vCounter + 1] = v7ceilingquarter;
            mapVertices[vCounter + 2] = v8ceilingquarter;
            mapVertices[vCounter + 3] = v6ceilingquarter;
            mapVertices[vCounter + 4] = v8ceilingquarter;
            mapVertices[vCounter + 5] = v5ceilingquarter;
            vCounter += 6;

            mapNormals[nCounter] = n3half;
            mapNormals[nCounter + 1] = n3half;
            mapNormals[nCounter + 2] = n3half;
            mapNormals[nCounter + 3] = n3half;
            mapNormals[nCounter + 4] = n3half;
            mapNormals[nCounter + 5] = n3half;
            nCounter += 6;

            mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
            mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
            mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
            mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
            tcCounter += 6;
                
            }
            

            // We check pixel color to be BLACK, we will only draw floor and roof
            if (COLOR_EQUAL(pixels[z*cubicmap.width + x], BLACK))
            {
                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v3;
                mapVertices[vCounter + 2] = v2;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v4;
                mapVertices[vCounter + 5] = v3;
                vCounter += 6;

                mapNormals[nCounter] = n4half;
                mapNormals[nCounter + 1] = n4half;
                mapNormals[nCounter + 2] = n4half;
                mapNormals[nCounter + 3] = n4half;
                mapNormals[nCounter + 4] = n4half;
                mapNormals[nCounter + 5] = n4half;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v7;
                mapVertices[vCounter + 2] = v8;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v8;
                mapVertices[vCounter + 5] = v5;
                vCounter += 6;

                mapNormals[nCounter] = n3half;
                mapNormals[nCounter + 1] = n3half;
                mapNormals[nCounter + 2] = n3half;
                mapNormals[nCounter + 3] = n3half;
                mapNormals[nCounter + 4] = n3half;
                mapNormals[nCounter + 5] = n3half;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                tcCounter += 6;
            }
        }
    }

    // Move data from mapVertices temp arrays to vertices float array
    mesh.vertexCount = vCounter;
    mesh.triangleCount = vCounter/3;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.colors = NULL;

    int fCounter = 0;

    // Move vertices data
    for (int i = 0; i < vCounter; i++)
    {
        mesh.vertices[fCounter] = mapVertices[i].x;
        mesh.vertices[fCounter + 1] = mapVertices[i].y;
        mesh.vertices[fCounter + 2] = mapVertices[i].z;
        fCounter += 3;
    }

    fCounter = 0;

    // Move normals data
    for (int i = 0; i < nCounter; i++)
    {
        mesh.normals[fCounter] = mapNormals[i].x;
        mesh.normals[fCounter + 1] = mapNormals[i].y;
        mesh.normals[fCounter + 2] = mapNormals[i].z;
        fCounter += 3;
    }

    fCounter = 0;

    // Move texcoords data
    for (int i = 0; i < tcCounter; i++)
    {
        mesh.texcoords[fCounter] = mapTexcoords[i].x;
        mesh.texcoords[fCounter + 1] = mapTexcoords[i].y;
        fCounter += 2;
    }

    RL_FREE(mapVertices);
    RL_FREE(mapNormals);
    RL_FREE(mapTexcoords);

    UnloadImageColors(pixels);   // Unload pixels color data

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}
