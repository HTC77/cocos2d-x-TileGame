#include "HudLayer.h"

bool HudLayer::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

    winSize = Director::getInstance()->getWinSize();
	_label = Label::createWithTTF("Score: 00", "fonts/Marker Felt.ttf", 28.0f);
	_label->setColor(Color3B(0, 0, 0));
	int margin = 10;
	_label->setPosition(Vec2(winSize.width - (_label->getContentSize().width / 2) - margin, _label->getContentSize().height / 2 + margin));
	this->addChild(_label);

    return true;
}

void HudLayer::numCollectedChanged(int numCollected)
{
	_label->setString(StringUtils::format("Score: %d", numCollected));
}

