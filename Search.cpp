/*
 * 盤面を動かして、良い手を探しに行く機能が集める機能が集まるプログラムです
 */

#include "sDrunk.hpp"
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

//ostream *debug;

//命令番号を入れるとコストを返してくれる
int costs[] = {0,4,4,4,4,2,2,2,2,1};

class GameSearch
{
    private:
    double score;
    GameState gs;
    string command;
    int cost;

    public:
    GameSearch(GameState *gamestate, double sc);
    GameSearch(const GameSearch &gs);
    double getScore();
    string getCommand();
    bool checkCost(int n);
    bool checkAction(int team, int samurai, int n);
    GameSearch doAction(int team, int samurai, int n);
    void addCost(int n);
    void addCommand(int n);

    GameSearch &operator=(const GameSearch &gSearch)
    {
      score = gSearch.score;
      gs = gSearch.gs;
      command = gSearch.command;
      cost = gSearch.cost;

      return *this;
    }
    
    //デバッグ用
    void debugStage();
};

bool compGameSearch(GameSearch a, GameSearch b)
{
    return a.getScore() > b.getScore();
}

GameSearch::GameSearch(GameState *gamestate, double sc)
{
    gs = *gamestate;
    score = sc;
    command = "";
    cost = 0;
}

GameSearch::GameSearch(const GameSearch &gSearch)
{
    score = gSearch.score;
    gs = gSearch.gs;
    command = gSearch.command;
    cost = gSearch.cost;
}

double GameSearch::getScore()
{
    return score;
}

string GameSearch::getCommand()
{
    return command + " 0";
}

bool GameSearch::checkCost(int n)
{
    return cost + costs[n] <= 7; 
}

bool GameSearch::checkAction(int team, int weapon, int n)
{
    bool isValid = gs.isValidAction(team, weapon, n);
    return isValid;
}

GameSearch GameSearch::doAction(int team, int weapon, int n)
{
    GameState nextState = gs;
    nextState.moveSamurai(team, weapon, n);
    double sc = evaluate(&nextState);
    GameSearch nextSearch( &nextState, sc );
    nextSearch.addCost(n);
    nextSearch.addCommand(n);
    // *debug << "============= return ===============" << endl;
    // nextSearch.debugStage();
    return nextSearch;
}

void GameSearch::addCost(int n)
{
    cost += costs[n];
}

void GameSearch::addCommand(int n)
{
    cost += costs[n];
    if(command != "")
    {
        command += " ";
    }
    command += '0' + n;
}

//デバッグ用
void GameSearch::debugStage()
{
    gs.showField();
}

string getCommand(GameState *gs)
{
    string result;
    vector<GameSearch> lookedStates;
    queue<GameSearch> states;
    double sc = evaluate(gs);
    GameSearch firstState(gs, sc);
    states.push(firstState);

    while( !states.empty() )
    {
        GameSearch gSearch = states.front();
        states.pop();

        for(int weapon = 0; weapon < 3; ++weapon)
        {
            for(int n = 1; n <= 8; ++n)
            {
                if( gSearch.checkCost(n) && gSearch.checkAction(0, weapon, n) )
                {
                    *debug << "weapon : " << weapon << " action : " << n << endl;
                    GameSearch newGSearch = gSearch.doAction(0, weapon, n);
                    // *debug << "============= catch ===============" << endl;
                    // newGSearch.debugStage();
                    lookedStates.push_back(newGSearch);
                    states.push(newGSearch);
                }
            }
        }
    }

    if(lookedStates.empty())
    {
        return "0";
    }

    sort(lookedStates.begin(), lookedStates.end(), compGameSearch);

    return lookedStates.at(0).getCommand();
}
