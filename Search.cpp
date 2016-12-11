/*
 * 盤面を動かして、良い手を探しに行く機能が集める機能が集まるプログラムです
 */

#include "sDrunk.hpp"
#include <string>
#include <vector>
#include <queue>

//命令番号を入れるとコストを返してくれる
int costs[] = {0,4,4,4,4,2,2,2,2,1}

class GameSerch
{
    private:
    double score;
    GameState *gs;
    string command;
    int cost;

    public:
    GameSerch(GameState *gamestate, double sc);
    double getScore();
    string getCommand();
    bool checkCost(int n);
    bool checkAction(int samurai, int n);
    GameSerch doAction(int samurai, int n);
    void addCommand(int n);
};

bool compGameSerch(GameSerch a, GameSerch b)
{
    return a.getScore() > b.getScore();
}

GameSerch::GameSerch(GameState *gamestate, double sc)
{
    gs = gamestate;
    score = sc;
    command = "";
    cost = 0;
}

GameSerch::getScore()
{
    return score;
}

GameSerch::getCommand()
{
    return command + " 0";
}

GameSerch::checkCost(int n)
{
    return cost + costs[n] <= 7; 
}

bool checkAction(int team, int weapon, int n)
{
    return gs->isValidAction(team, weapon, n);
}

GameSerch doAction(int team, int weapon, int n)
{
    GameState nextState = *gs;
    nextState.moveSamurai(team, weapon, n);
    GameSerch nextSerch( nextState, evaluate(nextState) );
    nextSerch.addCommand(n);
    return nextSerch;
}

GameSerch::addCommand(int n)
{
    cost += costs[n];
    if(command != "")
    {
        command += " ";
    }
    command += '0' + n;
}

string getCommand(GameState *gs)
{
    string result;
    vector<GameSerch> lookedStates;
    queue<GameSerch> states;
    GameSerch firstState(gs, evalute(gs) );
    states.push(firstState);

    while( !states.empty() )
    {
        GameSerch gSerch = states.front();
        lookedStates.push_back( gSerch );
        states.pop();
        for(int weapon = 0; weapon < 3; +weapon)
        {
            for(int n = 1; n <= 9; ++n)
            {
                if( gSerch.checkCost(n) && gState.checkAction(n) )
                {
                    GameSerch newGSerch = gSerch.doAction();
                    lookedStates.push_back(newGSerch);
                    states.push(newGSerch);
                }
            }
        }
    }

    if(lookedStates.empty())
    {
        return "0";
    }

    sort(lookedStates.begin(), lookedStates.end(), compGameSerch);

    return lookedStates.get(0).getCommand();

}