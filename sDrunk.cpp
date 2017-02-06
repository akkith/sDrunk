#include "sDrunk.hpp"
#include <unistd.h>
#include <cstdlib>
#include <cmath>
#include <fstream>

int playOrder;
ostream *debug;
bool debugFlag = true;

char getChar()
{
    return cin.get();
}

int getInt()
{
    char c;
    do
    {
        c = getChar();
    } while (isspace(c));
    int v = 0;
    bool negate = (c == '-');
    if (negate)
        c = getChar();
    do
    {
        v = 10 * v + c - '0';
        c = getChar();
    } while (!isspace(c));
    if (negate)
        v = -v;
    return v;
}

//移動(0, 無 1、南　２、東　３、北　４、西)
static int dx[] = {0, 0, 1, 0, -1};
static int dy[] = {0, 1, 0, -1, 0};

//拠点座標
static const int homes[2][3][2] = {
    {{0, 0}, {0, 7}, {7, 0}},
    {{14, 14}, {14, 7}, {7, 14}}};

//攻撃座標回転用
void rotate(int direction, int x0, int y0, int &x, int &y)
{
    switch (direction)
    {
    case 0:
        x = x0;
        y = y0;
        break;
    case 1:
        x = y0;
        y = -x0;
        break;
    case 2:
        x = -x0;
        y = -y0;
        break;
    case 3:
        x = -y0;
        y = x0;
        break;
    }
}

//各武器の攻撃マス数
static const int osize[3] = {4, 5, 7};

//各攻撃の範囲(下側正面x軸)
static const int ox[3][7] = {
    {0, 0, 0, 0},
    {0, 0, 1, 1, 2},
    {-1, -1, -1, 0, 1, 1, 1}};
//各攻撃の範囲（下側正面y軸）
static const int oy[3][7] = {
    {1, 2, 3, 4},
    {1, 2, 0, 1, 0},
    {-1, 0, 1, 1, 1, 0, -1}};
//２点間のマンハッタン距離
int getDistance(pair<int, int> p1, pair<int, int> p2)
{
    return abs(p1.first - p2.first) + abs(p1.second - p2.second);
}

//標準入力からの侍情報初期化
void SamuraiState::initSamuraiState(int a, int w)
{
    weapon = w;
    int s = (playOrder == a ? 0 : 1);
    homeX = homes[s][w][0];
    homeY = homes[s][w][1];
    x = getInt();
    y = getInt();
    done = getInt();
    hidden = getInt();
    recovery = getInt();
    if (debugFlag)
    {
        *debug << "# Samurai with weapon " << weapon << "@(" << x << "," << y << ")"
               << " Done: " << done
               << " Hidden: " << hidden
               << " Recovery: " << recovery << endl;
    }
}

//侍の攻撃被弾時の挙動
void SamuraiState::dead()
{
    x = homeX;
    y = homeY;
    recovery = recoveryTurns;
}

bool SamuraiState::operator!=(const SamuraiState other) const
{
    return (homeX != other.homeX) || (homeY != other.homeY);
}

GameState::GameState()
{
    samuraiStates.resize(6);
    field.resize(stageWidth * stageHeight);
    turn = 0;
    //isGameOver = false;
}

GameState::GameState(const GameState &gs)
{
    turn = gs.turn;
    for (int team = 0; team < 2; ++team)
    {
        for (int weapon = 0; weapon < 3; ++weapon)
        {
            samuraiStates[team * 3 + weapon] = gs.samuraiStates[team * 3 + weapon];
        }
    }

    for (int x = 0; x < stageWidth; ++x)
    {
        for (int y = 0; y < stageHeight; ++y)
        {
            field[y * stageHeight + x] = gs.field[y * stageHeight + x];
        }
    }
}

//標準入力からゲーム情報を取得
void GameState::readTurnInfo()
{
    //今のターンを取得
    turn = getInt();
    if (debugFlag)
    {
        *debug << "read turn" << endl;
    }
    //侍情報を取得
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            SamuraiState samurai;
            samurai.initSamuraiState(i, j);
            samuraiStates[i * 3 + j] = samurai;
            //setSamuraiState(i ,w, &samurai);
        }
    }
    if (debugFlag)
    {
        *debug << "read samurai" << endl;
    }
    //フィールド状況取得
    for (int y = 0; y != stageHeight; y++)
    {
        for (int x = 0; x != stageWidth; x++)
        {
            field[y * stageHeight + x] = getInt();
        }
    }
    if (debugFlag)
    {
        *debug << "read fields" << endl;
    }
}

int GameState::getTurn()
{
    return turn;
}

vector<SamuraiState> *GameState::getSamuraiStatesRef()
{
    return &samuraiStates;
}

//侍一人の情報をもらう
SamuraiState *GameState::getSamuraiRef(int team, int weapon)
{
    return &samuraiStates[team * 3 + weapon];
}

vector<int> *GameState::getFieldRef()
{
    return &field;
}

int GameState::getFieldColor(int p)
{
    return field.at(p);
}

//ゲーム終了か否か
bool GameState::isGameOver()
{
    return turn >= 96;
}

//ターンの更新シミュレーション
void GameState::turnUpdate()
{
    //ターンのカウントを増やす
    ++turn;
    //治療のカウントを減らす

    for (SamuraiState &ss : samuraiStates)
    {
        if (ss.recovery > 0)
        {
            --ss.recovery;
        }
    }
}

//行動可能か否か
bool GameState::isValidAction(const int team, const int wepon, const int action) const
{
    SamuraiState samurai = samuraiStates[team * 3 + wepon];
    // Cannot do anything under recovery
    if (samurai.recovery != 0 || samurai.done != 0)
        return false;
    switch (action)
    {
    case 1:
    case 2:
    case 3:
    case 4:
        // Occupation
        // Can occupy when not hidden
        return samurai.hidden == 0;
    case 5:
    case 6:
    case 7:
    case 8:
    {
        // Moving
        int nx = samurai.x;
        int ny = samurai.y;
        switch (action)
        {
        case 5:
            ny += 1;
            break;
        case 6:
            nx += 1;
            break;
        case 7:
            ny -= 1;
            break;
        case 8:
            nx -= 1;
            break;
        }
        // Cannot go out of the field
        if (nx < 0 || stageWidth <= nx || ny < 0 || stageHeight <= ny)
            return false;
        // Cannot enter enemy territory while hidden
        if (samurai.hidden != 0 && 3 <= field[ny * stageHeight + nx])
            return false;
        for (int a = 0; a != 2; a++)
        {
            for (SamuraiState ss : samuraiStates)
            {
                // When not hidden, cannot share a section with another apparent samurai
                if (samurai.hidden == 0 && nx == ss.x && ny == ss.y && ss.hidden == 0)
                    return false;
                // Cannot enter the home posstions of other samurai
                if (ss != samurai && nx == ss.homeX && ny == ss.homeY)
                    return false;
            }
        }
        return true;
    }
    case 9:
        if (samurai.hidden == 0)
        {
            // Hide
            // Cannot only hide itself in the territory
            if (3 <= field[samurai.y * stageHeight + samurai.x])
                return false;
        }
        else
        {
            // Expose
            for (int a = 0; a != 2; a++)
            {
                for (SamuraiState ss : samuraiStates)
                {
                    // Cannot expose itself in a section with another apparent samurai
                    if (ss != samurai &&
                        ss.hidden == 0 && samurai.x == ss.x && samurai.y == ss.y)
                        return false;
                }
            }
        }
        return true;
    default:
        if (debugFlag)
        {
            *debug << "Invalid action " << action << " tried" << endl;
        }
        exit(1);
    }
}

//侍番号と命令をもらいそのとうり動かしてみる
void GameState::moveSamurai(int team, int wepon, int action)
{
    if (!isValidAction(team, wepon, action))
    {
        return;
    }
    //行動対象の侍
    SamuraiState *samurai = &samuraiStates[team * 3 + wepon];
    switch (action)
    {
    case 0:
        //ターン経過
        turnUpdate();
    case 1:
    case 2:
    case 3:
    case 4:
        //攻撃
        attackSamurai(samurai, action);
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        //移動
        samurai->x += dx[action - 4];
        samurai->y += dy[action - 4];
        break;
    case 9:
        //潜伏
        samurai->hidden = samurai->hidden == 0 ? 1 : 0;
        break;
    default:
        break;
    }
}

//攻撃のシミュレーション
void GameState::attackSamurai(SamuraiState *samurai, int action)
{
    //侍の座標
    int samuraiX, samuraiY;
    samuraiX = samurai->x;
    samuraiY = samurai->y;
    //武器に合わせた攻撃マス数
    int attackSize = osize[samurai->weapon];
    for (int i = 0; i < attackSize; ++i)
    {
        int attackX, attackY;
        //向きに合わせた座標に変換
        rotate(action - 1, ox[samurai->weapon][i], oy[samurai->weapon][i], attackX, attackY);
        attackX += samuraiX;
        attackY += samuraiY;
        //攻撃座標が拠点でない場合
        bool isHome = false;
        //攻撃座標に敵がいた
        //bool onEnemy = false;
        for (int team = 0; team < 2; ++team)
        {
            for (int weapon = 0; weapon < 3; ++weapon)
            {
                SamuraiState &ss = samuraiStates[team * 3 + weapon];
                if (team == 1)
                {
                    if (attackX == ss.homeX && attackY == ss.homeY)
                    {
                        isHome |= true;
                    }
                    if (attackX == ss.x && attackY == ss.y)
                    {
                        //onEnemy |= true;
                        ss.dead();
                    }
                }
            }
        }

        if (0 <= attackX && attackX < stageWidth && 0 <= attackY && attackY < stageHeight && !isHome)
        {
            field[attackY * stageHeight + attackX] = samurai->weapon;
        }
    }
}

//デバッグ用：侍表示
void GameState::showSamurai()
{
    for (int team = 0; team < 2; ++team)
    {
        if (team == 0)
        {
            *debug << "My Team" << endl;
        }
        else
        {
            *debug << "Enemy Team" << endl;
        }

        for (int weapon = 0; weapon < 3; ++weapon)
        {
            //SamuraiState samurai = samuraiStates[team*3+weapon];
            SamuraiState samurai = *getSamuraiRef(team, weapon);
            *debug << "weapon : " << samurai.weapon << " x : " << samurai.x << ", y : " << samurai.y
                   << " hidden : " << samurai.hidden << " recovery : " << samurai.recovery << endl;
        }
        *debug << endl;
    }
}

//デバッグ用：フィールド表示
void GameState::showField()
{
    for (int y = 0; y < stageHeight; ++y)
    {
        for (int x = 0; x < stageWidth; ++x)
        {
            *debug << field[y * stageHeight + x];
        }
        *debug << endl;
    }
}

//コンストラクタ
Analysis::Analysis()
{
    //heatMap.resize(stageHeight * stageWidth);
    heatMap = vector<int>(stageHeight * stageWidth, 0);
    for (int i = 0; i < 6; ++i)
    {
        beforeHeatMaps.push_back(heatMap);
    }
    lookedField = vector<int>(stageWidth * stageHeight, 8);
    pressedField = vector<int>(3 * 3, 0);
    enemyAttackRange = vector<bool>(stageHeight * stageWidth, false);
    myAttackRange.resize(3);
    for (int i = 0; i < 3; ++i)
    {
        myAttackRange.at(i) = vector<bool>(stageHeight * stageWidth, false);
    }
    tisFlag = vector<bool>(3, false);

    if (playOrder == 0)
    {
        beacon.push_back(make_pair(7, 7));
        beacon.push_back(make_pair(0, 11));
        beacon.push_back(make_pair(7, 7));
    }
    else
    {
        beacon.push_back(make_pair(7, 7));
        beacon.push_back(make_pair(14, 3));
        beacon.push_back(make_pair(7, 7));
    }

    dashFlag.push_back(true);
    targetHeat.push_back(5);

    dashFlag.push_back(true);
    targetHeat.push_back(3);

    dashFlag.push_back(false);
    targetHeat.push_back(3);
}

//前ターンの情報と今ターンの情報を比較し目標座標を設定
void Analysis::update(GameState &gs)
{
    setLookedField(gs);
    if (gs.getTurn() > 1)
    {
        setLookedField(gs);
        setPressedField(gs);
        setHeatMap(beforeState, gs);
        setAttackRange(gs);
        setBeacon(gs);
        setSpearBeacon(gs);
        setSwordBeacon(gs);
        setAxeBeacon(gs);

        showHeatMap();
        //showEnemyAttackRange();
        //showTisFlag();
        showBeacon();
    }
    beforeState = gs;
}

void Analysis::setLookedField(GameState &gs)
{
    vector<int> *field = gs.getFieldRef();
    for (int i = 0; i < (int)field->size(); ++i)
    {
        int col = field->at(i);
        if (col != 9)
        {
            lookedField.at(i) = col;
        }
    }
}

void Analysis::setPressedField(GameState &gs)
{
    //フィールドを3*3のまとまりで区切ってその中の自分のマス色を数える
    pressedField = vector<int>(3 * 3, 0);
    vector<int> pressedAssist(3 * 3, 0);
    for (int x = 0; x < stageWidth; ++x)
    {
        for (int y = 0; y < stageHeight; ++y)
        {
            int col = lookedField.at(y * stageHeight + x);
            if (3 <= col)
            {
                int nx = x / 5;
                int ny = y / 5;
                pressedField.at(ny * 3 + nx) += 1;
            }
        }
    }
    
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            for (int i = 1; i < 5; ++i)
            {
                int nx = x + dx[i];
                int ny = y + dy[i];
                int np = ny * 3 + nx;
                if (0 <= nx && nx < 3 && 0 <= ny && ny < 3)
                {
                    pressedAssist.at(y * 3 + x) += pressedField.at(np);
                }
            }
        }
    }

    for (int i = 0; i < 3 * 3; ++i)
    {
        pressedField.at(i) += pressedAssist.at(i);
    }

    for(int i = 0; i < 3*3; ++i)
    {
        *debug << pressedField.at(i) << " ";
        if(i != 0 && i % 3 == 2)
        {
            *debug << endl;
        }
        
    }
    
}

void Analysis::setHeatMap(GameState &before, GameState &after)
{
    //変更点の入れ物
    vector<vector<pair<int, int>>> points(3);
    int movedSamurai = 0;

    //最近行動した侍の情報を初期化
    vector<SamuraiState> *beforeSS = before.getSamuraiStatesRef();
    vector<SamuraiState> *afterSS = after.getSamuraiStatesRef();
    vector<int> equals;
    vector<int> unequals;
    for (int w = 3; w < 6; ++w)
    {
        SamuraiState beforeS = beforeSS->at(w);
        SamuraiState afterS = afterSS->at(w);
        if (afterS.recovery > 0)
        {
            beforeHeatMaps.at(w - 3) = vector<int>(stageHeight * stageWidth, 0);
        }
        if (beforeS.done != afterS.done)
        {
            equals.push_back(w - 3);
        }
        else
        {
            unequals.push_back(w - 3);
        }
    }

    if (unequals.size() == 1)
    {
        movedSamurai = unequals.at(0);
        beforeHeatMaps.at(movedSamurai) = vector<int>(stageHeight * stageWidth, 0);
    }
    else
    {
        movedSamurai = equals.at(0);
        beforeHeatMaps.at(movedSamurai) = vector<int>(stageHeight * stageWidth, 0);
    }
    //vector<int> hMap(stageHeight*stageWidth, 0);
    //マップ情報
    vector<int> *beforeMap = before.getFieldRef();
    vector<int> *afterMap = after.getFieldRef();

    for (int x = 0; x < stageWidth; ++x)
    {
        //for (int i = 0; i < stageWidth * stageHeight; ++i)
        for (int y = 0; y < stageHeight; ++y)
        {
            int p = y * stageHeight + x;
            int b = beforeMap->at(p);
            int a = afterMap->at(p);
            if (b != a && 3 <= a && a <= 5)
            {
                //dropHeat(hMap, 2, x, y);
                int weapon = a - 3;
                points.at(weapon).push_back(make_pair(x, y));
                //dropHeat(beforeHeatMaps.at(weapon), 2, x, y);
            }
        }
    }

    for (int w = 0; w < 3; ++w)
    {
        if (points.at(w).size() >= 1)
        {
            dropHeat(beforeHeatMaps.at(w), 2, points.at(w));
        }
    }

    //侍情報を取得
    vector<SamuraiState> *afterSamurai = after.getSamuraiStatesRef();
    vector<SamuraiState> *beforeSamurai = before.getSamuraiStatesRef();
    //points.clear();
    for (int i = 3; i < 6; ++i)
    {
        int x, y, samuraiHeat;
        SamuraiState samurai = afterSamurai->at(i);
        SamuraiState bSamurai = beforeSamurai->at(i);
        if (samurai.hidden == 1 && bSamurai.hidden != 1)
        {
            x = bSamurai.x;
            y = bSamurai.y;
            samuraiHeat = 5;
            vector<pair<int, int>> tp = {make_pair(x, y)};
            dropHeat(beforeHeatMaps.at(samurai.weapon), -1, tp);
        }
        else if (samurai.hidden == 0 && !(samurai.x == samurai.homeX && samurai.y == samurai.homeY) && (bSamurai.x == -1 || movedSamurai == samurai.weapon))
        {
            x = samurai.x;
            y = samurai.y;
            samuraiHeat = 6;
        }
        else
        {
            x = -1;
            y = -1;
        }
        if (x != -1 && y != -1 && samurai.recovery == 0)
        {
            //dropHeat(hMap, samuraiHeat, x, y);
            vector<pair<int, int>> tp = {make_pair(x, y)};
            dropHeat(beforeHeatMaps.at(samurai.weapon), samuraiHeat, tp);
        }
    }
    //beforeHeatMaps.erase(beforeHeatMaps.begin());
    //beforeHeatMaps.push_back(hMap);
    calcHeatMap();
}

int Analysis::getHeat(int x, int y)
{
    return heatMap.at(y * stageHeight + x);
}

void Analysis::setBeacon(GameState &gs)
{
    vector<SamuraiState> *ss = gs.getSamuraiStatesRef();
    int inf = stageHeight * stageWidth;
    int diff[] = {100, 100, 100};
    int dist[] = {inf, inf, inf};

    for (int x = 0; x < stageWidth; ++x)
    {
        for (int y = 0; y < stageHeight; ++y)
        {
            int heat = heatMap.at(y * stageHeight + x);
            for (int w = 2; w < 3; ++w)
            {
                SamuraiState samurai = ss->at(w);
                int tdist = abs(samurai.x - x) + abs(samurai.y - y);
                int theat = abs(heat - targetHeat.at(w));
                if (heat != 0 && theat <= diff[w] && tdist <= dist[w] && (!enemyAttackRange.at(y * stageHeight + x) || tisFlag.at(w)))
                {
                    beacon.at(w) = make_pair(x, y);
                    dashFlag.at(w) = false;
                    if (tdist < dist[w])
                    {
                        dist[w] = tdist;
                    }
                    if (theat < diff[w])
                    {
                        diff[w] = theat;
                    }
                }
            }
        }
    }
}

//槍用ビーコン設定関数
void Analysis::setSpearBeacon(GameState &gs)
{
    if(gs.getTurn() < 6)
    {
        return;
    }
    SamuraiState *samurai = gs.getSamuraiRef(0, 0);
    bool notFound = true;
    int beaconHeat = 0;
    int dist = stageHeight * stageWidth;
    pair<int, int> tb = make_pair(-1, -1);
    //熱が５くらいあったら敵がいると思う
    pair<int, int> start = make_pair(samurai->x, samurai->y);
    vector<pair<int, pair<int, int>>> targets = searchHeat(start, 6);

    if (targets.empty() && gs.getTurn() < 18)
    {
        targets = searchHeat(start, 10);
    }
    if (!targets.empty() || samurai->recovery > 0)
    {
        dashFlag.at(0) = false;
    }

    for (pair<int, pair<int, int>> target : targets)
    {
        int heat = target.first;
        pair<int, int> point = target.second;
        // if (debugFlag)
        // {
        //     *debug << "Spears Beacon" << endl
        //            << heat << " : " << point.first << "," << point.second << endl;
        // }
        if (heat >= targetHeat.at(0) && heat >= beaconHeat)
        {
            int tdist = getDistance(start, point);
            bool isNeer = tdist < dist;
            bool canAttack = myAttackRange.at(0).at(point.second * stageHeight + point.first);
            int color = gs.getFieldColor(point.second * stageHeight + point.first);
            if ((isNeer || canAttack) && heat >= beaconHeat && 3 <= color)
            {
                dist = tdist;
                beaconHeat = heat;
                tb = point;
            }
        }
        else if (notFound)
        {
            tb = point;
        }
    }
    //高熱点が多すぎると正直どこいても不思議じゃない
    //敵の攻撃可能地点に踏み込まない
    if (tb != make_pair(-1, -1))
    {
        beacon.at(0) = tb;
    }
    else if (beacon.at(0) == make_pair(samurai->x, samurai->y))
    {
        beacon.at(0) = make_pair(-1, -1);
    }
}
//刀用ビーコン設定関数
void Analysis::setSwordBeacon(GameState &gs)
{
    if(gs.getTurn() < 6)
    {
        return;   
    }
    SamuraiState *samurai = gs.getSamuraiRef(0, 1);
    bool notFound = true;
    int dist = stageHeight * stageWidth;
    pair<int, int> tb = make_pair(-1, -1);
    //熱が５くらいあったら敵がいると思う
    pair<int, int> start = make_pair(samurai->x, samurai->y);
    vector<pair<int, pair<int, int>>> targets = searchHeat(start, 7);

    //周りに敵がいそうな場所を調べる
    for (pair<int, pair<int, int>> target : targets)
    {
        int heat = target.first;
        pair<int, int> point = target.second;
        if (heat >= 5 && lookedField.at(point.second*stageHeight+point.first) > 2)
        {
            notFound = false;
            int tDist = abs(start.first - point.first) + abs(start.second - point.second);
            if(tDist < dist)
            {
                dist = tDist;
                tb = point;
            }
            if(heat >= 6)
            {
                break;
            }
        }
    }
    *debug << "tb " << tb.first << "," << tb.second << endl;
    if (notFound)
    {
        //塗り
        beacon.at(1) = getNuriPoint(start);
        dashFlag.at(1) = false;
    }
    else
    {
        //戦い
        int nx = tb.first;
        int ny = tb.second;
        if (myAttackRange.at(1).at(ny * stageHeight + nx))
        {
            beacon.at(1) = tb;
            dashFlag.at(1) = false;
        }
        else
        {
            int tx = 2;
            int ty = 2;
            if (tb.first > start.first)
            {
                tx *= -1;
            }
            else if (tb.first == start.first)
            {
                tb.first = -1;
            }
            if (tb.second > start.second)
            {
                ty *= -1;
            }
            else if (tb.second == start.second)
            {
                tb.second = -1;
            }
            tb.first += tx;
            tb.second += ty;
            *debug << "tb chen " << tb.first << "," << tb.second << endl;
            beacon.at(1) = tb;
            int sub = abs(tb.first - start.first) + abs(tb.second - start.second);
            if (sub > 1)
            {
                dashFlag.at(1) = true;
            }
            else
            {
                dashFlag.at(1) = false;
            }
        }
    }
}

void Analysis::setAxeBeacon(GameState &gs)
{
    SamuraiState *samurai = gs.getSamuraiRef(0,2);
    pair<int,int> start = make_pair(samurai->x, samurai->y);
    vector<pair<int,pair<int,int>>> targets = searchHeat(start, 5);
    bool notFound = true;
    pair<int,int> tb = make_pair(-1,-1);
    int heat = 0;
    int dist = stageWidth * stageHeight;
    for(pair<int,pair<int,int>> target : targets)
    {
        int tHeat = target.first;
        pair<int,int> tPoint = target.second;
        int tDist = getDistance(start, tPoint);
        if(tHeat >=5 && tHeat >= heat && dist >= tDist)
        {
            notFound = false;
            tb = tPoint;
            heat = tHeat;
            dist = tDist;
        }
    }

    if(notFound)
    {
        beacon.at(2) = getNuriPoint(start);
        dashFlag.at(2) = false;
    }
    else
    {
        int nx = tb.first;
        int ny = tb.second;
        if (myAttackRange.at(2).at(ny * stageHeight + nx))
        {
            beacon.at(2) = make_pair(-1,-1);
        }
        else
        {
            if(nx > samurai->x)
            {
                tb.first = 0;
            }
            else
            {
                tb.first = 14;
            }
            if(ny > samurai->y)
            {
                tb.second = 0;
            }
            else
            {
                tb.second = 14;
            }
            beacon.at(2) = tb;
        }
    }

}

void Analysis::setAttackRange(GameState &gs)
{
    vector<SamuraiState> *ss = gs.getSamuraiStatesRef();
    vector<SamuraiState> myss = {ss->at(0), ss->at(1), ss->at(2)};
    vector<SamuraiState> enss = {ss->at(3), ss->at(4), ss->at(5)};
    tisFlag = vector<bool>(3, false);
    vector<bool> dummy(3, false);
    for (int i = 0; i < 3; ++i)
    {
        vector<SamuraiState> tmyss = {myss.at(i)};
        myAttackRange.at(i) = setKillzone(tmyss, enss, tisFlag);
    }
    enemyAttackRange = setKillzone(enss, myss, dummy);
}

void Analysis::calcHeatMap()
{
    heatMap = vector<int>(stageHeight * stageWidth, 0);
    for (vector<int> theat : beforeHeatMaps)
    {
        for (int i = 0; i < stageWidth * stageHeight; ++i)
        {
            heatMap.at(i) += theat.at(i);
        }
    }
}

vector<bool> Analysis::setKillzone(vector<SamuraiState> &aTeam,
                                   vector<SamuraiState> &bTeam,
                                   vector<bool> &tis)
{
    vector<bool> killzone(stageHeight * stageWidth, false);
    //vector<SamuraiState> *ss = gs.getSamuraiStatesRef();
    for (SamuraiState samurai : aTeam)
    {
        int w = samurai.weapon;
        int x = samurai.x;
        int y = samurai.y;
        for (int d = 1; d < 5; ++d)
        {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (0 > nx || nx >= stageWidth || 0 > ny || ny >= stageHeight)
            {
                continue;
            }
            int os = osize[w];
            for (int o = 0; o < os; ++o)
            {
                for (int i = 0; i < 4; ++i)
                {
                    int attackX, attackY;
                    rotate(i, ox[w][o], oy[w][o], attackX, attackY);
                    attackX += nx;
                    attackY += ny;
                    if (0 <= attackX && attackX < stageWidth && 0 <= attackY && attackY < stageHeight)
                    {
                        killzone.at(attackY * stageHeight + attackX) = true;
                        setTis(aTeam, bTeam, x, y, attackX, attackY, tis);
                    }
                }
            }
        }
    }
    return killzone;
}

void Analysis::setTis(vector<SamuraiState> &aTeam, vector<SamuraiState> &bTeam,
                      int x, int y, int ax, int ay,
                      vector<bool> &tis)
{
    for (SamuraiState samurai : bTeam)
    {
        if (samurai.x == ax && samurai.y == ay)
        {
            for (SamuraiState s : aTeam)
            {
                if (s.x == x && s.y == y)
                {
                    tis.at(s.weapon) = true;
                }
            }
        }
    }
}

//第２引数の座標にいる時の塗り拡げる用のビーコン設定関数
pair<int, int> Analysis::getNuriPoint(pair<int, int> point)
{
    int nx = point.first / 5;
    int ny = point.second / 5;
    pair<int, int> minPoint;
    int minScore = -1;
    for (int i = 1; i < 5; ++i)
    {
        int nnx = nx + dx[i];
        int nny = ny + dy[i];
        if (0 <= nnx && nnx < 3 && 0 <= nny && nny < 3)
        {
            int tScore = pressedField.at(nny * 3 + nnx);
            //*debug << "serch nuri b:"  << tScore << " x " << nnx << " y " << nny << endl;

            if (tScore > minScore)
            {
                minScore = tScore;
                minPoint = make_pair(nnx, nny);
            }
        }
    }

    //返り値用に加工する
    pair<int, int> result = minPoint;
    //*debug << "getNuri:"  << minScore << " x " << result.first << " y " << result.second << endl;
    result.first *= 5;
    result.first += 2;
    result.second *= 5;
    result.second += 2;
    if (result.first == point.first)
    {
        int one = 1;
        if(result.first > 7)
        {
            one *= -1;
        }
        result.first += one;
    }
    if (result.second == point.second)
    {
        int one = 1;
        if(result.second > 7)
        {
            one *= -1;
        }
        result.second += -1;
    }

    return result;
}

pair<int, int> Analysis::getAction(int weapon)
{
    return beacon.at(weapon);
}

bool Analysis::getDashFlag(int weapon)
{
    return dashFlag.at(weapon);
}

void Analysis::dropHeat(vector<int> &hMap, int heat, vector<pair<int, int>> points)
{
    pair<int, int> point;
    pair<int, pair<int, int>> seeker;
    vector<bool> looked(stageHeight * stageHeight, false);
    

    for (pair<int, int> p : points)
    {
        point = p;
        hMap.at(point.second * stageWidth + point.first) += heat;
        looked.at(point.second * stageHeight + point.first) = true;
    }
    if (heat < 0)
    {
        return;
    }

    for(pair<int,int> p : points)
    {
        queue<pair<int, pair<int, int>>> que;
        seeker = make_pair(heat, p);
        que.push(seeker);
        vector<bool> tlooked = looked;
        while (!que.empty())
        {
            seeker = que.front();
            que.pop();
            //int tHeat = seeker.first - 1;
            int tHeat = seeker.first / 2;
            point = seeker.second;
            if (tHeat <= 0)
            {
                continue;
            }

            for (int i = 1; i < 5; ++i)
            {
                int nx = point.first + dx[i];
                int ny = point.second + dy[i];
                int np = ny * stageHeight + nx;

                if (0 <= nx && nx < stageWidth && 0 <= ny && ny < stageHeight && !tlooked.at(np))
                {
                    hMap.at(np) += tHeat;
                    tlooked.at(np) = true;
                    que.push(make_pair(tHeat, make_pair(nx, ny)));
                }
            }
        }
    }
}

// void Analysis::dropHeat(vector<int> &hMap, int heat, vector<pair<int, int>> points)
// {
//     pair<int, int> point;
//     pair<int, pair<int, int>> seeker;
//     vector<bool> looked(stageHeight * stageHeight, false);
//     queue<pair<int, pair<int, int>>> que;

//     for (pair<int, int> p : points)
//     {
//         point = p;
//         seeker = make_pair(heat, point);
//         que.push(seeker);
//         hMap.at(point.second * stageWidth + point.first) += heat;
//         looked.at(point.second * stageHeight + point.first) = true;
//     }
//     if (heat < 0)
//     {
//         return;
//     }

//     while (!que.empty())
//     {
//         seeker = que.front();
//         que.pop();
//         int tHeat = seeker.first - 1;
//         //int tHeat = seeker.first / 2;
//         point = seeker.second;
//         if (tHeat <= 0)
//         {
//             continue;
//         }

//         for (int i = 1; i < 5; ++i)
//         {
//             int nx = point.first + dx[i];
//             int ny = point.second + dy[i];
//             int np = ny * stageHeight + nx;

//             if (0 <= nx && nx < stageWidth && 0 <= ny && ny < stageHeight && !looked.at(np))
//             {
//                 hMap.at(np) += tHeat;
//                 looked.at(np) = true;
//                 que.push(make_pair(tHeat, make_pair(nx, ny)));
//             }
//         }
//     }
// }

vector<pair<int, pair<int, int>>> Analysis::searchHeat(pair<int, int> p, int range)
{
    vector<pair<int, pair<int, int>>> result;
    int heat = heatMap.at(p.second * stageHeight + p.first);
    pair<int, int> startP = p;
    pair<int, pair<int, int>> seeker = make_pair(heat, p);
    queue<pair<int, pair<int, int>>> que;
    que.push(seeker);

    vector<bool> looked(stageHeight * stageWidth, false);
    looked.at(p.second * stageHeight + p.first) = true;

    while (!que.empty())
    {
        seeker = que.front();
        que.pop();
        int tHeat = seeker.first;
        pair<int, int> tp = seeker.second;
        if (tHeat >= heat && tHeat != 0)
        {
            result.push_back(seeker);
            heat = tHeat;
        }

        for (int i = 1; i <= 4; ++i)
        {
            int nx = tp.first + dx[i];
            int ny = tp.second + dy[i];
            if (0 <= nx && nx < stageWidth && 0 <= ny && ny < stageHeight && getDistance(startP, make_pair(nx, ny)) <= range && !looked.at(ny * stageHeight + nx))
            {
                int nh = heatMap.at(ny * stageHeight + nx);
                pair<int, int> np = make_pair(nx, ny);
                pair<int, pair<int, int>> ns = make_pair(nh, np);
                que.push(ns);
                looked.at(ny * stageHeight + nx) = true;
            }
        }
    }
    return result;
}

bool Analysis::checkEARange(int p)
{
    return enemyAttackRange.at(p);
}

void Analysis::showHeatMap()
{
    for (int i = 0; i < stageWidth * stageHeight; ++i)
    {
        *debug << heatMap.at(i);

        if ((i + 1) % stageWidth == 0)
        {
            *debug << endl;
        }
        else
        {
            *debug << ",";
        }
    }
}

void Analysis::showEnemyAttackRange()
{
    for (int i = 0; i < stageWidth * stageHeight; ++i)
    {
        int n = 0;
        if (enemyAttackRange.at(i))
        {
            n = 1;
        }
        *debug << n;

        if ((i + 1) % stageWidth == 0)
        {
            *debug << endl;
        }
    }
}

void Analysis::showTisFlag()
{
    for (int w = 0; w < 3; ++w)
    {
        string str = "not found";
        if (tisFlag.at(w))
        {
            str = "found";
        }
        *debug << "weapon " << w << ": target is " << str << endl;
    }
}

void Analysis::showBeacon()
{
    for (int w = 0; w < 3; ++w)
    {
        pair<int, int> p = beacon.at(w);
        *debug << "beacon " << w << " x:" << p.first << ", y:" << p.second << endl;
    }
}

//メイン関数
int main(int argc, char *argv[])
{
    //デバッグ用出力ファイル設定
    if (argc == 2 && strcmp(argv[1], "-d") == 0)
    {
        debug = &cerr;
    }
    else
    {
        debug = new ofstream("./dev/log");
    }

    //初期情報取得
    playOrder = getInt();
    cout << '0' << endl;
    //ゲーム情報保持
    GameState gState;
    Analysis analysis;

    //メインループ
    while (!gState.isGameOver())
    {
        gState.readTurnInfo();
        analysis.update(gState);
        if (debugFlag)
        {
            *debug << "Turn : " << gState.getTurn() << endl;
        }
        string command = getCommand(&gState, &analysis);
        if (debugFlag)
        {
            *debug << "================= command : " << command << " =========================" << endl;
        }
        cout << command << endl;
    }
}
