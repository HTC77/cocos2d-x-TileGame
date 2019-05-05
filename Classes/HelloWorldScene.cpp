/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

Scene* HelloWorld::createScene()
{
	auto scene = Scene::create();
	auto layer = HelloWorld::create();
	scene->addChild(layer);
	auto hud = HudLayer::create();
	scene->addChild(hud);
	layer->_hud = hud;
	return scene;
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("pickup.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("hit.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("move.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic("TileMap.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("TileMap.mp3",true);

	winSize = Director::getInstance()->getWinSize();
	_numCollected = 0;
	// tile map
	_tileMap = TMXTiledMap::create("TileMap.tmx");
	_tileMap->retain();

	// background
	_background = _tileMap->getLayer("Background");
	_background->retain();
	
	// foreground
	_foreground = _tileMap->getLayer("Foreground");
	_foreground->retain();

	// meta
	_meta = _tileMap->getLayer("Meta");
	_meta->setVisible(false);
	_meta->retain();

	this->addChild(_tileMap);
	mapWidth = _tileMap->getMapSize().width;
	mapHeight = _tileMap->getMapSize().height;
	tileWidth = _tileMap->getTileSize().width;
	tileHeight = _tileMap->getTileSize().height;

	// player
	TMXObjectGroup *objectGroup = _tileMap->getObjectGroup("Objects");

	if (objectGroup == NULL) {
		CCLOG("tile map has no objects object layer");
		return false;
	}

	ValueMap spawnPoint = objectGroup->getObject("SpawnPoint");

	int x = spawnPoint.at("x").asInt();
	int y = spawnPoint.at("y").asInt();

	_player = Sprite::create();
	_player->initWithFile("Player.png");
	_player->setPosition(Vec2(x, y));

	this->addChild(_player);
	this->setViewPointCenter(_player->getPosition());

	// touch listener
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::touchBegan, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(HelloWorld::touchEnded, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(
		touchListener, this);

    return true;
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void HelloWorld::setViewPointCenter(Vec2 position)
{
	int x = MAX(position.x, winSize.width / 2);
	int y = MAX(position.y, winSize.height / 2);
	x = MIN(x, (mapWidth * tileWidth) - winSize.width / 2);
	y = MIN(y, (mapHeight * tileHeight) - winSize.height / 2);
	Vec2 actualPosition = Vec2(x, y);

	Vec2 centerOfView = Vec2(winSize.width / 2, winSize.height / 2);
	Vec2 viewVec2 = centerOfView - actualPosition;
	this->setPosition(viewVec2);
}

bool HelloWorld::touchBegan(Touch* touch, Event* event)
{
	return true;
}

void HelloWorld::touchEnded(Touch* touch, Event* event)
{
	Vec2 touchLocation = touch->getLocationInView();
	touchLocation = Director::getInstance()->convertToGL(touchLocation);
	touchLocation = this->convertToNodeSpace(touchLocation);

	Vec2 playerPos = _player->getPosition();
	Vec2 diff = touchLocation - playerPos;

	if (abs(diff.x) > abs(diff.y)) {
		if (diff.x > 0) {
			playerPos.x += tileWidth;
		}
		else {
			playerPos.x -= tileWidth;
		}
	}
	else {
		if (diff.y > 0) {
			playerPos.y += tileHeight;
		}
		else {
			playerPos.y -= tileHeight;
		}
	}

	// safety check on the bounds of the map
	if (playerPos.x <= (mapWidth * tileWidth) &&
		playerPos.y <= (mapHeight * tileHeight) &&
		playerPos.y >= 0 &&
		playerPos.x >= 0)
	{
		setPlayerPosition(playerPos);
	}

	this->setViewPointCenter(_player->getPosition());
}

void HelloWorld::setPlayerPosition(Vec2 position)
{
	Vec2 tileCoord = this->tileCoordForPosition(position);
	int tileGid = 0;
	tileGid = static_cast<int>(_meta->getTileGIDAt(tileCoord));
	if (tileGid) {
		ValueMap properties = _tileMap->getPropertiesForGID(tileGid).asValueMap();
		if (!properties.empty()) {
			if (properties.find("Collidable") != properties.end()){
				 std::string collision = properties.at("Collidable").asString();
				 if (collision.c_str() && (collision == "True")) {
					 CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("hit.mp3");
				 	return;
				 }
			}
			if (properties.find("Collectable") != properties.end()){
				 std::string collision = properties.at("Collectable").asString();
				 if (collision.c_str() && (collision == "True")) {
					 CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("pickup.mp3");
					 _meta->removeTileAt(tileCoord);
					_foreground->removeTileAt(tileCoord);
					_numCollected++;
					_hud->numCollectedChanged(_numCollected);
				 }
			}
		}
	}
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("move.mp3");
	_player->setPosition(position);
}

Vec2 HelloWorld::tileCoordForPosition(Vec2 position)
{
	int x = position.x / tileWidth;
	int y = ((mapHeight * tileHeight) - position.y) / tileHeight;
	return Vec2(x, y);
}
