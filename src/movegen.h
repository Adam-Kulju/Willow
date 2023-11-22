#ifndef __movegen__
#define __movegen__
#include "constants.h"
#include "board.h"

void pawnmoves(struct board_info *board, struct list *list, int *key, unsigned char pos, bool color)
{

    if (!color)
    {

        unsigned char diff = pos + NORTH;

        if (!board->board[diff])
        { // if the square in front is empty, go forwards
            unsigned short int t = pos << 8;
            // Brings the current position to the beginning of the integer; if our move was h8-h7, this sets it up as 0x7700 so we can ad the move to h7 (0x7767)
            if (((diff) >> 4) == 7)
            {                                                                   // we have a promotion, so add all four promotions to the movelist.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4; // 4 = knight, 5 = bishop, 6 = rook, 7 = queen
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;
            }
            else
            {
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 0;
                // go forwards and see if we can go forwards again

                if ((pos >> 4) == 1 && !board->board[diff + NORTH])
                {

                    list[*key].move.move = t + diff + NORTH, list[(*key)++].move.flags = 0;
                }
            }
        }
        diff++;
        if (!(diff & 0x88) && board->board[diff] && (board->board[diff] & 1))
        { // capture to the right

            unsigned short int t = pos << 8;
            if (((diff) >> 4) == 7)
            { // we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;
            }
            else
            {
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 0;
            }
        }
        diff -= 2;
        if (!(diff & 0x88) && board->board[diff] && (board->board[diff] & 1))
        { // capture left
            unsigned short int t = pos << 8;
            if (((diff) >> 4) == 7)
            { // we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;
            }
            else
            {
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 0;
            }
        }
    }
    else
    {
        unsigned char diff = pos + SOUTH;

        if (!board->board[diff])
        { // if the square in front is empty, go forwards
            unsigned short int t = pos << 8;
            if (((diff) >> 4) == 0)
            { // we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;
            }
            else
            {
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 0; // go forwards and see if we can go forwards again

                if ((pos >> 4) == 6 && !board->board[diff + SOUTH])
                {

                    list[*key].move.move = t + diff + SOUTH, list[(*key)++].move.flags = 0;
                }
            }
        }
        diff++;
        if (!(diff & 0x88) && board->board[diff] && !(board->board[diff] & 1))
        { // capture to the right
            unsigned short int t = pos << 8;
            if (((diff) >> 4) == 0)
            { // we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;
            }
            else
            {
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 0;
            }
        }
        diff -= 2;
        if (!(diff & 0x88) && board->board[diff] && !(board->board[diff] & 1))
        {
            // capture left
            unsigned short int t = pos << 8;
            if (((diff) >> 4) == 0)
            { // we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;
            }
            else
            {
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 0;
            }
        }
    }
}

int movegen(struct board_info *board, struct list *list, bool color, bool incheck)
{ // a full pseudolegal move gen
    int key = 0;
    for (unsigned char i = 0; i < 0x80; i++)
    {
        if (i & 0x88)
        {
            i += 7;
            continue;
        }
        if (board->board[i] && (board->board[i] & 1) == color)
        {
            int piecetype = (board->board[i] >> 1) - 2;
            if (piecetype == -1)
            { // handle pawns
                pawnmoves(board, list, &key, i, color);
            }

            else
            {
                for (unsigned char dir = 0; dir < vectors[piecetype]; dir++)
                {
                    // Go in the directions that the piece can move, until we hit another piece or the edge of the board; then move on to the next direction.
                    for (int pos = i;;)
                    {
                        pos += vector[piecetype][dir];
                        if ((pos & 0x88) || (board->board[pos] && (board->board[pos] & 1) == color))
                        {
                            break;
                        }
                        list[key].move.move = (i << 8) + pos, list[key++].move.flags = 0;
                        if (board->board[pos] || !slide[piecetype])
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    if (board->epsquare)
    {
        // handle special moves; en passant and castling. If the en passant square (triggered by opponent moving a pawn two squares) is beside our pawn we can take it en passant.
        if (!((board->epsquare + 1) & 0x88) && board->board[board->epsquare + 1] == WPAWN + color)
        {
            if (!color)
            {
                list[key].move.move = ((board->epsquare + 1) << 8) + board->epsquare + NORTH, list[key++].move.flags = 0xC;
            }
            else
            {
                list[key].move.move = ((board->epsquare + 1) << 8) + board->epsquare + SOUTH, list[key++].move.flags = 0xC;
            }
        }
        if (!((board->epsquare - 1) & 0x88) && board->board[board->epsquare - 1] == WPAWN + color)
        {
            if (!color)
            {
                list[key].move.move = ((board->epsquare - 1) << 8) + board->epsquare + NORTH, list[key++].move.flags = 0xC;
            }
            else
            {
                list[key].move.move = ((board->epsquare - 1) << 8) + board->epsquare + SOUTH, list[key++].move.flags = 0xC;
            }
        }
    }
    if (incheck)
    {
        return key;
    }

    if (IS_DFRC){
        if (board->castling[color][0]) // If we have not moved our king or rook, are not in check, and will not castle through check, castling is pseudolegal
                                   // queenside castling
        {
            char x;
            bool flag = true;
            if (board->board[board->rookstartpos[color][0]] != WROOK + color){
                flag = false;
            }

            if (flag){
                for (x = MAX(board->kingpos[color] - 1, color * 0x70 + 3); x > MIN(color * 0x70 + 1, board->rookstartpos[color][0]); x--){ //handle xxxxxRKR, RxxxxxKR, and RKxxxxxR
                    if (board->board[x] && board->board[x] != WKING + color && !(board->board[x] == WROOK + color && board->rookstartpos[color][0] == x)){      
                                                                                //if you have xxxxxRKR, you want to pretend that the R on the left doesn't exist
                                                                                //on the other hand, xxRRKxxx can't be ignored
                        flag = false;
                        break;
                    }
                }
            }

            if (flag){
                for (x = board->kingpos[color] - 1; x > color * 0x70 + 2; x--){ //next, make sure we're not traveling through check. we don't need to handle RK----R- or R-K--R-- as both have no intermediate squares
                    if (isattacked(board, x, color^1)){
                        flag = false;
                        break;
                    }
                }               
            }
            if (flag){
                list[key].move.move = ((board->kingpos[color]) << 8) + board->rookstartpos[color][0], list[key++].move.flags = 0x8;
            }
        }  

        if (board->castling[color][1])
                                   // kingside castling
        {
            char x;
            bool flag = true;

            if (board->board[board->rookstartpos[color][1]] != WROOK + color){
                flag = false;
            }

            if (flag){

                for (x = MIN(board->kingpos[color] + 1, color * 0x70 + 5); x < color * 0x70 + 7; x++){ //handle xxxxxRKR, RxxxxxKR, and RKxxxxxR - here we never have to check if the h-file is occupied
                    if (board->board[x] && !(board->board[x] == WROOK + color && board->rookstartpos[color][1] == x)){      //if you have xRKRxxxx, you want to pretend that the R on the right doesn't exist
                                                                                                                    
                        flag = false;
                        break;
                    }
                }
            }

            if (flag){
                for (x = board->kingpos[color] + 1; x < color * 0x70 + 6; x++){ //next, make sure we're not traveling through check. we don't need to handle RK----R- or R-K--R-- as both have no intermediate squares
                    if (isattacked(board, x, color^1)){
                        flag = false;
                        break;
                    }
                }               
            }
            if (flag){
                list[key].move.move = ((board->kingpos[color]) << 8) + board->rookstartpos[color][1], list[key++].move.flags = 0x8;
            }
        }
    }

    else{
        if (board->castling[color][0]) // If we have not moved our king or rook, are not in check, and will not castle through check, castling is pseudolegal
                                   // queenside castling
        {
            char x = board->kingpos[color];
            if (board->board[x - 4] - color == WROOK && !board->board[x - 3] && !board->board[x - 2] && !board->board[x - 1] && !isattacked(board, x - 1, color ^ 1))
            {
                list[key].move.move = (x << 8) + x + WEST + WEST, list[key++].move.flags = 0x8;
            }
        }
        if (board->castling[color][1]) // kingside castling
        {
            char x = board->kingpos[color];
            if (board->board[x + 3] - color == WROOK && !board->board[x + 2] && !board->board[x + 1] && !isattacked(board, x + 1, color ^ 1))
            {
                list[key].move.move = ((board->kingpos[color]) << 8) + x + EAST + EAST, list[key++].move.flags = 0x8;
            }
        }
    }
    return key;
}

bool static_exchange_evaluation(struct board_info *board, struct move mve, bool color, int threshold)
{
    // Does a static exchange evaluation of the current position.

    if (mve.flags && mve.flags >> 2 != 1)
    { // castling and en passant both come out neutral; thus we can return immediately.
        return (threshold <= 0);
    }
    int to = mve.move & 0xFF;

    int gain = SEEVALUES[board->board[mve.move & 0xFF] >> 1]; // what you get from taking the piece
    int risk = SEEVALUES[board->board[mve.move >> 8] >> 1];   // what you lose if your piece gets taken!

    struct board_info board2 = *board;

    board2.board[mve.move >> 8] = BLANK;

    unsigned int attacker_pos = 0;

    int totalgain = gain - threshold; // how much you have gained after the first capture you made.

    if (totalgain < 0)
    {
        return false;
    }

    while (totalgain - risk < 0)
    { // an example with RxB BxR RxB

        int i = get_cheapest_attacker(&board2, to, &attacker_pos, color ^ 1);
        if (i == 10)
        {
            return true; // if opponent had no attackers, we just took a free piece!
        }

        totalgain -= risk;   // first we subtract the value of the piece that you just had taken (since this is from opponent's point of view). in our RXB case, that was a bishop, so we're now at -200.
        risk = SEEVALUES[i]; // if the bishop gets taken, he loses 300 (a minor piece).
        if (totalgain + risk < 0)
        { // if the piece he took with was low enough value that you can't get the material back by taking it (2 pawns for a bishop), return. not true here.
            return false;
        }
        board2.board[attacker_pos] = BLANK; // remove the piece to account for batteries.
                                            // and yeah this fails to discovered checks, but a discovered check will quickly cause a beta cutoff anyways so no big deal.

        i = get_cheapest_attacker(&board2, to, &attacker_pos, color); // now we look at it from our point of view.
        if (i == 10)
        { // if we have no attackers, it is losing, since the condition in the while loop guarantees at least equal if we do something like BxB.
            return false;
        }
        totalgain += risk; // we were at -200 before, now we're at 100.
        risk = SEEVALUES[i];

        board2.board[attacker_pos] = BLANK;
    }
    // another example: RxB QxR PxQ
    // RxB is not winning in and of itself, so we get the smallest attacker which turns out to be Q.
    // this sets totalgain to -300, but risk to 900.
    // we can take it, and it's with a pawn. Total gain becomes 600, risk becomes 100, and we return true.
    return true;
}

int see(struct board_info *board, struct move mve, bool color, int threshold)
{
    // Orders captures.
    if (mve.flags == 0xC) // En passant is rated just above all the other PxP captures.
    {
        return 2000000 + 100 * 10;
    }

    int attacker = SEEVALUES[board->board[(mve.move >> 8)] >> 1], victim = SEEVALUES[board->board[(mve.move & 0xFF)] >> 1];
    // Get the values of the victim and the attacker

    int v = 2000000; // ciekce's magic v!

    if (victim - attacker < threshold && !static_exchange_evaluation(board, mve, color, threshold))
    {
        // If the piece we're trying to capture is worth less than the attacker, and a static exchange evaluation also verifies the move looks bad, order it at the very end
        v = -v;
    }
    return v + (victim * 10) - (attacker / 100);
}

void selectionsort(struct list *list, int k, int t)
{
    // Performs a selection sort on the list, starting from index k
    int temp = k;
    int i = k;
    while (i < t)
    {
        if (list[i].eval > list[temp].eval)
        {
            temp = i;
        }
        i++;
    }
    struct list tempmove = list[temp];
    list[temp] = list[k];
    list[k] = tempmove;
}

int movescore(struct board_info *board, struct movelist *movelst, int *key, struct list *list, int depth, bool color, char type, int movelen, int threshold, ThreadInfo *thread_info, ttentry entry, bool score_quiets)
{
    // Given a list of moves, scores them for move ordering purposes.

    int lastpiecetype = 0, lastpiecedest = 0;
    bool isreply = false;
    if (depth > 0 && movelst[*key-1].piecetype != -1 && depth < 99){
        isreply = true;
        lastpiecetype = board->board[movelst[*key-1].move.move & 0xFF] - 2;
        lastpiecedest = movelst[*key-1].move.move & 0xFF;
    } 

    int i = 0;
    while (i < movelen)
    {
        list[i].eval = 1000000; // base

        /*if (isreply && movelst[*key-1].move.move != 0 && movelst[*key-1].piecetype < 0)
        {
            for (int i = 0; i < *key; i++){
                printf("%x\n", movelst[i].move.move);
            }
            printfull(board);
            exit(1);
        }*/

        if (type > None && ismatch(entry.bestmove, list[i].move)) // TT hit: gets the largest bonus.
        {
            list[i].eval += 10000000;
        }

        else if (list[i].move.flags == 7) // Queen promotions are almost certainly good for us, so order them right below the TT move.
        {
            list[i].eval += 5000000 + SEEVALUES[board->board[(list[i].move.move & 0xFF)] >> 1];
        }
        else if (board->board[list[i].move.move & 0xFF] && list[i].move.flags != 8) // Score captures with MVV-LVA and SEE.
        {
            list[i].eval = see(board, list[i].move, color, threshold) + thread_info->CAPHIST[color][list[i].move.move >> 8][list[i].move.move & 0xFF];
        }
        else if (!score_quiets){
            i++;
            continue;
        }
        else if (ismatch(list[i].move, thread_info->KILLERTABLE[depth][0])) // Killer moves
        {
            list[i].eval += 199;
        }

        else // And if none of those apply, score the move by its history score.
        {
            list[i].eval = thread_info->HISTORYTABLE[color][list[i].move.move >> 8][list[i].move.move & 0xFF];
            if (isreply){
                list[i].eval += thread_info->CONTHIST[lastpiecetype][lastpiecedest][board->board[list[i].move.move >> 8] - 2][list[i].move.move & 0xFF];
            }
            if (depth > 1 && depth < 90 &&  movelst[*key-2].piecetype != -1){
                list[i].eval += thread_info->CONTHIST[movelst[*key-2].piecetype][movelst[*key-2].move.move & 0xFF][board->board[list[i].move.move >> 8] - 2][list[i].move.move & 0xFF];
            }
            if (depth > 3 && depth < 90 && movelst[*key-4].piecetype != -1){
                list[i].eval += thread_info->CONTHIST[movelst[*key-4].piecetype][movelst[*key-4].move.move & 0xFF][board->board[list[i].move.move >> 8] - 2][list[i].move.move & 0xFF];
            }
        }

        i++;
    }
    return 0;
}

#endif