/*
 * 盤面の評価
 * 他に評価したいことがあれば鑵子を追加してください
 */


#include "sDrunk.hpp"

double territoryPoint = 1.0;
double koPoint = 1.0;
double hiddenPoint = 1.0;

//マップの点数計算
double evaluateStage(int field[stageWidth][stageHeight])
{
    int score = 0;
    for(int x = 0; x < stageWidth; ++x)
    {
        for(int y = 0; y < stageHeight; ++y)
        {
            int num = field[x][y];
            if(num < 3)
            {
                score += 1 * territoryPoint;
            }
            else if(num < 6)
            {
                //敵の領地だったとき
            }
            else
            {
                //それ以外
            }
        }
    }

    return score;
}

//敵を倒した時の得点
double evaluateSamuraiState(SamuraiState samuraiStates[2][3])
{
    double score = 0;
    for(int team = 0; team < 2; ++team)
    {
        for(int weapon = 0; weapon < 3; ++weapon)
        {
            SamuraiState& ss = samuraiStates[team][weapon];
            //やられている状態を治療ターンの有無で調べる
            if(ss.recovery > 0)
            {
                double p = 1.0 * koPoint;
                score += team == 0 ? -1 * p : p;
            }
            if(ss.hidden != 0)
            {
                double p = 1.0 * hiddenPoint;
                score += team == 0 ? -1 * p : p;  
            }
        }
    }
    return score;
}

//総合得点計算
double evaluate(GameState *gs)
{
    //合計得点
    double totalScore = 0;
    //マップに関する得点
    int field[stageWidth][stageHeight];
    gs->getField(field);
    totalScore += evaluateStage(field);
    //侍に関する得点
    SamuraiState ss[2][3];
    gs->getSamuraiStates(ss);
    totalScore += evaluateSamuraiState(ss);

    return totalScore;
}