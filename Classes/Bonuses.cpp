//
//  Bonuses.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#include "Bonuses.h"
#include "GameState.h"

#include <cocos2d.h>
using namespace cocos2d;
#include "Macros.h"

void Checkpoint::apply(GameState& state, int playerIndex)
{
    auto& pPl = state.players()[playerIndex];
    int totalPoints = 0;
    for(auto& cell : state.board().cells) {
        if(pPl->color == cell.color) {
            cell.color = Color::Gray;
            ++totalPoints;
            
            SEND_EVENT("RemoveColor", &cell);
        }
    }
    
    pPl->points += totalPoints;
    
    //TODO: this is hardcoded for Red player change it to be the current player
    if(pPl->color == Color::Red)
        SEND_EVENT("Score", &totalPoints);
}

void Arrow::update(GameState& state)
{
    dir = (Direction)(((int)dir + 1) % 4);
    SEND_EVENT("RotateArrow", this);
}

void Arrow::apply(GameState& state, int playerIndex)
{
    auto& pPl = state.players()[playerIndex];
    SEND_EVENT("TriggerArrow", &*pPl);
    switch (dir)
    {
        case Left:
        {
            int y = pPl->pos.y;
            for (int c = pPl->pos.x; c >= 0; --c){
                state.board()[Vec2(c, y)].color = pPl->color;
            }
            break;
        }
        case Up:
        {
            int x = pPl->pos.x;
            for (int c = pPl->pos.y; c < Board::boardSize; ++c) {
                state.board()[Vec2(x, c)].color = pPl->color;
            }
            break;
        }
        case Right:
        {
            int y = pPl->pos.y;
            for (int c = pPl->pos.x; c < Board::boardSize; ++c) {
                    state.board()[Vec2(c, y)].color = pPl->color;
            }
            break;
        }
        case Down:
        {
            int x = pPl->pos.x;
            for (int c = pPl->pos.y; c >= 0; --c) {
                state.board()[Vec2(x, c)].color = pPl->color;
            }
            break;
        }
        default:
            break;
    }
}