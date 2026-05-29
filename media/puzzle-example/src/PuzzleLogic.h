#ifndef PUZZLE_LOGIC_H
#define PUZZLE_LOGIC_H

#include <vector>

class PuzzleLogic {
public:
    PuzzleLogic(int width, int height);
    void moveTile(int x, int y);
    bool isSolvable();
    bool isSolved();
    int getTile(int x, int y);
    void getEmptyPosition(int &x, int &y) const;

private:
    int w, h;
    std::vector<int> board; // Flat representation of the grid
};

#endif