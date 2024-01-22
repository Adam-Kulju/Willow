#ifndef __search__
#define __search__

#include "globals.h"
#include "movegen.h"
#include "nnue.h"
#include <chrono>
#include <stdio.h>
#include <time.h>

struct nodeinfo {
  long int total_nodes;
  long int best_nodes;
  int depth;
};

struct nodeinfo info;

void updateHistory(int &entry, int score) {
  entry += score - entry * abs(score) / 16384;
}

void unmake(movelist *movelst, int *key, ThreadInfo *thread_info,
            long long unsigned int original_pos) {
  thread_info->CURRENTPOS = original_pos;
  thread_info->nnue_state.pop();
  movelst[*key - 1].move = nullmove;
  *key = *key - 1;
}

int quiesce(struct board_info *board, struct movelist *movelst, int *key,
            int alpha, int beta, int depth, int depthleft, bool color,
            bool incheck, ThreadInfo *thread_info)
// Performs a quiescence search on the given position.
{
  if (depth > maxdepth || depth >= 99) // update seldepth
  {
    maxdepth = depth;
  }
  thread_info->nodes++;
  if (thread_info->id == 0) {
    nodes++;
  } else if (thread_info->nodes % 1024 == 0) {
    nodes += 1024;
  }
  if (depthleft <= 0) // return if we are too deep
  {
    return incheck ? 0 : eval(board, color, thread_info);
  }

  if (thread_info->id == 0 &&
      !((nodes) & (CHECKTIME))) // return if we have run out of time, either
                                // alloted to search or overall
  {
    auto end = std::chrono::steady_clock::now();
    auto rightnow =
        (float)std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start_time)
            .count() /
        1000;

    if (rightnow > maximumtime ||
        rightnow > coldturkey) { // you MOVE if you're down to 0.1 seconds!
      return TIMEOUT;
    }
  }
  if ((NODES_IID && !((nodes) % (NODES_IID))) || thread_info->stop) {
    return TIMEOUT;
  }
  int evl = 0;
  char type;
  struct ttentry entry = TT[(thread_info->CURRENTPOS) & (_mask)];
  if (thread_info->CURRENTPOS == entry.zobrist_key)
  // Probe the transposition table. If we got an hit we may be able to cut of
  // immediately, if not it may stil be useful for move ordering.
  {
    type = entry.type;
    evl = entry.eval;
  } else {
    type = None;
    evl = 0;
  }
  if (type != None) {
    if (type == Exact) {
      return evl;
    } else if (type == LBound) { // a move that caused a beta cutoff
      if (evl >= beta) {
        // don't eval any further
        return evl;
      }
    } else { // a move that didn't raise alpha
      if (evl < alpha) {
        return evl;
      }
    }
  }
  long long unsigned int original_pos = thread_info->CURRENTPOS;

  int stand_pat;
  if (incheck) // if we're not in check get a stand pat result (i.e. the score
               // that we get by doing nothing)
  {
    stand_pat = -100000;
  } else {
    int ttscore = evl;
    stand_pat = eval(board, color, thread_info);
    if (type == 3 || (type == UBound && ttscore < stand_pat) ||
        (type == LBound &&
         ttscore >
             stand_pat)) // Use the evaluation from the transposition table as
                         // it is more accurate than the static evaluation.
    {
      stand_pat = entry.eval;
    }
  }

  int bestscore = stand_pat;

  if (!incheck) // if stand pat is good enough to beat beta we can cut off
                // immediately.
  {

    if (stand_pat >= beta) {
      return stand_pat;
    }
    if (stand_pat > alpha) {
      alpha = stand_pat;
    }
  }

  int falpha = alpha;

  struct list list[LISTSIZE];
  int listlen = movegen(board, list, color, incheck);

  movescore(board, movelst, key, list, 99, color, type, listlen,
            (stand_pat + QsearchFutilityThreshold < alpha ? 1 : -108), thread_info, entry, incheck);
  // score the moves

  struct move bestmove = nullmove;
  int legals = 0;
  for (int i = 0; i < listlen; i++) {
    selectionsort(list, i, listlen);

    if (!incheck) {
      if (list[i].eval < 1000200) // Break if we have moved beyond searching
                                  // winning captures and we are not in check
                                  // (if we are then we search all moves)
      {
        break;
      }
    }
    int piecetype = board->board[list[i].move.move >> 8] - 2;

    struct board_info board2 = *board;

    if (move(&board2, list[i].move, color, thread_info, true)) {
      continue;
    }
    legals++;
    move_add(
        board, movelst, key, list[i].move, color,
        (list[i].move.flags == 0xC || board->board[list[i].move.move & 0xFF]),
        thread_info, piecetype);

    list[i].eval =
        -quiesce(&board2, movelst, key, -beta, -alpha, depth + 1, depthleft - 1,
                 color ^ 1, isattacked(board, board->kingpos[color ^ 1], color),
                 thread_info);

    unmake(movelst, key, thread_info, original_pos);
    if (abs(list[i].eval) == TIMEOUT) // timeout detection
    {
      return TIMEOUT;
    }
    if (list[i].eval > bestscore) // update best move
    {
      bestmove = list[i].move;
      bestscore = list[i].eval;
    }
    if (list[i].eval >= beta) // handle fail high
    {
      insert(original_pos, 0, list[i].eval, LBound, list[i].move, search_age);
      return list[i].eval;
    }
    if (list[i].eval > alpha) // update alpha
    {
      alpha = list[i].eval;
    }
  }

  if (incheck &&
      bestscore ==
          -100000) // If we have no moves and we're in check, it's checkmate.
  {
    return -100000;
  }
  if (falpha != alpha) // Insert entry into the transposition table
  {
    insert(original_pos, 0, bestscore, Exact, bestmove, search_age);
  } else {
    insert(original_pos, 0, bestscore, UBound, bestmove, search_age);
  }
  return bestscore;
}

int alphabeta(struct board_info *board, struct movelist *movelst, int *key,
              int alpha, int beta, int depthleft, int depth, bool color,
              bool cutnode, bool incheck, struct move excludedmove,
              ThreadInfo *thread_info) {

  if (depth == 0 && thread_info->id == 0) {
    maxdepth = 0;
    info.total_nodes = 0;
    info.best_nodes = 0;
    info.depth = depthleft;
  }

  thread_info->nodes++;
  if (thread_info->id == 0) {
    nodes++;
  } else if (thread_info->nodes % 1024 == 0) {
    nodes += 1024;
  }

  if (thread_info->id == 0 && !((nodes) & (CHECKTIME))) // Timeout detection
  {
    auto end = std::chrono::steady_clock::now();
    auto rightnow =
        (float)std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start_time)
            .count() /
        1000;
    if (rightnow > maximumtime ||
        rightnow > coldturkey) { // you MOVE if you're down to 0.1 seconds!
      return TIMEOUT;
    }
  }
  if ((NODES_IID && !((nodes) % (NODES_IID))) || thread_info->stop) {
    return TIMEOUT;
  }
  if (depth > 0) {
    if (checkdraw2(movelst, key) > 0 || checkdraw1(board)) // Draw detection
    {
      return (nodes & 3) -
             2; // We fuzz the scores a bit; this helps the engine search for
                // better moves in a position it might think is otherwise drawn.
    }
    int mate_distance = 100000 - depth;
    if (mate_distance <
        beta) // Mate distance pruning; if we're at depth 10 but we've already
              // found a mate in 3, there's no point searching this.
    {
      beta = mate_distance;
      if (alpha >= beta) {
        return beta;
      }
    }
  }
  int evl;

  bool singularsearch = !ismatch(excludedmove, nullmove);

  char type;
  ttentry entry = TT[(thread_info->CURRENTPOS) & (_mask)];
  if (!singularsearch &&
      thread_info->CURRENTPOS ==
          entry.zobrist_key) // Probe the transposition table.
  {
    type = entry.type;
    evl = entry.eval;
  } else {
    type = None;
    evl = -1024;
  }

  int ttscore = evl;

  bool ispv =
      (beta != alpha + 1); // Are we in a PV (i.e. likely best line) node? This
                           // affects what type of pruning we can do.
  bool allnode = (!ispv && !cutnode);

  if (!ispv && type != None &&
      entry.depth >= depthleft) // Check to see if we can cutoff
  {
    if (type == Exact) {
      return evl;
    } else if (type == LBound) { // a move that caused a beta cutoff
      if (evl >= beta) {
        // don't eval any further
        return evl;
      }
    } else { // a move that didn't raise alpha
      if (evl < alpha) {
        return evl;
      }
    }
  }

  if (depthleft <= 0 ||
      depth >= 99) // if we're too deep drop into qsearch, adjusting based on
                   // depth if we get a mate score.
  {
    int b = quiesce(board, movelst, key, alpha, beta, depth, MaxQsearchDepth, color, incheck,
                    thread_info);
    if (b == -100000) {
      b += depth;
    }
    if (b == 100000) {
      b -= depth;
    }
    return b;
  }
  if (incheck) // we cannot evaluate the position when in check, because there
               // may be no good move to get out. Otherwise, the evaluation of a
               // position is very useful for pruning.
  {
    evl = -1000000;
  } else if (singularsearch) {
    evl = movelst[*key - 1].staticeval;
  } else {
    evl = eval(board, color, thread_info);
  }
  thread_info->KILLERTABLE[depth + 1][0] = nullmove;
  movelst[*key - 1].staticeval = evl;

  bool improving =
      (depth > 1 && !incheck &&
       movelst[*key - 1].staticeval > movelst[*key - 3].staticeval);
  // Is our position better than it was during our last move?

  if (type == Exact || (type == UBound && ttscore < evl) ||
      (type == LBound &&
       ttscore > evl)) // Use the evaluation from the transposition table as it
                       // is more accurate than the static evaluation.
  {
    evl = entry.eval;
  }
  // we reverse histscore so that we get how good the move was relative to our
  // side (if last move was a terrible move for the opponent, this board will
  // have high histscore and vice versa) if the board has high histscore, the
  // eval needs to be less above beta to cut off.

  // Reverse Futility Pruning: If our position is so good that we don't need to
  // move to beat beta + some margin, we cut off early.
  if (!ispv && !incheck && !singularsearch && abs(evl) < 50000 &&
      depthleft < MaxRfpDepth && evl - ((depthleft - improving) * RfpImprovingBonus) >= beta) {
    return evl;
  }

  bool isnull = (depth > 0 && movelst[*key - 1].piecetype == -1);

  // Null Move Pruning: If our position is good enough that we can give our
  // opponent an extra move and still beat beta with a reduced search, cut off.
  if (isnull == false && !ispv && !singularsearch && !incheck &&
      evl >= movelst[*key - 1].staticeval &&
      (evl >= beta + NmpThreshold * (depthleft < NmpThresholdDepth))) {

    bool ispiecew = false, ispieceb = false;
    for (int i = 1; i < 5; i++) {
      if (board->pnbrqcount[WHITE][i] > 0) {
        ispiecew = true;
      }
      if (board->pnbrqcount[BLACK][i] > 0) {
        ispieceb = true;
      }
    }
    if (ispiecew && ispieceb) {
      unsigned long long int a = thread_info->CURRENTPOS;
      struct board_info board2 = *board;
      board2.epsquare = 0;
      if (board->epsquare) {
        thread_info->CURRENTPOS ^= ZOBRISTTABLE[773];
      }
      thread_info->CURRENTPOS ^= ZOBRISTTABLE[772];
      move_add(&board2, movelst, key, nullmove, color, false, thread_info, -1);
      int R = NmpBase + (depthleft / NmpDepthDiv) + MIN((evl - beta) / NmpEvalDiv, NmpEvalMax);

      // We call it with a null window, because we don't care about what the
      // score is exactly, we only care if it beats beta or not.
      int nm = -alphabeta(&board2, movelst, key, -beta, -beta + 1,
                          depthleft - R, depth + 1, color ^ 1, !cutnode, false,
                          nullmove, thread_info);

      thread_info->CURRENTPOS = a;

      movelst[*key - 1].move = nullmove;
      *key = *key - 1;
      if (abs(nm) == TIMEOUT) {
        return TIMEOUT;
      }

      if (nm >= beta) {
        return evl;
      }
    }
  }

  // Initilalize the list of moves, generate them, and score them.
  struct list list[LISTSIZE];
  bool ismove = false;
  int betacount = 0;

  if ((ispv || cutnode) && type == None && depthleft > IIRMinDepth) {
    depthleft--;
  }
  int i = 0;
  unsigned long long int original_pos = thread_info->CURRENTPOS;
  int movelen = movegen(board, list, color, incheck);
  movescore(board, movelst, key, list, depth, color,
            (type && !entry.depth) ? None : type, movelen, 0, thread_info,
            entry, true);
  bool raisedalpha = false;
  if (depth == 0) {
    thread_info->currentmove.move = 0;
  }
  struct move bestmove = nullmove;
  bool quietsprune = false;
  int bestscore = -100000, score = bestscore;

  for (i = 0; i < movelen && !quietsprune; i++) {
    // First, make sure the move is legal, not skipped by futility pruning or
    // LMP, and that there's no errors making the move.
    selectionsort(list, i, movelen);
    bool iscap =
        (list[i].move.flags == 0xC || board->board[list[i].move.move & 0xFF] ||
         list[i].move.flags == 0x7) &&
        !(list[i].move.flags / 4 == 2);
    if (ismatch(excludedmove, list[i].move)) {
      continue;
    }
    struct board_info board2 = *board;
    int piecetype = board->board[list[i].move.move >> 8] - 2;

    if (move(&board2, list[i].move, color, thread_info, false)) {
      continue;
    }
    ismove = true;

    if (depth > 0 && !iscap && !ispv) {
      int newdepth =
          MAX(depthleft - LMRTABLE[depthleft - 1][betacount] + improving, 0);
      int futility_move_count =
          LmpBase + (depthleft * depthleft / (1 + (!improving)));
      // Late Move Pruning (LMP): at high depths, we can just not search quiet
      // moves after a while. They are very unlikely to be unavoidable even if
      // they are good and it saves time.
      if (newdepth < LmpDepth) {
        if (betacount >= futility_move_count) {
          quietsprune = true;
        }
      }
      // Futility Pruning: If our position is bad enough, only search captures
      // after this one.
      if ((!incheck && newdepth < FpDepth && list[i].eval < 1000200 &&
           evl + FpMargin1 + FpMargin2 * (newdepth) < alpha)) {
        quietsprune = true;
      }
    }

    // SEE pruning: if a quick check shows that we're hanging material, we skip
    // the move.
    if (depth && list[i].eval < 1000200 && bestscore > -50000 &&
        depthleft < SeePruningDepth &&
        !static_exchange_evaluation(board, list[i].move, color,
                                    (depthleft) *
                                        (iscap ? -SeePruningNoisyMargin * depthleft : -SeePruningQuietMargin))) {

      continue;
    }
    bool ischeck = isattacked(&board2, board2.kingpos[color ^ 1], color);
    int extension = 0;

    if (depth && depth < info.depth * 2) { // if we're not already in a singular
                                           // search, do singular search.

      if (!singularsearch && depthleft >= SeDepth && list[i].eval == 11000000 &&
          abs(evl) < 50000 && entry.depth >= depthleft - 3 && type != UBound) {
        int sBeta = ttscore - (depthleft * SeMargin / 64);
        int sScore = alphabeta(board, movelst, key, sBeta - 1, sBeta,
                               (depthleft - 1) / 2, depth, color, cutnode,
                               incheck, list[i].move, thread_info);

        if (sScore < sBeta) {
          extension = 1;
          if (!ispv && sScore + SeDoubleExtMargin < sBeta &&
              depth <
                  info.depth) { // Limit explosions for double extensions by
                                // only doing them if the depth is less than the
                                // depth we're "supposed" to be at or less than
                                // 15 (leaves room for a bunch near the root)
            extension++;
          }
        } else if (sBeta >= beta) {
          return sBeta;
        } else if ((ttscore <= alpha && cutnode)) {
          extension = -1;
        }
        else if (ttscore >= beta){
          extension = -2;
        }
      } else if (ischeck) {
        extension++;
      }
    }
    update_nnue(board, board2, list[i].move, thread_info, color);

    long int current_nodes = thread_info->nodes;
    move_add(&board2, movelst, key, list[i].move, color, iscap, thread_info,
             piecetype);

    int R = 1, newdepth = depthleft + extension;
    bool fullsearch = false;

    // LMR (Late Move Reductions): search moves sorted later on to a lesser
    // depth, only increasing the depth if they beat alpha at the reduced
    // depth.

    bool do_cutnode = false;

    if (betacount >= 1 + ispv && depthleft >= 3 && !(iscap && ispv)) {
      R = LMRTABLE[depthleft - 1][betacount];

      if (iscap && !ispv) {
        R = R * LmrNoisyDiv / 64;
        R -= std::clamp(thread_info->CAPHIST[color][list[i].move.move >> 8]
                                            [list[i].move.move & 0xFF] /
                            LmrNoisyHistDiv,
                        -2, 2);
      }
      if (ischeck) // Reduce reduction for checks or moves made in check
      {
        R--;
      }
      if (list[i].eval > 1000190 && !iscap) {
        R -= 2;
      }
      if (!improving) // reduce reduction if we are improving.
      {
        R++;
      }
      if (list[i].eval < 100000 && list[i].eval > -100000) {
        R -= std::clamp(list[i].eval / LmrHistDiv, -2, 2);
      }
      if (cutnode) {
        R += 2;
      }
      R = MAX(R, 1); // make sure the reduction doesn't go negative!

      if (newdepth - R < 1 && R > 1) {
        R = newdepth - 1;
      }

      // Search at a reduced depth with null window

      score = -alphabeta(&board2, movelst, key, -alpha - 1, -alpha,
                                newdepth - R, depth + 1, color ^ 1, true,
                                ischeck, nullmove, thread_info);
      if (abs(score) == TIMEOUT) {
        unmake(movelst, key, thread_info, original_pos);

        return TIMEOUT;
      }
      if (score > alpha && R > 1) {
        fullsearch = true;
      }
    }

    else {
      fullsearch = (!ispv) || betacount;
      do_cutnode = true;
    }

    // If a search at reduced depth fails high, search at normal depth with
    // null window.

    if (fullsearch) {
      score =
          -alphabeta(&board2, movelst, key, -alpha - 1, -alpha, newdepth - 1 + (score > bestscore + 60 + (newdepth * 2)),
                     depth + 1, color ^ 1, !cutnode,
                     ischeck, nullmove, thread_info);
      if (abs(score) == TIMEOUT) {
        unmake(movelst, key, thread_info, original_pos);

        return TIMEOUT;
      }
    }

    // If that fails high too, search with the full window.

    if ((score > alpha || !betacount) && ispv) {

      score = -alphabeta(&board2, movelst, key, -beta, -alpha,
                                newdepth - 1, depth + 1, color ^ 1, false,
                                ischeck, nullmove, thread_info);
      if (abs(score) == TIMEOUT) {
        unmake(movelst, key, thread_info, original_pos);

        return TIMEOUT;
      }
    }

    if (depth == 0 && thread_info->id == 0) {
      info.total_nodes += thread_info->nodes - current_nodes;
      if (score > bestscore) {
        info.best_nodes = thread_info->nodes - current_nodes;
      }
    }

    if (score > bestscore) // update best move
    {
      bestmove = list[i].move;
      bestscore = score;
    }

    if (score >= beta) // we're failing high.
    {
      if (depth == 0) {
        thread_info->currentmove = list[i].move;
      }
      bestmove = list[i].move;
      if (!singularsearch) {
        insert(original_pos, depthleft, bestscore, LBound, bestmove,
               search_age);
      }
      total++;
      betas += betacount + 1;

      int c =
          MIN(HistBonusMargin * (depthleft - 1),
              HistBonusMax); // Update history tables, countermoves, and killer moves.

      if (!iscap) {

        int lastpiecetype = 0, lastsquare = 0;
        bool isreply = false;

        if (depth > 0 && !isnull && movelst[(*key - 2)].move.move != 0) {
          isreply = true;
          lastpiecetype =
              board->board[movelst[(*key - 2)].move.move & 0xFF] - 2,
          lastsquare = movelst[(*key - 2)].move.move & 0xFF;
        }

        if (!ismatch(thread_info->KILLERTABLE[depth][0], list[i].move)) {
          thread_info->KILLERTABLE[depth][0] = list[i].move;
        }

        updateHistory(thread_info->HISTORYTABLE[color][(list[i].move.move >> 8)]
                                               [list[i].move.move & 0xFF],
                      c);
        if (isreply) {
          updateHistory(
              thread_info->CONTHIST[lastpiecetype][lastsquare][piecetype]
                                   [list[i].move.move & 0xFF],
              c);
        }
        if (depth > 1 && movelst[*key - 3].piecetype != -1) {
          updateHistory(
              thread_info->CONTHIST[movelst[*key - 3].piecetype]
                                   [movelst[*key - 3].move.move & 0xFF]
                                   [piecetype][list[i].move.move & 0xFF],
              c);
        }
        if (depth > 3 && movelst[*key - 5].piecetype != -1) {
          updateHistory(
              thread_info->CONTHIST[movelst[*key - 5].piecetype]
                                   [movelst[*key - 5].move.move & 0xFF]
                                   [piecetype][list[i].move.move & 0xFF],
              c);
        }

        for (int a = 0; a < i; a++) {
          if (!(list[a].move.flags == 0xC ||
                board->board[list[a].move.move & 0xFF])) {

            updateHistory(
                thread_info->HISTORYTABLE[color][(list[a].move.move >> 8)]
                                         [list[a].move.move & 0xFF],
                -c);
            if (isreply) {
              updateHistory(
                  thread_info->CONTHIST[lastpiecetype][lastsquare]
                                       [board->board[list[a].move.move >> 8] -
                                        2][list[a].move.move & 0xFF],
                  -c);
            }
            if (depth > 1 && movelst[*key - 3].piecetype != -1) {
              updateHistory(
                  thread_info->CONTHIST[movelst[*key - 3].piecetype]
                                       [movelst[*key - 3].move.move & 0xFF]
                                       [board->board[list[a].move.move >> 8] -
                                        2][list[a].move.move & 0xFF],
                  -c);
            }
            if (depth > 3 && movelst[*key - 5].piecetype != -1) {
              updateHistory(
                  thread_info->CONTHIST[movelst[*key - 5].piecetype]
                                       [movelst[*key - 5].move.move & 0xFF]
                                       [board->board[list[a].move.move >> 8] -
                                        2][list[a].move.move & 0xFF],
                  -c);
            }
          } else {
            updateHistory(thread_info->CAPHIST[color][(list[a].move.move >> 8)]
                                              [list[a].move.move & 0xFF],
                          -c);
          }
        }
      }

      else {
        updateHistory(thread_info->CAPHIST[color][(list[i].move.move >> 8)]
                                          [list[i].move.move & 0xFF],
                      c);
        for (int a = 0; a < i; a++) {
          if ((list[a].move.flags == 0xC ||
               board->board[list[a].move.move & 0xFF])) {
            updateHistory(thread_info->CAPHIST[color][(list[a].move.move >> 8)]
                                              [list[a].move.move & 0xFF],
                          -c);
          }
        }
      }

      unmake(movelst, key, thread_info, original_pos);
      return score;
    }

    unmake(movelst, key, thread_info, original_pos);
    if (score > alpha) {
      if (depth == 0) {
        thread_info->currentmove = list[i].move;
      }
      raisedalpha = true;
      alpha = score;
    }

    else if (!betacount && depth == 0) {
      insert(original_pos, depthleft, score, 1, list[i].move,
             search_age);
      return score;
    }
    betacount++;
  }

  if (!ismove) // if we have no legal moves, it's either checkmate or stalemate.
  {
    if (singularsearch) {
      return alpha;
    }
    if (incheck) {
      return -100000 + depth;
    } else {
      return 0;
    }
  }
  if (!singularsearch) {
    if (raisedalpha) // Insert move into TT table
    {
      insert(original_pos, depthleft, alpha, Exact, bestmove, search_age);
    } else {
      insert(original_pos, depthleft, bestscore, UBound, bestmove, search_age);
    }
  }

  return bestscore;
}

bool verifypv(struct board_info *board, struct move pvmove, bool incheck,
              bool color, ThreadInfo *thread_info)

// Verifies that the move from the transposition table is legal.
{
  struct list list[LISTSIZE];
  int movelen = movegen(board, list, color, incheck);
  for (int i = 0; i < movelen; i++) {
    if (ismatch(pvmove, list[i].move)) {
      unsigned long long int c = thread_info->CURRENTPOS;
      struct board_info board2 = *board;
      if (move(&board2, pvmove, color, thread_info, true)) {
        return false;
      }
      thread_info->nnue_state.pop();
      thread_info->CURRENTPOS = c;

      return true;
    }
  }
  return false;
}

int iid_time(struct board_info *board, struct movelist *movelst, float timeAlloted,
             int *key, bool color, bool ismove, bool isprint,
             struct move excludedmove, ThreadInfo *thread_info) {
  thread_info->nnue_state.reset_nnue(board);
  // Performs an Iterative Deepening search on the current position.

  float opttime = timeAlloted * OptTimeRatio / 100;
  clearHistory(false, thread_info);
  clearKiller(thread_info);
  thread_info->currentmove.move = 0;
  int alpha = -1000000, beta = 1000000;
  bool incheck = isattacked(board, board->kingpos[color], color ^ 1);
  int g = 0;
  int depth;
  struct move pvmove;
  // printf("%i %s %s\n", *key, movelst[*key-1].fen, movelst[*key-1].move);
  for (depth = 1;; depth++) {
    int delta = AspStartingWindow; // Aspiration windows: searching with a reduced window
                    // allows us to search less nodes, though it means we have
                    // to research if the score falls outside of those bounds.

    int tempdepth = depth;
    int evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color,
                        false, incheck, excludedmove, thread_info);

    while (abs(evl) != TIMEOUT && (evl <= alpha || evl >= beta)) {
      ;
      if (evl <=
          alpha) // If we fail low, print, widen the window, and try again.
      {
        char temp[6];
        if (isprint) {
          auto end = std::chrono::steady_clock::now();
          auto rightnow = std::chrono::duration_cast<std::chrono::milliseconds>(
                              end - start_time)
                              .count();
          printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv "
                 "%s\n",
                 depth, maxdepth, alpha, nodes, (long int)rightnow,
                 conv(pvmove, temp));
        }
        alpha -= delta;
        beta = (alpha + 3 * beta) / 4;
        delta += delta * 2 / 3;
        evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color,
                        false, incheck, excludedmove, thread_info);

        if (abs(evl) == TIMEOUT) {
          if (thread_info->currentmove.move == 0) {
            thread_info->currentmove = pvmove;
            depth--;
          }
          break;
        }
      } else if (evl >= beta) // If we fail high, widen the window
      {
        char temp[6];
        if (isprint) {
          auto end = std::chrono::steady_clock::now();
          auto rightnow = std::chrono::duration_cast<std::chrono::milliseconds>(
                              end - start_time)
                              .count();
          printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv "
                 "%s\n",
                 depth, maxdepth, beta, nodes, (long int)rightnow,
                 conv(thread_info->currentmove, temp));
        }
        pvmove = thread_info->currentmove;
        beta += delta;
        delta += delta * 2 / 3;

        // Reduce the depth by 1 (up to a max of 3 below the original depth).
        // The reason for this is that fail highs are usually not caused by
        // something really deep in the search, but rather a move early on that
        // had previously been overlooked due to depth conditions.
        tempdepth = MAX(tempdepth - 1, depth - MaxAspDepthReduction);
        evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color,
                        false, incheck, excludedmove, thread_info);
        if (abs(evl) == TIMEOUT) {
          thread_info->currentmove = pvmove;
          break;
        }
      }
    }
    if (abs(evl) ==
        TIMEOUT) // If we've run out of time and don't have a best move from
                 // this iteration, use the one from last iteration
    {
      if (thread_info->currentmove.move == 0) {
        thread_info->currentmove = pvmove;
        depth--;
      }
      break;
    }

    auto end = std::chrono::steady_clock::now();
    float rightnow =
        (float)std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start_time)
            .count();
    g = evl;
    pvmove = thread_info->currentmove;

    // Print search results, handling mate scores
    if (isprint) {

      if (g > 99900) {
        printf("info depth %i seldepth %i score mate %i nodes %lu time %li pv ",
               depth, maxdepth, (100001 - g) / 2, nodes, (long int)rightnow);
      } else if (g < -99900) {
        printf("info depth %i seldepth %i score mate %i nodes %lu time %li pv ",
               depth, maxdepth, (-100001 - g) / 2, nodes, (long int)rightnow);
      } else {
        printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv ",
               depth, maxdepth, g, nodes, (long int)rightnow);
      }
    }

    if (isprint) {

      int d = depth;
      if (abs(g) > 99900) {
        d = MIN(d, (100001 - g));
      }
      unsigned long long int op = thread_info->CURRENTPOS;
      struct board_info board2 = *board;
      bool c = color;

      // Print the principal variation, extracted from the TT table, as long as
      // it remains a legal line.
      int moves = 0;
      while (d > 0) {
        if (TT[thread_info->CURRENTPOS & _mask].zobrist_key !=
            thread_info->CURRENTPOS) {
          break;
        }
        struct move tempmove = TT[thread_info->CURRENTPOS & _mask].bestmove;

        if (!verifypv(&board2, tempmove, false, c, thread_info)) {
          break;
        }

        char temp[6];
        printf("%s ", conv(tempmove, temp));
        move(&board2, tempmove, c, thread_info, true);
        moves++;
        c ^= 1;
        d--;
      }
      for (int i = 0; i < moves; i++) {
        thread_info->nnue_state.pop();
      }
      thread_info->CURRENTPOS = op;
      printf("\n");

      thread_info->CURRENTPOS = op;
    }

    if (depth > 6) // Update the aspiration window
    {
      double besttimefraction = (double)info.best_nodes / info.total_nodes;
      opttime =
          MIN(timeAlloted * OptTimeRatio / 100 * ((NodeTmCoeff1 / 100) - besttimefraction) * NodeTmCoeff2 / 100, maximumtime);
      alpha = evl - AspStartingWindow;
      beta = evl + AspStartingWindow;
    }

    if (depth >= MAXDEPTH ||
        ((float)rightnow / 1000 > opttime &&
         thread_info->id == 0)) // If we've hit the soft cap for time, finish
                                // after the iteration.
    {
      break;
    }
  }
  char temp[8], temp2[8];
  if (isprint) {
    printf("bestmove %s\n", conv(thread_info->currentmove, temp));
  }
  return g;
}

void start_search(struct board_info *board, struct movelist *movelst,
                  float timeAlloted, int *key, bool color, ThreadInfo *thread_info,
                  int numThreads) {
  thread_info->board = *board;
  memcpy(thread_info->movelst, movelst, sizeof(movelist) * 1000);
  thread_info->key = *key;
  thread_info->stop = false;

  for (int i = thread_infos.size(); i < numThreads - 1; i++) {
    thread_infos.emplace_back();
  }

  for (unsigned int i = 0; i < thread_infos.size(); i++) {
    thread_infos[i] = *thread_info;
    thread_infos[i].id = i + 1;
  }
  nodes = 0;
  start_time = std::chrono::steady_clock::now();

  for (int i = 0; i < numThreads - 1; i++) {
    threads.emplace_back(iid_time, &thread_infos[i].board,
                         thread_infos[i].movelst, timeAlloted, &thread_infos[i].key,
                         color, false, false, nullmove, &thread_infos[i]);
  }
  iid_time(&thread_info->board, thread_info->movelst, timeAlloted,
           &thread_info->key, color, false, true, nullmove, thread_info);

  // Stop helper threads
  for (auto &td : thread_infos) {
    td.stop = true;
  }

  for (auto &th : threads) {
    if (th.joinable())
      th.join();
  }

  threads.clear();
  search_age++;
}

#endif