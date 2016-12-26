/*
 * 盤面の評価
 * 他に評価したいことがあれば鑵子を追加してください
 */


#include "sDrunk.hpp"

double territoryPoint = 1;
double koPoint = 1;
double hiddenPoint = 1;

ScoreBoard::ScoreBoard()
{
    mapScore = 0;
    samuraiScore = 0;
    hiddingScore = 0;
}

void ScoreBoard::setScores(double mScore, double sScore, double hScore)
{
    mapScore = mScore;
    samuraiScore = sScore;
    hiddingScore = hScore;
}

double ScoreBoard::getTotalScore()
{
    return mapScore + samuraiScore + hiddingScore;
}


//マップの点数計算
double evaluateStage(vector<int> field)
{
    int score = 0;
    for(int x = 0; x < stageWidth; ++x)
    {
        for(int y = 0; y < stageHeight; ++y)
        {
            int num = field[x * stageHeight + y];
            if(num < 3)
            {
                score += (1 * territoryPoint);
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
double evaluateSamuraiState(vector<SamuraiState> samuraiStates)
{
    double score = 0;
    for(int team = 0; team < 2; ++team)
    {
        for(int weapon = 0; weapon < 3; ++weapon)
        {
            SamuraiState& ss = samuraiStates[team*3+weapon];
            //やられている状態を治療ターンの有無で調べる
            if(ss.recovery > 0)
            {
                double p = 1 * koPoint;
                score += team == 0 ? (-1 * p) : p;
            }
            if(ss.hidden != 0)
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
    //int field[stageWidth][stageHeight];
    vector<int> * field = gs->getFieldRef();
    double stageScore = evaluateStage(*field);
    totalScore += stageScore;
    //侍に関する得点
    //SamuraiState ss[2][3];
    //gs->getSamuraiStates(ss);
    vector<SamuraiState> * ss = gs->getSamuraiStatesRef();
    double samuraiScore = evaluateSamuraiState(*ss);
    totalScore += samuraiScore;
    // gs->showSamurai();
    // gs->showField();
    // *debug << "Stage Score : " <<  stageScore
    //        << " Samurai Score : " << samuraiScore << endl << endl;

    return totalScore;
}
