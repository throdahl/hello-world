#ifndef MAP_H
#define MAP_H

#include <vector>
#include <math.h>
#include <iostream>

#define COLLISION_BUFFER 15.0f
#define TILE_SIZE 64
using namespace std;

class Map {
    private:
    vector<int> map;
        
    public:
        int mapX, mapY, mapS;
        vector<int> getMap() {
            return map;
        }
        Map(vector<int> mapV) {
            map = mapV;
            mapX = sqrt(map.size()), mapY = mapX, mapS = mapX * mapY;
        }
        void changeMap(int wall, int position){
            map[position] = wall;
        }
        bool checkColision(float x, float y){
            int gridX = (int)x / TILE_SIZE;
            int gridY = (int)y / TILE_SIZE;

            // Check the current cell and the eight surrounding cells.
            for(int i = gridX - 1; i <= gridX + 1; i++){
                for(int j = gridY - 1; j <= gridY + 1; j++){
                    // Skip if out of bounds.
                    if(i < 0 || j < 0 || i >= mapX || j >= mapY)
                        continue;

                    int index = j * mapX + i;
                    if(map[index] >= 1){  // This cell is a wall.
                        // Determine the wall cell's boundaries.
                        float cellLeft   = i * TILE_SIZE;
                        float cellTop    = j * TILE_SIZE;
                        float cellRight  = cellLeft + TILE_SIZE;
                        float cellBottom = cellTop + TILE_SIZE;
                    
                        // Find the closest point from the player (x, y) to the wall cell.
                        float closestX = (x < cellLeft)   ? cellLeft : (x > cellRight ? cellRight : x);
                        float closestY = (y < cellTop)    ? cellTop  : (y > cellBottom ? cellBottom : y);
                    
                        // Calculate the distance between the player and this closest point.
                        float distX = x - closestX;
                        float distY = y - closestY;
                        float distance = sqrt(distX * distX + distY * distY);
                    
                        // If the distance is less than our collision buffer, register a collision.
                        if(distance < COLLISION_BUFFER)
                            return true;
                    }
                }
            }
            return false;
        }
        int findMapIndex(float x, float y){
            int gridX = (int)x / TILE_SIZE;
            int gridY = (int)y / TILE_SIZE;
        
            int index = gridX + mapX * (gridY);
            return index;
        }
        void changeDoor(int position)
        {
            if(map[position] == 2) changeMap(-1, position);
        }
};

//--DISTANCE--
float pythag(float x1, float y1, float x2, float y2, float angle){
    return ( sqrt((x2-x1) * (x2-x1) + (y2-y1) * (y2-y1)));
}

#endif