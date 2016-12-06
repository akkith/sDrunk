/*
 * player.hppをベースにしています
 *　大きな変更点はゲーム状況を一括して保持するクラス GameState の作成です
 */

#include <iostream>
//#include <fstream>
#include <cstring>
#include <list>

using namespace std;

const int stageWidth = 15;
const int stageHeight = 15;
const int recoveryTurns = 18;
const int maxTurn = 96;

//extern int field[width][height];
extern int playOrder;
//extern int totalTurns;
//extern int currentTurn;

extern ostream* debug;

void rotate(int direction, int x0, int y0, int& x, int& y);

struct SamuraiState {
  int homeX, homeY;
  int weapon;
  int x, y;
  int done;
  int hidden;
  int recovery;
  void initSamuraiState(int a, int w);
  //bool isValidAction(const int action) const;

  bool operator!=(const SamuraiState other) const;
};

//extern SamuraiState samuraiStates[2][3];

// struct FieldUndo {
//   int *section;
//   int state;
//   FieldUndo(int* sect);
//   void apply();
// };

// struct SamuraiUndo {
//   SamuraiState* ss;
//   int x, y;
//   int hidden;
//   SamuraiUndo(SamuraiState* ss);
//   void apply();
// };

// struct Undo {
//   list<FieldUndo> fieldUndo;
//   list<SamuraiUndo> samuraiUndo;
//   void recField(int* sect);
//   void recSamurai(SamuraiState* si);
//   void apply();
// };

// struct GameInfo {
//   GameInfo();
//   void readTurnInfo();
//   bool isValid(SamuraiState& ss, int action) const;
//   void tryAction(SamuraiState& samurai, int action, Undo& undo,
// 		 int& territory, int& injury, int& hiding);
// };

//ゲーム状況
class GameState
{
    private:
    //現在のターン数
    int turn;
    //各侍の状況
    SamuraiState samuraiStates[2][3];
    //フィールド状況
    int field[stageWidth][stageHeight];

    public:
    //コンストラクタ
    GameState();
    //標準入力からのゲーム情報を取得
    void readTurnInfo();
    //侍情報更新
    //void setSamuraiState(int team, int no, SamuraiState *samuraiState);
    //フィールド更新
    void setField(int field[stageWidth][stageHeight]);
    //ゲーム終了判定
    bool isGameOver();

    //侍番号と命令番号で行動可能か否かを返す
    bool isValidAction(int team, int wepon, int action) const;
    //侍番号と命令をもらいそのとうり動かしてみる
    void moveSamurai(int wepon, int action);
};

// struct Player {
//   virtual void play(GameInfo& info) = 0;
// };

// extern Player* player;

