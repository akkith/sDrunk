/*
 * 盤面の評価
 * 他に評価したいことがあれば鑵子を追加してください
 */

#include "sDrunk.hpp"
//#include "Search.hpp"
#include "ScoreBoard.hpp"

void reviseStatePoint(vector<GameSearch> &searchs, Analysis *an, ScoreBoard *sb)
{
    for(GameSearch &ts : searchs)
    {
        vector<SamuraiState> *ss = ts.getGameStateRef()->getSamuraiStatesRef();
        for(int w = 0; w < 3; ++w)
        {
            SamuraiState samurai = ss->at(w);
            int x = samurai.x;
            int y = samurai.y;
            int p = y * stageHeight + x;
            if(an->checkEARange(p))
            {
                ts.addScore(sb->getDeadPenalty());
            }
        }
    }
}
