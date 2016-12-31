using namespace std;

//点数表
class ScoreBoard
{
  private:
    //盤面の点数
    double mapScore;
    //侍状況の点数
    double samuraiScore;
    //隠れているときの点数
    double hiddingScore;
    //各種倍率
    double mapValue;
    double samuraiValue;
    double hiddingValue;

  public:
    //ScoreBoard();
    ScoreBoard()
    {
        mapScore = 0;
        samuraiScore = 0;
        hiddingScore = 0;
        
        mapValue = 10;
        samuraiValue = 100;
        hiddingValue = 1;
    }
    //void setMapScore(double mScore);
    void setMapScore(double mScore)
    {
        mapScore = mScore;
    }
    //void setSamuraiScore(double sScore);
    void setSamuraiScore(double sScore)
    {
        samuraiScore = sScore;
    }
    //void setHiddingScore(double hScore);
    void setHiddingScore(double hScore)
    {
        hiddingScore = hScore;
    }
    //double getTotalScore() const;
    double getTotalScore() const
    {
        return (mapScore * mapValue) + (samuraiScore * samuraiValue) + (hiddingScore * hiddingValue);
    }

    //デバッグ用
    void showScore() const
    {
        *debug << "mapScore : " << mapScore << endl
                << "SamuraiScore : " << samuraiScore << endl
                << "hiddingScore : " << hiddingScore << endl
                << "Total : " << getTotalScore() << endl << endl;
    }

    bool operator<(const ScoreBoard &other)
    {
        return this->getTotalScore() < other.getTotalScore();
    }
};



//=========あまりよろしくないところにいる=====================

//行動可能か否か
bool isValidAction(GameState *gs, int team, int weapon, int action);
//シミュレーション
void simulateAction(GameState *gs, int team, int weapon, int action, ScoreBoard *sb);