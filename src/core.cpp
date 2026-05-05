#include "core.h"
#include "Entity.h"
#include "EntityManager.h"
#include "PhysicsEngine.h"
#include "EntityFactory.h"
#include "Player.h"
#include "Enemy.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <sstream>
#pragma comment(lib, "ws2_32.lib")

Engine* Engine::instance = nullptr;

Engine::Engine()
    : running(true)
    , score(0)
    , ammo(0)
    , connected(false)
    , gameStarted(false)
    , sock(INVALID_SOCKET)
    , entityManager(new EntityManager())
    , physicsEngine(new PhysicsEngine())
{}

Engine::~Engine() {
    delete entityManager;
    delete physicsEngine;
    if (sock != INVALID_SOCKET) closesocket(sock);
    WSACleanup();
    instance = nullptr;
}

Engine& Engine::getInstance() {
    if (instance == nullptr)
        instance = new Engine();
    return *instance;
}

void Engine::connectToServer() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port   = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "[Network] Could not connect. Running offline.\n";
        closesocket(sock);
        sock = INVALID_SOCKET;
        return;
    }

    connected = true;
    std::cout << "[Network] Connected to server!\n";
}

void Engine::sendPacket(const std::string& action, int x, int y) {
    if (sock == INVALID_SOCKET || !connected) return;
    std::string packet = std::to_string(myPlayerID) + ":" + action + ":"
                       + std::to_string(x) + ":" + std::to_string(y);
    send(sock, packet.c_str(), packet.size(), 0);
}

void Engine::receiveLoop() {
    char buffer[2048];
    while (running && connected) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            connected = false;
            break;
        }

        std::string msg(buffer, bytesReceived);

        if (msg.substr(0, 5) == "ROLE:") {
            myPlayerID = msg[5] - '0';
            myRole     = msg.substr(7);
        }
        else if (msg == "SERVER:START") {
            gameStarted = true;
        }
        else if (msg.substr(0, 11) == "WORLDSTATE:") {
            std::string body = msg.substr(11);
            std::stringstream ss(body);
            std::string scoreStr, p1Block, p2Block;
            getline(ss, scoreStr, '|');
            getline(ss, p1Block,  '|');
            getline(ss, p2Block,  '|');

            netScore = stoi(scoreStr);
            score    = netScore;

            auto parseBlock = [&](const std::string& block, int idx) {
                std::stringstream bs(block);
                std::string label, id, role, x, y, alive;
                getline(bs, label, ':');
                getline(bs, id,    ':');
                getline(bs, role,  ':');
                getline(bs, x,     ':');
                getline(bs, y,     ':');
                getline(bs, alive, ':');
                netPlayers[idx].x     = stoi(x);
                netPlayers[idx].y     = stoi(y);
                netPlayers[idx].alive = (alive == "1");
            };

            parseBlock(p1Block, 0);
            parseBlock(p2Block, 1);

            int idx = 0;
            for (auto& e : entityManager->getEntities()) {
                if (auto* p = dynamic_cast<Player*>(e.get())) {
                    if (idx < 2) {
                        p->setPosition({netPlayers[idx].x, netPlayers[idx].y});
                        if (!netPlayers[idx].alive) p->setActive(false);
                        idx++;
                    }
                }
            }
        }
        else if (msg.substr(0, 19) == "SERVER:PLAYER_LEFT:") {
            running = false;
        }
    }
}

void Engine::clearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD topLeft = {0, 0};
    SetConsoleCursorPosition(hConsole, topLeft);
}

void Engine::drawGrid(const std::vector<Entity*>& entities) {
    char buffer[GRID_HEIGHT][GRID_WIDTH];
    for (int row = 0; row < GRID_HEIGHT; ++row)
        for (int col = 0; col < GRID_WIDTH; ++col)
            buffer[row][col] = EMPTY_CELL;

    for (const Entity* entity : entities) {
        if (!entity->isActive()) continue;
        int x = entity->getPosition().x;
        int y = entity->getPosition().y;
        if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) continue;
        buffer[y][x] = entity->display();
    }

    std::cout << '+';
    for (int col = 0; col < GRID_WIDTH; ++col) std::cout << BORDER_H;
    std::cout << "+\n";

    for (int row = 0; row < GRID_HEIGHT; ++row) {
        std::cout << BORDER_V;
        for (int col = 0; col < GRID_WIDTH; ++col)
            std::cout << buffer[row][col];
        std::cout << BORDER_V << '\n';
    }

    std::cout << '+';
    for (int col = 0; col < GRID_WIDTH; ++col) std::cout << BORDER_H;
    std::cout << "+\n";
}

void Engine::drawHUD() {
    std::cout << "\nhhhh bro it's too easy!"
              << "  |  Player: " << myPlayerID
              << "  |  Role: " << myRole
              << "  |  Score: " << score << "\n\n";
    std::cout << "P1 [V]: [A] Left  [D] Right  [S] Shoot\n";
    std::cout << "P2 [M]: [H] Left  [L] Right  [SPACE] Jump  |  [Q] Quit\n";
}

void Engine::render() {
    clearScreen();
    std::vector<Entity*> entities;
    for (auto& e : entityManager->getEntities())
        if (e && e->isActive())
            entities.push_back(e.get());
    drawGrid(entities);
    drawHUD();
    std::cout << std::flush;
}

void Engine::processInput() {
    if (_kbhit()) {
        char key = static_cast<char>(toupper(_getch()));

        Player* top = nullptr;
        Player* bottom = nullptr;
        for (auto& e : entityManager->getEntities()) {
            if (auto* p = dynamic_cast<Player*>(e.get())) {
                if (p->isTop()) top = p;
                else bottom = p;
            }
        }

        if (connected) {
            switch (key) {
                case 'A': if (myRole == "TOP")    sendPacket("SLIDE", -1, 0); break;
                case 'D': if (myRole == "TOP")    sendPacket("SLIDE",  1, 0); break;
                case 'S': if (myRole == "TOP")    sendPacket("SHOOT",  0, 0); break;
                case 'H': if (myRole == "BOTTOM") sendPacket("SLIDE", -1, 0); break;
                case 'L': if (myRole == "BOTTOM") sendPacket("SLIDE",  1, 0); break;
                case ' ': if (myRole == "BOTTOM") sendPacket("JUMP",   0, 1); break;
                case 'Q': case 27: running = false; break;
            }
        } else {
            switch (key) {
                case 'A': if (top) top->move({-1, 0}); break;
                case 'D': if (top) top->move({1, 0});  break;
                case 'S':
                    if (top) {
                        std::unique_ptr<Entity> proj = top->shoot();
                        if (proj) entityManager->addEntity(std::move(proj));
                    }
                    break;
                case 'H': if (bottom) bottom->move({-1, 0}); break;
                case 'L': if (bottom) bottom->move({1, 0});  break;
                case ' ':
                    if (bottom) bottom->setVelocity({0, -5});
                    break;
                case 'Q': case 27: running = false; break;
            }
        }
    }
}

void Engine::updatePhysics() {
    for (auto& e : entityManager->getEntities()) {
        if (e && e->isActive())
            e->move({0, 0});
    }
    physicsEngine->detectAndResolveCollisions(entityManager->getEntities());

    for (auto& e : entityManager->getEntities()) {
        if (auto* p = dynamic_cast<Player*>(e.get())) {
            if (!p->isActive()) {
                if (connected) sendPacket("DEATH", p->isTop() ? 1 : 2, 0);
                running = false;
            }
        }
    }

    bool hasEnemy = false;
    for (auto& e : entityManager->getEntities()) {
        if (e && e->isActive() && e->getType() == EntityType::INSECT)
            hasEnemy = true;
    }
    if (!hasEnemy) {
        score++;
        entityManager->addEntity(EntityFactory::createEntity("Enemy", rand() % 1000, {rand() % 38 + 1, 1}));
    }
}

void Engine::run() {
    srand(time(nullptr));

    connectToServer();

    if (connected)
        recvThread = std::thread(&Engine::receiveLoop, this);

    auto p1 = EntityFactory::createEntity("Player", 1, {20, 0}, "Top");
    if (p1) {
        static_cast<Player*>(p1.get())->setAsTopPlayer(true);
        entityManager->addEntity(std::move(p1));
    }

    auto p2 = EntityFactory::createEntity("Player", 2, {20, 19}, "Bottom");
    if (p2) {
        static_cast<Player*>(p2.get())->setAsTopPlayer(false);
        entityManager->addEntity(std::move(p2));
    }

    entityManager->addEntity(EntityFactory::createEntity("Enemy", 3, {rand() % 38 + 1, 1}));

    if (connected) {
        std::cout << "Waiting for other player...\n";
        while (connected && !gameStarted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "Player " << myPlayerID << " | Role: " << myRole << " | GO!\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    while (running) {
        processInput();
        updatePhysics();
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    if (recvThread.joinable())
        recvThread.join();

    system("cls");
    std::cout << "\n\n";
    std::cout << "====================================\n";
    std::cout << "          damn you lost             \n";
    std::cout << "        it's too easy dude!         \n";
    std::cout << "====================================\n";
    std::cout << "\nPress any key to exit...\n";
    _getch();
}