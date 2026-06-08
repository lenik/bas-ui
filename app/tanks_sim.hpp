#pragma once

#include <string>
#include <vector>

struct TankBullet {
    int x = 0;
    int y = 0;
    int facing = 0;
};

struct TankState {
    int x = 11;
    int y = 6;
    int facing = 0;
    bool engine = false;
    std::string lastOp;
    std::string lastMessage;
    std::vector<TankBullet> bullets;
    int fireFlash = 0;
};

constexpr int kTankFieldW = 22;
constexpr int kTankFieldH = 12;

const char* tankFacingName(int facing);
void tankReset(TankState& tank);
void tankApplyAction(TankState& tank, const std::string& op, bool allowed);
void tankAdvanceBullets(TankState& tank);
void tankTick(TankState& tank);
