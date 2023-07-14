#ifndef __board__
#define __board__

#include "constants.h"
#include "globals.h"
#include "nnue.h"
#include <stdio.h>

void printfull(struct board_info *board) // Prints the board
{
    int i = 0x70;
    while (i >= 0)
    {

        printf("+---+---+---+---+---+---+---+---+\n");
        for (int n = i; n != i + 8; n++)
        {
            printf("| ");
            if (board->board[n] == BLANK)
            {
                printf("  ");
            }
            else
            {

                switch (board->board[n])
                {
                case WPAWN:
                    printf("P ");
                    break;
                case WKNIGHT:
                    printf("N ");
                    break;
                case WBISHOP:
                    printf("B ");
                    break;
                case WROOK:
                    printf("R ");
                    break;
                case WQUEEN:
                    printf("Q ");
                    break;
                case WKING:
                    printf("K ");
                    break;
                case BPAWN:
                    printf("p ");
                    break;
                case BKNIGHT:
                    printf("n ");
                    break;
                case BBISHOP:
                    printf("b ");
                    break;
                case BROOK:
                    printf("r ");
                    break;
                case BQUEEN:
                    printf("q ");
                    break;
                default:
                    printf("k ");
                    break;
                }
            }
        }
        printf("|\n");
        i -= 0x10;
    }
    printf("+---+---+---+---+---+---+---+---+\n\n");
}
void setfull(struct board_info *board)  //Sets up the board for the start of the game.
{
    char brd[0x80] = {
        WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WROOK,0,0,0,0,0,0,0,0,
        WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN,0,0,0,0,0,0,0,0,
        BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING, BBISHOP, BKNIGHT, BROOK,0,0,0,0,0,0,0,0,

        };
    memcpy(board->board, brd, 0x80);
    char count[2][5] = {    //the number of pieces on the board
        {8, 2, 2, 2, 1},
        {8, 2, 2, 2, 1}};
    memcpy(board->pnbrqcount, count, 10);
    board->castling[0][0] = true, board->castling[0][1] = true, board->castling[1][0] = true, board->castling[1][1] = true; //castling data
    board->kingpos[0] = 0x4, board->kingpos[1] = 0x74;  //king position data
    board->epsquare = 0;    //en passant square
}

void setmovelist(struct movelist *movelst, int *key) // Sets up the list of moves in the game.
{

    movelst[0].fen = CURRENTPOS;
    *key = 1;
    movelst[0].move.move = 0;
    movelst[1].move.move = 0;
    movelst[0].halfmoves = 0;
    return;
}

void setfromfen(struct board_info *board, struct movelist *movelst, int *key, char *fenstring, bool *color, int start) // Given an FEN, sets up the board to it.
{
    int i = 7, n = 0;
    int fenkey = start;
    // Set default board parameters, edit them later on as we add pieces to the board/read more info from the FEN.
    board->castling[WHITE][0] = false, board->castling[BLACK][0] = false, board->castling[WHITE][1] = false, board->castling[BLACK][1] = false;
    for (int i = 0; i < 5; i++)
    {
        board->pnbrqcount[WHITE][i] = 0;
        board->pnbrqcount[BLACK][i] = 0;
    }
    while (!isblank(fenstring[fenkey]))
    {
        if (fenstring[fenkey] == '/') // Go to the next rank down.
        {
            i--;
            n = 0;
        }
        else if (isdigit(fenstring[fenkey])) // A number in the FEN (of form P3pp) means there's three empty squares, so we go past them
        {
            for (int b = 0; b < atoi(&fenstring[fenkey]); b++)
            {
                board->board[n + (i << 4)] = BLANK;
                n++;
            }
        }
        else // For each piece, add it to the board and add it to the material count. If it's a king, update king position.
        {
            switch (fenstring[fenkey])
            {
            case 'P':
                board->board[n + (i << 4)] = WPAWN;
                board->pnbrqcount[WHITE][0]++;
                break;
            case 'N':
                board->board[n + (i << 4)] = WKNIGHT;
                board->pnbrqcount[WHITE][1]++;
                break;
            case 'B':
                board->board[n + (i << 4)] = WBISHOP;
                board->pnbrqcount[WHITE][2]++;
                break;
            case 'R':
                board->board[n + (i << 4)] = WROOK;
                board->pnbrqcount[WHITE][3]++;
                break;
            case 'Q':
                board->board[n + (i << 4)] = WQUEEN;
                board->pnbrqcount[WHITE][4]++;
                break;
            case 'K':
                board->board[n + (i << 4)] = WKING;
                board->kingpos[WHITE] = n + (i << 4);
                break;
            case 'p':
                board->board[n + (i << 4)] = BPAWN;
                board->pnbrqcount[BLACK][0]++;
                break;
            case 'n':
                board->board[n + (i << 4)] = BKNIGHT;
                board->pnbrqcount[BLACK][1]++;
                break;
            case 'b':
                board->board[n + (i << 4)] = BBISHOP;
                board->pnbrqcount[BLACK][2]++;
                break;
            case 'r':
                board->board[n + (i << 4)] = BROOK;
                board->pnbrqcount[BLACK][3]++;
                break;
            case 'q':
                board->board[n + (i << 4)] = BQUEEN;
                board->pnbrqcount[BLACK][4]++;
                break;
            default:
                board->board[n + (i << 4)] = BKING;
                board->kingpos[BLACK] = n + (i << 4);
                break;
            }
            n++;
        } // sets up the board based on fen
        fenkey++;
    }
    for (int i = 0; i < 8; i++)
    {
        for (int n = 8; n < 16; n++)
        {
            board->board[i * 16 + n] = BLANK;
        }
    }

    while (isblank(fenstring[fenkey]))
    {
        fenkey++;
    }
    if (fenstring[fenkey] == 'w') // Gets the color to move.
    {
        *color = WHITE;
    }
    else
    {
        *color = BLACK;
    }
    fenkey++;

    calc_pos(board, *color); // Gets the Zobrist Hash of the position (it doesn't matter that we do it before castling stuff)
    nnue_state.reset_nnue(board);

    setmovelist(movelst, key);
    *key = 0;

    while (isblank(fenstring[fenkey]))
    {
        fenkey++;
    }

    if (fenstring[fenkey] == '-')
    {
        fenkey++;
    }

    else
    {
        if (fenstring[fenkey] == 'K') // Get castling rights information
        {
            board->castling[WHITE][1] = true;
            fenkey++;
        }
        if (fenstring[fenkey] == 'Q')
        {
            board->castling[WHITE][0] = true;
            fenkey++;
        }
        if (fenstring[fenkey] == 'k')
        {
            board->castling[BLACK][1] = true;
            fenkey++;
        }
        if (fenstring[fenkey] == 'q')
        {
            board->castling[BLACK][0] = true;
        }
        fenkey++;
    }

    while (isblank(fenstring[fenkey]))
    {
        fenkey++;
    }

    if (fenstring[fenkey] == '-') // Get en passant square information
    {
        board->epsquare = 0;
    }
    else
    {
        board->epsquare = (atoi(&fenstring[fenkey + 1]) - 1) * 16 + ((fenstring[fenkey] - 97));
        // In Willow, the en passant square is the square of the piece to be taken en passant, and not the square behind.
        if (*color)
        {
            board->epsquare += NORTH;
        }
        else
        {
            board->epsquare += SOUTH;
        }
        fenkey++;
    }
    fenkey++;

    while (isblank(fenstring[fenkey]))
    {
        fenkey++;
    }

    movelst[*key].halfmoves = atoi(&fenstring[fenkey]); // Get halfmoves information.
    *key += 1;
}

int move(struct board_info *board, struct move move, bool color) // Perform a move on the board.
{

    if (board->epsquare)
    {
        CURRENTPOS ^= ZOBRISTTABLE[773]; // if en passant was possible last move, xor it so it is not.
    }
    CURRENTPOS ^= ZOBRISTTABLE[772]; // xor for turn
    if (!move.move)
    {
        board->epsquare = 0;
        return 0;
    }
    nnue_state.push();
    unsigned char from = move.move >> 8, to = move.move & 0xFF; // get the indexes of the move
    unsigned char flag = move.flags >> 2;
    if ((from & 0x88) || (to & 0x88))
    {
        printf("out of board index! %4x %x %i %x\n", move.move, (board->epsquare), color, move.flags);
        printfull(board);
        exit(0);
        return 1;
    }

    CURRENTPOS ^= ZOBRISTTABLE[(((board->board[from] - 2) << 6)) + from - ((from >> 4) << 3)]; // xor out the piece to be moved
    nnue_state.update_feature<false>(board->board[from], MAILBOX_TO_STANDARD[from]);

    if (flag != 3)
    { // handle captures and en passant - this is for normal moves
        if (board->board[to])
        {
            board->pnbrqcount[color ^ 1][((board->board[to] >> 1) - 1)]--;
            CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to] - 2) << 6)) + to - ((to >> 4) << 3)];
            nnue_state.update_feature<false>(board->board[to], MAILBOX_TO_STANDARD[to]);
        }
    }
    else
    { // and this branch handles en passant
        board->pnbrqcount[color ^ 1][0]--;
        CURRENTPOS ^= ZOBRISTTABLE[(((board->board[board->epsquare] - 2) << 6)) + board->epsquare - ((board->epsquare >> 4) << 3)];
        nnue_state.update_feature<false>(board->board[board->epsquare], MAILBOX_TO_STANDARD[board->epsquare]);
        board->board[board->epsquare] = BLANK;
    }

    if (flag == 1)
    { // handle promotions
        board->pnbrqcount[color][0]--;
        board->board[from] = (((move.flags & 3) + 2) << 1) + color;
        board->pnbrqcount[color][((move.flags & 3) + 1)]++;
    }

    if (from == board->kingpos[color])
    { // handle king moves
        board->castling[color][0] = false;
        board->castling[color][1] = false;
        board->kingpos[color] = to;
    }

    if (board->board[from] == WROOK + color)
    { // handle rook moves
        if (from == 0x70 * color)
        { // turn off queenside castling
            board->castling[color][0] = false;
        }
        else if (from == 0x7 + 0x70 * color)
        { // turn off kingside castling
            board->castling[color][1] = false;
        }
    }

    board->board[to] = board->board[from];
    board->board[from] = BLANK;
    CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to] - 2) << 6)) + to - ((to >> 4) << 3)]; // xor in the piece that has been moved
    nnue_state.update_feature<true>(board->board[to], MAILBOX_TO_STANDARD[to]);

    if (flag == 2)
    { // castle
        if ((to & 7) == 6)
        { // to = g file, meaning kingside

            board->board[to - 1] = board->board[to + 1];
            CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to + 1] - 2) << 6)) + to + 1 - (((to + 1) >> 4) << 3)]; // xor out the rook on hfile and xor it in on ffile
            CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to - 1] - 2) << 6)) + to - 1 - (((to - 1) >> 4) << 3)];
            nnue_state.update_feature<true>(board->board[to - 1], MAILBOX_TO_STANDARD[to - 1]);
            nnue_state.update_feature<false>(board->board[to + 1], MAILBOX_TO_STANDARD[to + 1]);
            board->board[to + 1] = BLANK;
        }
        else
        {
            board->board[to + 1] = board->board[to - 2];
            CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to - 2] - 2) << 6)) + to - 2 - (((to - 2) >> 4) << 3)]; // xor out the rook on afile and xor it in on dfile
            CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to + 1] - 2) << 6)) + to + 1 - (((to + 1) >> 4) << 3)];
            nnue_state.update_feature<false>(board->board[to - 2], MAILBOX_TO_STANDARD[to - 2]);
            nnue_state.update_feature<true>(board->board[to + 1], MAILBOX_TO_STANDARD[to + 1]);
            board->board[to - 2] = BLANK;
        }
    }

    board->epsquare = 0;
    if (!flag && abs(to - from) == 32 && board->board[to] == WPAWN + color)
    {
        board->epsquare = to;
        CURRENTPOS ^= ZOBRISTTABLE[773];
    }
    // printfull(board);
    return 0;
}

void move_add(struct board_info *board, struct movelist *movelst, int *key, struct move mve, bool color, bool iscap) // Add a move to the list of moves in the game.
{
    int k = *key;
    movelst[k].move = mve;
    movelst[k].fen = CURRENTPOS;
    if ((mve.flags >> 2) == 1 || (board->board[(mve.move & 0xFF)] == WPAWN + color) || iscap)
    { // if the move is a capture, or a promotion, or a pawn move, half move clock is reset.
        movelst[k].halfmoves = 0;
    }
    else
    {
        movelst[k].halfmoves = movelst[k - 1].halfmoves + 1; // otherwise increment it
    }
    *key = k + 1;
}

bool isattacked(struct board_info *board, unsigned char pos, bool encolor) // Is a particular square attacked by an enemy piece?
{
    // pawns
    if (!encolor)
    {
        if ((!((pos + SW) & 0x88) && board->board[pos + SW] == WPAWN) || (!((pos + SE) & 0x88) && board->board[pos + SE] == WPAWN))
        {
            return true;
        }
    }
    else
    {
        if ((!((pos + NE) & 0x88) && board->board[pos + NE] == BPAWN) || (!((pos + NW) & 0x88) && board->board[pos + NW] == BPAWN))
        {
            return true;
        }
    }
    // knights, kings, and sliders. Work outwards from the square until we bump into a piece/edge of board.
    unsigned char d, f;
    for (f = 0; f < 8; f++)
    {
        d = pos + vector[0][f];

        if (!(d & 0x88) && board->board[d] - encolor == WKNIGHT) // if we bump into a knight on its vector, we're attacked by it and return true
        {
            return true;
        }

        char vec = vector[4][f];
        d = pos + vec;

        if ((d & 0x88))
        {
            continue;
        }

        if (board->board[d] - encolor == WKING) // if we hit a king on our first square we're attacked by it
        {
            return true;
        }
        do
        {
            if (board->board[d])
            {
                // Queens attack a piece on all diagonals/sides, rooks attack only orthogonally, bishops attack only diagonally
                if ((board->board[d] & 1) == encolor &&
                    (board->board[d] - encolor == WQUEEN || (((f & 1) && board->board[d] - encolor == WROOK) || (!(f & 1) && board->board[d] - encolor == WBISHOP))))
                {
                    return true;
                }
                break;
            }
            d += vec;
        } while (!(d & 0x88));
    }

    return false;
}
char isattacked_mv(struct board_info *board, unsigned char pos, bool encolor)
// Same as above, but ignores kings. Slight speed boost for eval purposes.
{
    char flag = 0;
    // pawns
    if (!encolor)
    {
        if ((!((pos + SW) & 0x88) && board->board[pos + SW] == WPAWN) || (!((pos + SE) & 0x88) && board->board[pos + SE] == WPAWN))
        {
            return 2;
        }
    }
    else
    {
        if ((!((pos + NE) & 0x88) && board->board[pos + NE] == BPAWN) || (!((pos + NW) & 0x88) && board->board[pos + NW] == BPAWN))
        {
            return 2;
        }
    }
    // knights, kings, and sliders
    unsigned char d, f;
    for (f = 0; f < 8; f++)
    {
        d = pos + vector[0][f];

        if (!(d & 0x88) && board->board[d] - encolor == WKNIGHT)
        {
            return 2;
        }

        char vec = vector[4][f];
        d = pos + vec;

        if ((d & 0x88))
        {
            continue;
        }

        if (board->board[d] - encolor == WKING)
        {
            flag = 1;
            continue;
        }
        do
        {
            if (board->board[d])
            {
                if ((board->board[d] & 1) == encolor &&
                    (board->board[d] - encolor == WQUEEN || (((f & 1) && board->board[d] - encolor == WROOK) || (!(f & 1) && board->board[d] - encolor == WBISHOP))))
                {
                    return 2;
                }
                break;
            }
            d += vec;
        } while (!(d & 0x88));
    }

    return flag;
}

bool checkdraw1(struct board_info *board) // checks for material draws.
{
    if (board->pnbrqcount[0][0] || board->pnbrqcount[0][3] || board->pnbrqcount[0][4] ||
        board->pnbrqcount[1][0] || board->pnbrqcount[1][3] || board->pnbrqcount[1][4])
    {
        return false;
    }
    if (board->pnbrqcount[0][2] > 1 || board->pnbrqcount[1][2] > 1)
    {
        return false;
    }
    if (board->pnbrqcount[0][1] > 2 || board->pnbrqcount[1][1] > 2)
    {
        return false;
    }
    if ((board->pnbrqcount[0][1] && board->pnbrqcount[0][2]) || (board->pnbrqcount[0][1] && board->pnbrqcount[0][2]))
    {
        return false;
    }
    return true;
}
int checkdraw2(struct movelist *movelst, int *key) // checks for repetition draws.
{
    if (movelst[*key - 1].halfmoves > 99)
    {
        return 2;
    }
    int lmove = *key - 1;
    int k = lmove - 2;
    int rep = 0;

    while (k >= 1 && k >= lmove - 100)
    {
        if (movelst[k].fen == movelst[lmove].fen)
        {
            return 1;
        }
        k -= 2;
    }
    return rep;
}

int get_cheapest_attacker(struct board_info *board, unsigned int pos, unsigned int *attacker, bool encolor)
{ // returns 0 - 6 from blank to king
    char flag = 10;
    *attacker = 0;
    // pawns
    if (!encolor)
    {
        if (!((pos + SW) & 0x88) && board->board[pos + SW] == WPAWN)
        {
            *attacker = pos + SW; // immediately return because we're not hitting anything less valuable than a pawn!
            return 1;
        }
        else if (!((pos + SE) & 0x88) && board->board[pos + SE] == WPAWN)
        {
            *attacker = pos + SE;
            return 1;
        }
    }
    else
    {
        if (!((pos + NE) & 0x88) && board->board[pos + NE] == BPAWN)
        {
            *attacker = pos + NE;
            return 1;
        }
        else if (!((pos + NW) & 0x88) && board->board[pos + NW] == BPAWN)
        {
            *attacker = pos + NW;
            return 1;
        }
    }
    // knights, kings, and sliders
    unsigned char d, f;
    for (f = 0; f < 8; f++)
    {
        d = pos + vector[0][f];

        if (!(d & 0x88) && board->board[d] - encolor == WKNIGHT)
        {
            *attacker = d;
            return 2; // return if we hit a knight because we already checked for pawns.
        }

        char vec = vector[4][f];
        d = pos + vec;

        if ((d & 0x88))
        {
            continue;
        }

        if (board->board[d] - encolor == WKING)
        {
            if (flag > 6)
            {
                *attacker = d;
                flag = 6;
            }
            continue;
        }
        do
        {
            if (board->board[d])
            {
                if ((board->board[d] & 1) == encolor &&
                    (board->board[d] - encolor == WQUEEN || (((f & 1) && board->board[d] - encolor == WROOK) || (!(f & 1) && board->board[d] - encolor == WBISHOP))))
                {
                    if ((board->board[d] >> 1) < flag)
                    {
                        flag = (board->board[d] >> 1);
                        *attacker = d;
                    }
                }
                break;
            }
            d += vec;
        } while (!(d & 0x88));
    }

    return flag;
}

#endif