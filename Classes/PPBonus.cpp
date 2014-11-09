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
			auto action = Repeat::create(Sequence::create(
				DelayTime::create(0.08f),
				CallFunc::create([&]{ cell.color = player.color; }),
				DelayTime::create(0.08f),
				CallFunc::create([&]{ cell.color = PPColor::Empty; }), NULL), 2);
			cell.sprite->runAction(action);
			cell.color = PPColor::Empty;
			++player.points;
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
	switch (dir)
	{
	case Left:
		for (int c = player.getPosition().x; c >= 0; --c){
			int y = player.getPosition().y;
			board.at(c, player.getPosition().y).sprite->runAction(
				Sequence::createWithTwoActions(
				DelayTime::create(0.05f * (player.getPosition().x - c)),
				CallFunc::create([&, y, c] {
				board.at(c, y).color = player.color;
			})));
		}
		break;
	case Up:
		for (int c = player.getPosition().y; c < PPBoardSize; ++c) {
			int x = player.getPosition().x;
			board.at(player.getPosition().x, c).sprite->runAction(
				Sequence::createWithTwoActions(
				DelayTime::create(0.05f * (c - player.getPosition().y)),
				CallFunc::create([&, x, c] {
				board.at(x, c).color = player.color;
			})));
		}
		break;
	case Right:
		for (int c = player.getPosition().x; c < PPBoardSize; ++c) {
			int y = player.getPosition().y;
			board.at(c, player.getPosition().y).sprite->runAction(
				Sequence::createWithTwoActions(
				DelayTime::create(0.05f * (c - player.getPosition().x)),
				CallFunc::create([&, y, c] {
				board.at(c, y).color = player.color;
			})));
		}
		break;
	case Down:
		for (int c = player.getPosition().y; c >= 0; --c) {
			int x = player.getPosition().x;
			board.at(player.getPosition().x, c).sprite->runAction(
				Sequence::createWithTwoActions(
				DelayTime::create(0.05f * (player.getPosition().y - c)),
				CallFunc::create([&, x, c] {
				board.at(x, c).color = player.color;
			})));

		}
		break;
	default:
		break;
	}
}

void PPBonusManager::update(PPBoard& board, std::vector<std::unique_ptr<PPPlayer>> & players)
{
	static int step1 = 0;
	if (step1++ < steps_delay) {
		return;
	}
	step1 = 0;

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
		bool flag = true;

		while (flag){

			while (board.at(pos).bonus || check_player(pos)) {
				pos = get_pos();
			}

			bool isInside = false;
			for (auto checkpoint : checkpoints) {
				if (abs(checkpoint->getCell().x - pos.x) < 2 && abs(checkpoint->getCell().y - pos.y) < 2){
					isInside = true;
					pos = get_pos();
					break;
				}
			}
			if (!isInside){
				flag = false;
			}
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
	}

	static int step2 = 0;
	if (step2++ < steps_delay) {
		return;
	}
	step2 = 0;

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
	}
	else {
		auto found = find(checkpoints.begin(), checkpoints.end(), bonus);
		if (found != checkpoints.end()) {
			checkpoints.erase(found);
		}
	}
}
    