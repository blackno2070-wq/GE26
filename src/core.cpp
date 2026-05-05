#include "core.h"
#include "Entity.h"
#include "EntityManager.h"
#include "PhysicsEngine.h"
#include "EntityFactory.h"
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
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
    , connected(false)
    , gameStarted(false)
    , score(0)
    , sock(INVALID_SOCKET)
    , entityManager(new EntityManager())
{}

Engine::~Engine() {
    delete entityManager;
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
    std::cout << "[Network] Connected!\n";
}

void Engine::sendInput(const std::string& action) {
    if (sock == INVALID_SOCKET || !connected) return;
    std::string packet = myRole + ":" + action + "\n";
    send(sock, packet.c_str(), packet.size(), 0);
}

void Engine::applyServerState(const std::string& state) {
    // Format: STATE:id:symbol:x:y:active;id:symbol:x:y:active:SCORE:N
    // Split off SCORE first
    std::string entitiesPart = state;
    int scoreVal = score;

    auto scorePos = state.find(":SCORE:");
    if (scorePos != std::string::npos) {
        entitiesPart = state.substr(0, scorePos);
        scoreVal = stoi(state.substr(scorePos + 7));
        score = scoreVal;
    }

    // Remove "STATE:" prefix
    if (entitiesPart.substr(0, 6) == "STATE:")
        entitiesPart = entitiesPart.substr(6);

    // Clear and rebuild entities
    entityManager->clear();

    std::stringstream ss(entitiesPart);
    std::string token;
    while (getline(ss, token, ';')) {
        if (token.empty()) continue;
        std::stringstream es(token);
        std::string idStr, symbol, xStr, yStr, activeStr;
        getline(es, idStr,     ':');
        getline(es, symbol,    ':');
        getline(es, xStr,      ':');
        getline(es, yStr,      ':');
        getline(es, activeStr, ':');

        int id = stoi(idStr);
        int x  = stoi(xStr);
        int y  = stoi(yStr);

        std::unique_ptr<Entity> e;
        if      (symbol == "V") e = EntityFactory::createEntity("Player",     id, {x, y}, "Top");
        else if (symbol == "M") e = EntityFactory::createEntity("Player",     id, {x, y}, "Bottom");
        else if (symbol == "X") e = EntityFactory::createEntity("Enemy",      id, {x, y});
        else if (symbol == "|") e = EntityFactory::createEntity("Projectile", id, {x, y});

        if (e) {
            if (symbol == "V") static_cast<Player*>(e.get())->setAsTopPlayer(true);
            if (symbol == "M") static_cast<Player*>(e.get())->setAsTopPlayer(false);
            entityManager->addEntity(std::move(e));
        }
    }
}

void Engine::receiveLoop() {
    char buffer[4096];
    std::string leftover;

    while (running && connected) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            connected = false;
            running   = false;
            break;
        }

        leftover += std::string(buffer, bytes);

        // Process complete lines
        size_t pos;
        while ((pos = leftover.find('\n')) != std::string::npos) {
            std::string msg = leftover.substr(0, pos);
            leftover = leftover.substr(pos + 1);

            if (msg.substr(0, 5) == "ROLE:") {
                myRole = msg.substr(5);
                std::cout << "You are " << myRole << "\n";
            }
            else if (msg == "START") {
                gameStarted = true;
            }
            else if (msg.substr(0, 6) == "STATE:") {
                applyServerState(msg);
            }
            else if (msg.substr(0, 8) == "GAMEOVER") {
                running = false;
            }
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
    if (!_kbhit()) return;
    char key = static_cast<char>(toupper(_getch()));

    switch (key) {
        case 'A': if (myRole == "P1") sendInput("MOVE_LEFT");  break;
        case 'D': if (myRole == "P1") sendInput("MOVE_RIGHT"); break;
        case 'S': if (myRole == "P1") sendInput("SHOOT");      break;
        case 'H': if (myRole == "P2") sendInput("MOVE_LEFT");  break;
        case 'L': if (myRole == "P2") sendInput("MOVE_RIGHT"); break;
        case ' ': if (myRole == "P2") sendInput("JUMP");       break;
        case 'Q': case 27: running = false; break;
    }
}

void Engine::run() {
    connectToServer();

    if (connected)
        recvThread = std::thread(&Engine::receiveLoop, this);

    if (connected) {
        std::cout << "Waiting for other player...\n";
        while (connected && !gameStarted)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "GO! You are " << myRole << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    } else {
        // Offline fallback — spawn entities locally
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
        entityManager->addEntity(EntityFactory::createEntity("Enemy", 3, {20, 1}));
    }

    while (running) {
        processInput();
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
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