//
//  PPGameState.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#ifndef __PogoPainter__PPGameState__
#define __PogoPainter__PPGameState__

#include <vector>

#include "Board.h"
#include "BaseClasses.h"

using Players = std::vector<PlayerPtr>;

class GameState
{
public:
    struct {
        int tick;
        struct {
            bool has_bonus : 1;
            Bonus::Type bonus_type : 7;
            unsigned char bonus_data;
            Color color;
        } board[Board::boardSize * Board::boardSize];
        struct {
            Color color;
            Direction dir;
            std::array<unsigned char, 2> pos;
            int points;
        } player[4];
    } state;

    Board& board() { return mBoard; }
    Players& players() { return mPlayers; }
    int ticks() { return mTicks; }
    
    void serialize();
    void deserialize();
    
    void incrementTick() {
        ++mTicks;
    }

private:
    
    Board mBoard;
    Players mPlayers;
    int mTicks = 0;
};

#endif /* defined(__PogoPainter__PPGameState__) */
