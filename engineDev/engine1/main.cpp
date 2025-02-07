#include <iostream>
#include "engine.h"
#include <algorithm>

using namespace std;

struct vec3d
{
    float x, y, z;
};

struct triangle
{
    vec3d p[3];
    Pixel rgb;
};

struct mesh
{
    std::vector<triangle> tris;
};

struct mat4x4
{
    float m[4][4] = { 0 };
};

class rotatingCube : public Engine
{
private:

    mesh meshCube;
    mat4x4 matProj;
    vec3d vCamera;

    float fTheta;

    void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m)
    {
        o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
        o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
        o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
        float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

        if (w != 0.0f)
        {
            o.x /= w; o.y /= w; o.z /= w;
        }
    }

public:

    bool OnUserCreate() override
        {
            meshCube.tris = {

            // SOUTH
            { 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

            // EAST
            { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

            // NORTH
            { 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

            // WEST
            { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

            // TOP
            { 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

            // BOTTOM
            { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },

            };

            // Projection Matrix
            float fNear = 0.1f;
            float fFar = 1000.0f;
            float fFov = 90.0f;
            float fAspectRatio = (float)600 / (float)800;
            float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

            matProj.m[0][0] = fAspectRatio * fFovRad;
            matProj.m[1][1] = fFovRad;
            matProj.m[2][2] = fFar / (fFar - fNear);
            matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
            matProj.m[2][3] = 1.0f;
            matProj.m[3][3] = 0.0f;

            return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {

        // Clear Screen
        clear();

        // Set up rotation matrices
        mat4x4 matRotZ, matRotX;
        fTheta += 1.0f * fElapsedTime;

        // Rotation Z
        matRotZ.m[0][0] = cosf(fTheta);
        matRotZ.m[0][1] = sinf(fTheta);
        matRotZ.m[1][0] = -sinf(fTheta);
        matRotZ.m[1][1] = cosf(fTheta);
        matRotZ.m[2][2] = 1;
        matRotZ.m[3][3] = 1;

        // Rotation X
        matRotX.m[0][0] = 1;
        matRotX.m[1][1] = cosf(fTheta * 0.5f);
        matRotX.m[1][2] = sinf(fTheta * 0.5f);
        matRotX.m[2][1] = -sinf(fTheta * 0.5f);
        matRotX.m[2][2] = cosf(fTheta * 0.5f);
        matRotX.m[3][3] = 1;


        // Store triagles for rastering later
        vector<triangle> vecTrianglesToRaster;

        // Draw Triangles
        for (auto tri : meshCube.tris)
        {
            triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

            // Rotate in Z-Axis
            MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
            MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
            MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

            // Rotate in X-Axis
            MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
            MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
            MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

            // Offset into the screen
            triTranslated = triRotatedZX;
            triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
            triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
            triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

            // Use Cross-Product to get surface normal
            vec3d normal, line1, line2;
            line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
            line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
            line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

            line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
            line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
            line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

            normal.x = line1.y * line2.z - line1.z * line2.y;
            normal.y = line1.z * line2.x - line1.x * line2.z;
            normal.z = line1.x * line2.y - line1.y * line2.x;

            // It's normally normal to normalise the normal
            float l = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
            normal.x /= l; normal.y /= l; normal.z /= l;

            //if (normal.z < 0)
            if(normal.x * (triTranslated.p[0].x - vCamera.x) +
               normal.y * (triTranslated.p[0].y - vCamera.y) +
               normal.z * (triTranslated.p[0].z - vCamera.z) < 0.0f)
            {
                // Illumination
                vec3d light_direction = { 0.0f, 0.0f, -1.0f };
                float l = sqrtf(light_direction.x*light_direction.x + light_direction.y*light_direction.y + light_direction.z*light_direction.z);
                light_direction.x /= l; light_direction.y /= l; light_direction.z /= l;

                // How similar is normal to light direction
                float dp = max(0.1f,normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z);

                // Choose color
                triTranslated.rgb = Pixel(0,(int)(192.f*dp),(int)(192.f*dp));
                //triTranslated.rgb = Pixel((int)rand()%255,(int)(rand()%255*dp),(int)(rand()%255*dp));

                // Project triangles from 3D --> 2D
                MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
                MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
                MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);
                triProjected.rgb = triTranslated.rgb;

                // Scale into view
                triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
                triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
                triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;
                triProjected.p[0].x *= 0.5f * (float)nScreenWidth;
                triProjected.p[0].y *= 0.5f * (float)nScreenHeight;
                triProjected.p[1].x *= 0.5f * (float)nScreenWidth;
                triProjected.p[1].y *= 0.5f * (float)nScreenHeight;
                triProjected.p[2].x *= 0.5f * (float)nScreenWidth;
                triProjected.p[2].y *= 0.5f * (float)nScreenHeight;

                // Store triangle for sorting
                vecTrianglesToRaster.push_back(triProjected);
            }

        }

        // Sort triangles from back to front
        sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2)
        {
            float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
            float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
            return z1 > z2;
        });

        for (auto &triProjected : vecTrianglesToRaster)
        {
            // Rasterize triangle
            FillTriangle(triProjected.p[0].x, triProjected.p[0].y,
                triProjected.p[1].x, triProjected.p[1].y,
                triProjected.p[2].x, triProjected.p[2].y,
                triProjected.rgb);
            DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
                triProjected.p[1].x, triProjected.p[1].y,
                triProjected.p[2].x, triProjected.p[2].y,
                WHITE);
        }


        /*
        // Draw Triangles
        for (auto tri : meshCube.tris)
        {
            triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

            // Rotate in Z-Axis
            MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
            MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
            MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

            // Rotate in X-Axis
            MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
            MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
            MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

            // Offset into the screen
            triTranslated = triRotatedZX;
            triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
            triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
            triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

            // Project triangles from 3D --> 2D
            MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
            MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
            MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

            // Scale into view
            triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
            triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
            triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;
            triProjected.p[0].x *= 0.5f * nScreenWidth;
            triProjected.p[0].y *= 0.5f * nScreenHeight;
            triProjected.p[1].x *= 0.5f * nScreenWidth;
            triProjected.p[1].y *= 0.5f * nScreenHeight;
            triProjected.p[2].x *= 0.5f * nScreenWidth;
            triProjected.p[2].y *= 0.5f * nScreenHeight;

            // Rasterize triangle
            DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
                triProjected.p[1].x, triProjected.p[1].y,
                triProjected.p[2].x, triProjected.p[2].y,
                WHITE);

        }
*/
        // Update Title Bar
        fFrameTimer += fElapsedTime;
        nFrameCount++;
        if (fFrameTimer >= 1.0f)
        {
            fFrameTimer -= 1.0f;
            FPS = std::to_string(nFrameCount);
            nFrameCount = 0;
        }
        int k = 5;
        for(std::string::iterator it = FPS.begin(); it != FPS.end();  ++it)
        {
            DrawSprite(k, 5, new Sprite((int)*it-48),4);
            k += 20;
        }
        return true;
    }

private:
    float fFrameTimer = 1.0f;
    int nFrameCount = 0;
    std::string FPS = "0";
};


int main(int argc, const char* argv[])
{
    if(argc == 3){
        rotatingCube demo;
        std::cout << "w: " << atoi(argv[1]) << " h: " << atoi(argv[2]) << std::endl;
        demo.Construct(atoi(argv[1]),atoi(argv[2]));
        demo.Start();
    }
    return 0;
}
