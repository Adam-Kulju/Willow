#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#define WHITE 0
#define BLACK 1
#define BLANK -1
#define WPAWN 0
#define BPAWN 1
#define WKNIGHT 2
#define BKNIGHT 3
#define WBISHOP 4
#define BBISHOP 5
#define WROOK 6
#define BROOK 7
#define WQUEEN 8
#define BQUEEN 9
#define WKING 10
#define BKING 11
#define LISTSIZE 125
#define MOVESIZE 500
#define RAND() (rand() & 0x7fff)
        /*((u64)RAND()<<48) ^ ((u64)RAND()<<35) ^ ((u64)RAND()<<22) ^((u64)RAND()<< 9) ^ ((u64)RAND()>> 4);*/

struct board_info{
    char board[8][8];
    char pnbrqcount[2][5];
    bool castling[2][2];
    char kingpos[2];
};
struct movelist{
    char move[8];
    char fen[65];
};
struct list{
    char move[8];
    int eval;
};
const int boardsize = sizeof(struct board_info);
const int listsize = sizeof(struct list);
const int movesize = sizeof(struct movelist);

const int VALUES[5] = {100, 305, 333, 560, 950};

int evals;

short int pawntable[8][8] = {
     {0,  5,  5,  0,  5, 10, 50,  0},
     {0, 10, -5,  0,  5, 10, 50,  0},
     {0, 10, 10,  0, 10, 20, 50,  0},
     {0,-25,  0, 25, 27, 40, 60,  0},
     {0,-25,  0, 25, 27, 40, 60,  0},
    {0, 10, 10,  0, 10, 20, 50,  0},
    {0, 10, -5,  0,  5, 10, 50,  0},
    {0,  5,  5,  0,  5, 10, 50,  0}
};
short int knighttable[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  5,  0,  5,  0,-20,-40},
    {-20,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  0,-30},
    {-30,  5, 15, 20, 20, 15,  0,-30},
    {-20,  0, 10, 15, 15, 10,  0,-30},
    {-40,-20,  5,  0,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};
short int bishoptable[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  5, 10,  0,  5,  0,  0,-10},
    {-40,  0, 10, 10,  5,  5,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-40,  0, 10, 10,  5,  5,  0,-10},
    {-10,  5, 10,  0,  5,  0,  0,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};
short int kingtable[8][8] = {
    { 20, 20,-10,-20,-30,-30,-30,-30},
    { 30, 20,-20,-30,-40,-40,-40,-40},
    { 10,  0,-20,-40,-50,-50,-50,-50},
    {  0, -5,-20,-40,-50,-50,-50,-50},
    {  0, -5,-20,-40,-50,-50,-50,-50},
    { 10,  0,-20,-40,-50,-50,-50,-50},
    { 30, 20,-20,-30,-40,-40,-40,-40},
    { 20, 20,-10,-20,-30,-30,-30,-30}
};
short int kingtable2[8][8] = {
    {-50,-30,-30,-30,-30,-30,-30,-50},
    {-30,-30,-10,-10,-10,-10,-20,-40},
    {-30,  0, 20, 30, 30, 20,-10,-30},
    {-30,  0, 30, 40, 40, 30,  0,-20},
    {-30,  0, 30, 40, 40, 30,  0,-20},
    {-30,  0, 20, 30, 30, 20,-10,-30},
    {-30,-30,-10,-10,-10,-10,-20,-40},
    {-50,-30,-30,-30,-30,-30,-30,-50},
};
void move(struct board_info *board, char *move, char color){
    if (isupper(move[0])){
        int v = (short int) move[1]-97, vv = atoi(&move[2])-1, w = (short int) move[4]-97, ww = atoi(&move[5])-1;

        if (strchr(move, 'x')){
            board->pnbrqcount[(short int)color^1][(board->board[w][ww]-color)/4]--;
        }
        board->board[w][ww] = board->board[v][vv];
        board->board[v][vv] = BLANK;
        if (strchr(move, 'K')){
            board->castling[color][0] = false, board->castling[color][1] = false;
            board->kingpos[color] = w*8 + ww;
        }
        if (strchr(move, 'R')){
            if (v == 'a'){
                board->castling[color][0] = false;
            }
            else{
                board->castling[color][1] = false;
            }
        }
    }
    else if (isalpha(move[0])){
        int v = (short int) move[0]-97, vv = atoi(&move[1])-1, w = (short int) move[3]-97, ww = atoi(&move[4])-1;

        if (strchr(move, 'x')){
            if (move[5] == 'e'){
                board->board[w][vv] = BLANK;
                board->pnbrqcount[(short int)color^1][0]--;
            }
            else{
                board->pnbrqcount[(short int)color^1][(board->board[w][ww]-color)/4]--;
            }
        }
        board->board[w][ww] = board->board[v][vv];
        board->board[v][vv] = BLANK;
        
        if (move[5] && move[5] != 'e'){ //promotion
            board->pnbrqcount[(short int)color][0]--;
            if (move[5] == 'Q'){
                board->pnbrqcount[(short int)color][4]++;   
                board->board[w][ww] = WQUEEN + color;
            }
            else if (move[5] == 'N'){
                board->pnbrqcount[(short int)color][1]++;  
                board->board[w][ww] = WKNIGHT + color;
            }
            else if (move[5] == 'R'){
                board->pnbrqcount[(short int)color][3]++;  
                board->board[w][ww] = WROOK + color;
            }
            else if (move[5] == 'B'){
                board->pnbrqcount[(short int)color][2]++;  
                board->board[w][ww] = WBISHOP + color;
            }
        }
    }
    else{
        int rank;
        if (color){
            rank = 7;
        }
        else{
            rank = 0;
        }
        board->castling[color][0] = false, board->castling[color][1] = false;
        if (move[3]){
            board->board[0][rank] = BLANK, board->board[4][rank] = BLANK, board->board[3][rank] = WROOK+color, board->board[2][rank] = WKING+color;
        }
        else{
            board->board[0][rank] = BLANK, board->board[4][rank] = BLANK, board->board[3][rank] = WROOK+color, board->board[2][rank] = WKING+color;
        }
    }
}
void move_add(struct board_info *board, struct movelist *movelst, int *key, char *move, char color){
    int k = *key;
    memcpy(movelst[k].move, move, 8);
    memcpy(movelst[k].fen, movelst[k-1].fen, 65);
    if (move[0] == '0'){
        int rank = 7; if (color == WHITE){
            rank = 0;
        }
        movelst[k].fen[32+rank] = '-', movelst[k].fen[56+rank] = '-';
        if (!strcmp(move, "0-0")){
            
            if (color == WHITE){
                movelst[k].fen[40+rank] = 'R', movelst[k].fen[48+rank] = 'K';
            }
            else{
                movelst[k].fen[40+rank] = 'r', movelst[k].fen[48+rank] = 'k';
            }
        }
        else{
            if (color == WHITE){
                movelst[k].fen[24+rank] = 'R', movelst[k].fen[16+rank] = 'K';
            }
            else{
                movelst[k].fen[24+rank] = 'r', movelst[k].fen[16+rank] = 'k';
            }
        }
    }
    else{
        if (isupper(move[0])){
            movelst[k].fen[(((int)move[4]-97)<<3) + atoi(&move[5])-1] = movelst[k].fen[(((int)move[1]-97)<<3) + atoi(&move[2])-1];
            movelst[k].fen[(((int)move[1]-97)<<3) + atoi(&move[2])-1] = '-';
        }
        else{      
            movelst[k].fen[(((int)move[3]-97)<<3) + atoi(&move[4])-1] = movelst[k].fen[(((int)move[0]-97)<<3) + atoi(&move[1])-1]; 
            if (move[5]){
                if  (move[5] != 'e'){
                    if (color == WHITE){
                        movelst[k].fen[(((int)move[3]-97)<<3) + atoi(&move[4])-1] = move[5]; 
                    }
                    else{
                        movelst[k].fen[(((int)move[3]-97)<<3) + atoi(&move[4])-1] = tolower(move[5]); 
                    }
                }
                else{
                    if (color == WHITE){                       
                        movelst[k].fen[(((int)move[3]-97)<<3) + atoi(&move[4])-2] = '-';
                    }
                    else{
                        movelst[k].fen[(((int)move[3]-97)<<3) + atoi(&move[4])] = '-';
                    } 

                }
            }
            movelst[k].fen[(((int)move[0]-97)<<3) + atoi(&move[1])-1] = '-';
        }
    }
    *key = k+1;
}
void setfull(struct board_info *board){
    char brd[8][8] = {
        {WROOK, WPAWN, BLANK, BLANK, BLANK, BLANK, BPAWN, BROOK},
    {WKNIGHT, WPAWN, BLANK, BLANK, BLANK, BLANK, BPAWN, BKNIGHT},
    {WBISHOP, WPAWN, BLANK, BLANK, BLANK, BLANK, BPAWN, BBISHOP},
    {WQUEEN, WPAWN, BLANK, BLANK, BLANK, BLANK, BPAWN, BQUEEN},
    {WKING, WPAWN, BLANK, BLANK, BLANK, BLANK, BPAWN, BKING},
    {WBISHOP, WPAWN, BLANK, BLANK, BLANK, BLANK, BPAWN, BBISHOP},
    {WKNIGHT, WPAWN, BLANK, BLANK, BLANK, BLANK, BPAWN, BKNIGHT},
    {WROOK, WPAWN, BLANK, BLANK, BLANK, BLANK, BPAWN, BROOK}};
    memcpy(board->board, brd, 64);
    char count[2][5] = {
        {8, 2, 2, 2, 1},
        {8, 2, 2, 2, 1}
    };
    memcpy(board->pnbrqcount, count, 10);
    board->castling[0][0] = true, board->castling[0][1] = true, board->castling[1][0] = true, board->castling[1][1] = true;
    board->kingpos[0] = 32, board->kingpos[1] = 39;
}
void setmovelist(struct movelist *movelst, int *key){
    char fen[65] = "RP----prNP----pnBP----pbQP----pqKP----pkBP----pbNP----pnRP----pr\0";
    memcpy(movelst[0].fen, fen, 65);
    *key = 1;
    return;
}
void printfull(struct board_info *board){
    int i, n;
    for (i = 7; i > -1; i--){
        for (n = 0; n < 8; n++){
            if (board->board[n][i] == BLANK){
                printf("-- ");
            }
            else{
                if (board->board[n][i]%2){
                    printf("B");
                }
                else{
                    printf("W");
                }
                switch(board->board[n][i]-(board->board[n][i]%2)){
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
                    default:
                    printf("K "); break;
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}
void pawn_add(struct list *list, int *key, char file1, char rank1, char file2, char rank2, char capture, char promote, bool en_passant){
    int k = *key;
    list[k].move[0] = file1, list[k].move[1] = rank1, list[k].move[2] = capture, list[k].move[3] = file2, list[k].move[4] = rank2, list[k].move[5] = '\0';
    if (promote){
        list[k].move[5] = promote, list[k].move[6] = '\0';
    }
    else if (en_passant){
        list[k].move[5] = 'e', list[k].move[6] = 'p', list[k].move[7] = '\0';
    }
    list[k+1].move[0] = '\0';
    *key = k+1;
}
void piece_add(struct list *list, int *key, char file1, char rank1, char file2, char rank2, char capture, char type){
    int k = *key;
    list[k].move[0] = type, list[k].move[1] = file1, list[k].move[2] = rank1, list[k].move[3] = capture, list[k].move[4] = file2, list[k].move[5] = rank2;
    list[k].move[6] = '\0';
    list[k+1].move[0] = '\0';
    *key = k+1;
}
void king_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color){
    int n, i;
    for (n = file-1; n < file+2 && n < 8; n++){
        for (i = rank-1; i < rank+2 && i < 8; i++){
            if ((n == file && i == rank) || n < 0 || i < 0){
                continue;
            }
            if (board->board[n][i]%2 != color){
                if (board->board[n][i] == BLANK){
                    piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, '-', 'K');
                }
                else{
                    piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, 'x', 'K');
                }
            }
        }
    }
}
void knight_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color){
    int n, i;
    for (n = file-2; n < file+3 && n < 8; n++){
        if (n == file || n < 0){
            continue;
        }
        i = rank + 3-abs(n-file);
        if (i > -1 && i < 8 && board->board[n][i]%2 != color){
            if (board->board[n][i] == BLANK){
                piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, '-', 'N');
            }
            else{
                piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, '-', 'N');
            }
        }
        i = rank - 3-abs(n-file);
        if (i > -1 && i < 8 && board->board[n][i]%2 != color){
            if (board->board[n][i] == BLANK){
                piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, '-', 'N');
            }
            else{
                piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, '-', 'N');
            }
        }
    }
}
void bishop_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color){
    int letterchange, numchange;
    int n, i;
    for (letterchange = -1; letterchange < 2; letterchange += 2){
        for (numchange = -1; numchange < 2; numchange += 2){
            n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
                if (board->board[n][i] == BLANK){
                    piece_add(list, key, (char)file+97, rank + '0' +1, (char)n+97, i + '0' + 1, '-', 'B');
                }
                else{
                    piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, 'x', 'B');
                    break;
                }
                n += letterchange, i += numchange;
            }
        }
    }
}
void rook_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color){
    int letterchange, numchange;
    int n, i;
    for (letterchange = -1; letterchange < 2; letterchange += 1){
        for (numchange = -1; numchange < 2; numchange += 1){
            if (abs(letterchange+numchange)%2 == 0){
                continue;
            }
            n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
                if (board->board[n][i] == BLANK){
                    piece_add(list, key, (char)file+97, rank + '0' +1, (char)n+97, i + '0' + 1, '-', 'R');
                }
                else{
                    piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, 'x', 'R');
                    break;
                }
                n += letterchange, i += numchange;
            }
        }
    }
}
void queen_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color){
    int letterchange, numchange;
    int n, i;
    for (letterchange = -1; letterchange < 2; letterchange += 1){
        for (numchange = -1; numchange < 2; numchange += 1){
            if (letterchange == 0 && numchange == 0){
                continue;
            }
            n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
                if (board->board[n][i] == BLANK){
                    piece_add(list, key, (char)file+97, rank + '0' +1, (char)n+97, i + '0' + 1, '-', 'Q');
                }
                else{
                    piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, 'x', 'Q');
                    break;
                }
                n += letterchange, i += numchange;
            }
        }
    }    
}
void pawn_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color){
    int srank, lrank, diff;
    if (color == WHITE){
        srank = 1, lrank = 7;
    }
    else{
        srank = 6, lrank = 0;
    }
    diff = -color + (color^1);
    if (board->board[file][rank+diff] == BLANK){ //forward
        if (rank+diff == lrank){
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+97, rank+diff+'0'+1, '-', 'Q', false);
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+97, rank+diff+'0'+1, '-', 'R', false);
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+97, rank+diff+'0'+1, '-', 'B', false);
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+97, rank+diff+'0'+1, '-', 'N', false);
        }
        else{
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+97, rank+diff+'0'+1, '-', '\0', false);
            if (rank == srank && board->board[file][rank+(diff*2)] == BLANK){
                pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+97, rank+(diff*2)+'0'+1, '-', '\0', false);
            }
        }
    }
    if (file < 7 && board->board[file+1][rank+diff]%2 == (color^1)){ //capture right
        if (rank+diff == lrank){
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+98, rank+diff+'0'+1, 'x', 'Q', false);
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+98, rank+diff+'0'+1, 'x', 'R', false);
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+98, rank+diff+'0'+1, 'x', 'B', false);
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+98, rank+diff+'0'+1, 'x', 'N', false);
        }        
        else{
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+98, rank+diff+'0'+1, 'x', '\0', false);
        }
    }
    if (file > 0 && board->board[file-1][rank+diff]%2 == (color^1)){ //capture left
        if (rank+diff == lrank){
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+96, rank+diff+'0'+1, 'x', 'Q', false);
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+96, rank+diff+'0'+1, 'x', 'R', false);
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+96, rank+diff+'0'+1, 'x', 'B', false);
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+96, rank+diff+'0'+1, 'x', 'N', false);
        }        
        else{
            pawn_add(list, key, (char)file+97, rank+'0'+1, (char)file+96, rank+diff+'0'+1, 'x', '\0', false);
        }
    }
}
bool check_check(struct board_info *board, char color){
    int kingfile = board->kingpos[color]/8, kingrank = board->kingpos[color]%8;
    int letterchange, numchange;
    for (letterchange = -1; letterchange < 2; letterchange++){
        for (numchange = -1; numchange < 2; numchange++){           
            if (letterchange == 0 && numchange == 0){
                continue;
            }
            int n = kingfile + letterchange, i = kingrank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
                if (board->board[n][i]%2 == (color^1)){
                    
                    switch(board->board[n][i]-(color^1)){
                        case WQUEEN:
                        return true;
                        case WROOK:
                        if (abs(letterchange+numchange)%2 == 1){
                            return true;
                        } break;
                        case WBISHOP:
                        if (abs(letterchange+numchange)%2 == 0){
                            return true;
                        } break;
                        case WPAWN:
                        
                        if (abs(kingfile-n) == 1 && ((color == WHITE && i-kingrank == 1) || (color == BLACK && kingrank-i == 1))){
                            return true;
                        } break;
                        case WKING:
                        if (abs(kingfile-n) == 1 || abs(kingrank-1) == 1){
                            return true;
                        } break;
                        default:
                        break;
                    }
                    n = -10;
                }
                n += letterchange, i += numchange;
            }
        }
    }
    int n, i;
    for (n = kingfile-2; n < kingfile+3 && n < 8; n++){
        if (n == kingfile || n < 0){
            continue;
        }
        i = kingrank + 3-abs(n-kingfile);
        if (i > -1 && i < 8 && board->board[n][i]-(color^1) == WKNIGHT){
            return true;
        }
        i = kingfile - 3-abs(n-kingfile);
        if (i > -1 && i < 8 && board->board[n][i]-(color^1) == WKNIGHT){
            return true;
        }
    }
    return false;

}
void castle_k(struct board_info *board, struct list *list, int *key, struct movelist *movelst, char color){
    if (board->castling[color][1] == false){
        return;
    }
    int rank; if (color == WHITE){
        rank = 0;
    }
    else{
        rank = 7;
    }
    if (board->board[4][rank]-color != WKING || board->board[7][rank]-color != WROOK || board->board[5][rank] != BLANK || board->board[6][rank] != BLANK){
        return;
    }
    if (check_check(board, color)){
        return;
    }
    struct board_info board2;
    memcpy(board2.board, board->board, 64);
    board2.board[4][rank] = BLANK, board2.board[5][rank] = WKING+color;
    if (check_check(&board2, color)){
        return;
    }
    board2.board[5][rank] = BLANK, board2.board[6][rank] = WKING+color;
    if (check_check(&board2, color)){
        return;
    }
    int k = *key;
    list[k].move[0] = '0', list[k].move[1] = '-', list[k].move[2] = '0', list[k].move[3] = '\0';
    *key = k+1;
    return;
}
void castle_q(struct board_info *board, struct list *list, int *key, struct movelist *movelst, char color){
    if (board->castling[color][0] == false){
        return;
    }
    int rank; if (color == WHITE){
        rank = 0;
    }
    else{
        rank = 7;
    }
    if (board->board[4][rank]-color != WKING || board->board[1][rank]-color != WROOK || board->board[2][rank] != BLANK || board->board[3][rank] != BLANK){
        return;
    }
    if (check_check(board, color)){
        return;
    }
    struct board_info board2;
    memcpy(board2.board, board->board, 64);
    board2.board[4][rank] = BLANK, board2.board[3][rank] = WKING+color;
    if (check_check(&board2, color)){
        return;
    }
    board2.board[3][rank] = BLANK, board2.board[2][rank] = WKING+color;
    if (check_check(&board2, color)){
        return;
    }
    int k = *key;
    list[k].move[0] = '0', list[k].move[1] = '-', list[k].move[2] = '0', list[k].move[3] = '\0';
    *key = k+1;
    return;
}
int getpassantfile(struct movelist *movelst, int *key){
    int k = *key-1;  
    if (!islower(movelst[k].move[0])){
        return -1;
    }
    int a = atoi(&movelst[k].move[1]), b = atoi(&movelst[k].move[4]);

    if (abs(a-b) != 2){
        return -1;
    }
    return (int)movelst[k].move[0]-97;
}
void en_passant(struct board_info *board, struct list *list, int *listkey, int *movelistkey, struct movelist *movelst, char color){
    int file = getpassantfile(movelst, movelistkey);
    if (file == BLANK){
        return;
    }

    int rank, diff; if (color == WHITE){
        rank = 4, diff = 1;
    }
    else{
        rank = 3, diff = -1;
    }
    if (board->board[file][rank] != WPAWN + (color^1)){
        printf("an error occured"); //this should not happen
        exit(1);
        return;
    }
    if (file < 7 && board->board[file+1][rank] == WPAWN + color){
        pawn_add(list, listkey, (char)file+98, rank+'0'+1, (char)file+97, rank+diff+1+'0', 'x', '\0', true);
    }
    if (file > 0 && board->board[file-1][rank] == WPAWN + color){
        pawn_add(list, listkey, (char)file+96, rank+'0'+1, (char)file+97, rank+diff+1+'0', 'x', '\0', true);
    }
}
void movelist(struct board_info *board, struct list *list, struct movelist *movelst, int *mkey, char color){
    int key = 0;
    for (int n = 0; n < 8; n++){
        for (int i = 0; i < 8; i++){
            if (board->board[n][i]%2 == color){
                switch (board->board[n][i]-color){
                    case WPAWN:
                    pawn_moves(list, board, &key, n, i, color); break;
                    case WKNIGHT:
                    knight_moves(list, board, &key, n, i, color); break;
                    case WBISHOP:
                    bishop_moves(list, board, &key, n, i, color); break;
                    case WROOK:
                    rook_moves(list, board, &key, n, i, color); break;
                    case WQUEEN:
                    queen_moves(list, board, &key, n, i, color); break;
                    case WKING:
                    king_moves(list, board, &key, n, i, color); break;
                    default:
                    printf("error reading board\n");
                    printfull(board);
                    exit(1);
                }
            }
        }
    }
    castle_k(board, list, &key, movelst, color);
    castle_q(board, list, &key, movelst, color);
    en_passant(board, list, &key, mkey, movelst, color);
}

void remove_illegal(struct board_info *board, struct list *list, char color){ //for human moves
    int key = 0; while (list[key].move[0] != '\0'){
        struct board_info board2;
        memcpy(board2.board, board->board, 64);
        memcpy(&board2.kingpos, board->kingpos, 2);
        move(&board2, list[key].move, color);
        
        if (check_check(&board2, color)){    
            int temp = key;
            while (list[key].move[0] != '\0'){
                memcpy(list[key].move, list[key+1].move, 8);
                key++;
            }
            key = temp;
        }
        else{
            key++;
        }
    }
}

bool checkdraw1(struct board_info *board){
    if (board->pnbrqcount[0][0] || board->pnbrqcount[1][0] || board->pnbrqcount[0][3] || 
        board->pnbrqcount[1][3] || board->pnbrqcount[0][4] || board->pnbrqcount[1][4]){
        return false;
    }
    if (board->pnbrqcount[0][2] > 1 || board->pnbrqcount[0][2] > 1){
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
    int lmove = *key-1;
    int k = lmove-1;
    int rep = 0;
    while (k >= 0){
        if (!strcmp(movelst[k].fen, movelst[lmove].fen)){
            rep++;
            if (rep > 1){
                return 2;
            }
        }
        k--;
    }
    return rep;
}
int humanmove(struct board_info *board, struct movelist *movelst, int *key, char color){
    printfull(board);
    if (checkdraw2(movelst, key) || checkdraw1(board)){
        return 11;
    }
    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    movelist(board, list, movelst, key, color);
    remove_illegal(board, list, color);
    if (list[0].move[0] == '\0'){
        if (check_check(board, color)){
            return 1000;
        }
        else{
            return 11;
        }
    }
    while (true){
        char mve[8];
        printf("Enter the move to play: ");
        scanf("%s", mve);
        if (!strcmp(mve, "h")){
            int i = 0; while (list[i].move[0] != '\0'){
                printf("%s\n", list[i].move);
                i++;
            }
            printf("\n");
        }
        else if (!strcmp(mve, "r")){
            printf("Are you sure you want to resign? (y) for yes: ");
            scanf("%s", mve);
            if (!strcmp(mve, "y")){
                return 10;
            }
        }
        else{
            int i = 0; while (list[i].move[0] != '\0'){
                if (!strcmp(mve, list[i].move)){
                    move(board, mve, color);
                    move_add(board, movelst, key, mve, color);
                    return 0;
                }
                i++;
            }
            printf("Move %s is illegal\n", mve);
        }
    }
}
int see(struct board_info *board, char *mve){
    int i;
    int attacker, victim;
    if (islower(mve[0])){
        attacker = 100;
        i = 3;
    }
    else{
        i = 4;
        switch(mve[0]){
            case 'Q':
            attacker = 900;
            case 'K':
            attacker = 50;
            case 'R':
            attacker = 500;
            default:
            attacker = 300;
        }
    }
    switch(board->board[(int)mve[i]-97][atoi(&mve[i+1])-1]/2){
        case 0:
        victim = 100; break;
        case 3:
        victim = 500; break;
        case 4:
        victim = 900; break; 
        default:
        victim = 300; break;
    }
    return attacker-victim + 50;
}
int material(struct board_info *board){
    int wval = 0, bval = 0;
    for (int i = 0; i < 5; i++){
        wval += VALUES[i]*board->pnbrqcount[WHITE][i];
        bval += VALUES[i]*board->pnbrqcount[BLACK][i];
    }
    if (board->pnbrqcount[WHITE][2] > 1){
        wval += 15;
    }
    if (board->pnbrqcount[BLACK][2] > 1){
        bval += 15;
    }
    return wval-bval;
}
int pstscore(struct board_info *board){
    int wscore = 0, bscore = 0;
    for (int n = 0; n < 8; n++){
        for (int i = 0; i < 8; i++){
            if (board->board[n][i]%2 == WHITE){
                if (board->board[n][i] == WPAWN){
                    wscore += pawntable[n][i];
                }
                else if (board->board[n][i] == WBISHOP){
                    wscore += bishoptable[n][i];
                }
                else if (board->board[n][i] == WKNIGHT){
                    wscore += knighttable[n][i];
                }
                else if (board->board[n][i] == WKING){
                    wscore += kingtable[n][i];
                }
            }
            else if (board->board[n][i]%2 == BLACK){
                if (board->board[n][i] == BPAWN){
                    bscore += pawntable[n][7-i];
                }
                else if (board->board[n][i] == BBISHOP){
                    bscore += bishoptable[n][7-i];
                }
                else if (board->board[n][i] == BKNIGHT){
                    bscore += knighttable[n][7-i];
                }
                else if (board->board[n][i] == BKING){
                    bscore += kingtable[n][7-i];
                }
            }
        }
    }
    return wscore-bscore;
}
int eval(struct board_info *board, char color){
    evals++;
    int evl = pstscore(board) + material(board);
    if (color == WHITE){
        return evl;
    }
    else{
        return -evl;
    }
}
void movescore(struct board_info *board, struct list *list, int depth, char *firstmove, char color){
    int i = 0; while (list[i].move[0] != '\0'){
        if (depth == 0 && !strcmp(list[i].move, firstmove)){
            list[i].eval = 1000;
        }
        else if (strchr(list[i].move, 'x')){
            list[i].eval = see(board, list[i].move);
        }
        else{
            if (islower(list[i].move[0])){
                if (color == WHITE){
                    list[i].eval = pawntable[((int)list[i].move[3])-97][atoi(&list[i].move[4])-1];
                }
                else{
                    list[i].eval = pawntable[((int)list[i].move[3])-97][8-atoi(&list[i].move[4])-1];
                }
            }
            else if (list[i].move[0] == 'N'){
                if (color == WHITE){
                    list[i].eval = knighttable[((int)list[i].move[4])-97][atoi(&list[i].move[5])];
                }
                else{
                    list[i].eval = knighttable[((int)list[i].move[4])-97][atoi(&list[i].move[5])];
                }                
            }
            else if (list[i].move[0] == 'B'){
                if (color == WHITE){
                    list[i].eval = knighttable[((int)list[i].move[4])-97][atoi(&list[i].move[5])];
                }
                else{
                    list[i].eval = knighttable[((int)list[i].move[4])-97][atoi(&list[i].move[5])];
                }                    
            }
            else if (list[i].move[0] == '0'){
                list[i].eval = 40;
            }
            else{
                list[i].eval = 0;
            }
        }
        i++;
    }
}
void selectionsort(struct list *list, int k){
    int temp = k;
    int i = k; while (list[i].move[0] != '\0'){
        if (list[i].eval > list[temp].eval){
            temp = i;          
        }
        i++;
    }
    char tempmove[8];
    memcpy(tempmove, list[temp].move, 8);
    int tempevl = list[temp].eval;
    memcpy(list[temp].move, list[k].move, 8);
    list[temp].eval = list[k].eval;
    memcpy(list[k].move, tempmove, 8);
    list[k].eval = tempevl;
}
int alphabeta(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depth, int maxdepth, char color, char bestmove[8]){
    if (checkdraw1(board)){
        return 0;
    }
    int rep = checkdraw2(movelst, key); if (rep == 2){
        return 0;
    }
    if (depth > maxdepth){
        //printf("%i\n", -eval(board, color));
        return eval(board, color);
    }
    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    bool ismove = false;
    bool firstmove = true;
    movelist(board, list, movelst, key, color);
    movescore(board, list, depth, bestmove, color);
    int i = 0;
    while (list[i].move[0] != '\0'){
        selectionsort(list, i);
        struct board_info board2;
        memcpy(&board2, board, boardsize);
        move(&board2, list[i].move, color);    
        if (check_check(&board2, color)){
            i++;
            continue;
        }
        ismove = true;
        move_add(&board2, movelst, key, list[i].move, color);
        list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depth+1, maxdepth, color^1, bestmove);
        if (list[i].eval >= beta){
            *key = *key-1;
            return beta;
        }
        if (list[i].eval > alpha){
            alpha = list[i].eval;
            if (depth == 0){
                memcpy(bestmove, list[i].move, 8);
            }
        }
        *key = *key-1;
        if (firstmove && list[i].eval < alpha && depth == 0){
            return alpha;
        }
        firstmove = false;
        i++;
    }

    if (!ismove){
        if (check_check(board, color)){
            return -100000;
        }
        else{
            return 0;
        }
    }
    return alpha;
}
void iid(struct board_info *board, struct movelist *movelst, int maxdepth, int *key, char color, bool ismove){
    int alpha = -1000000, beta = 10000000;   
    char mve[8];
    for (int depth = 0; depth < maxdepth; depth++){
        int aspiration = 25;
        int evl = alphabeta(board, movelst, key, alpha, beta, 0, depth, color, mve);
        while (evl == alpha || evl == beta){
            if (evl == alpha){
                alpha -= aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, 0, depth, color, mve);
            }
            else if (evl == beta){
                beta += aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, 0, depth, color, mve);            
            }
        }
        printf("%s %i %i\n", mve, evl, evals);
        evals = 0;
    }
}

//main function to test out analyzing a position
int main(void){
    struct board_info board;
    setfull(&board);
    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    struct movelist movelst[MOVESIZE];
    int key;
    setmovelist(movelst, &key);   
    move(&board, "e2-e4", WHITE);
    move_add(&board, movelst, &key, "e2-e4", WHITE);    
    move(&board, "e7-e5", BLACK);
    move_add(&board, movelst, &key, "e7-e5", BLACK);
    char mve[8];
    int alpha = -1000000, beta = 1000000;
    iid(&board, movelst, 4, &key, WHITE, false);
    return 0;
}
