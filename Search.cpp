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

        if(beacon.second == -1)
        {
            useCommand.at(i).push_back(5);
            useCommand.at(i).push_back(7);
            beaconPoint.at(i).second = samurai.y;
        }
        else if(samurai.y < beacon.second)
        {
            useCommand.at(i).push_back(5);   
        }
        else if(samurai.y > beacon.second)
        {
            useCommand.at(i).push_back(7);   
        }

        if(beacon.first == -1)
        {
            useCommand.at(i).push_back(6);
            useCommand.at(i).push_back(8);
            beaconPoint.at(i).first = samurai.x;
        }
        else if(samurai.x < beacon.first)
        {
            useCommand.at(i).push_back(6);   
        }
        else if(samurai.x > beacon.first)
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

GameSearch GameSearch::doAction(int team, int weapon, int n, Analysis *an, int turn)
{
    GameSearch nextSearch = *this;
    ScoreBoard sb;
    if(turn >= 90)
    {
        sb.shiftFinish();
        //*debug << "shift final" << endl;
    }
    //nextSearch.gs.moveSamurai(team, weapon, n);
    simulateAction(nextSearch.getGameStateRef(), team, weapon, n, beaconPoint.at(weapon), &sb, an);
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

vector<GameSearch> createPattern(queue<GameSearch> *states, int weapon, Analysis *an)
{
    vector<GameSearch> result;
    GameSearch gSearch = states->front();

    //隠れる行動は命令の最初と最後だけでいい
    if (gSearch.isHidden(0, weapon))
    {
        if (gSearch.checkCost(9) && gSearch.checkAction(0, weapon, 9))
        {
            int turn = gSearch.getGameStateRef()->getTurn();
            GameSearch newGSearch = gSearch.doAction(0, weapon, 9, an, turn);
            result.push_back(newGSearch);
            states->push(newGSearch);
        }
    }
    vector<int> *actions = gSearch.getAction(weapon);
    // if(weapon == 0)
    // {
    //     *debug << "actions" << endl;
    //     for(int n : *actions)
    //     {
    //         *debug << n << endl;
    //     }
    // }
    while (!states->empty())
    {
        gSearch = states->front();
        states->pop();
        
        for (int n : *actions)
        {
            //*debug << "weapon : " << weapon << " n : " << n << endl;
            if (gSearch.checkCost(n) && gSearch.checkAction(0, weapon, n))
            {
                int turn = gSearch.getGameStateRef()->getTurn();
                //*debug << "weapon : " << weapon << " action : " << n << endl;
                GameSearch newGSearch = gSearch.doAction(0, weapon, n, an, turn);
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
                int turn = gSearch.getGameStateRef()->getTurn();
                GameSearch newGSearch = tgs.doAction(0, weapon, 9, an, turn);
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
        vector<GameSearch> tv = createPattern(&states, weapon, an);
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
    reviseStatePoint(lookedStates, an, &sb);
    sort(lookedStates.begin(), lookedStates.end(), compGameSearch);

    if(gs->getTurn() == 90)
    {
        for(GameSearch gSearch : lookedStates)
        {
            *debug << gSearch.getScore() << ":" << gSearch.getCommand() << endl;
        }
    }
    
    if (timerFlag)
    {
        showTime(2);
    }

    return lookedStates.at(0).getCommand();
}
