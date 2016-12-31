/*
 * player.hppをベースにしています
 *　大きな変更点はゲーム状況を一括して保持するクラス GameState の作成です
 */

#include <iostream>
//#include <fstream>
#include <cstring>
#include <list>
#include <vector>

using namespace std;

const int stageWidth = 15;
const int stageHeight = 15;
const int recoveryTurns = 18;
const int maxTurn = 96;

//extern int field[width][height];
extern int playOrder;
//extern int totalTurns;
//extern int currentTurn;

extern ostream *debug;

void rotate(int direction, int x0, int y0, int &x, int &y);

struct SamuraiState
{
  int homeX, homeY;
  int weapon;
  int x, y;
  int done;
  int hidden;
  int recovery;
  void initSamuraiState(int a, int w);
  //bool isValidAction(const int action) const;
  void dead();
  //比較用
  bool operator!=(const SamuraiState other) const;
};

/* ゲーム状況
 * 侍と地図を一括して保持する方
 *　どんどん大きくなっている、やばい。
 */
class GameState
{
private:
  //現在のターン数
  int turn;
  //各侍の状況
  //SamuraiState samuraiStates[2][3];
  //フィールド状況
  //int field[stageWidth][stageHeight];

  vector<SamuraiState> samuraiStates;
  vector<int> field;

public:
  //コンストラクタ
  GameState();
  //コピーコンストラクタ
  GameState(const GameState &gs);
  //標準入力からのゲーム情報を取得
  void readTurnInfo();
  //ステージ情報をもらう
  void getField(int (*f)[stageHeight]);
  vector<int> *getFieldRef();
  //侍一人の情報をもらう
  SamuraiState * getSamuraiRef(int team, int weapon);
  //侍の情報をもらう
  void getSamuraiStates(SamuraiState (*ss)[3]);
  vector<SamuraiState> * getSamuraiStatesRef();
  //フィールド更新
  void setField(int field[stageWidth][stageHeight]);
  //ゲーム終了判定
  bool isGameOver();

  //ターンの更新
  void turnUpdate();
  //侍番号と命令番号で行動可能か否かを返す
  bool isValidAction(int team, int wepon, int action) const;
  //侍番号と命令をもらいそのとうり動かしてみる
  void moveSamurai(int team, int wepon, int action);
  //よく使いそうなので攻撃は別にしておく
  void attackSamurai(SamuraiState *samurai, int action);

  GameState &operator=(const GameState &gs)
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
        field[x * stageHeight + y] = gs.field[x * stageHeight + y];
      }
    }

    return *this;
  }

  //デバッグ用：侍表示
  void showSamurai();
  //デバッグ用：フィールド表示
  void showField();
};



//現在のゲーム状況を渡してコマンドを返す関数
string getCommand(GameState *gs);
//ゲーム状況をもらってその時の評価を返す
double evaluate(GameState *gs);

// struct Player {
//   virtual void play(GameInfo& info) = 0;
// };

// extern Player* player;
