#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDl3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string_view>
#include <filesystem>
#include <thread>
#include <future>
#include "include/map.h"
#include "include/player.h"

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800
#define PI 3.14159265359
#define DR 0.0174533 /2
#define MAX_DEPTH 20

struct AppContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    vector<SDL_Texture*> textures;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
    bool fullScreen = false;
};

//--MAP--
vector<int> &&mapV =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,1,
    1,0,0,0,0,0,1,0,0,0,2,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,1,0,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

//--PLAYER AND MAP--
Player player;
Movement movement;
Map map1(mapV);


void drawRays(AppContext * app)
{
    
    //Draw celing gradient
    for (int y = 0; y < WINDOW_HEIGHT / 2; y++) {
        float t = (float)y / (WINDOW_HEIGHT / 2);
        float r = 0;
        float g = 0.7f;
        float b = 1.0f;
        SDL_SetRenderDrawColorFloat(app->renderer, r, g, b, 1.0f);
        SDL_RenderLine(app->renderer, 0, y, WINDOW_WIDTH, y);
    }
    
    // Draw floor gradient
    for (int y = WINDOW_HEIGHT / 2; y < WINDOW_HEIGHT; y++) {
        float t = (float)(y - WINDOW_HEIGHT / 2) / (WINDOW_HEIGHT / 2);
        float r = 0.2f + (0.5f - 0.2f) * t;
        float g = 0.1f + (0.3f - 0.1f) * t;
        float b = 0.1f + (0.2f - 0.1f) * t;
        SDL_SetRenderDrawColorFloat(app->renderer, r, g, b, 1.0f);
        SDL_RenderLine(app->renderer, 0, y, WINDOW_WIDTH, y);
    }

    int r, mx, my, mp, dof;
    float rayX, rayY, rayAngle, xOffset, yOffset;
    //FOV of 60 degrees
    rayAngle = player.angle - 60 * DR;
    if(rayAngle < 0)
        rayAngle += 2*PI;
    if(rayAngle > 2*PI)
        rayAngle -= 2*PI;
    //loop for each ray
    for(r = 0; r < 120; r++)
    {
        //--checking for horizontal lines--
        dof = 0;
        float aTan=-1/tan(rayAngle);
        float distanceH = INTMAX_MAX, horizontalX=player.x, horizontalY = player.y;
        float distanceT;
        //looking up
        if(rayAngle > PI) {
            rayY =(((int)player.y / TILE_SIZE) * TILE_SIZE) - 0.0001;
            rayX = (player.y-rayY)*aTan+player.x;
            yOffset= -TILE_SIZE;
            xOffset= -yOffset*aTan;
        }
        //looking down
        if(rayAngle < PI) {
            rayY =(((int)player.y / TILE_SIZE) * TILE_SIZE) +TILE_SIZE;
            rayX = (player.y-rayY)*aTan+player.x;
            yOffset= TILE_SIZE;
            xOffset= -yOffset*aTan;
        }
        //looking straight left or right
        if(rayAngle == 0 || rayAngle == PI){
            rayX = player.x;
            rayY = player.y;
            dof = MAX_DEPTH;
        }
        while(dof < MAX_DEPTH)
        {
            mx=(int) (rayX) / TILE_SIZE;
            my=(int) (rayY) / TILE_SIZE;
            mp=my*map1.mapX+mx;
            if(mp > 0 && mp<map1.mapX*map1.mapY && map1.getMap()[mp] > 0) { 
                horizontalX = rayX; horizontalY = rayY;
                distanceH = pythag(player.x, player.y, rayX, rayY, player.angle); 
                dof = MAX_DEPTH;
            } //hit wall
            else{ rayX+=xOffset; rayY+=yOffset; dof+= 1;}
        }

        //--checking for vertical lines--
        dof = 0;
        float nTan= -tan(rayAngle);
        float distanceV = INTMAX_MAX, verticalX=player.x, verticalY = player.y;
        
        //looking left
        if(rayAngle > PI/2 && rayAngle < 3*PI/2) {
            rayX =(((int)player.x / TILE_SIZE) * TILE_SIZE) - 0.0001;
            rayY = (player.x-rayX)*nTan+player.y;
            xOffset= -TILE_SIZE;
            yOffset= -xOffset*nTan;
        }
        //looking right
        if(rayAngle < PI/2 ||  rayAngle >3*PI/2) {
            rayX =(((int)player.x / TILE_SIZE) * TILE_SIZE) +TILE_SIZE;
            rayY = (player.x-rayX)*nTan+player.y;
            xOffset= TILE_SIZE;
            yOffset= -xOffset*nTan;
        }
        //looking straight up or down
        if(rayAngle == 0 || rayAngle == PI){
            rayX = player.x;
            rayY = player.y;
            dof = MAX_DEPTH;
        }
        while(dof < MAX_DEPTH)
        {
            mx=(int) (rayX) / TILE_SIZE;
            my=(int) (rayY) / TILE_SIZE;
            mp=my*map1.mapX+mx;
            if(mp > 0 && mp<map1.mapX*map1.mapY && map1.getMap()[mp] > 0) {
                verticalX = rayX; verticalY = rayY;
                distanceV = pythag(player.x, player.y, rayX, rayY, player.angle);
                dof = MAX_DEPTH;
            } //hit wall
            else{ rayX+=xOffset; rayY+=yOffset; dof+= 1;}
        }
        int textureX = 0;
        if(distanceH < distanceV) {
            rayX = horizontalX; 
            rayY = horizontalY; 
            distanceT = distanceH;
            textureX = ((int)rayX % TILE_SIZE);
        }
        if(distanceH > distanceV) {
            rayX = verticalX; 
            rayY = verticalY; 
            distanceT = distanceV;
            textureX = ((int)rayY % TILE_SIZE);

        }
        //get texture of ray
        int textureIndex = map1.getMap()[map1.findMapIndex(rayX, rayY)] - 1;
        
        //--Draw ray lines for testing--
       /*
       SDL_SetRenderDrawColorFloat(app->renderer, 0.0, 0.0, 1.0, 1);
       SDL_RenderLine(app->renderer, player.x, player.y, rayX, rayY);
       */
      
        
        //---2.5D raycast rendering---
        //fix fisheye
        float correctedDistance = distanceT * cos(rayAngle - player.angle);
        
        //shader based on distance
        float shadeDist = correctedDistance;
        if(shadeDist > 1250.0f) shadeDist = 1250.0f;
        float shade = 1.0f - (shadeDist / 1250.0f);
        Uint8 color = (Uint8)(shade * 255.0f);
        SDL_SetTextureColorMod(app->textures[textureIndex], color, color, color);

        //wall height calculations for each ray
        float fovRad = 60.0f * (PI / 180.0f);
        float projPlaneDist = (WINDOW_WIDTH / 2.0f) / tan(fovRad / 2.0f);
        float wallHeight = (TILE_SIZE / correctedDistance) * projPlaneDist;
        float wallTop = (WINDOW_HEIGHT - wallHeight) / 2;
        
        float sliceWidth = (WINDOW_WIDTH) / 120.0f;
        float wallSlice =  (sliceWidth * r);
        SDL_FRect wall = {wallSlice , wallTop, sliceWidth, wallHeight};

        //render texture onto slice
        float textureSize = 0.0f;
        if(textureIndex == 0) {textureSize = 640.0f;}
        else if(textureIndex == 1) {textureSize = 143.0f;}
        SDL_FRect srcRect= {(float)textureX * ((float)textureSize/(float) TILE_SIZE), 0, 0, (float)textureSize};
        SDL_RenderTexture(app->renderer, app->textures[textureIndex], &srcRect, &wall);

        rayAngle += DR;
        if(rayAngle < 0)
            rayAngle += 2*PI;
        if(rayAngle > 2*PI)
            rayAngle -= 2*PI;

    }
}

//--Draw player and map for testing
void drawPlayer(SDL_Renderer * renderer)
{
    float miniMapX = player.x, miniMapY = player.y;
    //player is a square
    SDL_FRect playerRect = {
        miniMapX,
        miniMapY,
        8,
        8
    };
    SDL_SetRenderDrawColorFloat(renderer, 1, 1, 0, 1);
    SDL_RenderFillRect(renderer, &playerRect);

    //Where the player is looking 
    SDL_SetRenderDrawColorFloat(renderer, 1, 1, 0, 1);
    SDL_RenderLine(renderer, player.x, player.y, player.x + player.dx*5, player.y + player.dy*5);
}
void drawMap(SDL_Renderer * renderer)
{
    float miniMapScale = WINDOW_WIDTH / 100;
    for(int y = 0; y < map1.mapY; y++)
    {
        for(int x = 0; x < map1.mapX; x++)
        {
            SDL_FRect tile = {
                x * miniMapScale,
                y * miniMapScale,
                miniMapScale,
                miniMapScale
            };
            if(map1.findMapIndex(player.x, player.y) == map1.findMapIndex(x * TILE_SIZE, y * TILE_SIZE))
                SDL_SetRenderDrawColorFloat(renderer, 1.0f,1.0f,0.0f,1.0f);
            else if(map1.getMap()[y*map1.mapX+x] > 0)
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
void drawBullets(SDL_Renderer* renderer, const std::vector<Bullet>& bullets) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color for bullets
    for (const auto& bullet : bullets) {
        
    }
}
SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto* app = (AppContext*)appstate;
    Uint32 frameStart = SDL_GetTicks();
    if(movement.up) {
        float newX = player.x + player.dx * 2;
        float newY = player.y + player.dy * 2;
        if(!map1.checkColision(newX, player.y)) player.x = newX;
        if(!map1.checkColision(player.x, newY)) player.y = newY;
    }
    if(movement.down) {
        float newX = player.x - player.dx * 1.5;
        float newY = player.y - player.dy * 1.5;
        if(!map1.checkColision(newX, player.y)) player.x = newX;
        if(!map1.checkColision(player.x, newY)) player.y = newY;
    }
    if(movement.left) {
        player.angle -= 0.1f;
        if(player.angle < 0) player.angle += 2*PI;
        player.dx = cos(player.angle) * 4;
        player.dy = sin(player.angle) * 4;
    }
    if(movement.right) {
        player.angle += 0.1f;
        if(player.angle > 2*PI) player.angle -= 2*PI;
        player.dx = cos(player.angle) * 4;
        player.dy = sin(player.angle) * 4;
    }
    if(movement.open) {
        int gridX = (int)player.x / TILE_SIZE;
        int gridY = (int)player.y / TILE_SIZE;
        int position = gridX + gridY * map1.mapX;
        int angle = player.angle / (2 * PI) * 360;
        //up
        if(angle >= 180 && angle <= 360) map1.changeDoor(position - map1.mapX);
        //down
        if(angle >= 0 && angle <= 180) map1.changeDoor(position + map1.mapX);
        //left
        if(angle >=90 && angle <= 270) map1.changeDoor(position - 1);
        //right
        if(angle >= 270 || angle <= 90) map1.changeDoor(position + 1);
        
    }
    if(movement.shoot) {
        player.bullets.push_back({player.x, player.y, player.dx, player.dy});
    }
    for(auto &bullet : player.bullets) {
        bullet.x += bullet.dx * 20;
        bullet.y += bullet.dy * 20;
        bullet.distance = pythag(player.x, player.y, bullet.x, bullet.y, player.angle);
        if(map1.getMap()[map1.findMapIndex(bullet.x, bullet.y)] > 0) {
            player.bullets.erase(player.bullets.begin());
            cout << "Bullet hit wall at " << map1.findMapIndex(bullet.x, bullet.y) << endl;
        }
    }
    SDL_SetRenderDrawColorFloat(app->renderer, 0.3, 0.3, 0.3, 0);
    SDL_RenderClear(app->renderer);
    SDL_Rect clipRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_SetRenderClipRect(app->renderer, &clipRect);
    drawRays(app);
    drawMap(app->renderer);
    /*
    drawPlayer(app->renderer);
    */
    drawBullets(app->renderer, player.bullets);
    SDL_RenderPresent(app->renderer);

    //cap frames per second
    
    // Original frame timing code for native builds
    Uint32 frameTime = SDL_GetTicks() - frameStart;
    if(frameTime < 16)
        SDL_Delay(32 - frameTime);
    if(app->app_quit != SDL_APP_CONTINUE)
        return app->app_quit;
    
    return SDL_APP_CONTINUE;
}

SDL_Texture* loadTexture(SDL_Renderer * renderer, const char* filePath) {
    SDL_Surface * surface = IMG_Load(filePath);
    if(!surface) {SDL_Log("IMG_LOAD ERROR"); return nullptr;}
    SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
}

SDL_AppResult SDL_Fail(){
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if(not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
        return SDL_Fail();
    }
    SDL_Window * window = SDL_CreateWindow("2.5D", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if(not window) {
        return SDL_Fail();
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (not renderer){
        return SDL_Fail();
    }
    if (not TTF_Init()) {
        return SDL_Fail();
    }

    auto basePathPtr = SDL_GetBasePath();
     if (not basePathPtr){
        return SDL_Fail();
    }
    const std::filesystem::path basePath = basePathPtr;
    vector<SDL_Texture *> texturesInit;
    texturesInit.push_back(loadTexture(renderer, "brickwall.jpg"));
    texturesInit.push_back(loadTexture(renderer, "door.jpg"));

    player.x = TILE_SIZE * COLLISION_BUFFER / 10;
    player.y = TILE_SIZE * COLLISION_BUFFER / 10;
    player.angle = 0;
    player.dx = cos(player.angle) * 5;
    player.dy = sin(player.angle) * 5;
    
    *appstate = new AppContext{
        .window = window,
        .renderer = renderer,
        .textures = texturesInit,
    };
 
    SDL_SetRenderVSync(renderer, -1);
 
    SDL_Log("Application started successfully!");

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    auto* app = (AppContext*)appstate;

    if(event->type == SDL_EVENT_QUIT){
        app->app_quit = SDL_APP_SUCCESS;
    }
    if(event->type == SDL_EVENT_KEY_DOWN) {
        if(event->key.scancode == SDL_SCANCODE_W) movement.up = true;
        if(event->key.scancode == SDL_SCANCODE_A) movement.left = true;
        if(event->key.scancode == SDL_SCANCODE_S) movement.down = true;
        if(event->key.scancode == SDL_SCANCODE_D) movement.right = true;
        if(event->key.scancode == SDL_SCANCODE_E) movement.open = true;
        if(event->key.scancode == SDL_SCANCODE_F) SDL_SetWindowFullscreen(app->window, app->fullScreen = !app->fullScreen);
        if(event->key.scancode == SDL_SCANCODE_ESCAPE) app->app_quit = SDL_APP_SUCCESS;
        if(event->key.scancode == SDL_SCANCODE_SPACE) movement.shoot = true;
    }
    if(event->type == SDL_EVENT_KEY_UP) {
        if(event->key.scancode == SDL_SCANCODE_W) movement.up = false;
        if(event->key.scancode == SDL_SCANCODE_A) movement.left = false;
        if(event->key.scancode == SDL_SCANCODE_S) movement.down = false;
        if(event->key.scancode == SDL_SCANCODE_D) movement.right = false;
        if(event->key.scancode == SDL_SCANCODE_E) movement.open = false;
        if(event->key.scancode == SDL_SCANCODE_SPACE) movement.shoot = false;
    }
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    auto * app = (AppContext*)appstate;
    if(app) {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }
    TTF_Quit();
    SDL_Quit(); 
}
