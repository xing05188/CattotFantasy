#pragma once
#include "cocos2d.h"
#include"Monster.h"
#include<vector>
#include<string>
#include<map>
USING_NS_CC;

#define TOWER_NUM 2

class BaseLevelScene;

class Tower {
protected:
	int index, grade;//炮塔序号和等级
	int range, demage;//当前防御塔可攻击的范围和伤害
public:
	Tower(int index_) :index(index_), grade(0) { range = range_table[index][grade]; demage = demage_table[index][grade]; }
	static std::string tower_table[TOWER_NUM][3];//记录所有炮塔的图片路径
	static std::string base_table[TOWER_NUM];//所有炮塔底座的图片路径
	static Vec2 anchorpoint_table[TOWER_NUM][2];//所有炮塔和底座锚点的设定
	static int build_cost[TOWER_NUM];//所有防御塔的建造花费
	static int up_cost[TOWER_NUM][2];//所有防御塔的升级花费
	static int range_table[TOWER_NUM][3];//所有防御塔的范围
	static int demage_table[TOWER_NUM][3];//所有防御塔的伤害
	static int sell_money[TOWER_NUM][3];//卖掉每座塔后增加的钱
	static std::map<int, std::string>sale_graph;//卖掉的图标
	static std::map<int, std::string>up_graph;//能买得起的图标
	static std::map<int, std::string>noup_graph;//买不起的图标
	static float interval_table[TOWER_NUM];

	Sprite* sprite_mark, * sp_base;
	Vec2 pos;//坐标
	float interval;//攻击间隔时间

	void build(BaseLevelScene*, Vec2);
	void destroy(BaseLevelScene*);
	//升级与卖掉部分
	static Sprite* curr_up, * curr_sale, * curr_range;
	void update(BaseLevelScene*, Vec2);
	void UpMenuAppear(BaseLevelScene* my_scene, Vec2& position);//升级菜单出现
	void UpMenuGone(BaseLevelScene*);//升级菜单消失
	int GetIndex()const { return index; }

	virtual void attack(BaseLevelScene*, std::vector<Monster*>&, int) = 0;
};


Tower* createTower(int index);



class Bottle :public Tower {
protected:
	static const float speed;//炮弹的速度,每秒多少像素
	float tower_angle;//炮塔朝向的角度，以向右为0度，逆时针增加
	Sprite* curr_shell;//指向子弹
	float curr_dis, de_time;
public:
	//构造函数
	Bottle(int index_) :Tower(index_), tower_angle(0) { }
	virtual void attack(BaseLevelScene*, std::vector<Monster*>&, int);//攻击怪物
	void ShellProduct(Scene* my_scene, Monster* monster);//将炮塔转到合适的方向，并产生炮弹，在炮口的正前方
	void ShellDemage(BaseLevelScene* my_scene, std::vector<Monster*>::iterator it);
	//攻击的特效，攻击时是否转向

	static std::string bottle_shell[3];
};


class Windmill :public Tower {
public:
	Windmill(int index_) :Tower(index_) {}
	virtual void attack(BaseLevelScene*, std::vector<Monster*>&,int);//攻击怪物
};
