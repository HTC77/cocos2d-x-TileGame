#ifndef __TileGame__HudLayer__
#define __TileGame__HudLayer__

#include "cocos2d.h"
USING_NS_CC;

class HudLayer : public Layer
{
public:

    virtual bool init();
    
    Size winSize;

    CREATE_FUNC(HudLayer);
	void numCollectedChanged(int numCollected);
private:
	Label* _label;
};

#endif // __TileGame__HudLayer__
