/*
 * player.hppをベースにしています
 *　大きな変更点はゲーム状況を一括して保持するクラス GameState の作成です
 */

#include <iostream>
#include <cstring>
#include <list>
#include <vector>
#include <queue>

using namespace std;

const int stageWidth = 15;
const int stageHeight = 15;
const int recoveryTurns = 18;
const int maxTurn = 96;

extern int playOrder;

extern ostream *debug;

void rotate(int direction, int x0, int y0, int &x, int &y);

int getDistance(pair<int,int> p1, pair<int,int> p2);

struct SamuraiState
{
  int homeX, homeY;
  int weapon;
  int x, y;
  int done;
  int hidden;
  int recovery;
  void initSamuraiState(int a, int w);
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

  vector<SamuraiState> samuraiStates;
  vector<int> field;

public:
  //コンストラクタ
  GameState();
  //コピーコンストラクタ
  GameState(const GameState &gs);
  //標準入力からのゲーム情報を取得
  void readTurnInfo();
  //経過ターンを返す
  int getTurn();
  //ステージ情報をもらう
  void getField(int (*f)[stageHeight]);
  vector<int> * getFieldRef();
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
        field[y * stageHeight + x] = gs.field[y * stageHeight + x];
      }
    }

    return *this;
  }

  //デバッグ用：侍表示
  void showSamurai();
  //デバッグ用：フィールド表示
  void showField();
};

//分析に使う
class Analysis
{
  private:
  //前ターンのゲーム情報
  GameState beforeState;
  //マップの重要度
  vector<int> heatMap;
  //ヒートマップ補助
  vector<vector<int>> beforeHeatMaps;
  //各侍の移動目標
  vector< pair<int,int> > beacon;
  //目標になる熱量
  vector<int> targetHeat;
  //移動優先状態
  vector<bool> dashFlag;
  //射程範囲内に敵がいるか否か
  vector<bool> tisFlag;
  //敵が一手で攻撃できるエリア
  vector<bool> enemyAttackRange;
  //味方が一手で攻撃できるエリア
  vector<bool> myAttackRange;

  public:
  Analysis();
  void update(GameState &gs);
  void setHeatMap(GameState &before, GameState &after);
  int getHeat(int x, int y);
  void setBeacon(GameState &gs);
  void setSpearBeacon(GameState &gs);
  void setAttackRange(GameState &gs);
  void calcHeatMap();
  vector<bool> setKillzone(vector<SamuraiState> &aTeam,
                           vector<SamuraiState> &bTeam,
                           vector<bool> &tis);
  void setTis(vector<SamuraiState> &aTeam,
              vector<SamuraiState> &bTeam,
              int x, int y, int ax, int ay,
              vector<bool> &tis);
  pair<int, int> getAction(int weapon);
  bool getDashFlag(int weapon);
  void dropHeat(vector<int> &hMap,int heat, int x, int y);
  vector<pair<int,pair<int,int>>> searchHeat(pair<int,int> p, int range);
  bool checkEARange(int p);

  void showHeatMap();
  void showEnemyAttackRange();
  void showTisFlag();
  void showBeacon();
};

//現在のゲーム状況を渡してコマンドを返す関数
string getCommand(GameState *gs, Analysis *an);
//ゲーム状況をもらってその時の評価を返す
double evaluate(GameState *gs);
