#ifndef __board__
#define __board__

#include "constants.h"
#include "globals.h"
#include <stdio.h>

void printfull(struct board_info *board){
    int i = 0x70;
    while (i >= 0){

        printf("+---+---+---+---+---+---+---+---+\n");
        for (int n = i; n != i+8; n++){
        printf("| ");
            if (board->board[n] == BLANK){
                printf("  ");
            }
            else{

                switch(board->board[n]){
                    case WPAWN:
                    printf("P "); break;
                    case WKNIGHT:
                    printf("N "); break;
                    case WBISHOP:
                    printf("B "); break;
                    case WROOK:
                    printf("R "); break;
                    case WQUEEN:
                    printf("Q "); break;
                    case WKING:
                    printf("K "); break;
                    case BPAWN:
                    printf("p "); break;
                    case BKNIGHT:
                    printf("n "); break;
                    case BBISHOP:
                    printf("b "); break;
                    case BROOK:
                    printf("r "); break;
                    case BQUEEN:
                    printf("q "); break;
                    default:
                    printf("k "); break;
                }
            }
        }
        printf("|\n");
        i -= 0x10;
    }
    printf("+---+---+---+---+---+---+---+---+\n\n");
}
void setfull(struct board_info *board){
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
    char count[2][5] = {
        {8, 2, 2, 2, 1},
        {8, 2, 2, 2, 1}
    };
    memcpy(board->pnbrqcount, count, 10);
    board->castling[0][0] = true, board->castling[0][1] = true, board->castling[1][0] = true, board->castling[1][1] = true;
    board->kingpos[0] = 0x4, board->kingpos[1] = 0x74;
    board->epsquare = 0;
}

void setmovelist(struct movelist *movelst, int *key){
    
    movelst[0].fen = CURRENTPOS;
    *key = 1;
    movelst[0].move.move = 0;
    movelst[1].move.move = 0;
    movelst[0].halfmoves = 0;
    return;
}


void setfromfen(struct board_info *board, struct movelist *movelst, int *key, char *fenstring, bool *color, int start){
    int i = 7, n = 0;
    int fenkey = start;
    board->castling[WHITE][0] = false, board->castling[BLACK][0] = false, board->castling[WHITE][1] = false, board->castling[BLACK][1] = false;
    for (int i = 0; i < 5; i++){
        board->pnbrqcount[WHITE][i] = 0;
        board->pnbrqcount[BLACK][i] = 0;
    }
    while (!isblank(fenstring[fenkey])){
        if (fenstring[fenkey] == '/'){
            i--;
            n = 0;
        }
        else if (isdigit(fenstring[fenkey])){
            for (int b = 0; b < atoi(&fenstring[fenkey]); b++){
                board->board[n+(i<<4)] = BLANK;
                n++;
            }
        }
        else{
                switch(fenstring[fenkey]){
                    case 'P':
                    board->board[n+(i<<4)] = WPAWN; board->pnbrqcount[WHITE][0]++; break;
                    case 'N':
                    board->board[n+(i<<4)] = WKNIGHT; board->pnbrqcount[WHITE][1]++; break;
                    case 'B':
                    board->board[n+(i<<4)] = WBISHOP; board->pnbrqcount[WHITE][2]++; break;
                    case 'R':
                    board->board[n+(i<<4)] = WROOK; board->pnbrqcount[WHITE][3]++; break;
                    case 'Q':
                    board->board[n+(i<<4)] = WQUEEN; board->pnbrqcount[WHITE][4]++; break;
                    case 'K':
                    board->board[n+(i<<4)] = WKING; board->kingpos[WHITE] = n+(i<<4); break;
                    case 'p':
                    board->board[n+(i<<4)] = BPAWN; board->pnbrqcount[BLACK][0]++; break;
                    case 'n':
                    board->board[n+(i<<4)] = BKNIGHT; board->pnbrqcount[BLACK][1]++; break;
                    case 'b':
                    board->board[n+(i<<4)] = BBISHOP; board->pnbrqcount[BLACK][2]++; break;
                    case 'r':
                    board->board[n+(i<<4)] = BROOK; board->pnbrqcount[BLACK][3]++; break;
                    case 'q':
                    board->board[n+(i<<4)] = BQUEEN; board->pnbrqcount[BLACK][4]++; break;
                    default:
                    board->board[n+(i<<4)] = BKING; board->kingpos[BLACK] = n+(i<<4); break;
                }         
                n++;   
        }   //sets up the board based on fen
        fenkey++;
    }

    while (isblank(fenstring[fenkey])){
        fenkey++;
    }
    if (fenstring[fenkey] == 'w'){
        *color = WHITE;
    }
    else{
        *color = BLACK;  //color to move
    }

    fenkey++;

    calc_pos(board, *color);

    setmovelist(movelst, key);
    *key = 0;

    while (isblank(fenstring[fenkey])){
        fenkey++;
    }

    if (fenstring[fenkey] == '-'){
        fenkey++;
    }

    else{
        if (fenstring[fenkey] == 'K'){
            board->castling[WHITE][1] = true;
            fenkey++;
        }
        if (fenstring[fenkey] == 'Q'){
            board->castling[WHITE][0] = true;
            fenkey++;
        }
        if (fenstring[fenkey] == 'k'){
            board->castling[BLACK][1] = true;
            fenkey++;
        }
        if (fenstring[fenkey] == 'q'){
            board->castling[BLACK][0] = true;
        }
        fenkey++;
    }

    while (isblank(fenstring[fenkey])){
        fenkey++;
    }

    if (fenstring[fenkey] == '-'){
        board->epsquare = 0;
    }
    if (fenstring[fenkey] == '-'){
        board->epsquare = 0;
    }
    else{
        board->epsquare = (atoi(&fenstring[fenkey+1])-1)*16 + ((fenstring[fenkey]-97));
        if (*color){
            board->epsquare += NORTH;
        }
        else{
            board->epsquare += SOUTH;
        }
        fenkey++;
    }
    fenkey++;

    while (isblank(fenstring[fenkey])){
        fenkey++;
    }    


    movelst[*key].halfmoves = atoi(&fenstring[fenkey]);
    *key += 1;
}

int move(struct board_info *board, struct move move, bool color){
    if (board->epsquare){
        CURRENTPOS ^= ZOBRISTTABLE[773]; //if en passant was possible last move, xor it so it is not.
    }
    CURRENTPOS ^= ZOBRISTTABLE[772]; //xor for turn
    if (!move.move){
        board->epsquare = 0;
        return 0;
    }
    unsigned char from = move.move>>8, to = move.move & 0xFF;
    unsigned char flag = move.flags >> 2;
    if ((from & 0x88) || (to & 0x88)){
        printf("out of board index! %4x %x %i %x\n", move.move, (board->epsquare), color, move.flags);
        printfull(board);    
        return 1;
    }

    CURRENTPOS ^= ZOBRISTTABLE[(((board->board[from]-2)<<6))+from-((from>>4)<<3)]; //xor out the piece to be moved
    

    if (flag != 3){ //handle captures and en passant
            if (board->board[to]){  
                board->pnbrqcount[color^1][((board->board[to]>>1) - 1)]--;
                CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to]-2)<<6))+to-((to>>4)<<3)];
            }
    }
    else{
        board->pnbrqcount[color^1][0]--;
        CURRENTPOS ^= ZOBRISTTABLE[(((board->board[board->epsquare]-2)<<6))+board->epsquare-((board->epsquare>>4)<<3)];
        board->board[board->epsquare] = BLANK;  
    }


    if (flag == 1){ //handle promotions
        board->pnbrqcount[color][0]--;
        board->board[from] = (((move.flags&3)+2)<<1) + color;
        board->pnbrqcount[color][((move.flags&3)+1)]++;
    }

    if (from == board->kingpos[color]){ //handle king moves
        board->castling[color][0] = false;
        board->castling[color][1] = false;
        board->kingpos[color] = to;
    }

    if (board->board[from] == WROOK + color){ //handle rook moves
        if (from == 0x70*color){ //turn off queenside castling
            board->castling[color][0] = false;
        }
        else if (from == 0x7 + 0x70*color){ //turn off kingside castling
            board->castling[color][1] = false;
        }
    }

    board->board[to] = board->board[from];
    board->board[from] = BLANK;
    CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to]-2)<<6))+to-((to>>4)<<3)]; //xor in the piece that has been moved


    if (flag == 2){ //castle
        if ((to & 7) == 6){ //to = g file, meaning kingside

            board->board[to-1] = board->board[to+1];
            CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to+1]-2)<<6))+to+1-(((to+1)>>4)<<3)]; //xor out the rook on hfile and xor it in on ffile
            CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to-1]-2)<<6))+to-1-(((to-1)>>4)<<3)];
            board->board[to+1] = BLANK;
        }
        else{
            board->board[to+1] = board->board[to-2];
            CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to-2]-2)<<6))+to-2-(((to-2)>>4)<<3)]; //xor out the rook on afile and xor it in on dfile
            CURRENTPOS ^= ZOBRISTTABLE[(((board->board[to+1]-2)<<6))+to+1-(((to+1)>>4)<<3)];
            board->board[to-2] = BLANK;
        }
    }

    board->epsquare = 0;
    if (!flag && abs(to-from) == 32 && board->board[to] == WPAWN+color){
        board->epsquare = to;
        CURRENTPOS ^= ZOBRISTTABLE[773]; 
    }
    //printfull(board);
    return 0;

}

void move_add(struct board_info *board, struct movelist *movelst, int *key, struct move mve, bool color, bool iscap){
    int k = *key;
    movelst[k].move = mve;
    movelst[k].fen = CURRENTPOS;
    if ((mve.flags>>2) == 1 || (board->board[(mve.move&0xFF)] == WPAWN+color) || iscap){ //if the move is a capture, or a promotion, or a pawn move, half move clock is reset.
        movelst[k].halfmoves = 0;
    }
    else{
        movelst[k].halfmoves = movelst[k-1].halfmoves+1;

    }
    *key = k+1;
}

bool isattacked(struct board_info *board, unsigned char pos, bool encolor){
    //pawns
    if (!encolor){
        if ((!((pos+SW)&0x88) && board->board[pos+SW] == WPAWN) || (!((pos+SE)&0x88) && board->board[pos+SE] == WPAWN)){
            return true;
        }
    }
    else{
        if ((!((pos+NE)&0x88) && board->board[pos+NE] == BPAWN) || (!((pos+NW)&0x88) && board->board[pos+NW] == BPAWN)){
            return true;
        }        
    }
    //knights, kings, and sliders
    unsigned char d, f;
    for (f = 0; f < 8; f++){
        d = pos + vector[0][f];
        
        if (!(d&0x88) && board->board[d]-encolor == WKNIGHT){
            return true;
        }

        char vec = vector[4][f];
        d = pos + vec;

        if ((d&0x88)){continue;}

        if (board->board[d]-encolor == WKING){
            return true;
        }
        do{
            if (board->board[d]){
                if ((board->board[d]&1) == encolor && 
                (board->board[d]-encolor == WQUEEN || ( ((f&1) && board->board[d]-encolor == WROOK) || (!(f&1) && board->board[d]-encolor == WBISHOP) ))){
                    return true;
                }
                break;
            }
            d += vec;
        } 
        while (!(d&0x88));
        
    }

    return false;
}
char isattacked_mv(struct board_info *board, unsigned char pos, bool encolor){
    char flag = 0;
    //pawns
    if (!encolor){
        if ((!((pos+SW)&0x88) && board->board[pos+SW] == WPAWN) || (!((pos+SE)&0x88) && board->board[pos+SE] == WPAWN)){
            return 2;
        }
    }
    else{
        if ((!((pos+NE)&0x88) && board->board[pos+NE] == BPAWN) || (!((pos+NW)&0x88) && board->board[pos+NW] == BPAWN)){
            return 2;
        }        
    }
    //knights, kings, and sliders
    unsigned char d, f;
    for (f = 0; f < 8; f++){
        d = pos + vector[0][f];
        
        if (!(d&0x88) && board->board[d]-encolor == WKNIGHT){
            return 2;
        }

        char vec = vector[4][f];
        d = pos + vec;

        if ((d&0x88)){continue;}

        if (board->board[d]-encolor == WKING){
            flag = 1; continue;
        }
        do{
            if (board->board[d]){
                if ((board->board[d]&1) == encolor && 
                (board->board[d]-encolor == WQUEEN || ( ((f&1) && board->board[d]-encolor == WROOK) || (!(f&1) && board->board[d]-encolor == WBISHOP) ))){
                    return 2;
                }
                break;
            }
            d += vec;
        } 
        while (!(d&0x88));
        
    }

    return flag;
}

bool checkdraw1(struct board_info *board){
    if (board->pnbrqcount[0][0] || board->pnbrqcount[0][3] || board->pnbrqcount[0][4] || 
        board->pnbrqcount[1][0] || board->pnbrqcount[1][3] || board->pnbrqcount[1][4]){
        return false;
    }
    if (board->pnbrqcount[0][2] > 1 || board->pnbrqcount[1][2] > 1){
        return false;
    }
    if (board->pnbrqcount[0][1] > 2 || board->pnbrqcount[1][1] > 2){
        return false;
    }
    if ((board->pnbrqcount[0][1] && board->pnbrqcount[0][2]) || (board->pnbrqcount[0][1] && board->pnbrqcount[0][2])){
        return false;
    }
    return true;

}
int checkdraw2(struct movelist *movelst, int *key){
    if (movelst[*key-1].halfmoves > 99){
        return 2;
    }
    int lmove = *key-1;
    int k = lmove-2;
    int rep = 0;

    while (k >= 1 && k >= lmove-100){
        if (movelst[k].fen == movelst[lmove].fen){
            return 1;
        }
        k -= 2;
    }
    return rep;
}

int get_cheapest_attacker(struct board_info *board, unsigned int pos, unsigned int *attacker, bool encolor){  //returns 0 - 6 from blank to king
    char flag = 10;
    *attacker = 0;
    //pawns
    if (!encolor){
        if (!((pos+SW)&0x88) && board->board[pos+SW] == WPAWN){
            *attacker = pos+SW;
            return 1;
        }
        else if (!((pos+SE)&0x88) && board->board[pos+SE] == WPAWN){
            *attacker = pos+SE;
            return 1;
        }
    }
    else{
        if (!((pos+NE)&0x88) && board->board[pos+NE] == BPAWN){
            *attacker = pos+NE;
            return 1;
        }
        else if (!((pos+NW)&0x88) && board->board[pos+NW] == BPAWN){
            *attacker = pos+NW;
            return 1;
        }        
    }
    //knights, kings, and sliders
    unsigned char d, f;
    for (f = 0; f < 8; f++){
        d = pos + vector[0][f];
        
        if (!(d&0x88) && board->board[d]-encolor == WKNIGHT){
            *attacker = d;
            return 2;
        }

        char vec = vector[4][f];
        d = pos + vec;

        if ((d&0x88)){continue;}

        if (board->board[d]-encolor == WKING){
            if (flag > 6){*attacker = d; flag = 6;}
             continue;
        }
        do{
            if (board->board[d]){
                if ((board->board[d]&1) == encolor && 
                (board->board[d]-encolor == WQUEEN || ( ((f&1) && board->board[d]-encolor == WROOK) || (!(f&1) && board->board[d]-encolor == WBISHOP) ))){
                    if ((board->board[d]>>1) < flag){
                        flag = (board->board[d]>>1);
                        *attacker = d;
                    }
                }
                break;
            }
            d += vec;
        } 
        while (!(d&0x88));
        
    }

    return flag;
}

#endif