    //
    //  PPBonus.cpp
    //  PogoPainter
    //
    //  Created by Aleksandar Angelov on 11/8/14.
    //
    //

    #include "PPBonus.h"
    #include "PPPlayer.h"

    void PPCheckpoint::apply(PPPlayer& player, PPBoard& board)
    {
        board.each([&](PPCell & cell) {
            if (cell.color == player.color) {
                Blink* blink = Blink::create(0.5f, 3);
                cell.sprite->runAction(blink);
                cell.color = PPColor::Empty;
                ++player.points;
                //TODO: Animate points
            }
        });
    }

    void PPBonus::update(PPBoard& board)
    {

    }

    void PPArrow::update(PPBoard& board)
    {
    dir = (PPDirection)(((int)dir + 1) % 4);
    }

    void PPArrow::apply(PPPlayer& player, PPBoard& board)
    {
        Blink* blink = Blink::create(1.0f, 5);
    switch (dir)
    {
    case Left:
        for (int c = player.getPosition().x; c >= 0; --c){
            board.at(c, player.getPosition().y).sprite->runAction(blink);
            board.at(c, player.getPosition().y).color = player.color;
        }
        break;
    case Up:
        for (int c = player.getPosition().y; c < PPBoardSize; ++c) {
            board.at(c, player.getPosition().y).sprite->runAction(blink);
            board.at(c, player.getPosition().y).color = player.color;
        }
        break;
    case Right:
        for (int c = player.getPosition().x; c < PPBoardSize; ++c) {
            board.at(c, player.getPosition().y).sprite->runAction(blink);
            board.at(c, player.getPosition().y).color = player.color;
        }
        break;
    case Down:
        for (int c = player.getPosition().y; c >= 0; --c) {
            board.at(player.getPosition().x, c).sprite->runAction(blink);
            board.at(player.getPosition().x, c).color = player.color;
        }
        break;
    default:
        break;
    }
    }

    void PPBonusManager::update(PPBoard& board, std::vector<std::unique_ptr<PPPlayer>> & players)
    {
    static int step = 0;
    if (step++ < steps_delay) {
        return;
    }
    step = 0;

    auto get_pos = [this]() {
        return Vec2(position_picker(generator), position_picker(generator));
    };

    auto check_player = [&](const Vec2 & where) {
        bool hit = false;
        for (auto & player : players) {
            hit = hit || player->getPosition() == where;
        }
        return hit;
    };

    if (target_checkpoints > checkpoints.size()) {
        auto pos = get_pos();
        while (board.at(pos).bonus || check_player(pos)) {
            pos = get_pos();
        }
        checkpoints.push_back(new PPCheckpoint(board.at(pos)));
        board.at(pos).bonus = std::unique_ptr<PPBonus>(checkpoints.back());
        if (surface) {
            checkpoints.back()->sprite->setPosition(checkpoints.back()->getCell().sprite->getPosition());
            checkpoints.back()->sprite->setScale(0.01);
            auto targetScale = checkpoints.back()->getCell().sprite->getScale();
            checkpoints.back()->sprite->runAction(ScaleTo::create(0.20f, targetScale));
            surface->addChild(checkpoints.back()->sprite);
        }
        return;
    }

    if (max_bonuses > bonuses.size()) {
        auto pos = get_pos();
        while (board.at(pos).bonus || check_player(pos)) {
            pos = get_pos();
        }

        PPBonus * bonus;
        switch (bonus_picker(generator))
        {
        case 0:
            bonus = new PPArrow(board.at(pos));
            break;
        case 1:
            bonus = new PPArrow(board.at(pos));
            break;
        case 2:
            bonus = new PPArrow(board.at(pos));
            break;
        }
        board.at(pos).bonus = std::unique_ptr<PPBonus>(bonus);
        bonuses.push_back(bonus);
        if (surface) {
            bonus->sprite->setPosition(bonus->getCell().sprite->getPosition());
            bonus->sprite->setScale(bonus->getCell().sprite->getScale());
            surface->addChild(bonus->sprite);
        }
    }
    }

    void PPBonusManager::each(std::function<void(PPBonus*)> fn)
    {
    for (auto & point : checkpoints) {
        fn(point);
    }

    for (auto bonus : bonuses) {
        fn(bonus);
    }
    }

    void PPBonusManager::removeBonus(PPBonus * bonus)
    {
    auto found = find(bonuses.begin(), bonuses.end(), bonus);
    if (found != bonuses.end()) {
        bonuses.erase(found);
    } else {
        auto found = find(checkpoints.begin(), checkpoints.end(), bonus);
        if (found != checkpoints.end()) {
            checkpoints.erase(found);
        }
    }
    }