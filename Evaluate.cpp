/*
 * 盤面の評価
 * 他に評価したいことがあれば鑵子を追加してください
 */

double territoryPoint = 0;
double koPoint = 0;
double hiddenPoint = 0;

#include "sDrunk.hpp"
#include "ScoreBoard.hpp"

bool evalFlag = false;

//マップの点数計算
double evaluateStage(vector<int> field)
{
    int score = 0;
    for (int x = 0; x < stageWidth; ++x)
    {
        for (int y = 0; y < stageHeight; ++y)
        {
            int num = field[x * stageHeight + y];
            if (num < 3)
            {
                score += (1 * territoryPoint);
            }
            else if (num < 6)
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
double evaluateSamuraiState(vector<SamuraiState> samuraiStates)
{
    double score = 0;
    for (int team = 0; team < 2; ++team)
    {
        for (int weapon = 0; weapon < 3; ++weapon)
        {
            SamuraiState &ss = samuraiStates[team * 3 + weapon];
            //やられている状態を治療ターンの有無で調べる
            if (ss.recovery > 0)
            {
                double p = 1 * koPoint;
                score += team == 0 ? (-1 * p) : p;
            }
            if (ss.hidden != 0)
            {
                double p = 1 * hiddenPoint;
                score += (team == 0) ? 0 : p;
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
    vector<int> *field = gs->getFieldRef();
    double stageScore = evaluateStage(*field);
    totalScore += stageScore;
    //侍に関する得点
    vector<SamuraiState> *ss = gs->getSamuraiStatesRef();
    double samuraiScore = evaluateSamuraiState(*ss);
    totalScore += samuraiScore;
    if (evalFlag)
    {
        gs->showSamurai();
        gs->showField();
        *debug << "Stage Score : " << stageScore
               << " Samurai Score : " << samuraiScore << endl
               << endl;
    }

    return totalScore;
}
