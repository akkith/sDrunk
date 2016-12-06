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
void GameState::moveSamurai(int wepon, int action)
{
    //do nothing
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
        //debug = new ofstream("/dev/null");
        debug = new ofstream("dev");
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
        cout << cnt % 3 << " " << 9 << " " << 0 << endl;
        ++cnt;
    }
}
