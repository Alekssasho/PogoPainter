//
//  PPGameState.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#include "GameState.h"
#include <initializer_list>
#include <mutex>
#include "Bonuses.h"

void GameState::serialize()
{
    state.tick = mTicks;
    int cell_id = 0;
    for (auto & cell : mBoard.cells) {
        auto & state_cell = state.board[cell_id++];
        state_cell.color = cell.color;
        if (cell.pBonus) {
            state_cell.has_bonus = true;
            state_cell.bonus_type = cell.pBonus->type;
            state_cell.bonus_data = cell.pBonus->getData();
        } else {
            state_cell.has_bonus = false;
        }
    }

    int pid = 0;
    for (auto & player : mPlayers) {
        auto & state_player = state.player[pid++];

        state_player.color = player->color;
        state_player.dir = player->currentDirection;
        state_player.pos = { player->pos.x, player->pos.y };
        state_player.points = player->points;
    }
}

void GameState::deserialize()
{
    mTicks = state.tick;
    int cell_id = 0;
    
    for (auto & cell : mBoard.cells) {
        auto & state_cell = state.board[cell_id++];
        cell.color = state_cell.color;
        if (state_cell.has_bonus && !cell.pBonus) {
            cell.pBonus = BonusInitializers.at(state_cell.bonus_type)(cell);
            cell.pBonus->setData(state_cell.bonus_data);
        } else if (!state_cell.has_bonus && cell.pBonus) {
            delete cell.pBonus;
        }
    }

    int pid = 0;
    for (auto & player : mPlayers) {
        auto & state_player = state.player[pid++];

        player->color = state_player.color;
        player->currentDirection = state_player.dir;
        player->pos = Vec2(state_player.pos[0], state_player.pos[1]);
        player->points = state_player.points;
    }
}

void GameState::client_deserialize()
{
    int pid = 0;
    for (auto & player : mPlayers) {
        auto & state_player = state.player[pid++];
        player->currentDirection = state_player.dir;
    }
}