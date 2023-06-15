#ifndef __eval__
#define __eval__
#include "constants.h"
#include "globals.h"
#include "board.h"

int piece_mobility(struct board_info *board, unsigned char i, bool color, unsigned char piecetype, int *mgscore, int *egscore )
{
    // Gets the mobility of a piece, as well as info about attacks on the enemy king and other pieces.

    isattacker = false;
    unsigned char mobility = 0;
    for (unsigned char dir = 0; dir < vectors[piecetype]; dir++)
    {
        // Move in each direction the piece can go to.
        for (int pos = i;;)
        {
            pos += vector[piecetype][dir];

            if ((pos & 0x88) || (board->board[pos] && (board->board[pos] & 1) == color)) // Break if we hit the edge of the board or our own piece.
            {
                break;
            }

            // Any square that is not controlled by an enemy pawn is a safe square as far as mobility is concerned.

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

            // Check if the move would take the piece near the enemy king
            if (KINGZONES[color ^ 1][board->kingpos[color ^ 1]][pos])
            {
                // Update the number of attacking pieces, and add a weight to the number of total attacks. The weight depends on the piece - a queen attack is more deadly than a knight attack.
                attackers[color] += (!isattacker);
                king_attack_count[color] += attacknums[piecetype];

                // Safe rook and queen contact checks get an additional bonus.

                if (piecetype == 2 && KINGZONES[color ^ 1][board->kingpos[color ^ 1]][pos] == 2)
                {
                    char temp = board->board[i];
                    board->board[i] = BLANK;
                    if (isattacked(board, pos, color) && isattacked_mv(board, pos, color ^ 1) != 2)
                    {

                        king_attack_count[color] += 2;
                    }

                    board->board[i] = temp;
                }
                else if (piecetype == 3 && KINGZONES[color ^ 1][board->kingpos[color ^ 1]][pos] != 3)
                {
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

                // Threat detection.
                if (piecetype == 0)
                { // knight
                    if (board->board[pos] > BKNIGHT && board->board[pos] / 2 != KING)
                    { // knights get bonuses for attacking queens, rooks, and bishops. No point attacking an enemy knight as it can just take your knight!
                        *mgscore += pieceattacksbonusmg[1][board->board[pos] / 2 - 2] * (-color + (color ^ 1));
                        *egscore  += pieceattacksbonuseg[1][board->board[pos] / 2 - 2] * (-color + (color ^ 1));
                        attacking_pieces[color]++;
                    }
                }
                else if (piecetype == 1)
                { // bishops get bonuses for attacking queens, rooks, and knights.
                    if ((board->board[pos] > BBISHOP || board->board[pos] / 2 == KNIGHT) && board->board[pos] / 2 != KING)
                    {
                        *mgscore += pieceattacksbonusmg[2][board->board[pos] / 2 - 2] * (-color + (color ^ 1));
                        *egscore  += pieceattacksbonuseg[2][board->board[pos] / 2 - 2] * (-color + (color ^ 1));
                        attacking_pieces[color]++;
                    }
                }
                else if (piecetype == 2)
                {
                    if (board->board[pos] / 2 == QUEEN)
                    { // rooks get bonuses for attacking queens.
                        *mgscore += pieceattacksbonusmg[3][board->board[pos] / 2 - 2] * (-color + (color ^ 1));
                        *egscore  += pieceattacksbonuseg[3][board->board[pos] / 2 - 2] * (-color + (color ^ 1));
                        attacking_pieces[color]++;
                    }
                }
                break;
            }
        }
    }
    return mobility;
}

void material(struct board_info *board, int *phase, int *mgscore, int *egscore )
{
    // Evaluates material and bishop pair bonus, and gets the phase of the game.
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
        *mgscore += VALUES[i] * board->pnbrqcount[WHITE][i];
        *mgscore -= VALUES[i] * board->pnbrqcount[BLACK][i];
        *egscore  += VALUES2[i] * board->pnbrqcount[WHITE][i];
        *egscore  -= VALUES2[i] * board->pnbrqcount[BLACK][i];
    }
    // 0 represents an entirely pieceless board (only kings and pawns), while MAXPHASE is at least the starting number of pieces on the board.
    if (*phase > MAXPHASE)
    {
        *phase = MAXPHASE;
    }

}

void pst(struct board_info *board, int phase, int *mgscore, int *egscore ) // A whale of a function.
{
    int tropism_nums[2][5] = {{0,0,0,0,0}, {0,0,0,0,0}};
    attacking_pieces[0] = 0, attacking_pieces[1] = 0;
    int indx = (2*((board->kingpos[WHITE] & 7) > 3)) + ((board->kingpos[BLACK] & 7) > 3);
    unsigned char spacew = 0, spaceb = 0; // represents the space area that White and Black have.
    int blockedpawns = 0; // the amount of blocked pawns in the position, gives space a weight.

    short int wbackwards[8], wadvanced[8], bbackwards[8], badvanced[8]; // gets the most advanced and furthest backwards pawns on the file - useful for pawn structure detection

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

        if (board->board[i])
        {
            bool mobilitybonus = false;
            int moves = 0;
            unsigned char piecetype = (board->board[i] >> 1) - 1, piececolor = (board->board[i] & 1);
            if (piecetype != 0 && piecetype != 5)
            { // pawns or kings
                moves = piece_mobility(board, i, piececolor, piecetype - 1, mgscore, egscore );
                mobilitybonus = true;
            }
            if ((piececolor))
            { // black piece
                *mgscore -= pstbonusesmg[indx][piecetype][i ^ 112 ^ ((indx == 1 || indx == 2) * 7)];
                *egscore  -= pstbonuseseg[indx][piecetype][i ^ 112 ^ ((indx == 1 || indx == 2) * 7)];
                if (mobilitybonus)
                {
                    *mgscore -= mobilitybonusesmg[piecetype - 1][moves];
                    *egscore  -= mobilitybonuseseg[piecetype - 1][moves];
                }
                if (piecetype != 5 && KINGZONES[WHITE][board->kingpos[WHITE]][i]){
                    tropism_nums[BLACK][piecetype]++;
                }
                // Blocked pawns are any pawns that either have an enemy pawn right in front of it, or two pawns a knight's move forwards
                // eg a white pawn on e4 gets blocked by black pawns on d6 and f6
                if (!piecetype && (board->board[i + SOUTH] == WPAWN || ((((i + SSW) & 0x88) || board->board[i + SSW] == WPAWN) && (((i + SSE) & 0x88) || board->board[i + SSE] == WPAWN))))
                {
                    blockedpawns++;
                }
                if (piecetype == 0)
                {
                    // if we're evaluating a pawn, is it attacking a piece of greater value?
                    if (!((i + SW) & 0x88) && board->board[i + SW] > BPAWN && board->board[i + SW] < WKING && !(board->board[i + SW] & 1))
                    {
                        attacking_pieces[BLACK]++;
                        *mgscore -= pieceattacksbonusmg[0][board->board[i + SW] / 2 - 2];
                        *egscore  -= pieceattacksbonuseg[0][board->board[i + SW] / 2 - 2];
                    }
                    if (!((i + SE) & 0x88) && board->board[i + SE] > BPAWN && board->board[i + SE] < WKING && !(board->board[i + SE] & 1))
                    {
                        attacking_pieces[BLACK]++;
                        *mgscore -= pieceattacksbonusmg[0][board->board[i + SE] / 2 - 2];
                        *egscore  -= pieceattacksbonuseg[0][board->board[i + SE] / 2 - 2];
                    }
                    // update pawn structure array - if we already have a pawn on that file, we've found a doubled pawn.
                    if (badvanced[(i & 7)] == 9)
                    {
                        badvanced[(i & 7)] = (i >> 4);
                    }
                    if ((i >> 4) > bbackwards[(i & 7)])
                    {
                        if (bbackwards[(i & 7)] != -1)
                        {

                            *mgscore -= doubledpen[0];
                            *egscore  -= doubledpen[1];
                        }
                        bbackwards[(i & 7)] = (i >> 4);
                    }
                }
            }
            else
            {
                // The same thing as above, but for white pieces.
                *mgscore += pstbonusesmg[indx][piecetype][i];
                *egscore  += pstbonuseseg[indx][piecetype][i];
                if (mobilitybonus)
                {
                    *mgscore += mobilitybonusesmg[piecetype - 1][moves];
                    *egscore  += mobilitybonuseseg[piecetype - 1][moves];
                }
                if (piecetype != 5 && KINGZONES[BLACK][board->kingpos[BLACK]][i]){
                    tropism_nums[WHITE][piecetype]++;
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
                        *mgscore += pieceattacksbonusmg[0][board->board[i + NW] / 2 - 2];
                        *egscore  += pieceattacksbonuseg[0][board->board[i + NW] / 2 - 2];
                    }
                    if (!((i + NE) & 0x88) && board->board[i + NE] > BPAWN && board->board[i + NE] < WKING && (board->board[i + NE] & 1))
                    {
                        attacking_pieces[WHITE]++;
                        *mgscore += pieceattacksbonusmg[0][board->board[i + NE] / 2 - 2];
                        *egscore  += pieceattacksbonuseg[0][board->board[i + NE] / 2 - 2];
                    }
                    if (wbackwards[(i & 7)] == 9)
                    {
                        wbackwards[(i & 7)] = (i >> 4);
                    }
                    if ((i >> 4) > wadvanced[(i & 7)])
                    {
                        if (wadvanced[(i & 7)] != -1)
                        {
                            *egscore  += doubledpen[0];
                            *mgscore += doubledpen[1];
                        }
                        wadvanced[(i & 7)] = (i >> 4);
                    }
                }
            }
        }
    }

    for (unsigned char i = 0; i < 8; i++)
    {
        if (i == 7) // Evaluate if the h-pawn is passed/isolated/etc.
        {
            if (wadvanced[7] != -1)
            {
                // If we have no pawns on the g-file at all, the h-pawn is isolated. If instead the furthest backwards one is further up the board, it's a backwards pawn.
                if (wadvanced[6] == -1)
                {
                    *mgscore += isopen[0];
                    *egscore  += isopen[1];
                }
                else if (wbackwards[7] < wbackwards[6])
                {

                    *mgscore += backwardspen[0]; *egscore  += backwardspen[1];
                }
            }

            if (badvanced[7] != 9)
            {
                if (badvanced[6] == 9)
                {
                    *mgscore -= isopen[0]; *egscore  -= isopen[1];
                }
                else if (bbackwards[7] > bbackwards[6])
                {
                    *mgscore -= backwardspen[0]; *egscore  -= backwardspen[1];
                }
            }

            if (wadvanced[7] != -1 && wadvanced[7] >= bbackwards[7] && wadvanced[7] >= bbackwards[6] - 1)
            {
                // Passed pawn check
                int pos = ((wadvanced[7] << 4)) + 7;

                if (wadvanced[7] >= bbackwards[6])
                {
                    if (board->board[pos + NORTH]) // If the square in front of the passed pawn is occupied, it's a blocked passed pawn and gets a bit less of a bonus.
                    {
                        *mgscore += blockedmgbonus[wadvanced[7]], *egscore  += blockedegbonus[wadvanced[7]];
                    }
                    else
                    {
                        *mgscore += passedmgbonus[wadvanced[7]], *egscore  += passedegbonus[wadvanced[7]];
                    }
                    if (board->board[pos + SW] == WPAWN)
                    { // Protected passed pawns get an additional bonus.
                        if (!board->board[pos + NORTH])
                        {
                            *mgscore += protectedpassedmg[wadvanced[7]], *egscore  += protectedpassedeg[wadvanced[7]];
                        }
                        else
                        {
                            *mgscore += blockedprotectedmg[wadvanced[7]], *egscore  += blockedprotectedeg[wadvanced[7]];
                        }
                    }
                }
                else if (!board->board[pos + NORTH]) // And candidate passed pawns (ie pawns that are one square away from being passed) also get bonuses if the square in front is empty.
                {
                    *mgscore += candidatepassedmg[wadvanced[7]], *egscore  += candidatepassedeg[wadvanced[7]];
                }
            }
            if (badvanced[7] != 9 && badvanced[7] <= wbackwards[7] && badvanced[7] <= wbackwards[6] + 1)
            {
                int pos = ((badvanced[7] << 4)) + 7;
                if (badvanced[7] <= wbackwards[6])
                {
                    if (board->board[pos + SOUTH])
                    {
                        *mgscore -= blockedmgbonus[7 - badvanced[7]], *egscore  -= blockedegbonus[7 - badvanced[7]];
                    }
                    else
                    {
                        *mgscore -= passedmgbonus[7 - badvanced[7]], *egscore  -= passedegbonus[7 - badvanced[7]];
                    }
                    if (board->board[pos + NW] == BPAWN)
                    {
                        if (!board->board[pos + SOUTH])
                        {
                            *mgscore -= protectedpassedmg[7 - badvanced[7]], *egscore  -= protectedpassedeg[7 - badvanced[7]];
                        }
                        else
                        {
                            *mgscore -= blockedprotectedmg[7 - badvanced[7]], *egscore  -= blockedprotectedeg[7 - badvanced[7]];
                        }
                    }
                }
                else if (!board->board[pos + SOUTH])
                {
                    *mgscore -= candidatepassedmg[7 - badvanced[7]], *egscore  -= candidatepassedeg[7 - badvanced[7]];
                }
            }
        }
        else if (i == 0)
        {
            // Same as above, but for pawns on the a-file.
            if (wadvanced[0] != -1)
            {

                if (wadvanced[1] == -1)
                { // evaluates isolated pawns for a+passed
                    *mgscore += isopen[0]; *egscore  += isopen[1];
                }
                else if (wbackwards[0] < wbackwards[1])
                {
                    *mgscore += backwardspen[0]; *egscore  += backwardspen[1];
                }
            }
            if (badvanced[0] != 9)
            {
                if (badvanced[6] == 9)
                {
                    *mgscore -= isopen[0]; *egscore  -= isopen[1];
                }
                else if (bbackwards[0] > bbackwards[1])
                {
                    *mgscore -= backwardspen[0]; *egscore  -= backwardspen[1];
                }
            }

            if (wadvanced[0] != -1 && wadvanced[0] >= bbackwards[0] && wadvanced[0] >= bbackwards[1] - 1)
            {
                // Passed pawn check
                int pos = ((wadvanced[0] << 4));
                if (wadvanced[0] >= bbackwards[1])
                {
                    if (board->board[pos + NORTH])
                    {
                        *mgscore += blockedmgbonus[wadvanced[0]], *egscore  += blockedegbonus[wadvanced[0]];
                    }
                    else
                    {
                        *mgscore += passedmgbonus[wadvanced[0]], *egscore  += passedegbonus[wadvanced[0]];
                    }
                    if (board->board[pos + SE] == WPAWN)
                    {
                        if (!board->board[pos + NORTH])
                        {
                            *mgscore += protectedpassedmg[wadvanced[0]], *egscore  += protectedpassedeg[wadvanced[0]];
                        }
                        else
                        {
                            *mgscore += blockedprotectedmg[wadvanced[0]], *egscore  += blockedprotectedeg[wadvanced[0]];
                        }
                    }
                }
                else if (!board->board[pos + NORTH])
                {
                    *mgscore += candidatepassedmg[wadvanced[0]], *egscore  += candidatepassedeg[wadvanced[0]];
                }
            }
            if (badvanced[0] != 9 && badvanced[0] <= wbackwards[0] && badvanced[0] <= wbackwards[1] + 1)
            {
                int pos = ((badvanced[0] << 4));
                if (badvanced[0] <= wbackwards[1])
                {
                    if (board->board[pos + SOUTH])
                    {
                        *mgscore -= blockedmgbonus[7 - badvanced[0]], *egscore  -= blockedegbonus[7 - badvanced[0]];
                    }
                    else
                    {
                        *mgscore -= passedmgbonus[7 - badvanced[0]], *egscore  -= passedegbonus[7 - badvanced[0]];
                    }
                    if (board->board[pos + NE] == BPAWN)
                    {
                        if (!board->board[pos + SOUTH])
                        {
                            *mgscore -= protectedpassedmg[7 - badvanced[0]], *egscore  -= protectedpassedeg[7 - badvanced[0]];
                        }
                        else
                        {
                            *mgscore -= blockedprotectedmg[7 - badvanced[0]], *egscore  -= blockedprotectedeg[7 - badvanced[0]];
                        }
                    }
                }
                else if (!board->board[pos + SOUTH])
                {
                    *mgscore -= candidatepassedmg[7 - badvanced[0]], *egscore  -= candidatepassedeg[7 - badvanced[0]];
                }
            }
        }
        else
        {
            // Pawn structure for b-g files
            if (wadvanced[i] != -1)
            {
                if (wadvanced[i - 1] == -1 && wadvanced[i + 1] == -1)
                { // evaluates isolated pawns for b-g files + passed pawns
                    *mgscore += isopen[0]; *egscore  += isopen[1];
                }
                else if (wbackwards[i] < wbackwards[i + 1] && wbackwards[i] < wbackwards[i - 1])
                {
                    *mgscore += backwardspen[0]; *egscore  += backwardspen[1];
                }
            }

            if (badvanced[i] != 9)
            {

                if (badvanced[i - 1] == 9 && badvanced[i + 1] == 9)
                {
                    *mgscore -= isopen[0]; *egscore  -= isopen[1];
                }
                else if (bbackwards[i] > bbackwards[i + 1] && bbackwards[i] > bbackwards[i - 1])
                {
                    *mgscore -= backwardspen[0]; *egscore  -= backwardspen[1];
                }
            }

            if (wadvanced[i] != -1 && wadvanced[i] >= bbackwards[i] && wadvanced[i] >= bbackwards[i - 1] - 1 && wadvanced[i] >= bbackwards[i + 1] - 1)
            {
                int pos = ((wadvanced[i] << 4)) + i;

                if (wadvanced[i] >= bbackwards[i - 1] && wadvanced[i] >= bbackwards[i + 1])
                {

                    if (board->board[pos + NORTH])
                    {
                        *mgscore += blockedmgbonus[wadvanced[i]], *egscore  += blockedegbonus[wadvanced[i]];
                    }
                    else
                    {
                        *mgscore += passedmgbonus[wadvanced[i]], *egscore  += passedegbonus[wadvanced[i]];
                    }
                    if (board->board[pos + SW] == WPAWN || board->board[pos + SE] == WPAWN)
                    {
                        if (!board->board[pos + NORTH])
                        {
                            *mgscore += protectedpassedmg[wadvanced[i]], *egscore  += protectedpassedeg[wadvanced[i]];
                        }
                        else
                        {
                            *mgscore += blockedprotectedmg[wadvanced[i]], *egscore  += blockedprotectedeg[wadvanced[i]];
                        }
                    }
                }
                else if (!board->board[pos + NORTH])
                {
                    *mgscore += candidatepassedmg[wadvanced[i]], *egscore  += candidatepassedeg[wadvanced[i]];
                }
            }
            if (badvanced[i] != 9 && badvanced[i] <= wbackwards[i - 1] + 1 && badvanced[i] <= wbackwards[i] && badvanced[i] <= wbackwards[i + 1] + 1)
            {
                int pos = ((badvanced[i] << 4)) + i;
                if (badvanced[i] <= wbackwards[i - 1] && badvanced[i] <= wbackwards[i + 1])
                {
                    if (board->board[pos + SOUTH])
                    {
                        *mgscore -= blockedmgbonus[7 - badvanced[i]], *egscore  -= blockedegbonus[7 - badvanced[i]];
                    }
                    else
                    {
                        *mgscore -= passedmgbonus[7 - badvanced[i]], *egscore  -= passedegbonus[7 - badvanced[i]];
                    }
                    if (board->board[pos + NW] == BPAWN || board->board[pos + NE] == BPAWN)
                    {
                        if (!board->board[pos + SOUTH])
                        {
                            *mgscore -= protectedpassedmg[7 - badvanced[i]], *egscore  -= protectedpassedeg[7 - badvanced[i]];
                        }
                        else
                        {
                            *mgscore -= blockedprotectedmg[7 - badvanced[i]], *egscore  -= blockedprotectedeg[7 - badvanced[i]];
                        }
                    }
                }
                else if (!board->board[pos + SOUTH])
                {
                    *mgscore -= candidatepassedmg[7 - badvanced[i]], *egscore  -= candidatepassedeg[7 - badvanced[i]];
                }
            }
        }
    }

    if (attackers[BLACK] > 1)
    {
        king_attack_count[BLACK] = MIN(king_attack_count[BLACK], 99);
        int pawnshield = 0;
        if (board->kingpos[WHITE] < 0x40)
        {
            // calculate king shelter. Friendly pawns within two ranks of the king (in front), as well as an enemy pawn right in front of the king, count as pawn shelter for a file.

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
        *mgscore -= kingdangertablemg[pawnshield][king_attack_count[BLACK]], *egscore  -= kingdangertableeg[pawnshield][king_attack_count[BLACK]];
        for (int i = 0; i < 5; i++){
            *mgscore -= tropism[0][i] * tropism_nums[BLACK][i], *egscore -= tropism[1][i] * tropism_nums[BLACK][i];
        }
    }

    if (attackers[WHITE] > 1)
    {
        king_attack_count[WHITE] = MIN(king_attack_count[WHITE], 99);
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
        *mgscore += kingdangertablemg[pawnshield][king_attack_count[WHITE]], *egscore  += kingdangertableeg[pawnshield][king_attack_count[WHITE]];
        for (int i = 0; i < 5; i++){
            *mgscore += tropism[0][i] * tropism_nums[WHITE][i], *egscore += tropism[1][i] * tropism_nums[WHITE][i];
        }
    }
    
    if (attacking_pieces[WHITE] > 1) // Having multiple threats at once is good.
    {
        *mgscore += multattacksbonus[0] * (attacking_pieces[WHITE] - 1);
        *egscore  += multattacksbonus[1] * (attacking_pieces[WHITE] - 1);
    }
    if (attacking_pieces[BLACK] > 1)
    {
        *mgscore -= multattacksbonus[0] * (attacking_pieces[BLACK] - 1);
        *egscore  -= multattacksbonus[1] * (attacking_pieces[BLACK] - 1);
    }

    // Add bishop pair bonuses
    if (board->pnbrqcount[WHITE][2] > 1)
    {
        *mgscore += bishop_pair[0][board->pnbrqcount[WHITE][0]];
        *egscore  += bishop_pair[1][board->pnbrqcount[WHITE][0]];
    }
    if (board->pnbrqcount[BLACK][2] > 1)
    {
        *mgscore -= bishop_pair[0][board->pnbrqcount[BLACK][0]];
        *egscore  -= bishop_pair[1][board->pnbrqcount[BLACK][0]];
    }

}

int eval(struct board_info *board, bool color)
{
    attackers[0] = 0, attackers[1] = 0;
    king_attack_count[0] = 0, king_attack_count[1] = 0;
    int phase = 0;
    int mgscore = 0, egscore  = 0;
    material(board, &phase, &mgscore, &egscore );
    int mtr = (phase * mgscore + (24 - phase) * egscore ) / 24;

    pst(board, phase, &mgscore, &egscore);
    int evl = (phase * mgscore + (24 - phase) * egscore ) / 24;
    if (color){
        mgscore -= tempo[0], egscore -= tempo[1];
    }
    else{
        mgscore += tempo[0], egscore += tempo[1];
    }
    if (evl >= 0 && mtr < 350 && board->pnbrqcount[WHITE][0] < 3 && phase < 7 && phase > 0) // Apply scaling.
    {           

        evl = evl * ((board->pnbrqcount[WHITE][0] + 1) * 3 - 1) / 10;
    }

    if (evl <= 0 && mtr > -350 && board->pnbrqcount[BLACK][0] < 3 && phase < 7 && phase > 0) // Same to Black.
    {                                                                         
        evl = evl * ((board->pnbrqcount[BLACK][0] + 1) * 3 - 1) / 10;
    }

    if (color == BLACK)
    {
        evl = -evl;
    }
    return evl;
}

#endif