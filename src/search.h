#ifndef __search__
#define __search__

#include "globals.h"
#include <stdio.h>
#include "movegen.h"
#include "nnue.h"
#include <time.h>

struct nodeinfo
{
    long int total_nodes;
    long int best_nodes;
    int depth;
};

struct nodeinfo info;

int quiesce(struct board_info *board, int alpha, int beta, int depth, int depthleft, bool color, bool incheck)
// Performs a quiescence search on the given position.
{
    if (depth > maxdepth) // update seldepth
    {
        maxdepth = depth;
    }
    nodes++;
    if (depthleft <= 0) // return if we are too deep
    {
        return incheck ? 0 : nnue_state.evaluate(color);
    }

    if (!((nodes) & (CHECKTIME))) // return if we have run out of time, either alloted to search or overall
    {
        float rightnow = ((float)(clock() - start_time)) / CLOCKS_PER_SEC;
        if (rightnow > maximumtime || rightnow > coldturkey)
        { // you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
    }
    if (NODES_IID && !((nodes) % (NODES_IID)))
    {
        return TIMEOUT;
    }
    int evl = 0;
    char type;
    if (CURRENTPOS == TT[(CURRENTPOS) & (_mask)].zobrist_key)
    // Probe the transposition table. If we got an hit we may be able to cut of immediately, if not it may stil be useful for move ordering.
    {
        type = TT[(CURRENTPOS) & (_mask)].type;
        evl = TT[(CURRENTPOS) & (_mask)].eval;
    }
    else
    {
        type = None;
        evl = 0;
    }
    if (type != None)
    {
        if (type == Exact)
        {
            return evl;
        }
        else if (type == LBound)
        { // a move that caused a beta cutoff
            if (evl >= beta)
            {
                // don't eval any further
                return evl;
            }
        }
        else
        { // a move that didn't raise alpha
            if (evl < alpha)
            {
                return evl;
            }
        }
    }
    long long unsigned int original_pos = CURRENTPOS;

    int stand_pat;
    if (incheck) // if we're not in check get a stand pat result (i.e. the score that we get by doing nothing)
    {
        stand_pat = -100000;
    }
    else
    {
        int ttscore = evl;
        stand_pat = nnue_state.evaluate(color);
        if (type == 3 || (type == UBound && ttscore < stand_pat) || (type == LBound && ttscore > stand_pat)) // Use the evaluation from the transposition table as it is more accurate than the static evaluation.
        {
            stand_pat = TT[(CURRENTPOS) & (_mask)].eval;
        }
    }

    int bestscore = stand_pat;
    int futility = -100000;

    if (!incheck) // if stand pat is good enough to beat beta we can cut off immediately.
    {

        if (stand_pat >= beta)
        {
            return stand_pat;
        }
        if (stand_pat > alpha)
        {
            alpha = stand_pat;
        }
        futility = stand_pat + 60;
    }

    int falpha = alpha;

    struct list list[LISTSIZE];
    int listlen = movegen(board, list, color, incheck);

    movescore(board, list, 99, color,
              (type != None && TT[CURRENTPOS & _mask].depth == 0 ? type : None),
              nullmove, listlen, -108);
    // score the moves; if our TT hit was from a qsearch node, use it for those purposes (one from an alpha beta node is not useful because this only searches captures)

    struct move bestmove = nullmove;
    int i = 0;

    while (i < listlen)
    {
        selectionsort(list, i, listlen);

        if (!incheck)
        {
            if (list[i].eval < 1000200) // Break if we have moved beyond searching winning captures and we are not in check (if we are then we search all moves)
            {
                break;
            }
            /*if (board->board[list[i].move.move & 0xFF] && futility + VALUES2[(board->board[list[i].move.move & 0xFF] >> 1) - 1] <= alpha)
            {
                // If even taking the piece for free is not enough to raise alpha, go on to the next move.
                bestscore = MAX(bestscore, futility + VALUES2[(board->board[list[i].move.move & 0xFF] >> 1) - 1]);
                i++;
                continue;
            }*/
        }
        struct board_info board2 = *board;

        if (move(&board2, list[i].move, color))
        {
            exit(1);
        }

        if (isattacked(&board2, board2.kingpos[color], color ^ 1)) // skip illegal moves
        {
            CURRENTPOS = original_pos;
            nnue_state.pop();
            i++;
            continue;
        }

        list[i].eval = -quiesce(&board2, -beta, -alpha, depth + 1, depthleft - 1, color ^ 1, isattacked(board, board->kingpos[color ^ 1], color));

        if (abs(list[i].eval) == TIMEOUT) // timeout detection
        {
            CURRENTPOS = original_pos;
            nnue_state.pop();
            return TIMEOUT;
        }
        if (list[i].eval > bestscore) // update best move
        {
            bestmove = list[i].move;
            bestscore = list[i].eval;
        }
        if (list[i].eval >= beta) // handle fail high
        {
            CURRENTPOS = original_pos;
            nnue_state.pop();
            insert(original_pos, 0, list[i].eval, LBound, list[i].move, search_age);
            return list[i].eval;
        }
        if (list[i].eval > alpha) // update alpha
        {
            alpha = list[i].eval;
        }
        CURRENTPOS = original_pos;
        nnue_state.pop();

        i++;
    }

    if (incheck && bestscore == -100000) // If we have no moves and we're in check, it's checkmate.
    {
        return -100000;
    }
    if (falpha != alpha) // Insert entry into the transposition table
    {
        insert(original_pos, 0, bestscore, Exact, bestmove, search_age);
    }
    else
    {
        insert(original_pos, 0, bestscore, UBound, bestmove, search_age);
    }
    return bestscore;
}

int alphabeta(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depthleft, int depth, bool color, bool isnull, bool incheck, struct move excludedmove)
{
    nodes++;

    if (depth == 0)
    {
        maxdepth = 0;
        info.total_nodes = 0;
        info.best_nodes = 0;
        info.depth = depthleft;
    }

    if (!((nodes) & (CHECKTIME))) // Timeout detection
    {
        float rightnow = ((float)(clock() - start_time)) / CLOCKS_PER_SEC;
        if (rightnow > maximumtime || rightnow > coldturkey)
        { // you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
    }
    if (NODES_IID && !((nodes) % (NODES_IID)))
    {
        return TIMEOUT;
    }
    if (depth > 0)
    {
        if (checkdraw2(movelst, key) > 0 || checkdraw1(board)) // Draw detection
        {
            return (nodes & 3) - 2; // We fuzz the scores a bit; this helps the engine search for better moves in a position it might think is otherwise drawn.
        }
        int mate_distance = 100000 - depth;
        if (mate_distance < beta) // Mate distance pruning; if we're at depth 10 but we've already found a mate in 3, there's no point searching this.
        {
            beta = mate_distance;
            if (alpha >= beta)
            {
                return beta;
            }
        }
    }
    int evl;

    bool singularsearch = !ismatch(excludedmove, nullmove);

    char type;
    if (!singularsearch && CURRENTPOS == TT[(CURRENTPOS) & (_mask)].zobrist_key) // Probe the transposition table.
    {
        type = TT[(CURRENTPOS) & (_mask)].type;
        evl = TT[(CURRENTPOS) & (_mask)].eval;
    }
    else
    {
        type = None;
        evl = -1024;
    }

    int ttscore = evl;

    bool ispv = (beta != alpha + 1); // Are we in a PV (i.e. likely best line) node? This affects what type of pruning we can do.

    if (!ispv && type != None && TT[(CURRENTPOS) & (_mask)].depth >= depthleft) // Check to see if we can cutoff
    {
        if (type == Exact)
        {
            return evl;
        }
        else if (type == LBound)
        { // a move that caused a beta cutoff
            if (evl >= beta)
            {
                // don't eval any further
                return evl;
            }
        }
        else
        { // a move that didn't raise alpha
            if (evl < alpha)
            {
                return evl;
            }
        }
    }

    if (depthleft <= 0 || depth >= 99) // if we're too deep drop into qsearch, adjusting based on depth if we get a mate score.
    {
        int b = quiesce(board, alpha, beta, depth, 15, color, incheck);
        if (b == -100000)
        {
            b += depth;
        }
        if (b == 100000)
        {
            b -= depth;
        }
        return b;
    }
    if (incheck) // we cannot evaluate the position when in check, because there may be no good move to get out. Otherwise, the evaluation of a position is very useful for pruning.
    {
        evl = -1000000;
    }
    else if (singularsearch)
    {
        evl = movelst[*key - 1].staticeval;
    }
    else
    {
        evl = nnue_state.evaluate(color);
    }
    movelst[*key - 1].staticeval = evl;

    bool improving = (depth > 1 && !incheck && movelst[*key - 1].staticeval > movelst[*key - 3].staticeval); // Is our position better than it was during our last move?

    if (type == Exact || (type == UBound && ttscore < evl) || (type == LBound && ttscore > evl)) // Use the evaluation from the transposition table as it is more accurate than the static evaluation.
    {
        evl = TT[(CURRENTPOS) & (_mask)].eval;
    }

    // Reverse Futility Pruning: If our position is so good that we don't need to move to beat beta + some margin, we cut off early.
    if (!ispv && !incheck && !singularsearch && depthleft < 9 && evl - ((depthleft - improving) * 80) >= beta)
    {
        return evl;
    }

    // Null Move Pruning: If our position is good enough that we can give our opponent an extra move and still beat beta with a reduced search, cut off.
    if (isnull == false && !ispv && !singularsearch && !incheck && depthleft > 2 &&
        (evl >= beta))
    {

        bool ispiecew = false, ispieceb = false;
        for (int i = 1; i < 5; i++)
        {
            if (board->pnbrqcount[WHITE][i] > 0)
            {
                ispiecew = true;
            }
            if (board->pnbrqcount[BLACK][i] > 0)
            {
                ispieceb = true;
            }
        }
        if (ispiecew && ispieceb)
        {
            unsigned long long int a = CURRENTPOS;
            struct board_info board2 = *board;
            board2.epsquare = 0;
            if (board->epsquare)
            {
                CURRENTPOS ^= ZOBRISTTABLE[773];
            }
            CURRENTPOS ^= ZOBRISTTABLE[772];
            move_add(&board2, movelst, key, nullmove, color, false);
            int R = 4 + (depthleft / 6) + MIN((evl - beta) / 200, 3);

            // We call it with a null window, because we don't care about what the score is exactly, we only care if it beats beta or not.
            int nm = -alphabeta(&board2, movelst, key, -beta, -beta + 1, depthleft - R, depth + 1, color ^ 1, true, false, nullmove);

            CURRENTPOS = a;

            movelst[*key - 1].move = nullmove;
            *key = *key - 1;
            if (abs(nm) == TIMEOUT)
            {
                return TIMEOUT;
            }

            if (nm >= beta)
            {
                return evl;
            }
        }
    }

    // Initilalize the list of moves, generate them, and score them.
    struct list list[LISTSIZE];
    bool ismove = false;
    int betacount = 0;
    int movelen = movegen(board, list, color, incheck);
    movescore(board, list, depth, color, type, depth > 1 ? movelst[*key - 1].move : nullmove, movelen, 0);

    // IID: if we're in a PV node and there's no hash hit, crummy move ordering is going to make the search take a long time; so we first do a reduced depth search to get a likely best move.
    if (ispv && type == None && depthleft > 3 && !singularsearch)
    {
        alphabeta(board, movelst, key, alpha, beta, depthleft - 2, depth, color, false, incheck, nullmove);
        type = TT[CURRENTPOS & _mask].type;
    }

    int i = 0;
    unsigned long long int original_pos = CURRENTPOS;
    bool raisedalpha = false;
    if (depth == 0)
    {
        currentmove.move = 0;
    }
    struct move bestmove = nullmove;
    int futility_move_count = (3 + depthleft * depthleft / (1 + (!improving)));
    int numquiets = 0;
    bool quietsprune = false;
    int bestscore = -100000;

    KILLERTABLE[depth + 1][0] = nullmove, KILLERTABLE[depth + 1][1] = nullmove;

    while (i < movelen)
    {
        // First, make sure the move is legal, not skipped by futility pruning or LMP, and that there's no errors making the move.
        selectionsort(list, i, movelen);
        bool iscap = (list[i].move.flags == 0xC || board->board[list[i].move.move & 0xFF]);
        if ((quietsprune && !iscap) || ismatch(excludedmove, list[i].move))
        {
            i++;
            continue;
        }
        struct board_info board2 = *board;

        if (move(&board2, list[i].move, color))
        {
            printfull(board);
            for (int b = 0; b < *key; b++)
            {
                char a[6];
                printf("%s *%x ", conv(movelst[b].move, a), movelst[b].move.flags);
            }
            printf("\n");
            exit(0);
        }
        if (isattacked(&board2, board2.kingpos[color], color ^ 1))
        {
            CURRENTPOS = original_pos;
            nnue_state.pop();
            i++;
            continue;
        }
        if (!ismove)
        {
            bestmove = list[i].move;
        }
        ismove = true;

        if (depth > 0 && !iscap && !ispv)
        {
            // Late Move Pruning (LMP): at high depths, we can just not search quiet moves after a while.
            // They are very unlikely to be unavoidable even if they are good and it saves time.
            if (depthleft < 4)
            {
                numquiets++;
                if (numquiets >= futility_move_count && depth > 0)
                {
                    quietsprune = true;
                }
            }
            // Futility Pruning: If our position is bad enough, only search captures after this one.
            if ((depthleft < 6 && list[i].eval < 1000200 && evl + 90 * (depthleft) + (improving * 40) < alpha))
            {
                quietsprune = true;
            }
        }

        // SEE pruning: if a quick check shows that we're hanging material, we skip the move.
        if (depth && list[i].eval < 1000200 && bestscore > -50000 && depthleft < 9 &&
            !static_exchange_evaluation(board, list[i].move, color, depthleft * ((iscap || list[i].move.flags >> 2 == 1) ? -90 : -50)))
        {
            CURRENTPOS = original_pos;
            nnue_state.pop();
            i++;
            continue;
        }
        bool ischeck = isattacked(&board2, board2.kingpos[color ^ 1], color);

        int extension = 0;

        if (depth && depth < info.depth * 2)
        { // if we're not already in a singular search, do singular search.

            if (!singularsearch && depthleft >= 7 && list[i].eval == 11000000 && abs(evl) < 50000 && TT[(CURRENTPOS) & (_mask)].depth >= depthleft - 3 && type != UBound)
            {
                int sBeta = ttscore - (depthleft * 3);

                CURRENTPOS = original_pos; // reset hash of the position for the singular search
                nnue_state.pop();          // pop the nnue_state to before we made our move. After singular search, we make the move again to reset the nnue state.

                int sScore = alphabeta(board, movelst, key, sBeta - 1, sBeta, (depthleft - 1) / 2, depth, color, false, incheck, list[i].move);

                board2 = *board;
                move(&board2, list[i].move, color);

                if (sScore < sBeta)
                {
                    extension = 1;
                    if (!ispv && sScore + 20 < sBeta && depth < info.depth)
                    { // Limit explosions for double extensions by only doing them if the depth is less than the depth we're "supposed" to be at
                        extension++;
                    }
                }
                else if (sBeta >= beta)
                {
                    CURRENTPOS = original_pos;
                    nnue_state.pop();
                    return sBeta;
                }
                /*else if (ttscore <= alpha || ttscore >= beta){
                    extension--;
                }*/
            }
            else if (ischeck)
            {
                // extension = 1;
            }
        }

        long int current_nodes = nodes;
        move_add(&board2, movelst, key, list[i].move, color, iscap);

        if (ispv == true && !betacount) // The first move of a PV node gets searched to full depth with a full window.
        {
            list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depthleft - 1 + extension, depth + 1, color ^ 1, false, ischeck, nullmove);
            if (abs(list[i].eval) == TIMEOUT)
            {
                movelst[*key - 1].move = nullmove;
                *key = *key - 1;
                CURRENTPOS = original_pos;
                nnue_state.pop();

                return TIMEOUT;
            }
        }

        else
        {
            // LMR (Late Move Reductions): search moves sorted later on to a lesser depth, only increasing the depth if they beat alpha at the reduced depth.
            int R;
            if (betacount < 1 + ispv || depthleft < 3) // Don't reduce winning captures or near the leaves
            {
                R = 0;
            }

            else if (iscap && ispv)
            {
                R = 0;
            }

            else
            {
                R = LMRTABLE[depthleft - 1][betacount];

                if (iscap && !ispv)
                {
                    R = R / 2;
                    if (list[i].eval > 100190)
                    {
                        R--;
                    }
                }
                if (ischeck || incheck) // Reduce reduction for checks or moves made in check
                {
                    R--;
                }
                if (list[i].eval > 100190)
                {
                    R--;
                }
                if (!ispv && type != Exact) // Increase the reduction if we got a TT hit and we're not in a PV node (we know the TT move is almost certainly best)
                {
                    R++;
                }
                if (improving) // reduce reduction if we are improving.
                {
                    R--;
                }
                if (type != None && (list[0].move.flags == 0xC || board->board[list[0].move.move & 0xFF])) // increase the reduction if the TT move was a capture
                {
                    R++;
                }
            }
            R = MAX(R, 0); // make sure the reduction doesn't go negative!

            // Search at a reduced depth with null window

            list[i].eval = -alphabeta(&board2, movelst, key, -alpha - 1, -alpha, depthleft - 1 - R + extension, depth + 1, color ^ 1, false, ischeck, nullmove);
            if (abs(list[i].eval) == TIMEOUT)
            {
                movelst[*key - 1].move = nullmove;
                *key = *key - 1;
                CURRENTPOS = original_pos;
                nnue_state.pop();

                return TIMEOUT;
            }

            // If a search at reduced depth fails high, search at normal depth with null window.

            if (list[i].eval > alpha && R > 0)
            {
                list[i].eval = -alphabeta(&board2, movelst, key, -alpha - 1, -alpha, depthleft - 1 + extension, depth + 1, color ^ 1, false, ischeck, nullmove);
                if (abs(list[i].eval) == TIMEOUT)
                {
                    movelst[*key - 1].move = nullmove;
                    *key = *key - 1;
                    CURRENTPOS = original_pos;
                    nnue_state.pop();

                    return TIMEOUT;
                }
            }

            // If that fails high too, search with the full window.

            if (list[i].eval > alpha && ispv)
            {

                list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depthleft - 1 + extension, depth + 1, color ^ 1, false, ischeck, nullmove);
                if (abs(list[i].eval) == TIMEOUT)
                {
                    movelst[*key - 1].move = nullmove;
                    *key = *key - 1;
                    CURRENTPOS = original_pos;
                    nnue_state.pop();

                    return TIMEOUT;
                }
            }
        }

        if (depth == 0)
        {
            info.total_nodes += nodes - current_nodes;
            if (list[i].eval > bestscore)
            {
                info.best_nodes = nodes - current_nodes;
            }
        }

        if (list[i].eval > bestscore) // update best move
        {
            bestmove = list[i].move;
            bestscore = list[i].eval;
        }

        if (list[i].eval >= beta) // we're failing high.
        {
            if (depth == 0)
            {
                currentmove = list[i].move;
            }
            bestmove = list[i].move;
            if (!singularsearch)
            {
                insert(original_pos, depthleft, bestscore, LBound, bestmove, search_age);
            }
            total++;
            betas += betacount + 1;

            int c = depthleft * depthleft + depthleft - 1; // Update history tables, countermoves, and killer moves.
            if (!iscap)
            {
                if (!ismatch(KILLERTABLE[depth][0], list[i].move))
                {
                    KILLERTABLE[depth][0] = list[i].move;
                }
                else if (!ismatch(KILLERTABLE[depth][1], list[i].move))
                {
                    KILLERTABLE[depth][1] = list[i].move;
                }

                HISTORYTABLE[color][(list[i].move.move >> 8)][list[i].move.move & 0xFF] += c;

                if (HISTORYTABLE[color][(list[i].move.move >> 8)][(list[i].move.move & 0xFF)] > 1000000)
                {
                    for (int a = 0; a < 0x80; a++)
                    {
                        for (int b = 0; b < 0x80; b++) // handle overflows of move scores, so that it doesn't get pushed up above captures/killers etc.
                        {
                            HISTORYTABLE[color][a][b] = (HISTORYTABLE[color][a][b] >> 1);
                        }
                    }
                }

                for (int a = 0; a < i; a++)
                {

                    if (!(list[a].move.flags == 0xC || board->board[list[a].move.move & 0xFF]))
                    {

                        HISTORYTABLE[color][(list[a].move.move >> 8)][list[a].move.move & 0xFF] -= c;

                        if (HISTORYTABLE[color][(list[a].move.move >> 8)][(list[a].move.move & 0xFF)] < -1000000)
                        {
                            for (int c = 0; c < 0x80; c++)
                            {
                                for (int b = 0; b < 0x80; b++)
                                {
                                    HISTORYTABLE[color][c][b] = (HISTORYTABLE[color][c][b] >> 1);
                                }
                            }
                        }
                    }
                }
            }

            if (depth > 1 && !isnull && !iscap && movelst[(*key - 2)].move.move != 0)
            { // key-1 is the move you made, key-2 is the move the opponent made

                COUNTERMOVES[(board->board[movelst[(*key) - 2].move.move & 0xFF] >> 1) - 1][movelst[(*key) - 2].move.move & 0xFF] = list[i].move;
            }
            movelst[(*key) - 1].move.flags = 0;
            *key = *key - 1;
            CURRENTPOS = original_pos;
            nnue_state.pop();
            return list[i].eval;
        }

        movelst[*key - 1].move = nullmove;
        *key = *key - 1;
        if (list[i].eval > alpha)
        {
            if (depth == 0)
            {
                currentmove = list[i].move;
            }
            raisedalpha = true;
            alpha = list[i].eval;
        }

        else if (!betacount && depth == 0)
        {
            insert(original_pos, depthleft, list[i].eval, 1, list[i].move, search_age);
            CURRENTPOS = original_pos;
            nnue_state.pop();
            return list[i].eval;
        }

        CURRENTPOS = original_pos;
        nnue_state.pop();
        betacount++;
        i++;
    }

    if (!ismove) // if we have no legal moves, it's either checkmate or stalemate.
    {
        if (singularsearch)
        {
            return alpha;
        }
        if (incheck)
        {
            return -100000 + depth;
        }
        else
        {
            return 0;
        }
    }
    if (!singularsearch)
    {
        if (raisedalpha) // Insert move into TT table
        {
            insert(original_pos, depthleft, alpha, Exact, bestmove, search_age);
        }
        else
        {
            insert(original_pos, depthleft, bestscore, UBound, bestmove, search_age);
        }
    }

    return bestscore;
}

bool verifypv(struct board_info *board, struct move pvmove, bool incheck, bool color)

// Verifies that the move from the transposition table is legal.
{
    struct list list[LISTSIZE];
    int movelen = movegen(board, list, color, incheck);
    for (int i = 0; i < movelen; i++)
    {
        if (ismatch(pvmove, list[i].move))
        {
            unsigned long long int c = CURRENTPOS;
            struct board_info board2 = *board;
            move(&board2, pvmove, color);
            nnue_state.pop();
            CURRENTPOS = c;

            if (isattacked(&board2, board2.kingpos[color], color ^ 1))
            {
                return false;
            }
            return true;
        }
    }
    return false;
}

int iid_time(struct board_info *board, struct movelist *movelst, float maxtime, int *key, bool color, bool ismove, bool isprint, struct move excludedmove)
{
    nnue_state.reset_nnue(board);
    // Performs an Iterative Deepening search on the current position.

    nodes = 0;
    maximumtime = maxtime * 2;
    float opttime = maxtime * 0.6;
    start_time = clock();
    clearHistory(false);
    clearKiller();
    currentmove.move = 0;
    int alpha = -1000000, beta = 1000000;
    bool incheck = isattacked(board, board->kingpos[color], color ^ 1);
    int g = 0;
    int depth;
    struct move pvmove;
    // printf("%i %s %s\n", *key, movelst[*key-1].fen, movelst[*key-1].move);
    for (depth = 1;; depth++)
    {
        int delta = 12; // Aspiration windows: searching with a reduced window allows us to search less nodes, though it means we have to research if the score falls outside of those bounds.

        int tempdepth = depth;
        int evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color, false, incheck, excludedmove);

        while (abs(evl) != TIMEOUT && (evl <= alpha || evl >= beta))
        {
            ;
            if (evl <= alpha) // If we fail low, print, widen the window, and try again.
            {
                char temp[6];
                if (isprint)
                {
                    printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv %s\n", depth, maxdepth, alpha, nodes, (long int)((float)clock() - start_time) * 1000 / CLOCKS_PER_SEC, conv(pvmove, temp));
                }
                alpha -= delta;
                beta = (alpha + 3 * beta) / 4;
                delta += delta * 2 / 3;
                evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color, false, incheck, excludedmove);

                if (abs(evl) == TIMEOUT)
                {
                    if (currentmove.move == 0)
                    {
                        currentmove = pvmove;
                        depth--;
                    }
                    break;
                }
            }
            else if (evl >= beta) // If we fail high, widen the window
            {
                char temp[6];
                if (isprint)
                {
                    printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv %s\n", depth, maxdepth, beta, nodes, (long int)((float)clock() - start_time) * 1000 / CLOCKS_PER_SEC, conv(currentmove, temp));
                }
                pvmove = currentmove;
                beta += delta;
                delta += delta * 2 / 3;

                // Reduce the depth by 1 (up to a max of 3 below the original depth). The reason for this is that fail highs are usually
                // not caused by something really deep in the search, but rather a move early on that had previously been overlooked due to depth conditions.
                tempdepth = MAX(tempdepth - 1, depth - 3);
                evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color, false, incheck, excludedmove);
                if (abs(evl) == TIMEOUT)
                {
                    currentmove = pvmove;
                    break;
                }
            }
        }
        if (abs(evl) == TIMEOUT) // If we've run out of time and don't have a best move from this iteration, use the one from last iteration
        {
            if (currentmove.move == 0)
            {
                currentmove = pvmove;
                depth--;
            }
            break;
        }

        clock_t time2 = clock() - start_time;
        g = evl;
        pvmove = currentmove;

        // Print search results, handling mate scores
        if (isprint)
        {
            if (g > 99900)
            {
                printf("info depth %i seldepth %i score mate %i nodes %lu time %li pv ", depth, maxdepth, (100001 - g) / 2, nodes, (long int)((float)clock() - start_time) * 1000 / CLOCKS_PER_SEC);
            }
            else if (g < -99900)
            {
                printf("info depth %i seldepth %i score mate %i nodes %lu time %li pv ", depth, maxdepth, (-100001 - g) / 2, nodes, (long int)((float)clock() - start_time) * 1000 / CLOCKS_PER_SEC);
            }
            else
            {
                printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv ", depth, maxdepth, g, nodes, (long int)((float)clock() - start_time) * 1000 / CLOCKS_PER_SEC);
            }
        }

        if (isprint)
        {

            int d = depth;
            if (abs(g) > 99900)
            {
                d = MIN(d, (100001 - g));
            }
            unsigned long long int op = CURRENTPOS;
            struct board_info board2 = *board;
            bool c = color;

            // Print the principal variation, extracted from the TT table, as long as it remains a legal line.
            while (d > 0)
            {
                if (TT[CURRENTPOS & _mask].zobrist_key != CURRENTPOS || ismatch(TT[CURRENTPOS & _mask].bestmove, nullmove) ||
                    !verifypv(&board2, TT[CURRENTPOS & _mask].bestmove, false, c))
                {
                    break;
                }
                char temp[6];
                printf("%s ", conv(TT[CURRENTPOS & _mask].bestmove, temp));
                move(&board2, TT[CURRENTPOS & _mask].bestmove, c);
                nnue_state.pop();
                c ^= 1;
                d--;
            }
            printf("\n");

            CURRENTPOS = op;
        }

        if (depth > 6) // Update the aspiration window
        {
            double besttimefraction = (double)info.best_nodes / info.total_nodes;
            opttime = MIN(maxtime * 0.6 * (1.5 - besttimefraction) * 1.35, maximumtime);
            alpha = evl - 12;
            beta = evl + 12;
        }

        if ((float)time2 / CLOCKS_PER_SEC > opttime || depth >= MAXDEPTH) // If we've hit the soft cap for time, finish after the iteration.
        {
            break;
        }
    }
    char temp[8], temp2[8];
    if (isprint)
    {
        printf("bestmove %s\n", conv(currentmove, temp));
    }
    search_age++;
    return g;
}

#endif