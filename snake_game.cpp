#include <iostream>
#include <conio.h>      // For _kbhit() and _getch() on Windows
#include <windows.h>    // For Sleep()
#include <vector>
#include <cstdlib>

using namespace std;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

class SnakeGame {
private:
    int width, height;
    int x, y, fruitX, fruitY, score;
    Direction dir;
    vector<pair<int, int>> tail;
    bool gameOver;

public:
    SnakeGame(int w = 20, int h = 20) : width(w), height(h), score(0), gameOver(false) {
        x = width / 2;
        y = height / 2;
        fruitX = rand() % width;
        fruitY = rand() % height;
        dir = STOP;
    }

    void Draw() {
        system("cls");
        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (j == 0) cout << "#";
                if (i == y && j == x)
                    cout << "O";
                else if (i == fruitY && j == fruitX)
                    cout << "F";
                else {
                    bool print = false;
                    for (auto t : tail) {
                        if (t.first == j && t.second == i) {
                            cout << "o";
                            print = true;
                        }
                    }
                    if (!print) cout << " ";
                }
                if (j == width - 1) cout << "#";
            }
            cout << endl;
        }
        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;
        cout << "Score: " << score << endl;
    }

    void Input() {
        if (_kbhit()) {
            switch (_getch()) {
            case 'a': dir = LEFT; break;
            case 'd': dir = RIGHT; break;
            case 'w': dir = UP; break;
            case 's': dir = DOWN; break;
            case 'x': gameOver = true; break;
            }
        }
    }

    void Logic() {
        if (dir == STOP) return;
        int prevX = x, prevY = y;
        int prev2X, prev2Y;
        if (!tail.empty()) {
            prev2X = tail[0].first;
            prev2Y = tail[0].second;
            tail[0] = { x, y };
            for (size_t i = 1; i < tail.size(); i++) {
                int tempX = tail[i].first, tempY = tail[i].second;
                tail[i] = { prev2X, prev2Y };
                prev2X = tempX; prev2Y = tempY;
            }
        }
        switch (dir) {
        case LEFT: x--; break;
        case RIGHT: x++; break;
        case UP: y--; break;
        case DOWN: y++; break;
        default: break;
        }
        // Game over conditions
        if (x < 0 || x >= width || y < 0 || y >= height)
            gameOver = true;
        for (auto t : tail)
            if (t.first == x && t.second == y)
                gameOver = true;
        // Eat fruit
        if (x == fruitX && y == fruitY) {
            score += 10;
            fruitX = rand() % width;
            fruitY = rand() % height;
            tail.push_back({ prevX, prevY });
        }
    }

    bool isGameOver() const { return gameOver; }
};

int main() {
    SnakeGame game;
    while (!game.isGameOver()) {
        game.Draw();
        game.Input();
        game.Logic();
        Sleep(100); // Sleep for 100 ms
    }
    std::cout << "Game Over!" << std::endl;
    return 0;
}