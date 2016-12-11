#include "sDrunk.hpp"
#include <unistd.h>
#include <cstdlib>
#include <fstream>

int playOrder;
ostream *debug;

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

//移動(1、南　２、東　３、北　４、南)
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
    *debug << "# Samurai with weapon " << weapon << "@(" << x << "," << y << ")"
           << " Done: " << done
           << " Hidden: " << hidden
           << " Recovery: " << recovery << endl;
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
    //isGameOver = false;
}

//標準入力からゲーム情報を取得
void GameState::readTurnInfo()
{
    //今のターンを取得
    turn = getInt();

    //侍情報を取得
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            SamuraiState samurai;
            samurai.initSamuraiState(i, j);
            samuraiStates[i][j] = samurai;
            //setSamuraiState(i ,w, &samurai);
        }
    }
    //フィールド状況取得
    for (int y = 0; y != stageHeight; y++)
    {
        for (int x = 0; x != stageWidth; x++)
        {
            field[x][y] = getInt();
        }
    }
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
    for (int i = 0; i < 2; ++i)
    {
        for (SamuraiState &ss : samuraiStates[i])
        {
            if (ss.recovery > 0)
            {
                --ss.recovery;
            }
        }
    }
}

//行動可能か否か
bool GameState::isValidAction(const int team, const int wepon, const int action) const
{
    SamuraiState samurai = samuraiStates[team][wepon];
    // Cannot do anything under recovery
    if (samurai.recovery != 0)
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
        if (samurai.hidden != 0 && 3 <= field[nx][ny])
            return false;
        for (int a = 0; a != 2; a++)
        {
            for (SamuraiState ss : samuraiStates[a])
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
            if (3 <= field[samurai.x][samurai.y])
                return false;
        }
        else
        {
            // Expose
            for (int a = 0; a != 2; a++)
            {
                for (SamuraiState ss : samuraiStates[a])
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
        *debug << "Invalid action " << action << " tried" << endl;
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
    SamuraiState *samurai = &samuraiStates[team][wepon];
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
        samurai->x += dx[action];
        samurai->y += dy[action];
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
                SamuraiState &ss = samuraiStates[team][weapon];
                if (team == 1)
                {
                    if (attackX == ss.homeX && attackY == ss.homeY)
                    {
                        isHome |= true;
                    }
                    if (attackX == ss.x && attackY == ss.homeY)
                    {
                        //onEnemy |= true;
                        ss.dead();
                    }
                }

                // もしかしたらこっちかも
                // SamuraiState *ss = &samuraiStates[team][weapon];
                // if(team == 1){
                //     if( attackX == ss->homeX && attackY == ss->homeY )
                //     {
                //         isHome |= true;
                //     }
                //     if( attackX == ss->x && attackY == ss->homeY )
                //     {
                //         //onEnemy |= true;
                //         ss->dead();
                //     }
                // }
            }
        }

        if (0 <= attackX && attackX <= stageWidth && 0 <= attackY && attackY <= stageHeight && !isHome)
        {
            field[attackX][attackY] = samurai->weapon;
        }
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
        //debug = new ofstream("dev");
    }
    //初期情報取得
    playOrder = getInt();
    cout << '0' << endl;
    //ゲーム情報保持
    GameState gState;
    //メインループ= 0;
    int cnt = 0;
    while (!gState.isGameOver())
    {
        gState.readTurnInfo();
        //player->play(info);
        return getCommand( gState );
        //cout << cnt % 3 << " " << 9 << " " << 0 << endl;
        //++cnt;
    }
}
