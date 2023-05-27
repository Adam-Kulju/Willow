#ifndef __search__
#define __search__

#include "globals.h"
#include "stdio.h"
#include "eval.h"
#include "movegen.h"
#include <time.h>

int quiesce(struct board_info *board, int alpha, int beta, int depth, int depthleft, bool color, bool incheck)
{
    if (depth > maxdepth)
    {
        maxdepth = depth;
    }
    nodes++;
    if (depthleft <= 0)
    {
        return incheck ? 0 : eval(board, color);
    }

    if (!((nodes) & (CHECKTIME)))
    {
        float rightnow = ((float)(clock() - start_time)) / CLOCKS_PER_SEC;
        if (rightnow > maximumtime || rightnow > coldturkey)
        { // you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
    }
    int evl = 0;
    char type;
    if (CURRENTPOS == TT[(CURRENTPOS) & (_mask)].zobrist_key)
    {
        type = TT[(CURRENTPOS) & (_mask)].type;
        evl = TT[(CURRENTPOS) & (_mask)].eval;
    }
    else
    {
        type = 'n';
        evl = 0;
    }
    if (type != 'n')
    {
        if (type == 3)
        {
            return evl;
        }
        else if (type == 2)
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
    if (incheck)
    {
        stand_pat = -100000;
    }
    else
    {
        stand_pat = eval(board, color);
    }

    int bestscore = stand_pat;
    int futility = -100000;

    if (!incheck)
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
    int listlen = 0;

    if (incheck)
    {
        listlen = movegen(board, list, color, true);
    }
    else
    {
        listlen = movegen(board, list, color, false);
    }

    if (movescore(board, list, 99, color, (type != 'n' && TT[CURRENTPOS & _mask].depth == 0) ? type : 'n', nullmove, listlen, -108))
    {
        printfull(board);
        exit(1);
    }

    struct move bestmove = nullmove;
    int i = 0;

    while (i < listlen)
    {
        selectionsort(list, i, listlen);

        if (!incheck)
        {
            if (list[i].eval < 1000200)
            {
                break;
            }
            if (board->board[list[i].move.move & 0xFF] && futility + VALUES2[(board->board[list[i].move.move & 0xFF] >> 1) - 1] <= alpha)
            {
                bestscore = MAX(bestscore, futility + VALUES2[(board->board[list[i].move.move & 0xFF] >> 1) - 1]);
                i++;
                continue;
            }
        }
        struct board_info board2 = *board;

        if (move(&board2, list[i].move, color))
        {
            exit(1);
        }

        if (isattacked(&board2, board2.kingpos[color], color ^ 1))
        {
            CURRENTPOS = original_pos;
            i++;
            continue;
        }

        list[i].eval = -quiesce(&board2, -beta, -alpha, depth + 1, depthleft - 1, color ^ 1, isattacked(board, board->kingpos[color ^ 1], color));

        if (abs(list[i].eval) == TIMEOUT)
        {
            CURRENTPOS = original_pos;
            return TIMEOUT;
        }
        if (list[i].eval > bestscore)
        {
            bestmove = list[i].move;
            bestscore = list[i].eval;
        }
        if (list[i].eval >= beta)
        {
            CURRENTPOS = original_pos;
            insert(original_pos, 0, list[i].eval, 2, list[i].move, search_age);
            return list[i].eval;
        }
        if (list[i].eval > alpha)
        {
            alpha = list[i].eval;
        }
        CURRENTPOS = original_pos;
        i++;
    }

    if (incheck && bestscore == -100000)
    {
        return -100000;
    }
    if (falpha != alpha)
    {
        insert(original_pos, 0, bestscore, 3, bestmove, search_age);
    }
    else
    {
        insert(original_pos, 0, bestscore, 1, bestmove, search_age);
    }
    return bestscore;
}

int alphabeta(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depthleft, int depth, bool color, bool isnull, bool incheck)
{
    nodes++;
    if (depth == 0)
    {
        maxdepth = 0;
    }
    if (!((nodes) & (CHECKTIME)))
    {
        float rightnow = ((float)(clock() - start_time)) / CLOCKS_PER_SEC;
        if (rightnow > maximumtime || rightnow > coldturkey)
        { // you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
    }
    if (depth > 0)
    {
        if (checkdraw2(movelst, key) > 0 || checkdraw1(board))
        {
            return (nodes & 3) - 2;
        }
        int mate_distance = 100000 - depth;
        if (mate_distance < beta)
        {
            beta = mate_distance;
            if (alpha >= beta)
            {
                return beta;
            }
        }
    }
    int evl;

    char type;
    if (CURRENTPOS == TT[(CURRENTPOS) & (_mask)].zobrist_key)
    {
        type = TT[(CURRENTPOS) & (_mask)].type;
        evl = TT[(CURRENTPOS) & (_mask)].eval;
    }
    else
    {
        type = 'n';
        evl = -1024;
    }
    bool ispv = (beta != alpha + 1);
    if (!ispv && type != 'n' && TT[(CURRENTPOS) & (_mask)].depth >= depthleft)
    {
        if (type == 3)
        {
            return evl;
        }
        else if (type == 2)
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

    if (depthleft <= 0)
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
    if (incheck)
    {
        evl = -1000000;
    }
    else
    {
        evl = eval(board, color);
    }
    movelst[*key - 1].staticeval = evl;

    bool improving = (depth > 1 && !incheck && movelst[*key - 1].staticeval > movelst[*key - 3].staticeval);

    if (type != 'n')
    {
        evl = TT[(CURRENTPOS) & (_mask)].eval;
    }

    if (!ispv && !incheck && depthleft < 9 && evl - ((depthleft - improving) * 80) >= beta)
    {
        return evl;
    }

    if (isnull == false && !ispv && !incheck && depthleft > 2 &&
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
            int nm = -alphabeta(&board2, movelst, key, -beta, -beta + 1, depthleft - R, depth + 1, color ^ 1, true, false);

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

    struct list list[LISTSIZE];

    bool ismove = false;
    int betacount = 0;
    int movelen = movegen(board, list, color, incheck);
    if (movescore(board, list, depth, color, type, depth > 1 ? movelst[*key - 1].move : nullmove, movelen, 0))
    {
        printfull(board);
        printf("%i\n", depth);
        for (int i = 0; i < *key; i++)
        {
            printf("%x\n", movelst[i].move.move);
        }
        exit(1);
    }

    if (ispv && type == 'n' && depthleft > 3)
    {
        alphabeta(board, movelst, key, alpha, beta, depthleft - 2, depth, color, false, incheck);
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

    while (i < movelen)
    {

        selectionsort(list, i, movelen);
        bool iscap = (list[i].move.flags == 0xC || board->board[list[i].move.move & 0xFF]);
        if (quietsprune && !iscap)
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
            if (depthleft < 4)
            {
                numquiets++;
                if (numquiets >= futility_move_count && depth > 0)
                {
                    quietsprune = true;
                }
            }

            if ((depthleft < 6 && list[i].eval < 1000200 && movelst[*key - 1].staticeval + 90 * (depthleft) + (improving * 40) < alpha))
            {
                quietsprune = true;
            }
        }
        if (depth && list[i].eval < 1000200 && bestscore > -50000 && depthleft < 9 &&
            !static_exchange_evaluation(board, list[i].move, color, depthleft * ((iscap || list[i].move.flags >> 2 == 1) ? -90 : -50)))
        {
            CURRENTPOS = original_pos;
            i++;
            continue;
        }
        bool ischeck = isattacked(&board2, board2.kingpos[color ^ 1], color);
        move_add(&board2, movelst, key, list[i].move, color, iscap);

        if (ispv == true && !betacount)
        {
            list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depthleft - 1, depth + 1, color ^ 1, false, ischeck);
            if (abs(list[i].eval) == TIMEOUT)
            {
                movelst[*key - 1].move = nullmove;
                *key = *key - 1;
                CURRENTPOS = original_pos;

                return TIMEOUT;
            }
        }

        else
        {
            int R;
            if (list[i].eval > 1000200 || depthleft < 3)
            {
                R = 0;
            }

            else
            {
                R = LMRTABLE[depthleft - 1][betacount];
                if (iscap)
                {
                    R >>= 1;
                }
                if (ischeck || incheck || list[i].eval > 1000190)
                {
                    R--;
                }
                if (list[i].eval > 1000190)
                {
                    R--;
                }
                if (!ispv && type != 3)
                {
                    R++;
                }
                if (improving)
                {
                    R--;
                }
                if (type != 'n' && (list[0].move.flags == 0xC || board->board[list[0].move.move & 0xFF]))
                {
                    R++;
                }
                R = MAX(R, 0);
            }

            list[i].eval = -alphabeta(&board2, movelst, key, -alpha - 1, -alpha, depthleft - 1 - R, depth + 1, color ^ 1, false, ischeck);
            if (abs(list[i].eval) == TIMEOUT)
            {
                movelst[*key - 1].move = nullmove;
                *key = *key - 1;
                CURRENTPOS = original_pos;

                return TIMEOUT;
            }

            if (list[i].eval > alpha && R > 0)
            {
                list[i].eval = -alphabeta(&board2, movelst, key, -alpha - 1, -alpha, depthleft - 1, depth + 1, color ^ 1, false, ischeck);
                if (abs(list[i].eval) == TIMEOUT)
                {
                    movelst[*key - 1].move = nullmove;
                    *key = *key - 1;
                    CURRENTPOS = original_pos;

                    return TIMEOUT;
                }
            }

            if (list[i].eval > alpha && ispv)
            {

                list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depthleft - 1, depth + 1, color ^ 1, false, ischeck);
                if (abs(list[i].eval) == TIMEOUT)
                {
                    movelst[*key - 1].move = nullmove;
                    *key = *key - 1;
                    CURRENTPOS = original_pos;

                    return TIMEOUT;
                }
            }
        }

        if (list[i].eval > bestscore)
        {
            bestmove = list[i].move;
            bestscore = list[i].eval;
        }
        if (list[i].eval >= beta)
        {
            if (depth == 0)
            {
                currentmove = list[i].move;
            }
            bestmove = list[i].move;
            insert(original_pos, depthleft, bestscore, 2, bestmove, search_age);
            total++;
            betas += betacount + 1;

            int c = depthleft * depthleft + depthleft - 1;
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

                // printf("%s\n", conv(list[i].move, b));
                HISTORYTABLE[color][(list[i].move.move >> 8)][list[i].move.move & 0xFF] += c;

                if (HISTORYTABLE[color][(list[i].move.move >> 8)][(list[i].move.move & 0xFF)] > 1000000)
                {
                    for (int a = 0; a < 0x80; a++)
                    {
                        for (int b = 0; b < 0x80; b++)
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

            if (depth > 1 && !isnull && !iscap)
            { // key-1 is the move you made, key-2 is the move the opponent made

                COUNTERMOVES[(board->board[movelst[(*key) - 2].move.move & 0xFF] >> 1) - 1][movelst[(*key) - 2].move.move & 0xFF] = list[i].move;
            }
            movelst[(*key) - 1].move.flags = 0;
            *key = *key - 1;
            CURRENTPOS = original_pos;
            return beta;
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
            return list[i].eval;
        }

        CURRENTPOS = original_pos;
        betacount++;
        i++;
    }

    if (!ismove)
    {
        if (incheck)
        {
            return -100000 + depth;
        }
        else
        {
            return 0;
        }
    }
    // printfull(board);
    if (raisedalpha)
    {
        // if (alpha != 0){
        insert(original_pos, depthleft, alpha, 3, bestmove, search_age);
        //}
    }
    else
    {

        insert(original_pos, depthleft, bestscore, 1, bestmove, search_age);
        //}
    }

    return bestscore;
}

bool verifypv(struct board_info *board, struct move pvmove, bool incheck, bool color)
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

float iid_time(struct board_info *board, struct movelist *movelst, float maxtime, int *key, bool color, bool ismove)
{
    /*if (*key < 5 && opening_book(board, movelst, key, color)){
        return 1;
    }*/
    nodes = 0;
    maximumtime = maxtime * 2;
    // printf("%f\n", maximumtime);
    // clearTT(false);
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
        int delta = 12;
        int tempdepth = depth;
        int asphigh = 25, asplow = 25;
        int evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color, false, incheck);

        while (abs(evl) != TIMEOUT && (evl <= alpha || evl >= beta))
        {
            if (evl <= alpha)
            {
                char temp[6];
                printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv %s\n", depth, maxdepth, alpha, nodes, (long int)((float)clock() - start_time) * 1000 / CLOCKS_PER_SEC, conv(pvmove, temp));
                alpha -= delta;
                beta = (alpha + 3 * beta) / 4;
                delta += delta * 2 / 3;
                evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color, false, incheck);

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
            else if (evl >= beta)
            {
                char temp[6];
                printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv %s\n", depth, maxdepth, beta, nodes, (long int)((float)clock() - start_time) * 1000 / CLOCKS_PER_SEC, conv(currentmove, temp));
                pvmove = currentmove;
                beta += delta;
                delta += delta * 2 / 3;
                tempdepth = MAX(tempdepth - 1, depth - 3);
                evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color, false, incheck);
                if (abs(evl) == TIMEOUT)
                {
                    currentmove = pvmove;
                    break;
                }
            }
        }
        if (abs(evl) == TIMEOUT)
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

        int d = depth;
        unsigned long long int op = CURRENTPOS;
        struct board_info board2 = *board;
        bool c = color;
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
            c ^= 1;
            d--;
        }
        printf("\n");
        CURRENTPOS = op;

        if ((float)time2 / CLOCKS_PER_SEC > maxtime * 0.6 || depth >= MAXDEPTH)
        {
            break;
        }
        if (depth > 6)
        {
            alpha = evl - 12;
            beta = evl + 12;
        }
    }
    char temp[8], temp2[8];
    printf("bestmove %s\n", conv(currentmove, temp));

    if (ismove)
    {
        bool iscap = (currentmove.flags == 0xC || board->board[currentmove.move & 0xFF]);
        move(board, currentmove, color);
        move_add(board, movelst, key, currentmove, color, iscap);
    }
    search_age++;
    return g;
}

#endif