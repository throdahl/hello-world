#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDl3/SDL_render.h>
#include <math.h>
#include "include/map.h"
#include "include/player.h"
using namespace std;

#define WINDOW_HEIGHT 512
#define WINDOW_WIDTH 1024
#define PI 3.14159265359
#define DR 0.0174533

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

float playerX, playerY, playerDeltaX, playerDeltaY, playerAngle;
int mapX = 8, mapY = 8, mapS = mapX * mapY;
vector<int> map =
{
    1,1,1,1,1,1,1,1,
    1,0,1,0,0,0,0,1,
    1,0,1,0,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,1,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,
};

float pythag(float x1, float y1, float x2, float y2, float angle){
    return ( sqrt((x2-x1) * (x2-x1) + (y2-y1) * (y2-y1)));
}
void drawRays()
{
    int r, mx, my, mp, dof;
    float rayX, rayY, rayAngle, xOffset, yOffset;
    rayAngle = playerAngle - 30 * DR;
    if(rayAngle < 0)
        rayAngle += 2*PI;
    if(rayAngle > 2*PI)
        rayAngle -= 2*PI;
    for(r = 0; r < 60; r++)
    {
        //--checking for horizontal lines--
        dof = 0;
        float aTan=-1/tan(rayAngle);
        float distanceH = INTMAX_MAX, horizontalX=playerX, horizontalY = playerY;
        float distanceT;
        //looking up
        if(rayAngle > PI) {
            rayY =(((int)playerY>>6)<<6) - 0.0001;
            rayX = (playerY-rayY)*aTan+playerX;
            yOffset= -64;
            xOffset= -yOffset*aTan;
        }
        //looking down
        if(rayAngle < PI) {
            rayY =(((int)playerY>>6)<<6) +64;
            rayX = (playerY-rayY)*aTan+playerX;
            yOffset= 64;
            xOffset= -yOffset*aTan;
        }
        //looking straight left or right
        if(rayAngle == 0 || rayAngle == PI){
            rayX = playerX;
            rayY = playerY;
            dof = 8;
        }
        while(dof < 8)
        {
            mx=(int) (rayX) >> 6;
            my=(int) (rayY) >> 6;
            mp=my*mapX+mx;
            if(mp > 0 && mp<mapX*mapY && map[mp] == 1) { 
                horizontalX = rayX; horizontalY = rayY;
                distanceH = pythag(playerX, playerY, rayX, rayY, playerAngle); 
                dof = 8;
            } //hit wall
            else{ rayX+=xOffset; rayY+=yOffset; dof+= 1;}
        }

        //--checking for vertical lines--
        dof = 0;
        float nTan= -tan(rayAngle);
        float distanceV = INTMAX_MAX, verticalX=playerX, verticalY = playerY;
        //looking left
        if(rayAngle > PI/2 && rayAngle < 3*PI/2) {
            rayX =(((int)playerX>>6)<<6) - 0.0001;
            rayY = (playerX-rayX)*nTan+playerY;
            xOffset= -64;
            yOffset= -xOffset*nTan;
        }
        //looking right
        if(rayAngle < PI/2 ||  rayAngle >3*PI/2) {
            rayX =(((int)playerX>>6)<<6) +64;
            rayY = (playerX-rayX)*nTan+playerY;
            xOffset= 64;
            yOffset= -xOffset*nTan;
        }
        //looking straight up or down
        if(rayAngle == 0 || rayAngle == PI){
            rayX = playerX;
            rayY = playerY;
            dof = 8;
        }
        while(dof < 8)
        {
            mx=(int) (rayX) >> 6;
            my=(int) (rayY) >> 6;
            mp=my*mapX+mx;
            if(mp > 0 && mp<mapX*mapY && map[mp] == 1) {
                verticalX = rayX; verticalY = rayY;
                distanceV = pythag(playerX, playerY, rayX, rayY, playerAngle);
                dof = 8;
            } //hit wall
            else{ rayX+=xOffset; rayY+=yOffset; dof+= 1;}
        }
        if(distanceH < distanceV) {rayX = horizontalX; rayY = horizontalY; distanceT = distanceH;}
        if(distanceH > distanceV) {rayX = verticalX; rayY = verticalY; distanceT = distanceV;}
        SDL_SetRenderDrawColorFloat(renderer, 0.0, 0.0, 1.0, 1);
        SDL_RenderLine(renderer, playerX, playerY, rayX, rayY);
        rayAngle += DR;
        if(rayAngle < 0)
            rayAngle += 2*PI;
        if(rayAngle > 2*PI)
            rayAngle -= 2*PI;
    }
}

void drawPlayer()
{
    //player is a 8x8 square
    SDL_FRect playerRect = {
        playerX,
        playerY,
        8,
        8
    };
    SDL_SetRenderDrawColorFloat(renderer, 1, 1, 0, 1);
    SDL_RenderFillRect(renderer, &playerRect);

    //Where the player is looking 
    SDL_SetRenderDrawColorFloat(renderer, 1, 1, 0, 1);
    SDL_RenderLine(renderer, playerX, playerY, playerX + playerDeltaX*5, playerY + playerDeltaY*5);
}
void drawMap()
{
    for(int y = 0; y < mapY; y++)
    {
        for(int x = 0; x < mapX; x++)
        {
            SDL_FRect tile = {
                x * 64.0f,
                y * 64.0f,
                64,
                64
            };
            if(map[y*mapX+x] == 1)
                SDL_SetRenderDrawColorFloat(renderer, 1.0f,1.0f,1.0f,1.0f);
            else
                SDL_SetRenderDrawColorFloat(renderer, 0.0f,0.0f,0.0f,0.0f);
            SDL_RenderFillRect(renderer, &tile);
            //outline
            SDL_SetRenderDrawColorFloat(renderer, 0.5f, 0.5f, 0.5f, 1.0f);
            SDL_FRect tileOutline = tile;
            SDL_RenderRect(renderer, &tileOutline);
        }
    }
}
SDL_AppResult SDL_AppIterate(void *appstate)
{
    
    SDL_SetRenderDrawColorFloat(renderer, 0.3, 0.3, 0.3, 0);
    SDL_RenderClear(renderer);
    drawMap();
    drawPlayer();
    drawRays();
    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }
    if (!SDL_CreateWindowAndRenderer("2.5D", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    playerX = 300;
    playerY = 300;
    playerDeltaX = cos(playerAngle) * 5;
    playerDeltaY = sin(playerAngle) * 5;

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if(event->type == SDL_EVENT_QUIT){
        return SDL_APP_SUCCESS;
    }
    if(event->key.scancode == SDL_SCANCODE_W) {
        playerX += playerDeltaX; 
        playerY += playerDeltaY;
     }
    if(event->key.scancode == SDL_SCANCODE_A) {
        playerAngle -= 0.1f;
        if(playerAngle < 0){
            playerAngle += 2*PI;
        }
        playerDeltaX = cos(playerAngle) * 5;
        playerDeltaY = sin(playerAngle) * 5;
    }
    
    if(event->key.scancode == SDL_SCANCODE_S) {
        playerX -= playerDeltaX; 
        playerY -= playerDeltaY;
     }
    
    if(event->key.scancode == SDL_SCANCODE_D) {
        playerAngle += 0.1f;
        if(playerAngle > 2*PI){
            playerAngle -= 2*PI;
        }
        playerDeltaX = cos(playerAngle) * 5;
        playerDeltaY = sin(playerAngle) * 5;
     }
        
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{

}