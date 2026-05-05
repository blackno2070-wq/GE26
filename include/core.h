#pragma once
#include "Vec2.h"
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

class Entity;
class EntityManager;
class PhysicsEngine;

constexpr int  GRID_WIDTH  = 40;
constexpr int  GRID_HEIGHT = 20;
constexpr char EMPTY_CELL  = '.';
constexpr char BORDER_H    = '-';
constexpr char BORDER_V    = '|';

#define SERVER_IP "127.0.0.1"  // his machine changes this to your IP
#define PORT 5555

class Engine {
public:
    static Engine& getInstance();

    Engine(const Engine&)            = delete;
    Engine& operator=(const Engine&) = delete;

    void run();
    void processInput();
    void updatePhysics();
    void render();
    void drawGrid(const std::vector<Entity*>& entities);
    void drawHUD();
    void clearScreen();

    bool isRunning() const { return running; }
    void stop()            { running = false; }

    int  getScore()  const { return score; }
    int  getAmmo()   const { return ammo;  }
    void setScore(int s)   { score = s;    }
    void setAmmo(int a)    { ammo  = a;    }

private:
    Engine();
    ~Engine();

    void connectToServer();
    void receiveLoop();
    void sendPacket(const std::string& action, int x, int y);

    SOCKET sock;
    std::thread recvThread;
    std::atomic<bool> connected;
    std::atomic<bool> gameStarted;
    int myPlayerID = 0;
    std::string myRole = "";

    struct NetPlayer {
        int x = 0, y = 0;
        bool alive = true;
    };
    NetPlayer netPlayers[2];
    int netScore = 0;

    bool           running;
    int            score;
    int            ammo;

    EntityManager* entityManager;
    PhysicsEngine* physicsEngine;

    static Engine* instance;
};