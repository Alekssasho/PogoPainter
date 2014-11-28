//
//  PogoPainterResultScene.h
//  PogoPainter
//
//  Created by Asen Lekov on 11/8/14.
//
//

#ifndef __PogoPainter__PogoPainterResultScene__
#define __PogoPainter__PogoPainterResultScene__

#include "cocos2d.h"

using namespace cocos2d;
using namespace std;

class PogoPainterResults : public LayerColor
{
private:
	vector<int> results;
public:
    static Scene* createScene();
    bool init() override;
    
	void registerEventListener(EventListener* listener);

	void setResults(const vector<int>& points);
    
    CREATE_FUNC(PogoPainterResults);
};

#endif /* defined(__PogoPainter__PogoPainterResultScene__) */
