/*
 * ゲームのステージ状況と命令をもらって動かしてみる
 * 得点表を入れるとそれに書き込んでくれる
 */

#include "sDrunk.hpp"
#include "ScoreBoard.hpp"
#include <vector>
#include <cmath>

//移動(1、南　２、東　３、北　４、西)
static int dx[] = {0, 0, 1, 0, -1};
static int dy[] = {0, 1, 0, -1, 0};

//各武器の攻撃マス数
static const int osize[3] = {4, 5, 7};

//各攻撃の範囲(下側正面x軸)
static const int ox[3][7] = {
    {0, 0, 0, 0},
    {0, 0, 1, 1, 2},
    {-1, -1, -1, 0, 1, 1, 1}};
//各攻撃の範囲（下側正面y軸）
static const int oy[3][7] = {
    {1, 2, 3, 4},
    {1, 2, 0, 1, 0},
    {-1, 0, 1, 1, 1, 0, -1}};


//与えられた侍が(nx,ny)に移動したとき、すでに他の侍がいないか調べる
bool onOtherSamurai(GameState *gs, SamuraiState *samurai, int nx, int ny)
{
    bool result = false;
    vector<SamuraiState> *sState = gs->getSamuraiStatesRef();
    for (SamuraiState ss : *sState)
    {
        if (*samurai != ss && ss.x == nx && ss.y == ny && ss.hidden == 0)
        {
            result |= true;
        }
    }
    return result;
}

bool isValidAction(GameState *gs, int team, int weapon, int action)
{
    SamuraiState *samurai = gs->getSamuraiRef(team, weapon);
    vector<int> *field = gs->getFieldRef();
    int x = samurai->x;
    int y = samurai->y;
    bool onOther = false;
    if (samurai->done == 1 || samurai->recovery > 0)
    {
        return false;
    }
    switch (action)
    {
    case 1:
    case 2:
    case 3:
    case 4:
        return samurai->hidden == 0;
    case 5:
    case 6:
    case 7:
    case 8:
        x += dx[action - 4];
        y += dy[action - 4];
        onOther = onOtherSamurai(gs, samurai, x, y);
        if (x < 0 || stageWidth <= x || y < 0 || stageHeight <= y || onOther)
        {
            return false;
        }
        if (samurai->hidden == 1 && field->at(y * stageHeight + x) >= 3)
        {
            return false;
        }
        
        return true;
        break;
    case 9:
        if (samurai->hidden == 0)
        {
            return field->at(y * stageHeight + x) < 3;
        }
        else
        {
            onOther = onOtherSamurai(gs, samurai, samurai->x, samurai->y);
            return !onOther;
        }
    }
    return false;
}

//ターンの更新シミュレーション
void turnUpdate(GameState * gs)
{
    gs->turnUpdate();   
}

//攻撃のシミュレーション
void doAttack(GameState *gs, SamuraiState *samurai, int action, ScoreBoard * sb, Analysis * an)
{
    //侍の座標
    int samuraiX, samuraiY;
    samuraiX = samurai->x;
    samuraiY = samurai->y;
    double nuriCnt = 0;
    double koCnt = 0;
    double etcScore = 0;
    //武器に合わせた攻撃マス数
    int attackSize = osize[samurai->weapon];
    for (int i = 0; i < attackSize; ++i)
    {
        int attackX, attackY;
        //向きに合わせた座標に変換
        rotate(action - 1, ox[samurai->weapon][i], oy[samurai->weapon][i], attackX, attackY);
        attackX += samuraiX;
        attackY += samuraiY;
        bool isHome = false;
        //攻撃座標に敵がいた
        //bool onEnemy = false;
        for (int team = 0; team < 2; ++team)
        {
            for (int weapon = 0; weapon < 3; ++weapon)
            {
                SamuraiState *ss = gs->getSamuraiRef(team, weapon);
                if (team == 1)
                {
                    //攻撃座標が拠点でない場合
                    //isHome = false;
                    if (attackX == ss->homeX && attackY == ss->homeY)
                    {
                        isHome |= true;
                    }
                    if (attackX == ss->x && attackY == ss->y && !isHome)
                    {
                        //onEnemy |= true;
                        ++koCnt;
                        ss->dead();
                    }
                }
            }
        }

        if (0 <= attackX && attackX < stageWidth && 0 <= attackY && attackY < stageHeight && !isHome)
        {
            vector<int> *field = gs->getFieldRef();
            if(field->at(attackY * stageHeight + attackX) >= 3)
            {
                ++nuriCnt;
                int heat = an->getHeat(attackX, attackY);
                if(heat >= 6)
                {
                     nuriCnt += 1;
                }
                etcScore += heat;
            }
            field->at(attackY * stageHeight + attackX) = samurai->weapon;  
        }
    }

    sb->setMapScore(samurai->weapon, nuriCnt);
    sb->setSamuraiScore(samurai->weapon, koCnt);
    sb->addEtcPoint(samurai->weapon, etcScore);
}

void simulateAction(GameState *gs, int team, int weapon, int action,
                    pair<int,int> bPoint, ScoreBoard * sb, Analysis * an)
{
    double hiddingScore = 0;
    //double diff = 0;
    SamuraiState *samurai = gs->getSamuraiRef(team, weapon);
    int diffX = abs(samurai->x - bPoint.first);
    int diffY = abs(samurai->y - bPoint.second);
    int beforeD = diffX + diffY;
    if (!isValidAction(gs, team, weapon, action))
    {
        return;
    }
    switch (action)
    {
    case 0:
        //ターン経過
        turnUpdate(gs);
    case 1:
    case 2:
    case 3:
    case 4:
        //攻撃
        doAttack(gs, samurai, action, sb, an);
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        //移動
        samurai->x += dx[action - 4];
        samurai->y += dy[action - 4];
        //++diff;
        break;
    case 9:
        //潜伏
        samurai->hidden = samurai->hidden == 0 ? 1 : 0;
        ++hiddingScore;
        break;
    default:
        break;
    }
    
    diffX = abs(samurai->x - bPoint.first);
    diffY = abs(samurai->y - bPoint.second);
    int afterD = diffX + diffY;
    sb->setHiddingScore(weapon, hiddingScore);
    sb->setMoveScore(weapon, beforeD - afterD);
}
