#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
#define LISTSIZE 150
#define MOVESIZE 500
#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL /* Least significant 31 bits */
#define TTSIZE 100000


/* The array for the state vector */
static unsigned long long mt[NN]; 
/* mti==NN+1 means mt[NN] is not initialized */
static int mti=NN+1; 

unsigned long long int ZOBRISTTABLE[773];
unsigned long long int CURRENTPOS;

char KILLERTABLE[25][2][8];


struct board_info{
    char board[8][8];
    char pnbrqcount[2][5];
    bool castling[2][2];
    char kingpos[2];
    short int mobility[2];

};
struct movelist{
    char move[8];
    char fen[65];
};
struct list{
    char move[8];
    int eval;
};

struct ttentry{
    unsigned long long int zobrist_key;
    char type;
    char bestmove[8];
    int eval;
    short int depth_searched;
};

struct ttentry TT[TTSIZE];

const int boardsize = sizeof(struct board_info);
const int listsize = sizeof(struct list);
const int movesize = sizeof(struct movelist);

const int VALUES[5] = {100, 305, 333, 560, 950};

long int evals;
int betas, total;
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
short int pawntable2[8][8] = {
    {0, 20, 20, 25, 33, 45, 80, 0},
    {0,  5,  5, 10, 17, 29, 50, 0},
    {0, -5, -5,  0, 10, 25, 50, 0},
    {0,-10,-10, -3,  2, 10, 50, 0},
    {0,-10,-10, -3,  2, 10, 50, 0},
    {0, -5, -5,  0, 10, 25, 50, 0},
    {0,  5,  5, 10, 17, 29, 50, 0},
    {0, 20, 20, 25, 33, 45, 80, 0}
};
short int knighttable[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-35,-20,  5,  0,  5,  0,-20,-40},
    {-20,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  0,-30},
    {-30,  5, 15, 20, 20, 15,  0,-30},
    {-20,  0, 10, 15, 15, 10,  0,-30},
    {-35,-20,  5,  0,  5,  0,-20,-40},
    {-50,-30,-30,-30,-30,-30,-40,-50}
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

/* Random number generator code is taken from the Mersenne Twister http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c*/

void init_genrand64(unsigned long long seed)
{
    mt[0] = seed;
    for (mti=1; mti<NN; mti++) 
        mt[mti] =  (6364136223846793005ULL * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
}

unsigned long long genrand64_int64(void)
{
    int i;
    unsigned long long x;
    static unsigned long long mag01[2]={0ULL, MATRIX_A};

    if (mti >= NN) {
        if (mti == NN+1) 
            init_genrand64(5489ULL); 

        for (i=0;i<NN-MM;i++) {
            x = (mt[i]&UM)|(mt[i+1]&LM);
            mt[i] = mt[i+MM] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        for (;i<NN-1;i++) {
            x = (mt[i]&UM)|(mt[i+1]&LM);
            mt[i] = mt[i+(MM-NN)] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        x = (mt[NN-1]&UM)|(mt[0]&LM);
        mt[NN-1] = mt[MM-1] ^ (x>>1) ^ mag01[(int)(x&1ULL)];

        mti = 0;
    }
  
    x = mt[mti++];

    x ^= (x >> 29) & 0x5555555555555555ULL;
    x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
    x ^= (x << 37) & 0xFFF7EEE000000000ULL;
    x ^= (x >> 43);

    return x;
}

void init_by_array64(unsigned long long init_key[],
		     unsigned long long key_length)
{
    unsigned long long i, j, k;
    init_genrand64(19650218ULL);
    i=1; j=0;
    k = (NN>key_length ? NN : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 3935559000370003845ULL))
          + init_key[j] + j;
        i++; j++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=NN-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 2862933555777941757ULL))
          - i;
        i++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
    }

    mt[0] = 1ULL << 63;
}

void setzobrist(){
    for (int i = 0; i < 773; i++){
        ZOBRISTTABLE[i] = genrand64_int64();
    }
}

void calc_pos(struct board_info *board){
    CURRENTPOS = 0^ZOBRISTTABLE[772];
    int i, n;
    for (i = 0; i < 8; i++){
        for (n = 0; n < 8; n++){
            if (board->board[n][i] != BLANK){
                CURRENTPOS ^= ZOBRISTTABLE[(board->board[n][i]<<6)+(n<<3)+i];
            }
        }
    }
}

void insert(unsigned long long int position, int depth_searched, int eval, char type, char *bestmove){
    int index = position%TTSIZE;
    if (TT[index].zobrist_key != -1){
        if (depth_searched > TT[index].depth_searched || type == '0'){
            ;
        }
        else{
        return;
        }
    }
    TT[index].zobrist_key = position;
    TT[index].depth_searched = depth_searched;
    TT[index].eval = eval;
    TT[index].type = type;
    memcpy(TT[index].bestmove, bestmove, 8);
}

char lookup(unsigned long long int position, int depth_searched, int *eval){
    int index = position%TTSIZE;
    if (TT[index].zobrist_key == position && depth_searched == TT[index].depth_searched){
        *eval = TT[index].eval;
        return TT[index].type;
    }
    *eval = -1024;
    return 'n';
}

void clearTT(){
    int i; for (i = 0; i < TTSIZE; i++){
        TT[i].zobrist_key = -1;
    }
}

//to access zobrist number for say white knight on a6 we go zobristtable[(64*piece)+(file-1*8) + rank-1]
//768-771 is castling
//color key is 772
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
void move(struct board_info *board, char *move, char color){

    if (isupper(move[0])){
        int v = (short int) move[1]-97, vv = atoi(&move[2])-1, w = (short int) move[4]-97, ww = atoi(&move[5])-1;

        CURRENTPOS ^= ZOBRISTTABLE[(board->board[v][vv]<<6)+(v<<3)+vv];
        CURRENTPOS ^= ZOBRISTTABLE[(board->board[v][vv]<<6)+(w<<3)+ww];

        if (strchr(move, 'x')){
            CURRENTPOS ^= ZOBRISTTABLE[(board->board[w][ww]<<6)+(w<<3)+ww];
            board->pnbrqcount[(short int)color^1][(board->board[w][ww]-(color^1))/2]--;
        }
        board->board[w][ww] = board->board[v][vv];
        board->board[v][vv] = BLANK;

        if (strchr(move, 'K')){
            CURRENTPOS ^= ZOBRISTTABLE[768+(color*2)];
            CURRENTPOS ^= ZOBRISTTABLE[769+(color*2)];
            board->castling[color][0] = false, board->castling[color][1] = false;
            board->kingpos[color] = w*8 + ww;
        }

        if (strchr(move, 'R')){
            if (v == 'a'){
                CURRENTPOS ^= ZOBRISTTABLE[768+(color*2)];
                board->castling[color][0] = false;
            }
            else{
                CURRENTPOS ^= ZOBRISTTABLE[769+(color*2)];
                board->castling[color][1] = false;
            }
        }
    }
    else if (isalpha(move[0])){
        int v = (short int) move[0]-97, vv = atoi(&move[1])-1, w = (short int) move[3]-97, ww = atoi(&move[4])-1;
        CURRENTPOS ^= ZOBRISTTABLE[(board->board[v][vv]<<6)+(v<<3)+vv];       
        if (strchr(move, 'x')){
            if (move[5] == 'e'){
                CURRENTPOS ^= ZOBRISTTABLE[(board->board[w][vv]<<6)+(w<<3)+vv];
                board->board[w][vv] = BLANK;
                board->pnbrqcount[(short int)color^1][0]--;
            }
            else{
                CURRENTPOS ^= ZOBRISTTABLE[(board->board[w][ww]<<6)+(w<<3)+ww];
                board->pnbrqcount[(short int)color^1][(board->board[w][ww]-(color^1))/2]--;
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

        CURRENTPOS ^= ZOBRISTTABLE[(board->board[w][ww]<<6)+(w<<3)+ww];
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

            CURRENTPOS ^= ZOBRISTTABLE[(board->board[0][rank]<<6)+(0<<3)+rank], CURRENTPOS ^= ZOBRISTTABLE[(board->board[4][rank]<<6)+(4<<3)+rank];
            CURRENTPOS ^= ZOBRISTTABLE[(board->board[0][rank]<<6)+(3<<3)+rank], CURRENTPOS ^= ZOBRISTTABLE[(board->board[4][rank]<<6)+(2<<3)+rank];

            board->board[0][rank] = BLANK, board->board[4][rank] = BLANK, board->board[3][rank] = WROOK+color, board->board[2][rank] = WKING+color;
        }
        else{
            
            CURRENTPOS ^= ZOBRISTTABLE[(board->board[7][rank]<<6)+(0<<3)+rank], ZOBRISTTABLE[(board->board[4][rank]<<6)+(4<<3)+rank];
            CURRENTPOS ^= ZOBRISTTABLE[(board->board[7][rank]<<6)+(4<<3)+rank], ZOBRISTTABLE[(board->board[4][rank]<<6)+(6<<3)+rank];

            board->board[7][rank] = BLANK, board->board[4][rank] = BLANK, board->board[5][rank] = WROOK+color, board->board[6][rank] = WKING+color;
        }

        CURRENTPOS ^= ZOBRISTTABLE[768+(color*2)];
        CURRENTPOS ^= ZOBRISTTABLE[769+(color*2)];
    }
    CURRENTPOS ^= ZOBRISTTABLE[772];
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
    board->mobility[0] = 0, board->mobility[1] = 0;
}

void setempty(struct board_info *board){
    char brd[8][8] = {
        {BKING, BLANK, BLANK, BLANK, WKING, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, WROOK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK}
    };
    memcpy(board->board, brd, 64);
    char count[2][5] = {
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0}
    };
    memcpy(board->pnbrqcount, count, 10);
    board->castling[0][0] = false, board->castling[0][1] = false, board->castling[1][0] = false, board->castling[1][1] = false;
    board->kingpos[0] = 5, board->kingpos[1] = 0;
    board->mobility[0] = 0, board->mobility[1] = 0;
}
void setmovelist(struct movelist *movelst, int *key, char *fen){
    
    memcpy(movelst[0].fen, fen, 65);
    *key = 1;
    movelst[0].move[0] = '\0';
    movelst[1].move[0] = '\0';
    return;
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
void king_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture){
    int n, i;
    for (n = file-1; n < file+2 && n < 8; n++){
        for (i = rank-1; i < rank+2 && i < 8; i++){
            if ((n == file && i == rank) || n < 0 || i < 0){
                continue;
            }
            if (board->board[n][i]%2 != color){
                if (board->board[n][i] == BLANK){
                    if (!needscapture){
                    piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, '-', 'K');
                    }
                }
                else{
                    piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, 'x', 'K');
                }
            }
        }
    }
}
void knight_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture){
    int n, i;
    for (n = file-2; n < file+3 && n < 8; n++){
        if (n == file || n < 0){
            continue;
        }
        i = rank + 3-abs(file-n);
        if (i > -1 && i < 8 && board->board[n][i]%2 != color){
            board->mobility[color] += 2;
            if (board->board[n][i] == BLANK){
                if (!needscapture){
                piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, '-', 'N');
                }
            }
            else{
                piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, 'x', 'N');
            }
        }
        i = rank - (3-abs(n-file));
        if (i > -1 && i < 8 && board->board[n][i]%2 != color){
            board->mobility[color] += 2;
            if (board->board[n][i] == BLANK){
                if (!needscapture){
                piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, '-', 'N');
                }
            }
            else{
                piece_add(list, key, (char)file+97, rank+'0'+1, (char)n+97, i+'0'+1, 'x', 'N');
            }
        }
    }
}
void bishop_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture){
    int letterchange, numchange;
    int n, i;
    for (letterchange = -1; letterchange < 2; letterchange += 2){
        for (numchange = -1; numchange < 2; numchange += 2){
            n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
                board->mobility[color] += 1;
                if (board->board[n][i] == BLANK){
                    if (!needscapture){
                    piece_add(list, key, (char)file+97, rank + '0' +1, (char)n+97, i + '0' + 1, '-', 'B');
                    }
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
void rook_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture){
    int letterchange, numchange;
    int n, i;
    for (letterchange = -1; letterchange < 2; letterchange += 1){
        for (numchange = -1; numchange < 2; numchange += 1){
            if (abs(letterchange+numchange)%2 == 0){
                continue;
            }
            n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
                board->mobility[color] += 1;
                if (board->board[n][i] == BLANK){
                    if (!needscapture){
                    piece_add(list, key, (char)file+97, rank + '0' +1, (char)n+97, i + '0' + 1, '-', 'R');
                    }
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
void queen_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture){
    int letterchange, numchange;
    int n, i;
    for (letterchange = -1; letterchange < 2; letterchange += 1){
        for (numchange = -1; numchange < 2; numchange += 1){
            if (letterchange == 0 && numchange == 0){
                continue;
            }
            n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
                board->mobility[color] += 1;
                if (board->board[n][i] == BLANK){
                    if (!needscapture){
                    piece_add(list, key, (char)file+97, rank + '0' +1, (char)n+97, i + '0' + 1, '-', 'Q');
                    }
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
void pawn_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture){
    int srank, lrank, diff;
    if (color == WHITE){
        srank = 1, lrank = 7;
    }
    else{
        srank = 6, lrank = 0;
    }
    diff = -color + (color^1);
    if (board->board[file][rank+diff] == BLANK && !needscapture){ //forward
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
                        if (abs(kingfile-n) == 1 && abs(kingrank-i) == 1){
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
        i = kingrank - (3-abs(n-kingfile));
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
    list[k+1].move[0] = '\0';
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
    list[k+1].move[0] = '\0';
    *key = k+1;
    return;
}
int getpassantfile(struct movelist *movelst, int *key){
    int k = *key;  
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
        printf("an error occured %i %i %s\n", color, file, movelst[*movelistkey].move); //this should not happen
        printfull(board);

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
    board->mobility[color] = 0;
    for (int n = 0; n < 8; n++){
        for (int i = 0; i < 8; i++){
            if (board->board[n][i]%2 == color){
                switch (board->board[n][i]-color){
                    case WPAWN:
                    pawn_moves(list, board, &key, n, i, color, false); break;
                    case WKNIGHT:
                    knight_moves(list, board, &key, n, i, color, false); break;
                    case WBISHOP:
                    bishop_moves(list, board, &key, n, i, color, false); break;
                    case WROOK:
                    rook_moves(list, board, &key, n, i, color, false); break;
                    case WQUEEN:
                    queen_moves(list, board, &key, n, i, color, false); break;
                    case WKING:
                    king_moves(list, board, &key, n, i, color, false); break;
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

void movelistq(struct board_info *board, struct list *list, struct movelist *movelst, int *mkey, char color){
    int key = 0;
    for (int n = 0; n < 8; n++){
        for (int i = 0; i < 8; i++){
            if (board->board[n][i]%2 == color){
                switch (board->board[n][i]-color){
                    case WPAWN:
                    pawn_moves(list, board, &key, n, i, color, true); break;
                    case WKNIGHT:
                    knight_moves(list, board, &key, n, i, color, true); break;
                    case WBISHOP:
                    bishop_moves(list, board, &key, n, i, color, true); break;
                    case WROOK:
                    rook_moves(list, board, &key, n, i, color, true); break;
                    case WQUEEN:
                    queen_moves(list, board, &key, n, i, color, true); break;
                    case WKING:
                    king_moves(list, board, &key, n, i, color, true); break;
                    default:
                    printf("error reading board\n");
                    printfull(board);
                    exit(1);
                }
            }
        }
    }    
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
    while (k >= 1){
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

int game_end(struct board_info *board, struct movelist *movelst, int *key, char color){  //checks if the side to MOVE (comp) has been mated or drawn
    if (checkdraw2(movelst, key) || checkdraw1(board)){
        return 11;
    }
    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    movelist(board, list, movelst, key, color);
    remove_illegal(board, list, color);
    if (list[0].move[0] != '\0'){
        return 0;
    }    
    else{
        if (check_check(board, color)){
            return 1000;
        }
        else{
            return 11;
        }
    }
}

int safecapture(struct board_info *board, int file, int rank, int attacker, int victim, char color){
    int n, i;
    for (n = file-2; n < file+3 && n < 8; n++){
        if (n == file || n < 0){
            continue;
        }
        i = rank + 3-abs(n-file);
        if (i > -1 && i < 8 && board->board[n][i]-(color^1) == WKNIGHT){
            return 2;
        }
        i = file - 3-abs(n-file);
        if (i > -1 && i < 8 && board->board[n][i]-(color^1) == WKNIGHT){
            return 2;
        }
    }  
    int letterchange, numchange;
    for (letterchange = -1; letterchange < 2; letterchange++){
        for (numchange = -1; numchange < 2; numchange++){           
            if (letterchange == 0 && numchange == 0){
                continue;
            }
            int n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
                if (board->board[n][i]%2 == (color^1)){
                    
                    switch(board->board[n][i]-(color^1)){
                        case WQUEEN:
                        return 2;
                        case WROOK:
                        if (abs(letterchange+numchange)%2 == 1){
                            return 2;
                        } break;
                        case WBISHOP:
                        if (abs(letterchange+numchange)%2 == 0){
                            return 2;
                        } break;
                        case WPAWN:
                        
                        if (abs(file-n) == 1 && ((color == WHITE && i-rank == 1) || (color == BLACK && rank-i == 1))){
                            return 2;
                        } break;
                        case WKING:
                        if (abs(file-n) == 1 || abs(rank-1) == 1){
                            return 2;
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
    return 0;
}

int see(struct board_info *board, char *mve, char color){
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
    switch(board->board[(int)mve[i]-97][atoi(&mve[i+1])-1]-(color^1)){
        case WPAWN:
        victim = 100; break;
        case WROOK:
        victim = 500; break;
        case WQUEEN:
        victim = 900; break; 
        default:
        victim = 300; break;
    }
    if (victim-attacker < 0 && safecapture(board, (int)mve[i]-97, atoi(&mve[i+1])-1, attacker, victim, color) == 0){
        return victim+100;
    }
    return victim-attacker + 200;
}


int material(struct board_info *board, int *kingdanger){
    int wval = 0, bval = 0;
    for (int i = 0; i < 5; i++){
        wval += VALUES[i]*board->pnbrqcount[WHITE][i];
        bval += VALUES[i]*board->pnbrqcount[BLACK][i];
    }
    *kingdanger = (wval + bval - 100*board->pnbrqcount[WHITE][0] - 100*board->pnbrqcount[BLACK][0] - 2000)/40;
    if (*kingdanger < 0){
        *kingdanger = 0;
    }
    if (*kingdanger > 40){
        *kingdanger = 40;
    }
    if (board->pnbrqcount[WHITE][2] > 1){
        wval += 15;
    }
    if (board->pnbrqcount[BLACK][2] > 1){
        bval += 15;
    }
    return wval-bval;
}
int pstscore(struct board_info *board, int kingdanger){
    int wscore = 0, bscore = 0;
    bool wiso[8], biso[8];
    for (int n = 0; n < 8; n++){
        bool wdblflag = false, bdblflag = false;
        wiso[n] = false, biso[n] = false;
        for (int i = 0; i < 8; i++){
            if (board->board[n][i]%2 == WHITE){
                if (board->board[n][i] == WPAWN){
                    wiso[n] = true;
                    wscore += (kingdanger*pawntable[n][i] + (40-kingdanger)*pawntable2[n][i])/40;
                    if (wdblflag){
                        wscore -= 15;
                    }
                    wdblflag = true;
                }
                else if (board->board[n][i] == WBISHOP){
                    wscore += bishoptable[n][i];
                }
                else if (board->board[n][i] == WKNIGHT){
                    wscore += knighttable[n][i];
                }
            }
            else if (board->board[n][i]%2 == BLACK){
                if (board->board[n][i] == BPAWN){
                    biso[n] = true;
                    bscore += (kingdanger*pawntable[n][7-i] + (40-kingdanger)*pawntable2[n][7-i])/40;
                    if (bdblflag){
                        bscore -= 15;
                    }
                    bdblflag = true;
                }
                else if (board->board[n][i] == BBISHOP){
                    bscore += bishoptable[n][7-i];
                }
                else if (board->board[n][i] == BKNIGHT){
                    bscore += knighttable[n][7-i];
                }

            }
        }
    if (n == 7){
            if (wiso[7] && !wiso[6]){
                wscore -= 15;
            }
            if (biso[7] && !biso[6]){
                bscore -= 15;
            }
        }
    else if (n != 0){
            if (!wiso[n] && wiso[n-1] && (n == 1 || !wiso[n-2])){
                wscore -= 15;
            }
            if (!biso[n] && biso[n-1] && (n == 1 || !biso[n-2])){
                bscore -= 15;
            }
        }
    }
    
    wscore += (kingdanger*kingtable[board->kingpos[0]/8][board->kingpos[0]%8] + (40-kingdanger)*kingtable2[board->kingpos[0]/8][board->kingpos[0]%8])/40;
    bscore += (kingdanger*kingtable[board->kingpos[1]/8][7-(board->kingpos[1]%8)] + (40-kingdanger)*kingtable2[board->kingpos[1]/8][7-(board->kingpos[1]%8)])/40;
    return wscore-bscore;
}

int kingsafety(struct board_info *board){
    int eval = 0;
    int wkingfile = board->kingpos[0]/8, wkingrank = board->kingpos[0]%8;
    for (int n = wkingfile - 2; n < wkingfile + 3 && n < 8; n++){
        if (n < 0){
            continue;
        }
        for (int i = wkingrank - 2; i < wkingrank + 3 && i < 8; i++){
            if (i < 0){
                continue;
            }
            if (board->board[n][i] == BLANK){
                eval -= 1;
            }
                else{
                switch (board->board[n][i]){
                    case BPAWN:
                    eval -= 2; break;
                    case BLANK:
                    eval -= 1; break;
                    case BROOK:
                    eval -= 5; break;
                    case BQUEEN:
                    eval -= 10; break;
                    case BBISHOP:
                    eval -= 3; break;
                    case BKNIGHT:
                    eval -= 3; break;
                    case WPAWN:
                    eval += 1; break;
                    case WROOK:
                    eval += 4; break;
                    case WQUEEN:
                    eval += 6; break;
                    case WBISHOP:
                    eval += 3; break;
                    case WKNIGHT:
                    eval += 3; break;
                    default:
                    break;
                }
            }
        }
    }
    int beval = 0;
    int bkingfile = board->kingpos[1]/8, bkingrank = board->kingpos[1]%8;
    for (int n = bkingfile - 2; n < bkingfile + 3 && n < 8; n++){
        if (n < 0){
            continue;
        }
        for (int i = bkingrank - 2; i < bkingrank + 3 && i < 8; i++){
            if (i < 0){
                continue;
            }
            if (board->board[n][i] == BLANK){
                beval -= 1;
            }
            else{
                switch (board->board[n][i]){
                    case WPAWN:
                    beval -= 2; break;
                    case WROOK:
                    beval -= 5; break;
                    case WQUEEN:
                    beval -= 10; break;
                    case WBISHOP:
                    beval -= 3; break;
                    case WKNIGHT:
                    beval -= 3; break;
                    case BPAWN:
                    beval += 1; break;
                    case BROOK:
                    beval += 4; break;
                    case BQUEEN:
                    beval += 6; break;
                    case BBISHOP:
                    beval += 3; break;
                    case BKNIGHT:
                    beval += 3; break;
                    default:
                    break;
                }
            }
        }
    }
    return eval - beval;
}
int eval(struct board_info *board, char color){
    evals++;
    int kingdanger; //0 represents full kingdanger, 40 represents complete middlegame
    int evl = material(board, &kingdanger);
    evl += pstscore(board, kingdanger);
    //evl += (board->mobility[WHITE] - board->mobility[BLACK])*4;
    evl += (kingdanger*kingsafety(board))/40;
    if (color == WHITE){
        return evl;
    }
    else{
        return -evl;
    }
}
void movescore(struct board_info *board, struct list *list, int depth, char *firstmove, char color){
    int evl;
    char ismatch = lookup(CURRENTPOS, depth-1, &evl);
    char mve[8];
    mve[0] = '\0';
    if (ismatch == '0' || ismatch == '1'){
        memcpy(mve, TT[CURRENTPOS%TTSIZE].bestmove, 8);
    }
    int i = 0; while (list[i].move[0] != '\0'){
        int evl;
        if (mve[0] != '\0' && !strcmp(list[i].move, mve)){
            list[i].eval = 1000;
        }
        else if (strchr(list[i].move, 'x')){
            list[i].eval = see(board, list[i].move, color);
        }
        else if (!strcmp(list[i].move, KILLERTABLE[depth][0])){
            list[i].eval = 199;
        }
        else if (!strcmp(list[i].move, KILLERTABLE[depth][1])){
            list[i].eval = 198;
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
int quiesce(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depth, int maxdepth, char color){
    
    int stand_pat = eval(board, color);
    if (depth > maxdepth){
        return stand_pat;
    }
    if (stand_pat >= beta){
        return beta;
    }
    if (stand_pat > alpha){
        alpha = stand_pat;
    }
    if (stand_pat + 1125 < alpha){
        return stand_pat;
    }
    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    bool incheck = check_check(board, color);
    if (incheck){
        movelist(board, list, movelst, key, color);
        remove_illegal(board, list, color);
    }
    else{
        movelistq(board, list, movelst, key, color);
    }
    char mve[8];
    mve[0] = '\0';
    movescore(board, list, 0, mve, color);
    int i = 0;
    bool ismove = false;
    while (list[i].move[0] != '\0'){
        selectionsort(list, i);
        if (list[i].eval < 0){
            return alpha;
        }
        struct board_info board2;
        memcpy(&board2, board, boardsize);
        move(&board2, list[i].move, color);
        if (!incheck && check_check(&board2, color)){
            i++;
            continue;
        }      
        ismove = true; 
        move_add(&board2, movelst, key, list[i].move, color);
        list[i].eval = -quiesce(&board2, movelst, key, -beta, -alpha, depth+1, maxdepth, color^1);
        if (list[i].eval >= beta){
            movelst[*key-1].move[0] = '\0';
            *key = *key-1;
            return beta;
        }
        if (list[i].eval > alpha){
            alpha = list[i].eval;
        }
        movelst[*key-1].move[0] = '\0';
        *key = *key-1;
        i++;
    }
    
    return alpha;
}

int alphabeta(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depth, int maxdepth, char color, char bestmove[8], bool isnull){
    if (checkdraw1(board)){       
        return 0;
    }
    int rep = checkdraw2(movelst, key); if (rep == 2){
        return 0;
    }
    if (depth > maxdepth){
        //printf("%i\n", -eval(board, color));
        int b = quiesce(board, movelst, key, -1000000, 1000000, 0, 6, color);        
        return b;
        
    }

    int evl;
    char type = lookup(CURRENTPOS, maxdepth, &evl);

    bool needs_evl = true;

    if (type != 'n'){
        if (type == '0'){
            //printfull(board);
            //printf("%i %s\n", evl, TT[CURRENTPOS%TTSIZE].bestmove);
            needs_evl = false;
            return evl;
        }
            else if (type == '1'){ //a move that caused a beta cutoff
                if (evl >= beta){
                    //don't eval any further
                    return beta;
                }
            }
            else{ //a move that didn't raise alpha
                if (evl < alpha){
                    //don't evl
                    return alpha;
                }
            }
        }
    if (isnull == false){
        bool ispiece = false;
        for (int i = 1; i < 5; i++){
            if (board->pnbrqcount[WHITE][i] > 0 || board->pnbrqcount[BLACK][i] > 0){
                ispiece = true;
                i = 5;
            }
        }
        if (ispiece == true){
            int staticevl = eval(board, color);
            if (staticevl >= beta){
                char n[8];
                int nullmove = alphabeta(board, movelst, key, -beta, -alpha, depth+3, maxdepth, color^1, n, true);
                if (nullmove >= beta){
                    return beta;
                }
            }
        }
    }

    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    bool ismove = false;
    bool firstmove = true;
    int betacount = 0;
    movelist(board, list, movelst, key, color);

    movescore(board, list, maxdepth, bestmove, color);
    int i = 0;
    unsigned long long int original_pos = CURRENTPOS;
    bool raisedalpha = false;

    while (list[i].move[0] != '\0'){      
        selectionsort(list, i);
        struct board_info board2;
        memcpy(&board2, board, boardsize);
        move(&board2, list[i].move, color);         
        if (check_check(&board2, color)){
            CURRENTPOS = original_pos;
            i++;
            continue;
        }
        ismove = true;
        
        move_add(&board2, movelst, key, list[i].move, color);
        char bestmve[8];
        bestmve[0] = '\0';   
        if (raisedalpha == false && betacount > 3){
            list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depth+3, maxdepth, color^1, bestmve, false);
            if (list[i].eval > alpha){
                list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depth+1, maxdepth, color^1, bestmve, false);
            }
        }
        else{
            list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depth+1, maxdepth, color^1, bestmve, false);
        }
        if (depth == 0){
            printf("%s %i\n", list[i].move, list[i].eval);
        }
        if (list[i].eval >= beta){
            memcpy(bestmove, list[i].move, 8);
            insert(original_pos, maxdepth, beta, '1', bestmove);
            total++;
            if (betacount < 2){
                betas++;
            }
            if (betacount > 15){
                //printf("%s %i %i\n", list[i].move,list[i].eval, depth);
                //printfull(board);
            }
            if (!strchr(list[i].move, 'x')){
                if (strcmp(KILLERTABLE[depth][0], list[i].move)){
                    memcpy(KILLERTABLE[depth][0], list[i].move, 8);
                }
                else if (strcmp(KILLERTABLE[depth][1], list[i].move)){
                    memcpy(KILLERTABLE[depth][1], list[i].move, 8);
                }
            }
            movelst[*key-1].move[0] = '\0';
            *key = *key-1;
            CURRENTPOS = original_pos;
            return beta;
        }
        betacount++;
        if (list[i].eval > alpha){
            raisedalpha = true;
            alpha = list[i].eval;
            memcpy(bestmove, list[i].move, 8);
        }
        movelst[*key-1].move[0] = '\0';
        *key = *key-1;
        if (firstmove && list[i].eval < alpha && depth == 0){
            CURRENTPOS = original_pos;
            return alpha;
        }
        firstmove = false;
        CURRENTPOS = original_pos;
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
    //printfull(board);
    if (raisedalpha){
        insert(original_pos, maxdepth, alpha, '0', bestmove);
    }
    else{
        char j[8];
        j[0] = '\0';
        insert(original_pos, maxdepth, alpha, '2', j);
    }
    return alpha;
}
void iid(struct board_info *board, struct movelist *movelst, int maxdepth, int *key, char color, bool ismove){
    clearTT();
    int alpha = -1000000, beta = 1000000;   
    char mve[8];
    for (int depth = 0; depth < maxdepth; depth++){
        clock_t time = clock();
        int aspiration = 25;
        int evl = alphabeta(board, movelst, key, alpha, beta, 0, depth, color, mve, false);
        while (evl == alpha || evl == beta){
            if (evl == alpha){
                alpha -= aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, 0, depth, color, mve, false);
            }
            else if (evl == beta){
                beta += aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, 0, depth, color, mve, false);            
            }
        }
        clock_t time2 = clock()-time;
        printf("depth %i: %s %f %li %f secs\n", depth+1, mve, (float)evl/100, evals, (float)time2/CLOCKS_PER_SEC);
        evals = 0;

    }
    if (ismove){
        move(board, mve, color);
        move_add(board, movelst, key, mve, color);
    }
}

void iid_time(struct board_info *board, struct movelist *movelst, int maxtime, int *key, char color, bool ismove){
    clearTT();
    int alpha = -1000000, beta = 1000000;   
    long int totalevls = 0;
    char mve[8];
    clock_t time = clock();
    for (int depth = 0; ; depth++){        
        int aspiration = 25;
        int evl = alphabeta(board, movelst, key, alpha, beta, 0, depth, color, mve, false);
        while (evl == alpha || evl == beta){
            if (evl == alpha){
                alpha -= aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, 0, depth, color, mve, false);
            }
            else if (evl == beta){
                beta += aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, 0, depth, color, mve, false);            
            }
        }
        clock_t time2 = clock()-time;
        printf("depth %i: %s %f %li/%li %f secs\n", depth+1, mve, (float)evl/100, evals, totalevls+evals, (float)time2/CLOCKS_PER_SEC);
        totalevls += evals;
        evals = 0;
        if ((float)time2/CLOCKS_PER_SEC > maxtime){
            break;
        }
    }
    if (ismove){
        move(board, mve, color);
        move_add(board, movelst, key, mve, color);
    }
}

void game(int time){
    unsigned long long init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    init_by_array64(init, 4);
    struct board_info board;
    setfull(&board);    
    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    struct movelist movelst[MOVESIZE];
    setzobrist();
    calc_pos(&board);    
    int key;
    char fen[65] = "RP----prNP----pnBP----pbQP----pqKP----pkBP----pbNP----pnRP----pr\0";
    //char fen[65] =   "k---K--------R--------------------------------------------------\0";
    setmovelist(movelst, &key, fen);   
    char color, temp;
    int game_end_flag = 0;
    printf("What color would you like to play as? (W/B): ");
    scanf("%c", &temp);
    printf("%c\n", temp);
    if (temp == 'W'){
        color = WHITE;
    }
    else{
        color = BLACK;
    }
    if (color == BLACK){
        iid_time(&board, movelst, time, &key, WHITE, true);
    }
    while (game_end_flag == 0){
        game_end_flag = humanmove(&board, movelst, &key, color);
        if (game_end_flag != 0){
            break;
        }        
        game_end_flag = game_end(&board, movelst, &key, color^1);
        if (game_end_flag != 0){
            break;
        }
        printfull(&board);
        printf("COMPUTER MOVING: \n");
        iid_time(&board, movelst, time, &key, color^1, true);
    }
}

int main(void){
    //game(10);
    unsigned long long init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    init_by_array64(init, 4);

    struct board_info board;
    //setfull(&board);
    setfull(&board);
    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    struct movelist movelst[MOVESIZE];
    setzobrist();
    calc_pos(&board);
    
    int key;
    char fen[65] = "RP----prNP----pnBP----pbQP----pqKP----pkBP----pbNP----pnRP----pr\0";
    //char fen[65] =   "k---K--------R--------------------------------------------------\0";
    setmovelist(movelst, &key, fen);   
    move(&board, "Nb1-c3", WHITE);
    move_add(&board, movelst, &key, "Nb1-c3", WHITE);
    move(&board, "e7-e5", BLACK);
    move_add(&board, movelst, &key, "e7-e5", BLACK);
    printfull(&board);
    printf("%i\n", eval(&board, BLACK));
    exit(0);
    iid(&board, movelst, 1, &key, BLACK, false);
    return 0;
}
