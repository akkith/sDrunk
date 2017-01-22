//#include "sDrunk.hpp"
//#include "ScoreBoard.hpp"

class GameSearch
{
  private:
    double score;
    GameState gs;
    string command;
    int cost;
    vector< vector<int> > useCommand;
    vector< pair<int,int> > beaconPoint;

  public:
    GameSearch(GameState *gamestate, double sc, Analysis *analysis);
    GameSearch(const GameSearch &gs);
    GameState * getGameStateRef();
    void setScore(double sc);
    void addScore(double sc);
    double getScore();
    void setCommand(string cmd);
    string getCommand();
    void setCost(int n);
    bool checkCost(int n);
    bool isHidden(int team, int weapon);
    bool checkAction(int team, int samurai, int n);
    GameSearch doAction(int team, int samurai, int n, Analysis * an);
    void addCost(int n);
    void addCommand(int n);
    vector<int> *getAction(int weapon);

    GameSearch &operator=(const GameSearch &gSearch)
    {
        score = gSearch.score;
        gs = gSearch.gs;
        command = gSearch.command;
        cost = gSearch.cost;
        useCommand = gSearch.useCommand;

        return *this;
    }

    //デバッグ用
    void debugStage();
    void showCommand();
};
