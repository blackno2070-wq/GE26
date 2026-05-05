#pragma once

// ============================================================
//  Vec2.h  —  2D Vector Utility
//  Shared by: ALL teams (Core, Gameplay, Networking)
//
//  A lightweight 2D integer vector used for positions,
//  directions, and velocities across the entire engine.
// ============================================================

struct Vec2 {
    int x, y;

    Vec2(int x = 0, int y = 0) : x(x), y(y) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(int s)          const { return {x * s,   y * s};   }

    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return !(*this == o); }
};