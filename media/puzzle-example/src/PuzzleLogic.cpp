#include "PuzzleLogic.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>

PuzzleLogic::PuzzleLogic(int width, int height) : w(width), h(height) {
    board.reserve(width * height); 
    for (int i = 1; i < width * height; ++i) {
        board.push_back(i);
    }
    board.push_back(0);
 
    unsigned seed = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::mt19937 rng(seed);
    do {
        std::shuffle(board.begin(), board.end(), rng);
    } while (!isSolvable() || isSolved());
}

void PuzzleLogic::getEmptyPosition(int &x, int &y) const {
    int emptyIndex = static_cast<int>(std::find(board.begin(), board.end(), 0) - board.begin());
    if (emptyIndex >= 0 && emptyIndex < w * h) {
        x = emptyIndex % w;
        y = emptyIndex / w;
    } else {
        x = 0;
        y = 0;
    }
}

void PuzzleLogic::moveTile(int x, int y) {
    if (x < 0 || x >= w || y < 0 || y >= h) return;

    int emptyX, emptyY;
    getEmptyPosition(emptyX, emptyY);

    int dx = x - emptyX;
    int dy = y - emptyY;
    if (std::abs(dx) + std::abs(dy) == 1) {
        std::swap(board[emptyY * w + emptyX], board[y * w + x]);
    }
}

bool PuzzleLogic::isSolved() {
    for (int i = 0; i < w * h - 1; ++i) {
        if (board[i] != i + 1) return false;
    }
    return board[w * h - 1] == 0;
}

bool PuzzleLogic::isSolvable() {
    int inversions = 0;
    for (int i = 0; i < w * h; ++i) {
        if (board[i] == 0) continue;
        for (int j = i + 1; j < w * h; ++j) {
            if (board[j] != 0 && board[i] > board[j]) ++inversions;
        }
    }
    if (w % 2 == 1) {
        return (inversions % 2) == 0;
    }
    int emptyIndex = static_cast<int>(std::find(board.begin(), board.end(), 0) - board.begin());
    int emptyRow = emptyIndex / w;
    return ((inversions + emptyRow) % 2) == 1;
}

int PuzzleLogic::getTile(int x, int y) {
    if (x < 0 || x >= w || y < 0 || y >= h) return -1;
    return board[y * w + x];
}
