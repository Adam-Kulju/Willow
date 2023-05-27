#ifndef __eval__
#define __eval__
#include "constants.h"
#include "globals.h"
#include "board.h"

int piece_mobility(struct board_info *board, unsigned char i, bool color, unsigned char piecetype, int *score)
{

    isattacker = false;
    unsigned char mobility = 0;
    for (unsigned char dir = 0; dir < vectors[piecetype]; dir++)
    {
        for (int pos = i;;)
        {
            pos += vector[piecetype][dir];

            if ((pos & 0x88) || (board->board[pos] && (board->board[pos] & 1) == color))
            {
                break;
            }

            if (color)
            {
                if ((((pos + SE) & 0x88) || (board->board[pos + SE] != WPAWN)) && (((pos + SW) & 0x88) || (board->board[pos + SW] != WPAWN)))
                {
                    mobility++;
                }
            }
            else
            {
                if ((((pos + NE) & 0x88) || (board->board[pos + NE] != BPAWN)) && (((pos + NW) & 0x88) || (board->board[pos + NW] != BPAWN)))
                {
                    mobility++;
                }
            }

            if (KINGZONES[color ^ 1][board->kingpos[color ^ 1]][pos])
            {
                attackers[color] += (!isattacker);
                king_attack_count[color] += attacknums[piecetype];
                if (piecetype == 2 && KINGZONES[color ^ 1][board->kingpos[color ^ 1]][pos] == 2)
                { // ROOK
                    char temp = board->board[i];
                    board->board[i] = BLANK;
                    if (isattacked(board, pos, color) && isattacked_mv(board, pos, color ^ 1) != 2)
                    {

                        king_attack_count[color] += 2;
                    }

                    board->board[i] = temp;
                }
                else if (piecetype == 3 && KINGZONES[color ^ 1][board->kingpos[color ^ 1]][pos] != 3)
                { // QUEEN
                    attackers[color] += (!isattacker);
                    char temp = board->board[i];
                    board->board[i] = BLANK;
                    if (isattacked(board, pos, color) && isattacked_mv(board, pos, color ^ 1) != 2)
                    {

                        king_attack_count[color] += 6;
                    }

                    board->board[i] = temp;
                }
                isattacker = true;
            }
            if (board->board[pos] || !slide[piecetype])
            {
                if (piecetype == 0)
                { // knight
                    if (board->board[pos] > BKNIGHT && board->board[pos] / 2 != KING)
                    { // knights get bonuses for attacking queens, rooks, and bishops.
                        *score += pieceattacksbonus[1][board->board[pos] / 2 - 2] * (-color + (color ^ 1));
                        attacking_pieces[color]++;
                    }
                }
                else if (piecetype == 1)
                { // bishops get bonuses for attacking queens, rooks, and knights.
                    if ((board->board[pos] > BBISHOP || board->board[pos] / 2 == KNIGHT) && board->board[pos] / 2 != KING)
                    {
                        *score += pieceattacksbonus[2][board->board[pos] / 2 - 2] * (-color + (color ^ 1));
                        attacking_pieces[color]++;
                    }
                }
                else if (piecetype == 2)
                {
                    if (board->board[pos] / 2 == QUEEN)
                    { // rooks get bonuses for attacking queens.
                        *score += pieceattacksbonus[3][board->board[pos] / 2 - 2] * (-color + (color ^ 1));
                        attacking_pieces[color]++;
                    }
                }
                break;
            }
        }
    }
    return mobility;
}

int material(struct board_info *board, int *phase)
{
    int mgscore = 0, egscore = 0;
    int val = 0;
    for (int i = 0; i < 5; i++)
    {
        if (i == 4)
        {
            *phase += board->pnbrqcount[WHITE][i] << 2;
            *phase += board->pnbrqcount[BLACK][i] << 2;
        }
        else
        {
            *phase += ((i + 1) >> 1) * board->pnbrqcount[WHITE][i];
            *phase += ((i + 1) >> 1) * board->pnbrqcount[BLACK][i];
        }
        mgscore += VALUES[i] * board->pnbrqcount[WHITE][i];
        mgscore -= VALUES[i] * board->pnbrqcount[BLACK][i];
        egscore += VALUES2[i] * board->pnbrqcount[WHITE][i];
        egscore -= VALUES2[i] * board->pnbrqcount[BLACK][i];
    }
    if (*phase > MAXPHASE)
    {
        *phase = MAXPHASE;
    }

    val = ((*phase) * mgscore + (24 - (*phase)) * egscore) / 24;
    if (board->pnbrqcount[WHITE][2] > 1)
    {
        val += bishop_pair[board->pnbrqcount[WHITE][0]];
    }
    if (board->pnbrqcount[BLACK][2] > 1)
    {
        val -= bishop_pair[board->pnbrqcount[BLACK][0]];
    }
    return val;
}

int pst(struct board_info *board, int phase)
{

    attacking_pieces[0] = 0, attacking_pieces[1] = 0;

    int score = 0;
    unsigned char spacew = 0, spaceb = 0;
    int mgscore = 0, egscore = 0;
    int blockedpawns = 0;

    short int wbackwards[8], wadvanced[8], bbackwards[8], badvanced[8];

    for (unsigned char i = 0; i < 8; i++)
    {
        wadvanced[i] = -1, badvanced[i] = 9;
        wbackwards[i] = 9, bbackwards[i] = -1;
    }
    for (unsigned char i = 0; i < 0x80; i++)
    {
        if ((i & 0x88))
        {
            i += 7;
            continue;
        }
        if (phase > 16)
        {
            if (CENTERWHITE[i])
            {

                if (board->board[i] != WPAWN && ((((i + NW) & 0x88) || board->board[i + NW] != BPAWN) && (((i + NE) & 0x88) || board->board[i + NE] != BPAWN)))
                {

                    spacew++;
                    if ((board->board[i + NORTH] == WPAWN || board->board[i + (NORTH * 2)] == WPAWN || board->board[i + (NORTH * 3)] == WPAWN) && !isattacked(board, i, BLACK))
                    {
                        spacew++;
                    }
                }
            }
            else if (CENTERBLACK[i])
            {
                if (board->board[i] != BPAWN && ((((i + SW) & 0x88) || board->board[i + SW] != WPAWN) && (((i + SE) & 0x88) || board->board[i + SE] != WPAWN)))
                {
                    spaceb++;
                    if ((board->board[i + SOUTH] == BPAWN || board->board[i + (SOUTH * 2)] == BPAWN || board->board[i + (SOUTH * 3)] == BPAWN) && !isattacked(board, i, WHITE))
                    {
                        spaceb++;
                    }
                }
            }
        }

        if (board->board[i])
        {
            bool mobilitybonus = false;
            int moves = 0;
            unsigned char piecetype = (board->board[i] >> 1) - 1, piececolor = (board->board[i] & 1);
            if (piecetype != 0 && piecetype != 5)
            { // pawns or kings
                moves = piece_mobility(board, i, piececolor, piecetype - 1, &score);
                mobilitybonus = true;
            }
            if ((piececolor))
            { // black piece
                mgscore -= pstbonusesm[piecetype][i ^ 112];
                egscore -= pstbonusese[piecetype][i ^ 112];
                if (mobilitybonus)
                {
                    // printf("%i %i\n", moves, i);
                    mgscore -= mobilitybonusesmg[piecetype - 1][moves];
                    egscore -= mobilitybonuseseg[piecetype - 1][moves];
                }
                if (!piecetype && (board->board[i + SOUTH] == WPAWN || ((((i + SSW) & 0x88) || board->board[i + SSW] == WPAWN) && (((i + SSE) & 0x88) || board->board[i + SSE] == WPAWN))))
                {
                    blockedpawns++;
                }
                if (piecetype == 0)
                {
                    if (!((i + SW) & 0x88) && board->board[i + SW] > BPAWN && board->board[i + SW] < WKING && !(board->board[i + SW] & 1))
                    {
                        attacking_pieces[BLACK]++;
                        score -= pieceattacksbonus[0][board->board[i + SW] / 2 - 2];
                    }
                    if (!((i + SE) & 0x88) && board->board[i + SE] > BPAWN && board->board[i + SE] < WKING && !(board->board[i + SE] & 1))
                    {
                        attacking_pieces[BLACK]++;
                        score -= pieceattacksbonus[0][board->board[i + SE] / 2 - 2];
                    }
                    if (badvanced[(i & 7)] == 9)
                    {
                        badvanced[(i & 7)] = (i >> 4);
                    }
                    if ((i >> 4) > bbackwards[(i & 7)])
                    {
                        if (bbackwards[(i & 7)] != -1)
                        {

                            mgscore -= doubledpen;
                            egscore -= doubledpen;
                        }
                        bbackwards[(i & 7)] = (i >> 4);
                    }
                }
            }
            else
            {
                mgscore += pstbonusesm[piecetype][i];
                egscore += pstbonusese[piecetype][i];
                if (mobilitybonus)
                {
                    mgscore += mobilitybonusesmg[piecetype - 1][moves];
                    egscore += mobilitybonuseseg[piecetype - 1][moves];
                }
                if (!piecetype && (board->board[i + NORTH] == BPAWN || ((((i + NNW) & 0x88) || board->board[i + NNW] == BPAWN) && (((i + NNE) & 0x88) || board->board[i + NNE] == BPAWN))))
                {
                    blockedpawns++;
                }
                if (piecetype == 0)
                {
                    if (!((i + NW) & 0x88) && board->board[i + NW] > BPAWN && board->board[i + NW] < WKING && (board->board[i + NW] & 1))
                    {
                        attacking_pieces[WHITE]++;
                        score += pieceattacksbonus[0][board->board[i + NW] / 2 - 2];
                    }
                    if (!((i + NE) & 0x88) && board->board[i + NE] > BPAWN && board->board[i + NE] < WKING && (board->board[i + NE] & 1))
                    {
                        attacking_pieces[WHITE]++;
                        score += pieceattacksbonus[0][board->board[i + NE] / 2 - 2];
                    }
                    if (wbackwards[(i & 7)] == 9)
                    {
                        wbackwards[(i & 7)] = (i >> 4);
                    }
                    if ((i >> 4) > wadvanced[(i & 7)])
                    {
                        if (wadvanced[(i & 7)] != -1)
                        {
                            egscore += doubledpen;
                            mgscore += doubledpen;
                        }
                        wadvanced[(i & 7)] = (i >> 4);
                    }
                }
            }
        }
    }

    for (unsigned char i = 0; i < 8; i++)
    {
        if (i == 7)
        {
            if (wadvanced[7] != -1)
            {
                if (wadvanced[6] == -1)
                { // evaluates isolated pawns for h file + passed pawns for h file
                    score += isopen;
                }
                else if (wbackwards[7] < wbackwards[6])
                {

                    score += backwardspen;
                }
            }

            if (badvanced[7] != 9)
            {
                if (badvanced[6] == 9)
                {
                    score -= isopen;
                }
                else if (bbackwards[7] > bbackwards[6])
                {
                    score -= backwardspen;
                }
            }

            if (wadvanced[7] != -1 && wadvanced[7] >= bbackwards[7] && wadvanced[7] >= bbackwards[6])
            {
                int pos = ((wadvanced[7] << 4)) + 7;
                if (board->board[pos + NORTH])
                {
                    mgscore += blockedmgbonus[wadvanced[7]], egscore += blockedegbonus[wadvanced[7]];
                }
                else
                {
                    mgscore += passedmgbonus[wadvanced[7]], egscore += passedegbonus[wadvanced[7]];
                }
            }
            if (badvanced[7] != 9 && badvanced[7] <= wbackwards[7] && badvanced[7] <= wbackwards[6])
            {
                int pos = ((badvanced[7] << 4)) + 7;
                if (board->board[pos + SOUTH])
                {
                    mgscore -= blockedmgbonus[7 - badvanced[7]], egscore -= blockedegbonus[7 - badvanced[7]];
                }
                else
                {
                    mgscore -= passedmgbonus[7 - badvanced[7]], egscore -= passedegbonus[7 - badvanced[7]];
                }
            }
        }
        else if (i == 0)
        {
            if (wadvanced[0] != -1)
            {

                if (wadvanced[1] == -1)
                { // evaluates isolated pawns for a+passed
                    score += isopen;
                }
                else if (wbackwards[0] < wbackwards[1])
                {
                    score += backwardspen;
                }
            }
            if (badvanced[0] != 9)
            {
                if (badvanced[6] == 9)
                {
                    score -= isopen;
                }
                else if (bbackwards[0] > bbackwards[1])
                {
                    score -= backwardspen;
                }
            }

            if (wadvanced[0] != -1 && wadvanced[0] >= bbackwards[0] && wadvanced[0] >= bbackwards[1])
            {
                int pos = ((wadvanced[0] << 4));
                if (board->board[pos + NORTH])
                {
                    mgscore += blockedmgbonus[wadvanced[0]], egscore += blockedegbonus[wadvanced[0]];
                }
                else
                {
                    mgscore += passedmgbonus[wadvanced[0]], egscore += passedegbonus[wadvanced[0]];
                }
            }
            if (badvanced[0] != 9 && badvanced[0] <= wbackwards[0] && badvanced[0] <= wbackwards[1])
            {
                int pos = ((badvanced[0] << 4));
                if (board->board[pos + SOUTH])
                {
                    mgscore -= blockedmgbonus[7 - badvanced[0]], egscore -= blockedegbonus[7 - badvanced[0]];
                }
                else
                {
                    mgscore -= passedmgbonus[7 - badvanced[0]], egscore -= passedegbonus[7 - badvanced[0]];
                }
            }
        }
        else
        {
            if (wadvanced[i] != -1)
            {
                if (wadvanced[i - 1] == -1 && wadvanced[i + 1] == -1)
                { // evaluates isolated pawns for b-g files
                    score += isopen;
                }
                else if (wbackwards[i] < wbackwards[i + 1] && wbackwards[i] < wbackwards[i - 1])
                {
                    score += backwardspen;
                }
            }

            if (badvanced[i] != 9)
            {

                if (badvanced[i - 1] == 9 && badvanced[i + 1] == 9)
                {
                    score -= isopen;
                }
                else if (bbackwards[i] > bbackwards[i + 1] && bbackwards[i] > bbackwards[i - 1])
                {
                    score -= backwardspen;
                }
            }

            if (wadvanced[i] != -1 && wadvanced[i] >= bbackwards[i] && wadvanced[i] >= bbackwards[i - 1] && wadvanced[i] >= bbackwards[i + 1])
            {
                int pos = ((wadvanced[i] << 4)) + i;

                if (board->board[pos + NORTH])
                {
                    mgscore += blockedmgbonus[wadvanced[i]], egscore += blockedegbonus[wadvanced[i]];
                }
                else
                {
                    mgscore += passedmgbonus[wadvanced[i]], egscore += passedegbonus[wadvanced[i]];
                }
            }
            if (badvanced[i] != 9 && badvanced[i] <= wbackwards[i - 1] && badvanced[i] <= wbackwards[i] && badvanced[i] <= wbackwards[i + 1])
            {
                int pos = ((badvanced[i] << 4)) + i;

                if (board->board[pos + SOUTH])
                {
                    mgscore -= blockedmgbonus[7 - badvanced[i]], egscore -= blockedegbonus[7 - badvanced[i]];
                }
                else
                {
                    mgscore -= passedmgbonus[7 - badvanced[i]], egscore -= passedegbonus[7 - badvanced[i]];
                }
            }
        }
    }

    if (phase > 16)
    {
        int weight0 = 0, weight1 = 0;

        for (int i = 0; i < 5; i++)
        {
            weight0 += board->pnbrqcount[WHITE][i];
            weight1 += board->pnbrqcount[BLACK][i];
        }
        if ((blockedpawns) > 9)
        {
            (blockedpawns) = 9;
        }
        weight0 = weight0 - 3 + 1 + blockedpawns;
        weight1 = weight1 - 3 + 1 + blockedpawns;
        if (weight0 < 0)
        {
            weight0 = 0;
        }
        if (weight1 < 0)
        {
            weight1 = 0;
        }
        int space = ((((spacew * weight0 * weight0) >> 4) - ((spaceb * weight1 * weight1) >> 4))) >> 1;
        score += space * 5 / 10;
    }

    if (attackers[BLACK] > 1)
    {
        if (king_attack_count[BLACK] > 99)
        {
            king_attack_count[BLACK] = 99;
        }
        int pawnshield = 0;
        if (board->kingpos[WHITE] < 0x40)
        {
            if ((!((board->kingpos[WHITE] + WEST) & 0x88)) && (board->board[board->kingpos[WHITE] + WEST] == WPAWN || board->board[board->kingpos[WHITE] + NW] == WPAWN))
            {
                pawnshield++;
            }
            if ((((board->board[board->kingpos[WHITE] + NORTH]) >> 1) == PAWN) || board->board[board->kingpos[WHITE] + NORTH + NORTH] == WPAWN || board->board[board->kingpos[WHITE] + NORTH + NORTH + NORTH] == WPAWN)
            {
                pawnshield++;
            }
            if ((!((board->kingpos[WHITE] + EAST) & 0x88)) && (board->board[board->kingpos[WHITE] + EAST] == WPAWN || board->board[board->kingpos[WHITE] + NE] == WPAWN))
            {
                pawnshield++;
            }
        }
        mgscore -= kingdangertablemg[pawnshield][king_attack_count[BLACK]], egscore -= kingdangertableeg[pawnshield][king_attack_count[BLACK]];
    }

    if (attackers[WHITE] > 1)
    {
        if (king_attack_count[WHITE] > 99)
        {
            king_attack_count[WHITE] = 99;
        }
        int pawnshield = 0;
        if (board->kingpos[BLACK] > 0x3a)
        {
            if (!((board->kingpos[BLACK] + WEST) & 0x88) && (board->board[board->kingpos[BLACK] + WEST] == BPAWN || board->board[board->kingpos[BLACK] + SW] == BPAWN))
            {
                pawnshield++;
            }
            if ((((board->board[board->kingpos[BLACK] + SOUTH]) >> 1) == PAWN) || board->board[board->kingpos[BLACK] + SOUTH + SOUTH] == BPAWN || board->board[board->kingpos[BLACK] + SOUTH + SOUTH + SOUTH] == BPAWN)
            {
                pawnshield++;
            }
            if (!((board->kingpos[BLACK] + EAST) & 0x88) && (board->board[board->kingpos[BLACK] + EAST] == BPAWN || board->board[board->kingpos[BLACK] + SE] == BPAWN))
            {
                pawnshield++;
            }
        }
        mgscore += kingdangertablemg[pawnshield][king_attack_count[WHITE]], egscore += kingdangertableeg[pawnshield][king_attack_count[WHITE]];
    }
    score += (phase * mgscore + (24 - phase) * egscore) / 24;

    if (attacking_pieces[WHITE] > 1)
    {
        score += multattacksbonus * (attacking_pieces[WHITE] - 1);
    }
    if (attacking_pieces[BLACK] > 1)
    {
        score -= multattacksbonus * (attacking_pieces[BLACK] - 1);
    }

    return score;
}

int eval(struct board_info *board, bool color)
{
    attackers[0] = 0, attackers[1] = 0;
    king_attack_count[0] = 0, king_attack_count[1] = 0;
    int phase = 0;
    int evl = material(board, &phase);

    evl += pst(board, phase);
    if (board->pnbrqcount[WHITE][0] <= 1 && evl >= 0 && evl < 400 && phase != 0)
    { // if White is up material, we want to stop it from trading pawns
        if (board->pnbrqcount[WHITE][0] == 0)
        {
            evl >>= 2;
        }
        else
        {
            evl >>= 1;
        }
    }
    if (board->pnbrqcount[BLACK][0] <= 1 && evl <= 0 && evl > -400 && phase != 0)
    {
        if (board->pnbrqcount[BLACK][0] == 0)
        {
            evl >>= 2;
        }
        else
        {
            evl >>= 1;
        }
    }
    if (color == BLACK)
    {
        evl = -evl;
    }
    return evl + TEMPO;
}

#endif