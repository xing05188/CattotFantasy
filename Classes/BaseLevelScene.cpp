#include "BaseLevelScene.h"
#include<vector>
#include"MonsterConfigs.h"
#include"Monster.h"
#include "json/document.h"
#include "json/rapidjson.h"
#include "ui/CocosGUI.h"
#include"music.h"
#include"themeScene.h"
#include"Tower.h"
#include"GameManager.h"
using namespace ui;
#define DEBUG_MODE

#define CELL_SIZE 64
const Color3B money_color(154, 101, 25);

std::string Tower::tower_table[TOWER_NUM][3] = { {"Towers/bottle0.png","Towers/bottle1.png","Towers/bottle2.png"},
    {"Towers/windmill0.png","Towers/windmill1.png","Towers/windmill2.png"} };
std::string Tower::base_table[TOWER_NUM] = { "Towers/bottlebase.png" ,"Towers/windmillbase.png" };
Vec2 Tower::anchorpoint_table[TOWER_NUM][2] = { {Vec2(0.5,0.5),Vec2(0.4,0.46)},{Vec2(0.5,0.7),Vec2(0.5,0.35)} };

int Tower::build_cost[TOWER_NUM] = { 100,160 };
int Tower::demage_table[TOWER_NUM][3] = { {20,35,50},{30,45,60} };
int Tower::range_table[TOWER_NUM][3] = { {200,300,400},{400,500,600} };

int Tower::up_cost[TOWER_NUM][2] = { {180,260},{220,260} };
int Tower::sell_money[TOWER_NUM][3] = { {80,224,432},{128,304,512} };

std::map<int, std::string> Tower::sale_graph = { {80,"Towers/sale_80.png"} ,{ 128,"Towers/sale_128.png" } ,
    { 144,"Towers/sale_144.png" } ,{ 224,"Towers/sale_224.png" }, { 304,"Towers/sale_304.png" },
    { 352,"Towers/sale_352.png" } ,{ 432,"Towers/sale_432.png" } ,{ 512,"Towers/sale_512.png" } ,
{ 608,"Towers/sale_608.png" } };

std::map<int, std::string> Tower::up_graph = { {180,"Towers/up_180.png"},{220,"Towers/up_220.png"},
    {260,"Towers/up_260.png"},{320,"Towers/up_320.png"} };

std::map<int, std::string> Tower::noup_graph = { {180,"Towers/noup_180.png"},{220,"Towers/noup_220.png"},
    {260,"Towers/noup_260.png"},{320,"Towers/noup_320.png"} };

Sprite* Tower::curr_sale;
Sprite* Tower::curr_up;
Sprite* Tower::curr_range;

const float Bottle::speed = 800;

float Tower::interval_table[TOWER_NUM] = { 0.8,0.8 };

std::string Bottle::bottle_shell[3] = { "Towers/shell1-1.png","Towers/shell1-2.png" ,"Towers/shell1-3.png" };
//游戏过程中不需要变动的量的初始化

extern Music a;
GameManager* manager;

extern bool level_is_win[3];
extern bool is_newgame[3];
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in BaseLevelScene.cpp\n");
}
void BaseLevelScene::updatemoney(int add) {
    money += add;
    std::string text = std::to_string(money); // 将数字转换为字符串
    money_lable->setString(text);
}
void BaseLevelScene::doublespeed(Ref* pSender) {
    isDoubleSpeed = !isDoubleSpeed; // 切换二倍速状态
    a.button_music();
    MenuItemImage* button = static_cast<MenuItemImage*>(pSender);
    if (isDoubleSpeed) {
        button->setNormalImage(Sprite::create("CarrotGuardRes/UI/doubleSpeed.png"));
        button->setSelectedImage(Sprite::create("CarrotGuardRes/UI/doubleSpeed.png"));
        scheduler->setTimeScale(2.0f); //实现加速效果
    }
    else {
        button->setNormalImage(Sprite::create("CarrotGuardRes/UI/normalSpeed.png"));
        button->setSelectedImage(Sprite::create("CarrotGuardRes/UI/normalSpeed.png"));
        scheduler->setTimeScale(1.0f); //实现减速效果
    }
}
void BaseLevelScene::pause_all(Ref* pSender) {
    isPaused = !isPaused; // 切换暂停状态
    a.button_music();
    MenuItemImage* button = static_cast<MenuItemImage*>(pSender);
    if (isPaused) {//暂停状态
        button->setNormalImage(Sprite::create("CarrotGuardRes/UI/continueButton.png"));
        button->setSelectedImage(Sprite::create("CarrotGuardRes/UI/continueButton.png"));
        // 添加顶部暂停标识
        auto pauseTop = Sprite::create("CarrotGuardRes/UI/pausing.png");
        pauseTop->setName("pauseTop");
        pauseTop->setPosition(450, 610);
        pauseTop->setScale(2.0f);
        this->addChild(pauseTop, 10);
        Director::getInstance()->pause();
    }
    else {//恢复
        button->setNormalImage(Sprite::create("CarrotGuardRes/UI/pauseButton.png"));
        button->setSelectedImage(Sprite::create("CarrotGuardRes/UI/pauseButton.png"));
        //移除暂停标识
        this->removeChildByName("pauseTop");
        Director::getInstance()->resume();
    }
}
void BaseLevelScene::menu_all(Ref* pSender) {
    a.button_music();
    Director::getInstance()->pause();
    auto visibleSize = Director::getInstance()->getVisibleSize();//分辨率大小
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Size screenSize = Director::getInstance()->getWinSize();    //获取屏幕大小
    //创建灰色遮罩层
    auto menuLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    menuLayer->setPosition(Vec2::ZERO);
    this->addChild(menuLayer, 10);
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [menuLayer](Touch* touch, Event* event) {
        return true;
        };
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, menuLayer);
    //添加暂停菜单背景
    auto menuBackground = Sprite::create("CarrotGuardRes/UI/gameMenu.png");
    menuBackground->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    menuBackground->setScale(1.5f);
    menuLayer->addChild(menuBackground, 0);
    auto menu = Menu::create();
    menu->setPosition(Vec2::ZERO);
    menuLayer->addChild(menu, 1);
    //添加暂停菜单上相关功能按钮
    auto continueButton = MenuItemImage::create("CarrotGuardRes/UI/continueNormal.png", "CarrotGuardRes/UI/continueSelected.png");
    continueButton->setPosition(Vec2(screenSize.width * 0.495, screenSize.height * 0.649));
    continueButton->setScale(1.5);
    auto restartButton = MenuItemImage::create("CarrotGuardRes/UI/restartNormal.png", "CarrotGuardRes/UI/restartSelected.png");
    restartButton->setPosition(Vec2(screenSize.width * 0.495, screenSize.height * 0.51));
    restartButton->setScale(1.5);
    auto chooseButton = MenuItemImage::create("CarrotGuardRes/UI/chooseLevelNormal.png", "CarrotGuardRes/UI/chooseLevelSelected.png");
    chooseButton->setPosition(Vec2(screenSize.width * 0.495, screenSize.height * 0.375));
    chooseButton->setScale(1.5);
    // 继续游戏选项
    continueButton->setCallback([this, menuLayer](Ref* psender) {
        a.button_music();
        this->removeChild(menuLayer);
        // 判断在点击菜单按钮之前是否点击过暂停按钮，防止出现bug
        if (isPaused == 0) {
            Director::getInstance()->resume();
        }
        });

    //重新开始游戏选项
    restartButton->setCallback([this, menuLayer](Ref* psender) {
        a.button_music();
        this->removeChild(menuLayer);
        auto scene = BaseLevelScene::createScene(levelId);   
        Director::getInstance()->replaceScene(scene);
        Director::getInstance()->resume();
        });

    //选择关卡选项
    chooseButton->setCallback([this, menuLayer](Ref* psender) {
        a.button_music();
        this->removeChild(menuLayer);
        auto themeScene = themescene::createScene();
        Director::getInstance()->replaceScene(themeScene);
        Director::getInstance()->resume();
        });
    menu->addChild(continueButton, 1);
    menu->addChild(chooseButton, 1);
    menu->addChild(restartButton, 1);
}

//倒计时
void BaseLevelScene::CountDown(std::function<void()> onComplete)
{

    auto countBackground = Sprite::create("CarrotGuardRes/UI/countBackground.png");
    auto count1 = Sprite::create("CarrotGuardRes/UI/countOne.png");
    auto count2 = Sprite::create("CarrotGuardRes/UI/countTwo.png");
    auto count3 = Sprite::create("CarrotGuardRes/UI/countThree.png");
    Label* count0 = Label::createWithSystemFont("GO", "Arial-BoldMT", 100);

    countBackground->setPosition(480, 320);
    count1->setPosition(480, 320);
    count2->setPosition(480, 320);
    count3->setPosition(480, 320);
    count0->setPosition(480, 320);

    countBackground->setVisible(false);
    count1->setVisible(false);
    count2->setVisible(false);
    count3->setVisible(false);
    count0->setVisible(false);

    this->addChild(countBackground, 2);
    this->addChild(count1, 2);
    this->addChild(count2, 2);
    this->addChild(count3, 2);
    this->addChild(count0, 2);
    // 设置倒数sequence动作
    auto countdown = Sequence::create(
        CallFunc::create([=] {
            countBackground->setVisible(true);
            count3->setVisible(true);
            a.countSound();
            }),
        DelayTime::create(1),

        CallFunc::create([=] {
            this->removeChild(count3);
            }),
        CallFunc::create([=] {
            count2->setVisible(true);
            a.countSound();
            }),
        DelayTime::create(1),

        CallFunc::create([=] {
            this->removeChild(count2);
            }),
        CallFunc::create([=] {
            count1->setVisible(true);
            a.countSound();
            }),
        DelayTime::create(1),

        CallFunc::create([=] {
            this->removeChild(count1);
            count0->setVisible(true);
            a.countSound();
            }),
        DelayTime::create(1),

        CallFunc::create([=] {
            this->removeChild(count0);
            this->removeChild(countBackground);

            // 倒计时结束，触发回调
            if (onComplete) {
                onComplete();  // 执行回调函数
            }
            }),
        nullptr  // Sequence结束
    );

    // 运行倒计时动作
    this->runAction(countdown);
}
// 定义关卡地图文件路径数组
const std::vector<std::string> BaseLevelScene::mapFiles = {
    "map/map1.tmx",  // 关卡 1
    "map/map2.tmx",  // 关卡 2
    "map/map3.tmx",  // 关卡 3
    // 可以继续添加其他关卡的地图文件路径
};

// 创建场景时传入关卡编号
Scene* BaseLevelScene::createScene(int level) {
    auto scene = BaseLevelScene::create();  // 创建 BaseLevelScene
    if (scene && scene->initWithLevel(level))
    {
        return scene;
    }
    return scene;
}
//初始化ui组件
void BaseLevelScene::initUI()
{
    //加money
    money_lable = Label::createWithTTF("1000", "fonts/arial.ttf", 27);
    money_lable->setPosition(Vec2(160, 610));
    this->addChild(money_lable, 3);
    //添加返回按钮
    auto menu = Menu::create();
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    //倍数
    auto doubleSpeedButton = MenuItemImage::create("CarrotGuardRes/UI/normalSpeed.png", "CarrotGuardRes/UI/doubleSpeed.png", CC_CALLBACK_1(BaseLevelScene::doublespeed, this));
    if (doubleSpeedButton == nullptr)
    {
        problemLoading("'normalSpeed.png' and 'doubleSpeed.png'");
    }
    else
    {
        doubleSpeedButton->setPosition(670, 610);
        doubleSpeedButton->setScale(1);
        menu->addChild(doubleSpeedButton);
    }
    //暂停
    auto pauseSpeedButton = MenuItemImage::create("CarrotGuardRes/UI/pauseButton.png", "CarrotGuardRes/UI/continueButton.png", CC_CALLBACK_1(BaseLevelScene::pause_all, this));
    if (pauseSpeedButton == nullptr)
    {
        problemLoading("'pauseButton.png' and 'continueButton.png'");
    }
    else
    {
        pauseSpeedButton->setPosition(770, 610);
        pauseSpeedButton->setScale(1);
        menu->addChild(pauseSpeedButton);
    }
    //菜单
    auto menuButton = MenuItemImage::create("CarrotGuardRes/UI/gameMenuNormal.png", "CarrotGuardRes/UI/gameMenuSelected.png", CC_CALLBACK_1(BaseLevelScene::menu_all, this));
    if (menuButton == nullptr)
    {
        problemLoading("'gameMenuNormal.png' and 'gameMenuSelected'");
    }
    else
    {
        menuButton->setPosition(870, 610);
        menuButton->setScale(1);
        menu->addChild(menuButton);
    }
    /*
    auto jineng1Button = MenuItemImage::create("Carrot/jineng1.png", "Carrot/jineng1.png", CC_CALLBACK_1(BaseLevelScene::Jineng1, this));
    if (jineng1Button == nullptr)
    {
        problemLoading("'jineng1.png'");
    }
    else
    {
        jineng1Button->setPosition(800, 100);
        jineng1Button->setScale(2);
        menu->addChild(jineng1Button);
    }
    auto jineng2Button = MenuItemImage::create("Carrot/jineng2.png", "Carrot/jineng2.png", CC_CALLBACK_1(BaseLevelScene::Jineng2, this));
    if (jineng2Button == nullptr)
    {
        problemLoading("'jineng2.png'");
    }
    else
    {
        jineng2Button->setPosition(870, 100);
        jineng2Button->setScale(2);
        menu->addChild(jineng2Button);
    }
    */
}
//更新
void BaseLevelScene::update(float deltaTime) {
    manager->update(deltaTime);
    if(manager->CheckLose())
    {
   // gameover(false);
    }
    for (auto it = towers.begin(); it != towers.end(); it++) {

        if (it->second->interval >= it->second->interval_table[it->second->GetIndex()]) {
            /* Director::getInstance()->end();*/
             /*return;*/
            it->second->attack(this, GameManager::getInstance()->monsters);
        }

        it->second->interval += deltaTime;
    }
}
//初始化关卡
bool BaseLevelScene::initWithLevel(int level)
{
    if (!Scene::init())
    {
        return false;
    }
    initUI();                                  //初始化ui
    this->levelId = level;                     //存储关卡编号
    this->loadMap();                           // 加载对应关卡的地图
    manager = GameManager::getInstance(this);  // 初始化GameManager
    manager->initLevel(level);                 //初始化怪兽 路径和存档等资源
    // 调用倒计时函数并传递回调
    CountDown([=] {
              //计时结束后才能开始怪兽波
        manager->startMonsterWaves();
        });
    this->scheduleUpdate();                    //启动更新逻辑




    // 1. 创建植物图层
    plantsLayer = Layer::create();  // 创建一个新的图层，用于存放植物
    this->addChild(plantsLayer, 10); // 将植物图层添加到场景，zOrder为3，确保它位于其他层之上
    addMouseListener();  // 添加鼠标监听

    cell_flag = 1;
    buy_tower[0].push_back("Towers/affordbottle.png"); buy_tower[1].push_back("Towers/unaffordbottle.png");
    buy_tower[0].push_back("Towers/affordwindmill.png"); buy_tower[1].push_back("Towers/unaffordwindmill.png");
    index_table.push_back(0);
    index_table.push_back(1);
    InitMapData();
    return true;
}

// 默认的 init 方法
bool BaseLevelScene::init() {
    if (!Scene::init()) {
        return false;
    }
    return true;
}
/**************************************************
 *****************地图相关*************************
 **************************************************/
 // 加载地图的函数
void BaseLevelScene::loadMap() {
    if (levelId <= 0 || levelId > mapFiles.size()) {
        CCLOG("Invalid level number!");
        return;
    }

    // 加载关卡地图
    tileMap = TMXTiledMap::create(mapFiles[levelId - 1]);
    if (tileMap) {
        // 获取屏幕大小
        auto screenSize = Director::getInstance()->getVisibleSize();
        auto tileMapSize = tileMap->getContentSize();
        //获取瓦片大小
        tileSize = tileMap->getTileSize();
        CCLOG("Tile size: (%f, %f)", tileSize.width, tileSize.height);
        // 缩放瓦片地图，使其填满整个屏幕
        tileMap->setScaleX(screenSize.width / tileMapSize.width);
        tileMap->setScaleY(screenSize.height / tileMapSize.height);

        // 设置瓦片地图的锚点为 (0, 0)，使地图左下角与屏幕左下角对齐
        tileMap->setAnchorPoint(Vec2::ZERO);
        tileMap->setPosition(Vec2::ZERO);
        // 获取并隐藏 plantable 层
        plantableLayer = tileMap->getLayer("plantable");
        if (plantableLayer) {
            plantableLayer->setVisible(false);  // 不显示 plantable 层
        }
        else {
            CCLOG("Layer 'plantable' not found in map: %s", mapFiles[levelId - 1].c_str());
        }
        // 将瓦片地图添加到当前场景中
        this->addChild(tileMap, 0);
#ifdef DEBUG_MODE
        drawGrid();
#endif
    }
    else {
        CCLOG("Failed to load map: %s", mapFiles[levelId - 1].c_str());
    }
}

/**************************************************
 *****************炮塔相关*************************
 **************************************************/
 //开启鼠标监听
void BaseLevelScene::addMouseListener() {
    auto listener = EventListenerMouse::create();
    listener->onMouseDown = CC_CALLBACK_1(BaseLevelScene::handleMouseDown, this); // 鼠标按下时的回调

    // 将监听器添加到事件派发器
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}
//处理鼠标点击事件
void BaseLevelScene::handleMouseDown(EventMouse* event) {
    // 获取鼠标点击位置
    Vec2 mousePos = event->getLocation();
    CCLOG("Mouse clicked at screen position: (%f, %f)", mousePos.x, mousePos.y);
    // 调用 handlePlant 方法来处理种植
    handlePlant(mousePos);
}
//处理种植炮塔的判断
void BaseLevelScene::handlePlant(const Vec2& position) {
    CCLOG("position: (%f, %f)", position.x, position.y);
    // 将鼠标点击的屏幕坐标转换为瓦片坐标
    Vec2 tileCoord = Vec2(floor(position.x / tileSize.width), floor(position.y / tileSize.height));
    //下面日志证明已经实现了网格的转化
    CCLOG("tileCoord: (%f, %f)", tileCoord.x, tileCoord.y);

    // 边界检查
    if (tileCoord.x < 0 || tileCoord.y < 0 ||
        tileCoord.x >= tileMap->getMapSize().width ||
        tileCoord.y >= tileMap->getMapSize().height) {
        CCLOG("Tile coordinate is out of bounds.");
        return;
    }

    // 判断该瓦片是否可种植
    auto tileGID = tileMap->getLayer("plantable")->getTileGIDAt(tileCoord);

    if (!cell_flag) {//如果已经出现种植或者升级菜单，则再次点击时判断是否进行种植或升级(删除)操作，并使菜单消失
        if (map_data[int(last_position.x / CELL_SIZE)][int(last_position.y / CELL_SIZE)].flag)
            PlantMenuGone(Vec2(position.x, Director::getInstance()->getVisibleSize().height - position.y));
        else UpMenuGone(Vec2(position.x, Director::getInstance()->getVisibleSize().height - position.y));
        return;
    }

    if (tileGID != 0) {
        CCLOG("Tile at (%f, %f) is plantable.", tileCoord.x, tileCoord.y);
        // 这里实现种植逻辑
       // 将瓦片坐标转换为屏幕坐标
       // 获取地图的高度（瓦片的行数）
        float mapHeight = tileMap->getMapSize().height;

        // 转换为屏幕坐标
        float screenX = tileCoord.x * tileSize.width + tileSize.width / 2;
        float screenY = (mapHeight - tileCoord.y - 1) * tileSize.height + tileSize.height / 2;

        Vec2 mapPos = Vec2(screenX, screenY);

        // 输出屏幕坐标
        CCLOG("Screen Position: (%f, %f)", mapPos.x, mapPos.y);

        //如果这地方没有塔，那么打开种植菜单，否则打开升级菜单
        if (map_data[int(mapPos.x / CELL_SIZE)][int(mapPos.y / CELL_SIZE)].flag)
            PlantMenuAppear(mapPos);
        else UpMenuAppear(mapPos);
    }

    else
    {
        CCLOG("Tile at (%f, %f) is not plantable.", tileCoord.x, tileCoord.y);
    }

}
/**************************************************
 *****************工具函数*************************
 **************************************************/
 //画网格线
#ifdef DEBUG_MODE
void BaseLevelScene::drawGrid() {
    // 获取地图的大小和瓦片的大小
    Size mapSize = tileMap->getContentSize();
    float tileWidth = 64.0f;  // 假设每个瓦片宽度为 64 像素
    float tileHeight = 64.0f; // 假设每个瓦片高度为 64 像素

    // 创建一个 DrawNode 用于绘制网格
    auto drawNode = DrawNode::create();
    this->addChild(drawNode, 100);  // 将 DrawNode 添加到场景中，层级设置为 100，确保在地图之上

    // 绘制垂直线
    for (float x = 0; x <= mapSize.width; x += tileWidth) {
        drawNode->drawLine(Vec2(x, 0), Vec2(x, mapSize.height), Color4F::WHITE);
    }

    // 绘制水平线
    for (float y = 0; y <= mapSize.height; y += tileHeight) {
        drawNode->drawLine(Vec2(0, y), Vec2(mapSize.width, y), Color4F::WHITE);
    }
}
#endif // DEBUG_MODE

//瓦格坐标转地图坐标的工具函数
Vec2 BaseLevelScene::gridToScreenCenter(const Vec2& gridPoint) {
    // 从 tileMap 获取地图高度（网格数量）
    float mapHeight = tileMap->getMapSize().height;
    // 转换为屏幕坐标
    float screenX = gridPoint.x * tileSize.height + tileSize.width / 2;
    float screenY = (mapHeight - gridPoint.y - 1) * tileSize.height + tileSize.height / 2;
    return Vec2(screenX, screenY);
}





void BaseLevelScene::gameover(bool is_win) {
    Director::getInstance()->pause();
    // 设置灰色遮罩层
    auto menuLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    menuLayer->setPosition(Vec2::ZERO);
    this->addChild(menuLayer, 10);
    // 创建菜单
    auto menu = Menu::create();
    menu->setPosition(Vec2::ZERO);
    menuLayer->addChild(menu, 1);
    // 游戏胜利
    if (is_win) {
        level_is_win[levelId - 1] = true;
        //添加游戏获胜界面
        auto gameWinBackground = Sprite::create("CarrotGuardRes/UI/WinGame.png");
        gameWinBackground->setPosition(Vec2(480, 320));
        gameWinBackground->setScale(1.5f);
        menuLayer->addChild(gameWinBackground, 0);
        //添加获胜的金萝卜标识
        auto goldenCarrot = Sprite::create("CarrotGuardRes/UI/goldenCarrot.png");
        goldenCarrot->setPosition(Vec2(960 * 0.493, 640 * 0.7));
        menuLayer->addChild(goldenCarrot, 0);

        // 胜利的相关提示语
        /*
        _curNumberLabel = Label::createWithSystemFont(StringUtils::format("%d", _currNum > _monsterWave ? _monsterWave : _currNum), "Arial", 32);
        _curNumberLabel->setColor(Color3B::YELLOW);
        _curNumberLabel->setPosition(960 * 0.51, 640 * 0.54);
        */
        Label* loseWordLeft = Label::createWithSystemFont("fought off", "Arial", 30);
        loseWordLeft->setPosition(960 * 0.36, 640 * 0.54);
        Label* loseWordRight = Label::createWithSystemFont("waves", "Arial", 30);
        loseWordRight->setPosition(960 * 0.60, 640 * 0.545);

        //this->addChild(_curNumberLabel, 10);
        this->addChild(loseWordLeft, 10);
        this->addChild(loseWordRight, 10);
        //继续游戏按钮
        auto continueButton = MenuItemImage::create("CarrotGuardRes/UI/continueNormal.png", "CarrotGuardRes/UI/continueSelected.png");
        continueButton->setPosition(Vec2(960 * 0.613, 640 * 0.375));
        continueButton->setScale(1.38);
        continueButton->setCallback([this, menuLayer](Ref* psender) {
            a.button_music();
            //若当前未到开放的最后一关，则进行下一关
            if (levelId < 3) {
                auto scene = BaseLevelScene::createScene(levelId + 1);
                Director::getInstance()->replaceScene(scene);
            }
            //若当前已经是开放的最后一关，则返回选择关卡界面
            else {
                auto themeScene = themescene::createScene();
                Director::getInstance()->replaceScene(themeScene);
            }
            });
        menu->addChild(continueButton, 1);
    }
    // 游戏失败
    else {
        auto gameLoseBackground = Sprite::create("CarrotGuardRes/UI/LoseGame.png");
        gameLoseBackground->setPosition(Vec2(960 / 2 + 960 * 0.01, 640 / 2 + 640 * 0.015));
        gameLoseBackground->setScale(1.5f);
        menuLayer->addChild(gameLoseBackground, 0);

        // 游戏失败的相关提示语
        /*
        _curNumberLabel = Label::createWithSystemFont(StringUtils::format("%d", _currNum - 1), "Arial", 32);// 暂时没搞currnum为什么会大1，所以先-1
        _curNumberLabel->setColor(Color3B::YELLOW);
        _curNumberLabel->setPosition(960 * 0.51, 640 * 0.54);*/
        Label* loseWordLeft = Label::createWithSystemFont("fought off", "Arial", 30);
       
        loseWordLeft->setPosition(960 * 0.36, 640 * 0.54);
        Label* loseWordRight = Label::createWithSystemFont("waves", "Arial", 30);
        loseWordRight->setPosition(960 * 0.60, 640 * 0.54);

        //this->addChild(_curNumberLabel, 10);
        this->addChild(loseWordLeft, 10);
        this->addChild(loseWordRight, 10);
        //重新游戏按钮
        auto againButton = MenuItemImage::create("CarrotGuardRes/UI/AgainNormal.png", "CarrotGuardRes/UI/AgainSelected.png");
        againButton->setPosition(Vec2(960 * 0.61, 640 * 0.37));
        againButton->setScale(0.9);
        // 重新开始按钮的选项
        againButton->setCallback([this, menuLayer](Ref* psender) {
            a.button_music();
            this->removeChild(menuLayer);
            auto scene = BaseLevelScene::createScene(levelId);
            Director::getInstance()->replaceScene(scene);
            Director::getInstance()->resume();
            });
        menu->addChild(againButton, 1);
    }
    // 选择游戏关卡按钮
    auto chooseButton = MenuItemImage::create("CarrotGuardRes/UI/chooseLevelNormal.png", "CarrotGuardRes/UI/chooseLevelSelected.png");
    chooseButton->setPosition(Vec2(960 * 0.38, 640 * 0.37));
    chooseButton->setScale(1.4);
    //选择关卡选项
    chooseButton->setCallback([this, menuLayer](Ref* psender) {
        a.button_music();
        this->removeChild(menuLayer);
        auto themeScene = themescene::createScene();
        Director::getInstance()->replaceScene(themeScene);
        Director::getInstance()->resume();
        });
    menu->addChild(chooseButton, 1);
}



void BaseLevelScene::PlantMenuAppear(Vec2 mapPos)
{
    cell_flag = 0;
    auto location = mapPos;
    last_position = location;
    auto cell = Sprite::create("Towers/cell.png");
    cell->setScale((float)64 / 200);
    cell->setPosition(location);
    this->addChild(cell); curr_cell = cell;
    for (int i = 0, size = buy_tower[0].size(); i < size; i++) {
        int tool = Tower::build_cost[index_table[i]] > money ? 1 : 0;
        auto tower_graph = Sprite::create(buy_tower[tool][i]);
        tower_graph->setScale((float)CELL_SIZE / tower_graph->getContentSize().width);
        tower_graph->setPosition(Vec2(location.x - CELL_SIZE * (size - 1) / 2 + CELL_SIZE * i, location.y + CELL_SIZE));
        this->addChild(tower_graph); remove_table.push_back(tower_graph);
    }
}

void BaseLevelScene::PlantMenuGone(Vec2 position)
{
    auto location = position;
    int size = remove_table.size();
    if ((location.x > last_position.x - CELL_SIZE * size / 2 && location.x < last_position.x + CELL_SIZE * size / 2)
        && (location.y > last_position.y + CELL_SIZE / 2 && location.y < last_position.y + 3 * CELL_SIZE / 2)) {
        int index = index_table[(location.x - last_position.x + CELL_SIZE * size / 2) / CELL_SIZE];
        if (money >= Tower::build_cost[index]) {
            Tower* this_tower = createTower(index);
            this_tower->build(this, last_position);
            int x = int(last_position.x / CELL_SIZE), y = int(last_position.y / CELL_SIZE);
            towers[map_data[x][y].key] = this_tower;
            map_data[x][y].flag = 0;
        }
    }
    this->removeChild(curr_cell); curr_cell->release();
    for (int i = 0; i < remove_table.size(); i++) { this->removeChild(remove_table[i]); remove_table[i]->release(); }
    remove_table.clear();
    cell_flag = 1;
}


void BaseLevelScene::InitMapData()
{
    for (int i = 0; i < X; i++)
        for (int j = 0; j < Y; j++) { map_data[i][j].flag = 1; map_data[i][j].key = j * X + i; }
}

void BaseLevelScene::UpMenuAppear(Vec2& position)
{
    last_position = position;
    int key = map_data[int(position.x / CELL_SIZE)][int(position.y / CELL_SIZE)].key;
    towers[key]->UpMenuAppear(this, position);
    cell_flag = 0;
}

void BaseLevelScene::UpMenuGone(Vec2& position)
{
    int key = BaseLevelScene::map_data[int(last_position.x / CELL_SIZE)][int(last_position.y / CELL_SIZE)].key;
    towers[key]->UpMenuGone(this);
    if (position.x > last_position.x - CELL_SIZE / 2 && position.x < last_position.x + CELL_SIZE / 2 &&
        position.y > last_position.y - 3 * CELL_SIZE / 2 && position.y < last_position.y - CELL_SIZE / 2) {
        towers[key]->destroy(this);
        delete towers[key];
        towers.erase(key);
        map_data[int(last_position.x / CELL_SIZE)][int(last_position.y / CELL_SIZE)].flag = 1;
    }
    else if (position.x > last_position.x - CELL_SIZE / 2 && position.x < last_position.x + CELL_SIZE / 2 &&
        position.y > last_position.y + CELL_SIZE / 2 && position.y < last_position.y + 3 * CELL_SIZE / 2) {
        towers[key]->update(this, last_position);
    }
    cell_flag = 1;
}
