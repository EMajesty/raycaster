#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>

#include "textures/textures.ppm"
#include "textures/sky.ppm"
#include "textures/sprites.ppm"

float degToRad(float a)
{
    return a * M_PI / 180.0;
}

float fixAng(float a)
{
    if(a > 359){ a -= 360; }
    if(a < 0){ a += 360; }
    return a;
}

float calcDistance(float aX, float aY, float bX, float bY, float ang)
{
    return cos(degToRad(ang)) * (bX - aX) - sin(degToRad(ang)) * (bY - aY);
}

typedef struct
{
    int w,a,s,d; // button states
} ButtonKeys;

ButtonKeys Keys;

float playerX, playerY, playerDeltaX, playerDeltaY, playerAngle;
float frame1, frame2, fps;

//----------------Map----------------------------------------------------------
#define mapX 8 // map width
#define mapY 8 // map height
#define mapS 64 // map cube size

// use values 0-4

// walls
int mapW[] =
{
    1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,1,
    1,0,1,0,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,1,0,0,1,
    1,0,0,0,1,0,0,1,
    1,0,0,0,4,0,0,1,
    1,1,1,1,1,1,1,1,
}; 

// floors
int mapF[] =
{
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,  
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,  
};

// ceiling
int mapC[] =
{
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,2,2,2,0,0,0,0,
    0,2,2,2,2,0,0,0,
    0,0,0,0,0,0,0,0, 
};

//----------------Player-------------------------------------------------------
void drawPlayer2D()
{
    glColor3f(1,1,0);
    glPointSize(8);
    glLineWidth(4);
    glBegin(GL_POINTS);
    glVertex2i(playerX, playerY);
    glEnd();

    glBegin(GL_LINES);
    glVertex2i(playerX, playerY);
    glVertex2i(playerX + playerDeltaX * 20, playerY + playerDeltaY * 20);
    glEnd();
}
//-----------------------------------------------------------------------------

//----------------Draw rays and walls------------------------------------------
void drawRays2D()
{
    int ray, mx, my, mp, dof, side;
    float rx, ry, rayAngle, xo, yo, vx, vy, disV, disH;

    rayAngle = fixAng(playerAngle + 30);

    for(ray = 0; ray < 120; ray++)
    {
        int vmt = 0, hmt = 0;

        //---Check Vertical Lines---
        dof = 0;
        side = 0;
        disV = 100000;
        float rayTan = tan(degToRad(rayAngle));

        // looking left
        if(cos(degToRad(rayAngle)) > 0.001)
        {
            rx = (((int)playerX>>6)<<6)+64;
            ry = (playerX - rx) * rayTan + playerY;
            xo = 64;
            yo = -xo * rayTan;
        }

        // looking right
        else if(cos(degToRad(rayAngle)) < -0.001)
        {
            rx = (((int)playerX>>6)<<6) - 0.0001;
            ry = (playerX - rx) * rayTan + playerY;
            xo = -64;
            yo = -xo * rayTan;
        } else
        {
            rx = playerX;
            ry = playerY;
            dof = 8;
        }

        while(dof < 8)
        {
            mx = (int)(rx)>>6;
            my = (int)(ry)>>6;
            mp = my * mapX + mx;

            // hit wall
            if(mp > 0 && mp < mapX * mapY && mapW[mp] > 0)
            {
                vmt = mapW[mp] - 1;
                dof = 8;
                disV = cos(degToRad(rayAngle)) * (rx - playerX) - sin(degToRad(rayAngle)) * (ry - playerY);
            } else
            {
                // check next vertical line
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }

        vx = rx;
        vy = ry;

 
        //---Check Horizontal Lines---
        dof = 0;
        disH = 100000;
        rayTan = 1.0/rayTan;

        // looking up
        if(sin(degToRad(rayAngle)) > 0.001) 
        {
            ry = (((int)playerY>>6)<<6) - 0.0001;
            rx = (playerY - ry) * rayTan + playerX;
            yo = -64;
            xo = -yo * rayTan;
        }

        // looking down
        else if(sin(degToRad(rayAngle)) < -0.001) 
        {
            ry = (((int)playerY>>6)<<6)+64;
            rx = (playerY - ry) * rayTan + playerX;
            yo = 64;
            xo = -yo * rayTan;
        }

        // looking straight left or right
        else
        {
            rx = playerX;
            ry = playerY;
            dof = 8;
        }

        while(dof < 8)
        {
            mx = (int)(rx)>>6;
            my = (int)(ry)>>6;
            mp = my * mapX + mx;
            
            // hit wall
            if(mp > 0 && mp < mapX * mapY && mapW[mp] > 0)
            {
                hmt = mapW[mp] - 1;
                dof = 8;
                disH = cos(degToRad(rayAngle)) * (rx - playerX) - sin(degToRad(rayAngle)) * (ry - playerY);
            } else
            {
                // check next horizontal line
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }
       
        float shade = 1;
        glColor3f(0, 0.8, 0);

        // horizontal hit first
        if(disV < disH)
        {
            hmt = vmt;
            shade = 0.5;
            rx = vx;
            ry = vy;
            disH = disV;
            glColor3f(0, 0.6, 0);
        }

        //---Draw 3D Walls---

        // fix fisheye
        int ca = fixAng(playerAngle - rayAngle);
        disH = disH * cos(degToRad(ca));

        int lineH = (mapS * 640) / (disH);
        float ty_step = 32.0 / (float)lineH;
        float ty_off = 0;

        if(lineH > 640)
        {
            ty_off = (lineH - 640) / 2.0;
            lineH = 640;
        }
        int lineOff = 320- (lineH >> 1);

        int y;
        float ty = ty_off * ty_step; // + hmt * 32;
        float tx;

        if(shade == 1)
        {
            tx = (int)(rx / 2.0) % 32;
            if(rayAngle > 180)
            {
                tx = 31 - tx;
            }
        } else
        {
            tx = (int)(ry / 2.0) % 32;
            if(rayAngle > 90 && rayAngle < 270)
            {
                tx = 31 - tx;
            }
        }

        for(y = 0; y < lineH; y++)
        {
            int pixel = ((int)ty * 32 + (int)tx) * 3 + (hmt * 32 * 32 * 3);
            int red = textures[pixel + 0] * shade;
            int green = textures[pixel + 1] * shade;
            int blue = textures[pixel + 2] * shade;

            glPointSize(8);
            glColor3ub(red, green, blue);
            glBegin(GL_POINTS);
            glVertex2i(ray * 8, y + lineOff);
            glEnd();

            ty += ty_step;
        }

        // draw floors
        for(y = lineOff + lineH; y < 640; y++)
        {
            float dy = y - (640 / 2.0);
            float deg = degToRad(rayAngle);
            float raFix = cos(degToRad(fixAng(playerAngle - rayAngle)));

            tx = playerX / 2 + cos(deg) * 158 * 2 * 32 / dy / raFix;
            ty = playerY / 2 - sin(deg) * 158 * 2 * 32 / dy / raFix;

            int mp = mapF[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
            int pixel = (((int)(ty) & 31) * 32 + ((int)(tx) & 31)) * 3 + mp * 3;
            int red = textures[pixel + 0] * 0.7;
            int green = textures[pixel + 1] * 0.7;
            int blue = textures[pixel + 2] * 0.7;

            glPointSize(8);
            glColor3ub(red, green, blue);
            glBegin(GL_POINTS);
            glVertex2i(ray * 8, y);
            glEnd();

            // draw ceiling
            mp = mapC[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
            pixel = (((int)(ty) & 31) * 32 + ((int)(tx) & 31)) * 3 + mp * 3;
            red = textures[pixel + 0];
            green = textures[pixel + 1];
            blue = textures[pixel + 2];

            if(mp > 0)
            {
                glPointSize(8);
                glColor3ub(red, green, blue);
                glBegin(GL_POINTS);
                glVertex2i(ray * 8, 640 - y);
                glEnd();
            }
        }

        // go to next ray, 60 total
        rayAngle = fixAng(rayAngle - 0.5);
    }
}

void ButtonDown(unsigned char key, int x, int y)
{
    if(key=='a'){ Keys.a=1;}
    if(key=='d'){ Keys.d=1;}
    if(key=='w'){ Keys.w=1;}
    if(key=='s'){ Keys.s=1;}
    if(key=='e')
    {
        int xOffset = 0;
        if(playerDeltaX < 0){ xOffset = -25; } else { xOffset = 25; }
        
        int yOffset = 0;
        if(playerDeltaY < 0){ yOffset = -25; } else { yOffset = 25; }

        int ipx = playerX / 64.0;
        int ipx_add_xo = (playerX + xOffset) / 64.0;
        int ipy = playerY / 64.0;
        int ipy_add_yo = (playerY + yOffset) / 64.0;

        if(mapW[ipy_add_yo * mapX + ipx_add_xo] == 4)
        {
            mapW[ipy_add_yo * mapX + ipx_add_xo] = 0;
        }
    }
    glutPostRedisplay();
}

void ButtonUp(unsigned char key, int x, int y)
{
    if(key=='a'){ Keys.a=0;}
    if(key=='d'){ Keys.d=0;}
    if(key=='w'){ Keys.w=0;}
    if(key=='s'){ Keys.s=0;}
    glutPostRedisplay();
}

void resize(int w, int h)
{
    glutReshapeWindow(960, 640);
}

void drawSky()
{
    int x, y;
    for(y = 0; y < 40; y++)
    {
        for(x = 0; x < 120; x++)
        {
            int xOffset = (int)playerAngle * 2 - x;
            if(xOffset < 0){ xOffset += 120; }
            xOffset = xOffset % 120;

            int pixel = (y * 120 + xOffset) * 3;
            int red = sky[pixel + 0];
            int green = sky[pixel + 1];
            int blue = sky[pixel + 2];

            glPointSize(8);
            glColor3ub(red, green, blue);
            glBegin(GL_POINTS);
            glVertex2i(x * 8, y * 8);
            glEnd();
        }
    }
}

void display()
{
    // flames per second
    frame2 = glutGet(GLUT_ELAPSED_TIME);
    fps = (frame2 - frame1);
    frame1 = glutGet(GLUT_ELAPSED_TIME);
    
    // buttons
    if(Keys.a==1)
    {
        playerAngle += 0.2 * fps;
        playerAngle = fixAng(playerAngle);
        playerDeltaX = cos(degToRad(playerAngle));
        playerDeltaY = -sin(degToRad(playerAngle));
    }

    if(Keys.d==1)
    {
        playerAngle -= 0.2 * fps;
        playerAngle = fixAng(playerAngle);
        playerDeltaX = cos(degToRad(playerAngle));
        playerDeltaY = -sin(degToRad(playerAngle));
    }

    // x offset to check map
    int xOffset = 0;
    if(playerDeltaX < 0)
    {
        xOffset = -20;
    } else
    {
        xOffset = 20;
    }

    // y offset to check map
    int yOffset = 0;
    if(playerDeltaY < 0)
    {
        yOffset = -20;
    } else
    {
        yOffset = 20;
    }

    // x position and offset
    int iPosX = playerX / 64.0;
    int iPosXAddXOffset = (playerX + xOffset) / 64.0;
    int iPosXSubXOffset = (playerX - xOffset) / 64.0;

    // y position and offset
    int iPosY = playerY / 64.0;
    int iPosYAddYOffset = (playerY + yOffset) / 64.0;
    int iPosYSubYOffset = (playerY - yOffset) / 64.0;

    if(Keys.w==1)
    {
        if(mapW[iPosY * mapX + iPosXAddXOffset] == 0)
        {
            playerX += playerDeltaX * 0.2 * fps;
        }
        if(mapW[iPosYAddYOffset * mapX + iPosX] == 0)
        {
            playerY += playerDeltaY * 0.2 * fps;
        }
    }

    if(Keys.s==1)
    {
        if(mapW[iPosY * mapX + iPosXSubXOffset] == 0)
        {
            playerX -= playerDeltaX * 0.2 * fps;
        }
        if(mapW[iPosYSubYOffset * mapX + iPosX] == 0)
        {
            playerY -= playerDeltaY * 0.2 * fps;
        }
    }

    glutPostRedisplay();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSky();
    drawRays2D();
    glutSwapBuffers();
}

void init()
{
    glClearColor(0.3, 0.3, 0.3, 0);
    gluOrtho2D(0, 960, 640, 0);
    playerX = 150;
    playerY = 400;
    playerAngle = 90;
    playerDeltaX = cos(degToRad(playerAngle));
    playerDeltaY = sin(degToRad(playerAngle));
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(960, 640);
    glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH) / 2 - 960 / 2, glutGet(GLUT_SCREEN_HEIGHT) / 2 - 640 / 2);
    glutCreateWindow("");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(ButtonDown);
    glutKeyboardUpFunc(ButtonUp);
    glutMainLoop();
}
