/*
 * 盤面を動かして、良い手を探しに行く機能が集める機能が集まるプログラムです
 */

#include "sDrunk.hpp"
//#include "Search.hpp"
#include "ScoreBoard.hpp"
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <time.h>

//ostream *debug;
bool timerFlag = true;
clock_t timerStart;
clock_t timerStop;

void showTime(int n)
{
    timerStop = clock();
    *debug << "No " << n << " : " << (double)(timerStop - timerStart) / CLOCKS_PER_SEC << endl;  
}

//命令番号を入れるとコストを返してくれる
int costs[] = {0, 4, 4, 4, 4, 2, 2, 2, 2, 1};

// class GameSearch
// {
//   private:
//     double score;
//     GameState gs;
//     string command;
//     int cost;
//     vector< vector<int> > useCommand;
//     vector< pair<int,int> > beaconPoint;

//   public:
//     GameSearch(GameState *gamestate, double sc, Analysis *analysis);
//     GameSearch(const GameSearch &gs);
//     GameState * getGameStateRef();
//     void setScore(double sc);
//     void addScore(double sc);
//     double getScore();
//     void setCommand(string cmd);
//     string getCommand();
//     void setCost(int n);
//     bool checkCost(int n);
//     bool isHidden(int team, int weapon);
//     bool checkAction(int team, int samurai, int n);
//     GameSearch doAction(int team, int samurai, int n);
//     void addCost(int n);
//     void addCommand(int n);
//     vector<int> *getAction(int weapon);

//     GameSearch &operator=(const GameSearch &gSearch)
//     {
//         score = gSearch.score;
//         gs = gSearch.gs;
//         command = gSearch.command;
//         cost = gSearch.cost;
//         useCommand = gSearch.useCommand;

//         return *this;
//     }

//     //デバッグ用
//     void debugStage();
//     void showCommand();
// };

bool compGameSearch(GameSearch a, GameSearch b)
{
    return a.getScore() > b.getScore();
}

GameSearch::GameSearch(GameState *gamestate, double sc, Analysis *analysis)
{
    gs = *gamestate;
    score = sc;
    command = "";
    cost = 0;
    useCommand.resize(3);
    beaconPoint.resize(3);
    vector<SamuraiState> *ss = gs.getSamuraiStatesRef();
    for(int i = 0; i < 3; ++i)
    {
        SamuraiState samurai = ss->at(i);
        pair<int,int> beacon = analysis->getAction(i);
        bool dashFlag = analysis->getDashFlag(i);
        beaconPoint.at(i) = beacon;
        for(int n = 1; n <= 4 && !dashFlag ; ++n)
        {
            useCommand.at(i).push_back(n);
        }
        if(samurai.y == -1)
        {
            useCommand.at(i).push_back(5);
            useCommand.at(i).push_back(7);
        }
        else if(samurai.y < beacon.second)
        {
            useCommand.at(i).push_back(5);   
        }
        else if(samurai.y > beacon.second)
        {
            useCommand.at(i).push_back(7);   
        }
        if(samurai.x == -1)
        {
            useCommand.at(i).push_back(6);
            useCommand.at(i).push_back(8);
        }
        else if(samurai.x < beacon.first)
        {
            useCommand.at(i).push_back(6);   
        }
        else if(samurai.x > beacon.second)
        {
            useCommand.at(i).push_back(8);   
        }
    }
}

GameSearch::GameSearch(const GameSearch &gSearch)
{
    score = gSearch.score;
    gs = gSearch.gs;
    command = gSearch.command;
    cost = gSearch.cost;
    useCommand = gSearch.useCommand;
    beaconPoint = gSearch.beaconPoint;
}

GameState * GameSearch::getGameStateRef()
{
    return &gs;
}

void GameSearch::setScore(double sc)
{
    score = sc;
}

void GameSearch::addScore(double sc)
{
    score += sc;
}

double GameSearch::getScore()
{
    return score;
}

void GameSearch::setCommand(string cmd)
{
    command = cmd;
}

string GameSearch::getCommand()
{
    return command + " 0";
}

void GameSearch::setCost(int n)
{
    cost = n;
}

bool GameSearch::checkCost(int n)
{
    return cost + costs[n] <= 7;
}

bool GameSearch::isHidden(int team, int weapon)
{
    SamuraiState targetSamurai = *gs.getSamuraiRef(team, weapon);
    return targetSamurai.hidden == 1;
}

bool GameSearch::checkAction(int team, int weapon, int n)
{
    bool isValid = isValidAction(&gs, team, weapon, n);
    return isValid;
}

GameSearch GameSearch::doAction(int team, int weapon, int n)
{
    GameSearch nextSearch = *this;
    ScoreBoard sb;
    //nextSearch.gs.moveSamurai(team, weapon, n);
    simulateAction(nextSearch.getGameStateRef(), team, weapon, n, beaconPoint.at(weapon), &sb);
    //double sc = evaluate(&(nextSearch.gs));
    double sc = sb.getTotalScore(weapon);
    nextSearch.addScore(sc);
    nextSearch.addCost(n);
    nextSearch.addCommand(n);
    //sb.showScore();
    return nextSearch;
}

void GameSearch::addCost(int n)
{
    cost += costs[n];
}

void GameSearch::addCommand(int n)
{
    if (command != "")
    {
        command += " ";
    }
    command += '0' + n;
}

vector<int> * GameSearch::getAction(int weapon)
{
    return &(useCommand.at(weapon));
}

//デバッグ用
void GameSearch::debugStage()
{
    gs.showField();
}

void GameSearch::showCommand()
{
    *debug << "command : " << command << endl;
}

vector<GameSearch> createPattern(queue<GameSearch> *states, int weapon)
{
    vector<GameSearch> result;
    GameSearch gSearch = states->front();

    //隠れる行動は命令の最初と最後だけでいい
    if (gSearch.isHidden(0, weapon))
    {
        if (gSearch.checkCost(9) && gSearch.checkAction(0, weapon, 9))
        {
            GameSearch newGSearch = gSearch.doAction(0, weapon, 9);
            result.push_back(newGSearch);
            states->push(newGSearch);
        }
    }
    
    while (!states->empty())
    {
        gSearch = states->front();
        states->pop();
        vector<int> *actions = gSearch.getAction(weapon);
        for (int n : *actions)
        {
            //*debug << "weapon : " << weapon << " n : " << n << endl;
            if (gSearch.checkCost(n) && gSearch.checkAction(0, weapon, n))
            {
                //*debug << "weapon : " << weapon << " action : " << n << endl;
                GameSearch newGSearch = gSearch.doAction(0, weapon, n);
                result.push_back(newGSearch);
                //newGSearch.showCommand();
                states->push(newGSearch);
            }
        }
    }

    //結果の中で隠れていないものは隠れるパターンも作る
    vector<GameSearch> hiddenState;
    for (GameSearch tgs : result)
    {
        if (!tgs.isHidden(0, weapon))
        {
            if (tgs.checkCost(9) && tgs.checkAction(0, weapon, 9))
            {
                GameSearch newGSearch = tgs.doAction(0, weapon, 9);
                hiddenState.push_back(newGSearch);
                //states->push(newGSearch);
            }
        }
    }

    result.insert(result.end(), hiddenState.begin(), hiddenState.end());
    return result;
}

string getCommand(GameState *gs, Analysis *an)
{
    if (timerFlag)
    {
        timerStart = clock();
        *debug << "start timer" << endl;
    }
    string result;
    vector<GameSearch> lookedStates;
    queue<GameSearch> states;
    //double sc = evaluate(gs);
    double sc = 0;
    GameSearch firstState(gs, sc, an);
    if (timerFlag)
    {
        showTime(1);
    }

    for (int weapon = 0; weapon < 3; ++weapon)
    {
        GameSearch tSearch = firstState;
        tSearch.addCommand(weapon);
        states.push(tSearch);
        vector<GameSearch> tv = createPattern(&states, weapon);
        lookedStates.insert(lookedStates.end(), tv.begin(), tv.end());
    }

    if (lookedStates.empty())
    {
        if (timerFlag)
        {
            showTime(2);
        }
        return "0 0";
    }

    ScoreBoard sb;
    reviseStatePoint(&lookedStates, an, &sb);
    sort(lookedStates.begin(), lookedStates.end(), compGameSearch);
    
    if (timerFlag)
    {
        showTime(2);
    }

    return lookedStates.at(0).getCommand();
}
