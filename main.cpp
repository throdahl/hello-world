#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDl3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <vector>
#include <iostream>
using namespace std;

#define WINDOW_HEIGHT 512
#define WINDOW_WIDTH 1080
#define PI 3.14159265359
#define DR 0.0174533 /2

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture * wallTexture = NULL;
float playerX, playerY, playerDeltaX, playerDeltaY, playerAngle;
struct Movement{
    bool up = false, down = false, left = false, right = false;
};
Movement movement;
int mapX = 8, mapY = 8, mapS = mapX * mapY;
vector<int> map =
{
    1,1,1,1,1,1,1,1,
    1,0,1,0,0,0,0,1,
    1,0,1,0,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,1,0,1,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,
};

float pythag(float x1, float y1, float x2, float y2, float angle){
    return ( sqrt((x2-x1) * (x2-x1) + (y2-y1) * (y2-y1)));
}
void drawRays()
{
    SDL_FRect ceiling = {WINDOW_WIDTH/2, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT/2};
    SDL_SetRenderDrawColorFloat(renderer, 0.3, 0.6, 1, 1);
    SDL_RenderFillRect(renderer, &ceiling);
    SDL_FRect floor = {WINDOW_WIDTH/2, WINDOW_HEIGHT/2, WINDOW_WIDTH/2, WINDOW_HEIGHT/2};
    SDL_SetRenderDrawColorFloat(renderer, 0.3, 1, 0.6, 1);
    SDL_RenderFillRect(renderer, &floor);
    int r, mx, my, mp, dof;
    float rayX, rayY, rayAngle, xOffset, yOffset;
    rayAngle = playerAngle - 60 * DR;
    if(rayAngle < 0)
        rayAngle += 2*PI;
    if(rayAngle > 2*PI)
        rayAngle -= 2*PI;
    for(r = 0; r < 120; r++)
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
        int textureX = 0;
        if(distanceH < distanceV) {
            rayX = horizontalX; 
            rayY = horizontalY; 
            distanceT = distanceH;
            textureX = ((int)rayX % 64);
        }
        if(distanceH > distanceV) {
            rayX = verticalX; 
            rayY = verticalY; 
            distanceT = distanceV;
            textureX = ((int)rayY % 64);

        }
        SDL_SetRenderDrawColorFloat(renderer, 0.0, 0.0, 1.0, 1);
        SDL_RenderLine(renderer, playerX, playerY, rayX, rayY);
        //---2.5D raycast rendering---
        //fix fisheye
        float correctedDistance = distanceT * cos(rayAngle - playerAngle);
        
        //shader based on distance
        float shade = 1.0f - (correctedDistance / 750.0f);
        Uint8 color = (Uint8)(shade * 255.0f);
        SDL_SetTextureColorMod(wallTexture, color, color, color);
        //wall height calculations for each ray
        float wallHeight = (mapS * 360) / correctedDistance;
        float wallTop = (WINDOW_HEIGHT - wallHeight) / 2;
        float sliceWidth = (WINDOW_WIDTH / 2.0f) / 120.0f;
        float wallSlice = (WINDOW_WIDTH / 2.0f) + (sliceWidth * r);
        SDL_FRect wall = {wallSlice , wallTop, sliceWidth, wallHeight};

        //render texture onto slice
        SDL_FRect srcRect= {(float)textureX * (640.f/64.0f), 0, 640.0f/120.0f, 640.0f};
        SDL_RenderTexture(renderer, wallTexture, &srcRect, &wall);

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
    Uint32 frameStart = SDL_GetTicks();
    if(movement.up) {
        float newX = playerX + playerDeltaX;
        float newY = playerY + playerDeltaY;

        int gridX = (int)newX / mapS;
        int gridY = (int) playerY / mapS;
        if(map[gridY * mapX + gridX] == 0) playerX = newX;

        gridX = (int) playerX / mapS;
        gridY = (int) newY / mapS;
        if(map[gridY * mapX + gridX] == 0) playerY = newY;
    }
    if(movement.down) {
        float newX = playerX - playerDeltaX;
        float newY = playerY - playerDeltaY;

        int gridX = (int)newX / mapS;
        int gridY = (int) playerY / mapS;
        if(map[gridY * mapX + gridX] == 0) playerX = newX;

        gridX = (int) playerX / mapS;
        gridY = (int) newY / mapS;
        if(map[gridY * mapX + gridX] == 0) playerY = newY;
    }
    if(movement.left) {
        playerAngle -= 0.1f;
        if(playerAngle < 0)
            playerAngle += 2*PI;
        playerDeltaX = cos(playerAngle) * 4;
        playerDeltaY = sin(playerAngle) * 4;
    }
    if(movement.right) {
        playerAngle += 0.1f;
        if(playerAngle > 2*PI)
            playerAngle -= 2*PI;
        playerDeltaX = cos(playerAngle) * 4;
        playerDeltaY = sin(playerAngle) * 4;
    }
    SDL_SetRenderDrawColorFloat(renderer, 0.3, 0.3, 0.3, 0);
    SDL_RenderClear(renderer);
    drawMap();
    drawPlayer();
    drawRays();
    SDL_RenderPresent(renderer);

    //cap frames per second
    Uint32 frameTime = SDL_GetTicks() - frameStart;
    if(frameTime < 16)
        SDL_Delay(32 - frameTime);
    return SDL_APP_CONTINUE;
}

SDL_Texture* loadTexture(SDL_Renderer * renderer, const char* filePath) {
    SDL_Surface * surface = IMG_Load(filePath);
    if(!surface) {SDL_Log("IMG_LOAD ERROR"); return nullptr;}
    SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
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
    wallTexture = loadTexture(renderer, "assets/brickwall.jpg");
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
    if(event->type == SDL_EVENT_KEY_DOWN) {
        if(event->key.scancode == SDL_SCANCODE_W) {
            
            movement.up = true;
         }
        if(event->key.scancode == SDL_SCANCODE_A) {

            movement.left = true;
        }

        if(event->key.scancode == SDL_SCANCODE_S) {

            movement.down = true;
         }
     
        if(event->key.scancode == SDL_SCANCODE_D) {

            movement.right = true;
         }

    }
    if(event->type == SDL_EVENT_KEY_UP) {
        if(event->key.scancode == SDL_SCANCODE_W) movement.up = false;
        if(event->key.scancode == SDL_SCANCODE_A) movement.left = false;
        if(event->key.scancode == SDL_SCANCODE_S) movement.down = false;
        if(event->key.scancode == SDL_SCANCODE_D) movement.right = false;
    }
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    
}