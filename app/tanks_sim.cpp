#include "tanks_sim.hpp"

#include <algorithm>

namespace {

void facingDelta(int facing, int& dx, int& dy) {
    dx = 0;
    dy = 0;
    switch (facing) {
    case 0:
        dy = -1;
        break;
    case 1:
        dx = 1;
        break;
    case 2:
        dy = 1;
        break;
    case 3:
        dx = -1;
        break;
    }
}

} // namespace

const char* tankFacingName(int facing) {
    static const char* names[] = {"North", "East", "South", "West"};
    if (facing >= 0 && facing < 4) {
        return names[facing];
    }
    return "?";
}

void tankReset(TankState& tank) {
    tank.x = kTankFieldW / 2;
    tank.y = kTankFieldH / 2;
    tank.facing = 0;
    tank.engine = false;
    tank.bullets.clear();
    tank.fireFlash = 0;
}

void tankApplyAction(TankState& tank, const std::string& op, bool allowed) {
    if (!allowed) {
        return;
    }
    tank.lastOp = op;
    if (op == "start") {
        tank.engine = true;
        return;
    }
    if (op == "stop") {
        tank.engine = false;
        return;
    }
    if (op == "fire") {
        int dx = 0;
        int dy = 0;
        facingDelta(tank.facing, dx, dy);
        TankBullet bullet;
        bullet.x = tank.x + dx;
        bullet.y = tank.y + dy;
        bullet.facing = tank.facing;
        if (bullet.x >= 0 && bullet.x < kTankFieldW && bullet.y >= 0 && bullet.y < kTankFieldH) {
            tank.bullets.push_back(bullet);
        }
        tank.fireFlash = 10;
        return;
    }
    if (op == "left") {
        tank.facing = (tank.facing + 3) % 4;
        return;
    }
    if (op == "right") {
        tank.facing = (tank.facing + 1) % 4;
        return;
    }
    int dx = 0;
    int dy = 0;
    facingDelta(tank.facing, dx, dy);
    if (op == "backward") {
        dx = -dx;
        dy = -dy;
    }
    if (op == "forward" || op == "backward") {
        tank.x = std::clamp(tank.x + dx, 0, kTankFieldW - 1);
        tank.y = std::clamp(tank.y + dy, 0, kTankFieldH - 1);
    }
}

void tankAdvanceBullets(TankState& tank) {
    std::vector<TankBullet> next;
    next.reserve(tank.bullets.size());
    for (auto bullet : tank.bullets) {
        int dx = 0;
        int dy = 0;
        facingDelta(bullet.facing, dx, dy);
        bullet.x += dx;
        bullet.y += dy;
        if (bullet.x >= 0 && bullet.x < kTankFieldW && bullet.y >= 0 && bullet.y < kTankFieldH) {
            next.push_back(bullet);
        }
    }
    tank.bullets = std::move(next);
}

void tankTick(TankState& tank) {
    tankAdvanceBullets(tank);
    if (tank.fireFlash > 0) {
        --tank.fireFlash;
    }
}
