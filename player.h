#ifndef PLAYER_H
#define PLAYER_H

using namespace std;
struct Movement{
    bool up = false, down = false, left = false, right = false, open = false, shoot = false;
};
struct Bullet{
    float x, y, dx, dy, distance;
};
struct Player{
    float x, y, dx, dy, angle;
    vector<Bullet> bullets;
};
#endif