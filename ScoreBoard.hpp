#include "Search.hpp"
using namespace std;

//点数表
class ScoreBoard
{
  private:
    //塗りの点数
    double mapScore[3];
    //侍状況の点数
    double samuraiScore[3];
    //隠れているときの点数
    double hiddingScore[3];
    //目標座標への距離
    double moveScore[3];
    //その他得点
    double etcPoint[3];
    //各種倍率
    double mapValue[3];
    double samuraiValue[3];
    double hiddingValue[3];
    double moveValue[3];
    //やられたときのマイナス値
    double deadPenalty;

  public:
    //ScoreBoard();
    ScoreBoard()
    {
        for (int i = 0; i < 3; ++i)
        {
            mapScore[i] = 0;
            samuraiScore[i] = 0;
            hiddingScore[i] = 0;
            moveScore[i] = 0;
            etcPoint[i] = 0;
        }
        //槍の設定
        mapValue[0] = 10;
        samuraiValue[0] = 100;
        hiddingValue[0] = 1;
        moveValue[0] = 1;
        //刀の設定
        mapValue[1] = 10;
        samuraiValue[1] = 100;
        hiddingValue[1] = 1;
        moveValue[1] = 1;
        //鉞の設定
        mapValue[2] = 10;
        samuraiValue[2] = 100;
        hiddingValue[2] = 1;
        moveValue[2] = 1;

        deadPenalty = -1000;
    }
    
    void setMapScore(int weapon, double mScore)
    {
        mapScore[weapon] = mScore;
    }
    
    void setSamuraiScore(int weapon, double sScore)
    {
        samuraiScore[weapon] = sScore;
    }
    
    void setHiddingScore(int weapon, double hScore)
    {
        hiddingScore[weapon] = hScore;
    }

    void setMoveScore(int weapon, double moScore)
    {
        moveScore[weapon] = moScore;
    }

    void addEtcPoint(int weapon, double point)
    {
        etcPoint[weapon] += point;
    }

    double getTotalScore(int i) const
    {
        return (mapScore[i] * mapValue[i]) + (samuraiScore[i] * samuraiValue[i])
             + (hiddingScore[i] * hiddingValue[i]) + (moveScore[i] * moveValue[i])
             + etcPoint[i];
    }

    double getDeadPenalty()
    {
        return deadPenalty;
    }

    //デバッグ用
    void showScore() const
    {
        for (int i = 0; i < 3; ++i)
        {
            *debug << "mapScore : " << mapScore[i] << endl
                   << "SamuraiScore : " << samuraiScore[i] << endl
                   << "hiddingScore : " << hiddingScore[i] << endl
                   << "Etc point : " << etcPoint[i] << endl
                   << "Total : " << getTotalScore(i) << endl
                   << endl;
        }
    }

    // bool operator<(const ScoreBoard &other)
    // {
    //     return this->getTotalScore() < other.getTotalScore();
    // }
};

//=========あまりよろしくないところにいる=====================

//行動可能か否か
bool isValidAction(GameState *gs, int team, int weapon, int action);
//シミュレーション
void simulateAction(GameState *gs, int team, int weapon, int action,
                    pair<int,int> bPoint, ScoreBoard *sb, Analysis *an);
//盤面の得点の補正
void reviseStatePoint(vector<GameSearch> *searchs, Analysis *an, ScoreBoard *sb);
