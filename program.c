#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>     
#include <string.h>
#include <time.h>
#include <math.h>
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
#define MOVESIZE 1000
#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL /* Least significant 31 bits */
#define TTSIZE 1 << 20
#define _mask (1 << 20) - 1
#define TIMEOUT 111111
#define TEMPO 5
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAXPHASE 24
#define LIKELYDRAW -111110
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef _WIN32
int pipe;
HANDLE hstdin;
#endif
clock_t start_time;
float maximumtime;
float coldturkey;
bool isattacker;
char attacksw;
char attacksb;
//typedef int MOVE;
/* The array for the state vector */
static unsigned long long mt[NN]; 
/* mti==NN+1 means mt[NN] is not initialized */
static int mti=NN+1; 
char passed[2][8];
unsigned long long int ZOBRISTTABLE[773];
unsigned long long int CURRENTPOS;
char KILLERTABLE[50][2][8];
unsigned long int HISTORYTABLE[2][64][64];
char pvstack[5000][8];
int pvptr = 0;
struct board_info{
    char board[8][8];
    char pnbrqcount[2][5];
    bool castling[2][2];
    char kingpos[2];
    short int epsquare;
};

char current_mobility;

struct movelist{
    char move[8];
    //char fen[65];
    long long unsigned int fen;
    char halfmoves;
};
struct list{
    char move[8];
    int eval;
    bool ischeck;
};

struct ttentry{
    unsigned long long int zobrist_key;
    char type;
    char bestmove[8];
    int eval;
    char depth;
    char age;
};
char currentmove[8];
struct ttentry TT[TTSIZE];

const int boardsize = sizeof(struct board_info);
const int listsize = sizeof(struct list);
const int movesize = sizeof(struct movelist);


long int evals;
long int totals;
int betas, total;
 const int VALUES[5] = {70, 348, 364, 473, 1024};
 const int VALUES2[5] = {91, 285, 295, 502, 932};
 
  short int pawntable[8][8] = {
  {0, -28, -22, -29, -18, -16,  70,   0},
  {0,  -1, -10, -13,   2,  -2, 104,   0},
  {0, -13,   1,  -3,  -2,  13,  35,   0},
  {0,   5,   5,   8,   8,  13,  68,   0},
  {0,   7,  18,  12,  11,  53,  43,   0},
  {0,  37,  13,  12,  12,  52,  98,   0},
  {0,  36,  28,  -2,   6,  21,  36,   0},
  {0, -17, -12, -26, -23, -23,  -7,   0}
  };

  short int pawntable2[8][8] = {
  {0,  16,   7,  20,  32,  74, 179,   0},
  {0,   4,   3,   9,  20,  77, 154,   0},
  {0,   8,  -3,  -2,   9,  56, 144,   0},
  {0,   5,  -6,  -7,  -5,  35, 116,   0},
  {0,   5,  -5,  -7,  -5,  25, 126,   0},
  {0,  -5,  -5,  -7,  -1,  31, 114,   0},
  {0, -10, -13,  -2,  10,  62, 148,   0},
  {0,  -7,  -8,   1,  12,  64, 178,   0}
  };

short int knighttable[8][8] = {
{-104, -28, -25, -14,  -9, -47, -73, -166},
{-18, -52,  -9,   4,  10,  57, -40, -89},
{-57, -12,   3,  12,  19,  36,  70, -49},
{-32,   7,  10,  15,  50,  64,  35, -49},
{-14,   8,  19,  23,  28,  84,  23, -49},
{-25,  16,  10,  18,  47, 109,  62, -97},
{-12, -13,  25,  21,  13,  73,   7, -49},
{-23, -16, -20,  -6,  23,  43, -17, -107}
};

short int knighttable2[8][8] = {
{-28, -41, -22, -16, -15, -21, -24, -57},
{-41, -18,  -4,  -3,   2, -23,  -6, -37},
{-19,  -9,  -2,  15,  19,   7, -27, -16},
{-10,  -4,  12,  19,  18,   4,  -5, -28},
{-16,  -2,  10,  18,  19,  -6, -11, -28},
{-14, -19,  -3,  16,  43, -13, -28, -27},
{-46, -22, -18,   3,   8, -19, -24, -67},
{-65, -41, -22, -14, -13, -30, -52, -108}
};

short int bishoptable[8][8] = {
{-33,   4,   3,  -5,  -1, -12, -24, -35},
{-13,  24,  16,  11,   1,  34,  42,   2},
{  1,  14,  13,   8,  30,  40, -20, -82},
{-24,   8,  14,  23,  46,  37, -10, -35},
{ -7,  18,  10,  32,  34,  34,  29, -26},
{  0,  22,  26,   4,  33,  50,  55, -42},
{-37,  39,  17,   9,   5,  36,  16,   6},
{-20,   4,   8,   5,  -4,  -4, -47,  -8}
};

short int bishoptable2[8][8] = {
{-17, -14, -10,  -6,  -1,   2,  -7, -13},
 {-5, -17,  -2,   0,   7,  -7,  -8, -21},
 {-8,  -8,   7,  10,   4,  -3,   5, -10},
  {4,  -2,   9,  12,   1,  -5, -12,  -9},
 {-2,  -2,  12,   3,  10,  -4,  -6,  -8},
 {-6,  -5,   0,   8,   6,   3, -17,  -9},
 {-2, -14,  -6,  -6,   3,  -2,  -5, -18},
{-16, -24, -13,  -9,   0,   3, -14, -24}
};

short int rooktable[8][8] = {
{-15, -39, -41, -36, -23,  -6,  23,  28},
{-10, -16, -25, -25, -10,  18,  28,  39},
{  5, -19, -15, -11,   6,  25,  53,  27},
 {15,  -9, -16,  -2,  25,  35,  58,  46},
 {21,   0,   3,   8,  22,  17,  76,  59},
  {7,   7,  -1,  -7,  34,  44,  64,   9},
{-29,  -5,  -5,   5,  -5,  60,  24,  30},
{-14, -69, -33, -23, -19,  16,  43,  42}
};

short int rooktable2[8][8] = {
{  1,   0,   1,   7,  13,   9,   7,   6},
  {4,  -5,   3,  10,   9,   7,   8,   4},
  {4,   2,  -1,  10,  13,   6,   1,   8},
 {-1,   1,   1,   3,   1,   0,   0,   3},
 {-8,  -8,  -9,  -5,   0,   2, -11,   3},
 {-7, -12, -11,  -3,   1,  -4,  -4,  10},
  {8, -12, -10,  -8,   3,  -9,   5,   7},
{-16,  -1, -13,  -7,   6,  -4,   0,   1}
};

short int queentable[8][8] = {
{ -1, -31, -15,  -7, -27, -14, -21, -30},
{-11,  -2,   6, -25, -24, -17, -37,  -3},
  {2,  15,  -9,  -5, -15,   5,  -5,  25},
 {18,  17,  -1, -13, -19,   4,   1,   9},
 {-4,  24,   3,   0,  -6,  25, -16,  55},
{-22,  17,   1,  -8,  10,  51,  54,  42},
{-31,   0,  11,  -4,  -7,  41,  23,  41},
{-47,   1,   2,  -7, -13,  47,  51,  41}
};

short int queentable2[8][8] = {
{-32, -20, -16, -15,   6, -19, -13, -11},
{-25, -20, -24,  31,  25,   7,  22,  19},
{-19, -28,  18,  23,  27,  10,  33,  18},
{-36, -12,   9,  49,  46,  47,  41,  24},
 {-3, -14,  13,  33,  56,  44,  59,  21},
{-30, -23,  17,  31,  36,  29,  22,  16},
{-20, -35,   9,  35,  53,  15,  27,   7},
{-40, -32,   2,  28,  29,   2,  -3,  14}
};

short int kingtable[8][8] = {
{-15,   2, -15, -49, -17,  -9,  29, -65},
 {39,   8, -13,   0, -20,  25,  -1,  23},
 {12, -11, -22, -27, -12,   2, -20,  16},
{-59, -64, -45, -39, -26, -15,  -7, -15},
 {-6, -45, -42, -45, -30, -19,  -8, -56},
{-32, -18, -28, -43, -24,   7,  -4, -34},
 {31,  17, -11, -33, -14,  23, -37,   2},
  {8,   9, -31, -55, -39, -22, -29,  13}
};

short int kingtable2[8][8] = {
{-54, -27, -22, -19,  -9,   8, -13, -74},
{-40,  -8,  -3,  -5,  20,  18,  17, -35},
{-23,   6,  12,  19,  21,  20,  15, -19},
 {-8,  19,  25,  23,  27,  19,  15, -19},
{-22,  21,  27,  29,  25,  23,  18, -11},
{-10,  11,  20,  25,  34,  46,  38,  15},
{-36,  -3,   7,  10,  27,  45,  27,   3},
{-56, -18, -12, -13,  -1,  13,  10, -18}
};

short int bishop_pair[9] = { 10,  16,  21,  22,  22,  20,  12,  11,  16};
short int passedmgbonus[8] = {  0,   3,   4,   6,  26,  73, 140,   0};
short int passedegbonus[8] = { 0,   3,   6,  28,  57,  98, 184,   0};
short int knightmobilitymg[9] = {-27,  -3,   5,   6,  13,  18,  22,  24,  21};
short int bishopmobilitymg[14] = {-18,  -7,   4,  11,  16,  18,  20,  22,  26,  29,  37,  38,  45,  49};
short int rookmobilitymg[15] = {-17, -10,  -7,  -2,  -3,   3,   7,  12,  18,  19,  19,  24,  27,  26,  26};
short int knightmobilityeg[9] = {-40, -22,   2,  16,  15,  19,  19,  20,  14};
short int bishopmobilityeg[14] = {-32, -13,  -7,   1,  13,  20,  24,  27,  30,  28,  31,  33,  44,  44};
short int rookmobilityeg[15] = {-33,  -3,  13,  25,  41,  48,  56,  57,  59,  64,  68,  73,  76,  72,  74};
short int doubledpen = -14;
short int isopen =  -11;
short int backwardspen = -8;

short int kingdangertable[100] = {
    0,  0,   1,   2,   3,   5,   7,   9,  12,  15,
  18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
  68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
 140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
 260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
 377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
 494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
 500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
 500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
 500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};
short int king_attack_count[2];

short int futility[4] = {125, 125, 300, 300};
char safecapture(struct board_info *board, int file, int rank, char color);
int com_uci( struct board_info *board, struct movelist *movelst, int *key, char *color);


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
    for (int i = 0; i < 64; i++){
        for (int n = 0; n < 64; n++){
            HISTORYTABLE[WHITE][i][n] = 0;
            HISTORYTABLE[BLACK][i][n] = 0;
        }
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

void insert(unsigned long long int position, int depthleft, int eval, char type, char *bestmove){
    int index = position & _mask;
    if (TT[index].zobrist_key == position && TT[index].depth >= depthleft && TT[index].age < 2){
        return;
    }
    TT[index].zobrist_key = position;
    TT[index].depth = depthleft;
    TT[index].eval = eval;
    TT[index].type = type;
    TT[index].age = 0;
    memcpy(TT[index].bestmove, bestmove, 8);
}


void clearTT(bool delete){
    int i; for (i = 0; i < TTSIZE; i++){
        if (delete){
        TT[i].zobrist_key = 0;
        TT[i].depth = 0;
        TT[i].age = 0;
        }
        else{
            if (TT[i].age > 5){
                TT[i].zobrist_key = 0;
                TT[i].depth = 0;     
                TT[i].age = 0;           
            }
            else{
                TT[i].age++;
            }
        }
        
    }
}

void clearPV(){
    int i; for (i = 0; i < 5000; i++){
        pvstack[i][0] = '\0';
    }
    pvptr = 0;
}

void clearHistory(){
    int n, i;
        for (int i = 0; i < 64; i++){
        for (int n = 0; n < 64; n++){
            HISTORYTABLE[WHITE][i][n] = 0;
            HISTORYTABLE[BLACK][i][n] = 0;
        }
    }
}
void clearKiller(){
    for (int i = 0; i < 50; i++){
        KILLERTABLE[i][0][0] = '\0';
        KILLERTABLE[i][1][0] = '\0';
    }
}

//to access zobrist number for say white knight on a6 we go zobristtable[(64*piece)+(file-1*8) + rank-1]
//768-771 is castling
//color key is 772
void printfull(struct board_info *board, char color){
    int i, n;
    int until, diff; if (color == WHITE){
        i = 7, until = -1, diff = -1;
    }
    else{
        i = 0, until = 8, diff = 1;
    }
    int nstart, nuntil, ndiff; if (color == WHITE){
        nstart = 0, nuntil = 8, ndiff = 1;
    }
    else{
        nstart = 7, nuntil = -1, ndiff = -1;
    }
    for (; i != until; i += diff){

        printf("+---+---+---+---+---+---+---+---+\n");
        for (n = nstart; n != nuntil; n += ndiff){
        printf("| ");
            if (board->board[n][i] == BLANK){
                printf("  ");
            }
            else{

                switch(board->board[n][i]){
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
    }
    printf("+---+---+---+---+---+---+---+---+\n\n");
}
void move(struct board_info *board, char *move, char color){
    board->epsquare = -10;
    if (isupper(move[0])){
        int v =  move[1]-97, vv = atoi(&move[2])-1, w =  move[4]-97, ww = atoi(&move[5])-1;
        if (v < 0 || v > 7 || vv < 0 || vv > 7){
            exit(0);
        }
        CURRENTPOS ^= ZOBRISTTABLE[(board->board[v][vv]<<6)+(v<<3)+vv];

        if (strchr(move, 'x')){
            CURRENTPOS ^= ZOBRISTTABLE[(board->board[w][ww]<<6)+(w<<3)+ww];
            board->pnbrqcount[color^1][(board->board[w][ww]-(color^1))/2]--;
        }
        board->board[w][ww] = board->board[v][vv];
        board->board[v][vv] = BLANK;

        CURRENTPOS ^= ZOBRISTTABLE[(board->board[w][ww]<<6)+(w<<3)+ww];
        if (move[0] == 'K'){
            if (board->castling[color][0] || board->castling[color][1]){
                CURRENTPOS ^= ZOBRISTTABLE[768+(color*2)];
                CURRENTPOS ^= ZOBRISTTABLE[769+(color*2)];
                board->castling[color][0] = false, board->castling[color][1] = false;
            }
            board->kingpos[color] = w*8 + ww;
        }

        else if (move[0] == 'R'){
            if (move[1] == 'a' && board->castling[color][0]){
                CURRENTPOS ^= ZOBRISTTABLE[768+(color*2)];
                board->castling[color][0] = false;
            }
            else if (board->castling[color][1]){
                CURRENTPOS ^= ZOBRISTTABLE[769+(color*2)];
                board->castling[color][1] = false;
            }
        }
    }
    else if (isalpha(move[0])){
        int v =  move[0]-97, vv = atoi(&move[1])-1, w =  move[3]-97, ww = atoi(&move[4])-1;
        if (v < 0 || v > 7 || vv < 0 || vv > 7){
            exit(0);
        }
        CURRENTPOS ^= ZOBRISTTABLE[(board->board[v][vv]<<6)+(v<<3)+vv];       
        if (strchr(move, 'x')){
            if (move[5] == 'e'){
                CURRENTPOS ^= ZOBRISTTABLE[(board->board[w][vv]<<6)+(w<<3)+vv];
                board->board[w][vv] = BLANK;
                board->pnbrqcount[color^1][0]--;
            }
            else{
                CURRENTPOS ^= ZOBRISTTABLE[(board->board[w][ww]<<6)+(w<<3)+ww];
                board->pnbrqcount[color^1][(board->board[w][ww]-(color^1))/2]--;
            }
        }
        else if (abs(vv-ww) == 2){
            board->epsquare = (w*8)+ww;
        }
        board->board[w][ww] = board->board[v][vv];
        board->board[v][vv] = BLANK;
        
        if (move[5] && move[5] != 'e'){ //promotion
            board->pnbrqcount[color][0]--;
            if (move[5] == 'Q'){
                board->pnbrqcount[color][4]++;   
                board->board[w][ww] = WQUEEN + color;
            }
            else if (move[5] == 'N'){
                board->pnbrqcount[color][1]++;  
                board->board[w][ww] = WKNIGHT + color;
            }
            else if (move[5] == 'R'){
                board->pnbrqcount[color][3]++;  
                board->board[w][ww] = WROOK + color;
            }
            else if (move[5] == 'B'){
                board->pnbrqcount[color][2]++;  
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
            board->kingpos[color] = 16+rank;
        }
        else{
            
            CURRENTPOS ^= ZOBRISTTABLE[(board->board[7][rank]<<6)+(0<<3)+rank], ZOBRISTTABLE[(board->board[4][rank]<<6)+(4<<3)+rank];
            CURRENTPOS ^= ZOBRISTTABLE[(board->board[7][rank]<<6)+(4<<3)+rank], ZOBRISTTABLE[(board->board[4][rank]<<6)+(6<<3)+rank];

            board->board[7][rank] = BLANK, board->board[4][rank] = BLANK, board->board[5][rank] = WROOK+color, board->board[6][rank] = WKING+color;
            board->kingpos[color] = 48+rank;
        }

        CURRENTPOS ^= ZOBRISTTABLE[768+(color*2)];
        CURRENTPOS ^= ZOBRISTTABLE[769+(color*2)];
    }
    CURRENTPOS ^= ZOBRISTTABLE[772];
}
void move_add(struct board_info *board, struct movelist *movelst, int *key, char *move, char color){
    int k = *key;
    memcpy(movelst[k].move, move, 8);
    movelst[k].fen = CURRENTPOS;
    if (islower(move[0]) || strchr(move, 'x')){
        movelst[k].halfmoves = 0;
    }
    else{
        movelst[k].halfmoves = movelst[k-1].halfmoves+1;
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
    board->epsquare = -10;
}

void setempty(struct board_info *board){
    char brd[8][8] = {
        {BROOK, BKING, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, BLANK},
        {BLANK, BLANK, BLANK, BLANK, BLANK, BLANK, WKING, WROOK}
    };
    memcpy(board->board, brd, 64);
    char count[2][5] = {
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0}
    };
    memcpy(board->pnbrqcount, count, 10);
    board->castling[0][0] = false, board->castling[0][1] = false, board->castling[1][0] = false, board->castling[1][1] = false;
    board->kingpos[0] = 62, board->kingpos[1] = 1;
    board->epsquare = -10;
}

void setmovelist(struct movelist *movelst, int *key){
    
    movelst[0].fen = CURRENTPOS;
    *key = 1;
    movelst[0].move[0] = '\0';
    movelst[1].move[0] = '\0';
    movelst[0].halfmoves = 0;
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
                    piece_add(list, key, file+97, rank+'0'+1, n+97, i+'0'+1, '-', 'K');
                    }
                }
                else{
                    piece_add(list, key, file+97, rank+'0'+1, n+97, i+'0'+1, 'x', 'K');
                }
            }
        }
    }
}
void knight_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture, bool isgen){
    char n, i;
    isattacker = false;
    current_mobility = 0;
    for (n = file-2; n < file+3 && n < 8; n++){
        if (n == file || n < 0){
            continue;
        }
        i = rank + 3-abs(file-n);
        if (i > -1 && i < 8 && board->board[n][i]%2 != color){
            if (!isgen && (((color == WHITE && i > 5) || (color == BLACK && i < 2))||
            ((n == 0 || board->board[n-1][i+(-color+(color^1))] != WPAWN + (color^1)) &&
             (n == 7 || board->board[n+1][i+(-color+(color^1))] != WPAWN + (color^1))))){
                current_mobility++;
             }
            if (isgen){
                if (board->board[n][i] == BLANK){
                    
                    if (!needscapture){
                    piece_add(list, key, file+97, rank+'0'+1, n+97, i+'0'+1, '-', 'N');
                    }
                }
                else{
                    piece_add(list, key, file+97, rank+'0'+1, n+97, i+'0'+1, 'x', 'N');
                }
            }
            else if (n > (board->kingpos[color^1]/8)-2 && board->board[n][i] != WKING+(color^1) && n < (board->kingpos[color^1]/8)+2 &&
             i > ((board->kingpos[color^1]%8) - 2 - (color^1)) && i < ((board->kingpos[color^1]%8) + 2 + color)){
                king_attack_count[color] += 2;
                isattacker = true;
             }
        }
        i = rank - (3-abs(n-file));
        if (i > -1 && i < 8 && board->board[n][i]%2 != color){
            if (!isgen && (((color == WHITE && i > 5) || (color == BLACK && i < 2))||
            ((n == 0 || board->board[n-1][i+(-color+(color^1))] != WPAWN + (color^1)) &&
             (n == 7 || board->board[n+1][i+(-color+(color^1))] != WPAWN + (color^1))))){
                current_mobility++;
             }
            if (isgen){
                if (board->board[n][i] == BLANK){
                    
                    if (!needscapture){
                    piece_add(list, key, file+97, rank+'0'+1, n+97, i+'0'+1, '-', 'N');
                    }
                }
                else{
                    piece_add(list, key, file+97, rank+'0'+1, n+97, i+'0'+1, 'x', 'N');
                }
            }
            else if (n > (board->kingpos[color^1]/8)-2 && board->board[n][i] != WKING+(color^1) && n < (board->kingpos[color^1]/8)+2 &&
             i > ((board->kingpos[color^1]%8) - 2 - (color^1)) && i < ((board->kingpos[color^1]%8) + 2 + color)){
                isattacker = true;
                king_attack_count[color] += 2;
             }
        }
    }
}
void bishop_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture, bool isgen){
    char letterchange, numchange;
    isattacker = false;
    char n, i;
    current_mobility = 0;
    for (letterchange = -1; letterchange < 2; letterchange += 2){
        for (numchange = -1; numchange < 2; numchange += 2){
            n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
            if (!isgen && (((color == WHITE && i > 5) || (color == BLACK && i < 2))||
            ((n == 0 || board->board[n-1][i+(-color+(color^1))] != WPAWN + (color^1)) &&
             (n == 7 || board->board[n+1][i+(-color+(color^1))] != WPAWN + (color^1))))){
                current_mobility++;
             }
             if (!isgen){
                if (board->board[n][i] != WKING+(color^1) && n > (board->kingpos[color^1]/8)-2 && n < (board->kingpos[color^1]/8)+2 &&
                    i > ((board->kingpos[color^1]%8) - 2 - (color^1)) && i < ((board->kingpos[color^1]%8) + 2 + color)){
                    isattacker = true;
                    king_attack_count[color] += 2;
                }
             }
                else if (board->board[n][i] == BLANK){
                    if (!needscapture){
                            piece_add(list, key, file+97, rank + '0' +1, n+97, i + '0' + 1, '-', 'B');
                    }
                }

                else{
                        piece_add(list, key, file+97, rank+'0'+1, n+97, i+'0'+1, 'x', 'B');
                }
                if (board->board[n][i] != BLANK){
                    break;
                }
                n += letterchange, i += numchange;
            }
        }
    }
}
void rook_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture, bool isgen){
    char letterchange, numchange;
    char n, i;
    isattacker = false;
    current_mobility = 0;
    for (letterchange = -1; letterchange < 2; letterchange += 1){
        for (numchange = -1; numchange < 2; numchange += 1){
            if (abs(letterchange+numchange)%2 == 0){
                continue;
            }
            n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
            if (!isgen && (((color == WHITE && i > 5) || (color == BLACK && i < 2))||
            ((n == 0 || board->board[n-1][i+(-color+(color^1))] != WPAWN + (color^1)) &&
             (n == 7 || board->board[n+1][i+(-color+(color^1))] != WPAWN + (color^1))))){
                current_mobility++;
             }

             if (!isgen){

                if (board->board[n][i] != WKING+(color^1) && n > (board->kingpos[color^1]/8)-2 && n < (board->kingpos[color^1]/8)+2 &&
                    i > ((board->kingpos[color^1]%8) - 2 - (color^1)) && i < ((board->kingpos[color^1]%8) + 2 + color)){
                    king_attack_count[color] += 3;
                    isattacker = true;
                    char temp = board->board[n][i];
                    board->board[n][i] = WROOK+color, board->board[file][rank] = BLANK;
                    if (abs(n-(board->kingpos[color^1]/8)) + abs(i-(board->kingpos[color^1]%8)) == 1 && 
                    safecapture(board, n, i, color) != 0 && safecapture(board, n, i, color^1) != 2){

                        king_attack_count[color] += 2;
                    }
                    board->board[n][i] = temp, board->board[file][rank] = WROOK+color;
                }
             }

                else if (board->board[n][i] == BLANK){
                    if (!needscapture){
                            piece_add(list, key, file+97, rank + '0' +1, n+97, i + '0' + 1, '-', 'R');
                    }
                }
                else{
                        piece_add(list, key, file+97, rank+'0'+1, n+97, i+'0'+1, 'x', 'R');
                }
                if (board->board[n][i] != BLANK){
                    break;
                }
                n += letterchange, i += numchange;
            }
        }
    }
}
void queen_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture, bool isgen){
    int letterchange, numchange;
    isattacker = false;
    int n, i;
    for (letterchange = -1; letterchange < 2; letterchange += 1){
        for (numchange = -1; numchange < 2; numchange += 1){
            if (letterchange == 0 && numchange == 0){
                continue;
            }
            n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){

                if (!isgen){

                if (!isgen && board->board[n][i] != WKING+(color^1) && n > (board->kingpos[color^1]/8)-2 && n < (board->kingpos[color^1]/8)+2 &&
                    i > ((board->kingpos[color^1]%8) - 2 - (color^1)) && i < ((board->kingpos[color^1]%8) + 2 + color)){
                    king_attack_count[color] += 5;
                    isattacker = true;
                    char temp = board->board[n][i];
                    board->board[n][i] = WQUEEN+color, board->board[file][rank] = BLANK;
                    if (abs(n-(board->kingpos[color^1]/8)) <= 1 && abs(i-(board->kingpos[color^1]%8)) <= 1 
                    && safecapture(board, n, i, color) != 0 && safecapture(board, n, i, color^1) != 2){
                        king_attack_count[color] += 6;
                    }
                    board->board[n][i] = temp, board->board[file][rank] = WQUEEN+color;
                }

                }
                else if (board->board[n][i] == BLANK){
                    if (!needscapture){
                            piece_add(list, key, file+97, rank + '0' +1, n+97, i + '0' + 1, '-', 'Q');
                    }
                }
                else{
                    piece_add(list, key, file+97, rank+'0'+1, n+97, i+'0'+1, 'x', 'Q');
                }
                if (board->board[n][i] != BLANK){
                    break;
                }
                n += letterchange, i += numchange;
            }
        }
    }    
}
void pawn_moves(struct list *list, struct board_info *board, int *key, char file, char rank, char color, bool needscapture, bool isgen){
    int srank, lrank, diff;
    if (color == WHITE){
        srank = 1, lrank = 7;
    }
    else{
        srank = 6, lrank = 0;
    }
    diff = -color + (color^1);
    if (board->board[file][rank+diff] == BLANK && !needscapture){ //forward
        if (rank+diff == lrank && isgen){
            pawn_add(list, key, file+97, rank+'0'+1, file+97, rank+diff+'0'+1, '-', 'Q', false);
            pawn_add(list, key, file+97, rank+'0'+1, file+97, rank+diff+'0'+1, '-', 'R', false);
            pawn_add(list, key, file+97, rank+'0'+1, file+97, rank+diff+'0'+1, '-', 'B', false);
            pawn_add(list, key, file+97, rank+'0'+1, file+97, rank+diff+'0'+1, '-', 'N', false);
        }
        else if (isgen){
            pawn_add(list, key, file+97, rank+'0'+1, file+97, rank+diff+'0'+1, '-', '\0', false);
            if (rank == srank && board->board[file][rank+(diff*2)] == BLANK){
                pawn_add(list, key, file+97, rank+'0'+1, file+97, rank+(diff*2)+'0'+1, '-', '\0', false);
            }

        }
        if (!isgen && file > (board->kingpos[color^1]/8)-2 && file < (board->kingpos[color^1]/8)+2 &&
            rank+diff > (board->kingpos[color^1]%8 - 2 - (color^1)) && rank+diff < (board->kingpos[color^1]%8 + 2 + color)){
            king_attack_count[color] += 1;
        }
    }
    if (file < 7 && board->board[file+1][rank+diff]%2 == (color^1)){ //capture right
        if (rank+diff == lrank && isgen){
            pawn_add(list, key, file+97, rank+'0'+1, file+98, rank+diff+'0'+1, 'x', 'Q', false);
            pawn_add(list, key, file+97, rank+'0'+1, file+98, rank+diff+'0'+1, 'x', 'R', false);
            pawn_add(list, key, file+97, rank+'0'+1, file+98, rank+diff+'0'+1, 'x', 'B', false);
            pawn_add(list, key, file+97, rank+'0'+1, file+98, rank+diff+'0'+1, 'x', 'N', false);
        }        
        else if (isgen){
            pawn_add(list, key, file+97, rank+'0'+1, file+98, rank+diff+'0'+1, 'x', '\0', false);
        }
            if (!isgen && file+1 > (board->kingpos[color^1]/8)-2 && file+1 < (board->kingpos[color^1]/8)+2 &&
                rank+diff > (board->kingpos[color^1]%8 - 2 - (color^1)) && rank+diff < (board->kingpos[color^1]%8 + 2 + color)){
                king_attack_count[color] += 1;
             }
    }
    if (file > 0 && board->board[file-1][rank+diff]%2 == (color^1)){ //capture left
        if (rank+diff == lrank && isgen){
            pawn_add(list, key, file+97, rank+'0'+1, file+96, rank+diff+'0'+1, 'x', 'Q', false);
            pawn_add(list, key, file+97, rank+'0'+1, file+96, rank+diff+'0'+1, 'x', 'R', false);
            pawn_add(list, key, file+97, rank+'0'+1, file+96, rank+diff+'0'+1, 'x', 'B', false);
            pawn_add(list, key, file+97, rank+'0'+1, file+96, rank+diff+'0'+1, 'x', 'N', false);
        }        
        else if (isgen){
            pawn_add(list, key, file+97, rank+'0'+1, file+96, rank+diff+'0'+1, 'x', '\0', false);
        }
            if (!isgen && file-1 > (board->kingpos[color^1]/8)-2 && file-1 < (board->kingpos[color^1]/8)+2 &&
                rank+diff > (board->kingpos[color^1]%8 - 2 - (color^1)) && rank+diff < (board->kingpos[color^1]%8 + 2 + color)){
                king_attack_count[color] += 1;
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
                        if (abs(kingfile-n) <= 1 && abs(kingrank-i) <= 1){
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
    board2.kingpos[color] = board->kingpos[color] + 8;
    if (check_check(&board2, color)){
        return;
    }
    board2.board[5][rank] = BLANK, board2.board[6][rank] = WKING+color;
    board2.kingpos[color] += 8;
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
    if (board->board[4][rank]-color != WKING || board->board[0][rank]-color != WROOK || 
    board->board[1][rank] != BLANK || board->board[2][rank] != BLANK || board->board[3][rank] != BLANK){
        return;
    }
    if (check_check(board, color)){
        return;
    }
    struct board_info board2;
    memcpy(board2.board, board->board, 64);
    board2.board[4][rank] = BLANK, board2.board[3][rank] = WKING+color;
    board2.kingpos[color] = board->kingpos[color] - 8;
    if (check_check(&board2, color)){
        return;
    }
    board2.board[3][rank] = BLANK, board2.board[2][rank] = WKING+color;
    board2.kingpos[color] -= 8;
    if (check_check(&board2, color)){
        return;
    }
    int k = *key;
    list[k].move[0] = '0', list[k].move[1] = '-', list[k].move[2] = '0', list[k].move[3] = '-', list[k].move[4] = '0', list[k].move[5] = '\0';
    list[k+1].move[0] = '\0';
    *key = k+1;
    return;
}
void en_passant(struct board_info *board, struct list *list, int *listkey, int *movelistkey, struct movelist *movelst, char color){
    if (board->epsquare == -10){
        return;
    }
    int file = board->epsquare/8, rank = board->epsquare%8;
    int diff; if (color == WHITE){
        diff = 1;
    }
    else{
        diff = -1;
    }
    if (board->board[file][rank] != WPAWN + (color^1)){ //almost always because it tried to en passant after a null move
        /*printf("an error occured %i %i %i %s\n", color, file, rank, movelst[*movelistkey-1].move);
        printfull(board, color);

        exit(1);*/
        return;
    }
    if (file < 7 && board->board[file+1][rank] == WPAWN + color){
        pawn_add(list, listkey, file+98, rank+'0'+1, file+97, rank+diff+1+'0', 'x', '\0', true);
    }
    if (file > 0 && board->board[file-1][rank] == WPAWN + color){
        pawn_add(list, listkey, file+96, rank+'0'+1, file+97, rank+diff+1+'0', 'x', '\0', true);
    }
}
void movelist(struct board_info *board, struct list *list, struct movelist *movelst, int *mkey, char color){
    int key = 0;
    int k;
    for (int n = 0; n < 8; n++){
        for (int i = 0; i < 8; i++){
            if (board->board[n][i]%2 == color){
                switch (board->board[n][i]-color){
                    case WPAWN:
                    pawn_moves(list, board, &key, n, i, color, false, true); break;
                    case WKNIGHT:
                    knight_moves(list, board, &key, n, i, color, false, true); break;
                    case WBISHOP:
                    bishop_moves(list, board, &key, n, i, color, false, true); break;
                    case WROOK:
                    rook_moves(list, board, &key, n, i, color, false, true); break;
                    case WQUEEN:
                    queen_moves(list, board, &key, n, i, color, false, true); break;
                    case WKING:
                    king_moves(list, board, &key, n, i, color, false); break;
                    default:
                    printf("error reading board\n");
                    for (int i = 1; i < *mkey; i++){
                        printf("%s ", movelst[i].move);
                    }
                    printfull(board, color);
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
                    pawn_moves(list, board, &key, n, i, color, true, true); break;
                    case WKNIGHT:
                    knight_moves(list, board, &key, n, i, color, true, true); break;
                    case WBISHOP:
                    bishop_moves(list, board, &key, n, i, color, true, true); break;
                    case WROOK:
                    rook_moves(list, board, &key, n, i, color, true, true); break;
                    case WQUEEN:
                    queen_moves(list, board, &key, n, i, color, true, true); break;
                    case WKING:
                    king_moves(list, board, &key, n, i, color, true); break;
                    default:
                    printf("error reading board\n");
                    printfull(board, color);
                    exit(1);
                }
            }
        }
    }    
    en_passant(board, list, &key, mkey, movelst, color);
}

void remove_illegal(struct board_info *board, struct list *list, char color){ //for human moves
    long long unsigned int original_pos = CURRENTPOS;
    int key = 0; while (list[key].move[0] != '\0'){
        struct board_info board2;
        memcpy(board2.board, board->board, 64);
        memcpy(board2.kingpos, board->kingpos, 2);
        
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
    CURRENTPOS = original_pos;
}

bool checkdraw1(struct board_info *board){
    if (board->pnbrqcount[0][0] || board->pnbrqcount[0][3] || board->pnbrqcount[0][4] || 
        board->pnbrqcount[1][0] || board->pnbrqcount[1][3] || board->pnbrqcount[1][4]){
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
    if (movelst[*key-1].halfmoves > 99){
        return 2;
    }
    int lmove = *key-1;
    int k = lmove-2;
    int rep = 0;
    while (k >= 1){
        if (movelst[k].fen == movelst[lmove].fen){
            rep++;
            if (rep > 1){
                return 2;
            }
        }
        k -= 2;
    }
    return rep;
}
int humanmove(struct board_info *board, struct movelist *movelst, int *key, char color){
    printfull(board, color);
    if (checkdraw2(movelst, key) == 2 || checkdraw1(board)){
        printf("n\n");
        return 11;
    }
    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    movelist(board, list, movelst, key, color);
    remove_illegal(board, list, color);
    if (list[0].move[0] == '\0'){
        if (check_check(board, color)){
            return -1000;
        }
        else{
            return 11;
        }
    }
    while (true){
        char mve[8];
        printf("Enter the move to play: ");
        if (scanf("%7s", mve)){char c; while ((c = fgetc(stdin)) != '\n' && c != EOF);}
        if (!strcmp(mve, "h")){
            int i = 0; while (list[i].move[0] != '\0'){
                printf("%s\n", list[i].move);
                i++;
            }
            printf("\n");
        }
        else if (!strcmp(mve, "r")){
            printf("Are you sure you want to resign? (y) for yes: ");
            if(scanf("%7s", mve)){char c; while ((c = fgetc(stdin)) != '\n' && c != EOF);}
            if (!strcmp(mve, "y")){
                return 10;
            }
        }
        else{
            int i = 0; while (list[i].move[0] != '\0'){
                if (!strcmp(mve, list[i].move)){
                    move(board, mve, color);
                    //memcpy(&movelst[*key].boardstate, board, boardsize);
                    move_add(board, movelst, key, mve, color);
                    return 0;
                }
                i++;
            }
            printf("Move %s is illegal\n", mve);
        }
    }
}


char* uci_move(char *move, char *temp, char color){
    if (move[0] == '0'){
        
        if (color == WHITE){
            if (move[3] == '\0'){ //kingside castling
                strcpy(temp, "e1g1\0");
            }
            else{
                strcpy(temp, "e1c1\0");
            }
        }
        else{
            if (move[3] == '\0'){ //kingside castling
                strcpy(temp, "e8g8\0");
            }
            else{
                strcpy(temp, "e8c8\0");
            }
        }
    }
    else{
        int pos; if (isupper(move[0])){
            pos = 1;
        }
        else{
            pos = 0;
        }
        temp[0] = move[pos], temp[1] = move[pos+1], temp[2] = move[pos+3], temp[3] = move[pos+4];
        if (pos == 0 && move[5] != '\0' && move[5] != 'e'){
            temp[4] = move[5];
            temp[5] = '\0';
        }
        else{
            temp[4] = '\0';
        }
    }
    return temp;
}

char* move_to_uci(char *uci, char *temp, char color, struct board_info *board){
    if ((color == WHITE && board->board[4][0] == WKING) || (color == BLACK && board->board[4][7] == BKING)){
    if (uci[0] == 'e' && (uci[1] == '1' || uci[1] == '8') && uci[2] == 'g'){
        strcpy(temp, "0-0\0");
        return temp;
    }
    else if (uci[0] == 'e' && (uci[1] == '1' || uci[1] == '8') && uci[2] == 'c'){
        strcpy(temp, "0-0-0\0");
        return temp;
    }
    }
        int pos = 1;
        temp[6] = '\0';
        switch (board->board[uci[0]-97][atoi(&uci[1])-1] - (board->board[uci[0]-97][atoi(&uci[1])-1]%2)){
            case WPAWN:
            pos = 0; 
            if (isalpha(uci[4])){temp[5] = toupper(uci[4]);}
            else {temp[5] = '\0';} break;
            case WKNIGHT:
            temp[0] = 'N'; break;
            case WBISHOP:
            temp[0] = 'B'; break;
            case WROOK:
            temp[0] = 'R'; break;
            case WQUEEN:
            temp[0] = 'Q'; break;
            case WKING:
            temp[0] = 'K'; break;
            default:
            printf("error\n"); break;
        }
        temp[pos] = uci[0], temp[pos+1] = uci[1], temp[pos+3] = uci[2], temp[pos+4] = uci[3];
        temp[pos+2] = '-';
        if (board->board[uci[2]-97][atoi(&uci[3])-1] != BLANK){
            temp[pos+2] = 'x';
        }
        if (pos == 0 && (color == WHITE && ((uci[2]-97)*8) + atoi(&uci[3])-2 == board->epsquare) || (color == BLACK && ((uci[2]-97)*8) + atoi(&uci[3]) == board->epsquare)){
            temp[pos+2] = 'x'; temp[5] = 'e'; temp[6] = 'p'; temp[7] = '\0';
        }
    return temp;
}

#ifdef _WIN32
int pipe;
HANDLE hstdin;
#endif
int InputPending()
/* http://talkchess.com/forum3/viewtopic.php?p=943992#p943992 */
{  // checks for waiting input in pipe
#ifdef _WIN32
  int init; HANDLE inp; DWORD cnt = 0;
  if(!init) inp = GetStdHandle(STD_INPUT_HANDLE);
  if (!PeekNamedPipe(inp, NULL, 0, NULL, &cnt, NULL)){exit(1);}
#else
  int cnt;
  if (ioctl(0, FIONREAD, &cnt)) return 1;
#endif
  return cnt;
}


char *getsafe(char *buffer, int count)
{
    	char *result = buffer, *np;
	if ((buffer == NULL) || (count < 1)){
        result =  NULL;
    }

	else if (count == 1)
		*result = '\0';

	else if ((result = fgets(buffer, count, stdin)) != NULL)
	if (np = strchr(buffer, '\n'))
		*np = '\0';
	return result;
}


int init(){
#ifdef _WIN32
    unsigned int dw;
    hstdin = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(hstdin, &dw);

    if (!pipe) {
        SetConsoleMode(hstdin,dw&~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(hstdin);
    } else {
        setvbuf(stdin,NULL,_IONBF,0);
        setvbuf(stdout,NULL,_IONBF,0);
    }
#endif
    printf("Willow 2.5, by Adam Kulju\n");
    return 0;
}



int game_end(struct board_info *board, struct movelist *movelst, int *key, char color){  //checks if the side to MOVE (comp) has been mated or drawn
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

char safecapture(struct board_info *board, int file, int rank, char color){
    int n, i;
    int flag = 2;
    for (n = file-2; n < file+3 && n < 8; n++){
        if (n == file || n < 0){
            continue;
        }
        i = rank + (3-abs(n-file));
        if (i > -1 && i < 8 && board->board[n][i]-(color^1) == WKNIGHT){
                return 0;
        }
        i = rank - (3-abs(n-file));
        if (i > -1 && i < 8 && board->board[n][i]-(color^1) == WKNIGHT){
                return 0;
        }
    }  
    int letterchange, numchange;
    for (letterchange = -1; letterchange < 2; letterchange++){
        for (numchange = -1; numchange < 2; numchange++){           
            if (!letterchange && !numchange){
                continue;
            }
            int n = file + letterchange, i = rank + numchange;
            while (n > -1 && n < 8 && i > -1 && i < 8 && board->board[n][i]%2 != color){
                if (board->board[n][i]%2 == (color^1)){                  
                    switch(board->board[n][i]-(color^1)){
                        case WQUEEN:
                        return 0;
                        case WROOK:
                        if (abs(letterchange+numchange)%2 == 1){
                            return 0;
                        } break;
                        case WBISHOP:
                        if (abs(letterchange+numchange)%2 == 0){                           
                            return 0;
                        } break;
                        case WPAWN:
                        
                        if (abs(file-n) == 1 && ((color == WHITE && i-rank == 1) || (color == BLACK && rank-i == 1))){
                            return 0;
                        } break;
                        case WKING:
                        if (abs(file-n) == 1 && abs(rank-1) == 1){
                            flag = 1; //if there are no other attackers, returning 1 will let us check if that piece is backed up because if so the king can't take it.
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
    return flag;
}

int see(struct board_info *board, char *mve, char color, bool isque){

    int i;
    int attacker, victim;
    if (islower(mve[0])){
            if (mve[5] == 'e'){
        return 1000;
    }
        attacker = 1000;
        i = 3;
    }
    else{
        i = 4;
        switch(mve[0]){
            case 'Q':
            attacker = 9000; break;
            case 'R':
            attacker = 5000; break;
            case 'K':
            attacker = 9500; break;
            default:
            attacker = 3000; break;
        }
    }

    switch(board->board[(int)mve[i]-97][atoi(&mve[i+1])-1]-(color^1)){
        case WPAWN:
        victim = 1000; break;
        case WROOK:
        victim = 5000; break;
        case WQUEEN:
        victim = 9000; break; 
        default:
        victim = 3000; break;
    }
    char a = safecapture(board, (int)mve[i]-97, atoi(&mve[i+1])-1, color);
    if (a == 2){
        return (victim-(attacker/1000));
    }
    if (a == 1){
        char temp = board->board[(int)mve[i-3]-97][atoi(&mve[i-2])-1];

        board->board[(int)mve[i-3]-97][atoi(&mve[i-2])-1] = BLANK;
        
        a = safecapture(board, (int)mve[i]-97, atoi(&mve[i+1])-1, color^1);

        board->board[(int)mve[i-3]-97][atoi(&mve[i-2])-1] = temp;
        if (a != 2){ //this means that the opponent can't take that piece with his king. In this case the higher material the better.
            //printfull(board, color);
            //printf("%s\n", mve);
            return (victim+(attacker/1000));
        }
    }
    if (victim-attacker >= 0){
        return (victim-(attacker/100));
    }

    return 150+((victim-attacker)/1000);
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

int material(struct board_info *board, int *phase){
    int wval = 0, bval = 0;
    for (int i = 0; i < 5; i++){
        if (i == 4){
                *phase += 4*board->pnbrqcount[WHITE][i];
                *phase += 4*board->pnbrqcount[BLACK][i];
        }
        else{
            *phase += ((i+1)/2)*board->pnbrqcount[WHITE][i];
            *phase += ((i+1)/2)*board->pnbrqcount[BLACK][i];
        }
    }
    if (*phase > MAXPHASE){
        *phase = MAXPHASE;
    }
    
    for (int i = 0; i < 5; i++){
        wval += (*phase*VALUES[i]*board->pnbrqcount[WHITE][i] + (24-*phase)*VALUES2[i]*board->pnbrqcount[WHITE][i])/24;
        bval += (*phase*VALUES[i]*board->pnbrqcount[BLACK][i] + (24-*phase)*VALUES2[i]*board->pnbrqcount[BLACK][i])/24;
    }    

    if (board->pnbrqcount[WHITE][2] > 1){
        wval += bishop_pair[board->pnbrqcount[WHITE][0]];
    }
    if (board->pnbrqcount[BLACK][2] > 1){
        bval += bishop_pair[board->pnbrqcount[BLACK][0]];
    }
    return wval-bval;
}
float passedmult(struct board_info *board, int pawnfile, int pawnrank, bool endgameflag, char color){
    //return 1;
    float mult = 1;
    if ((color == WHITE && board->board[pawnfile][pawnrank+1] != BLANK) || (color == BLACK && board->board[pawnfile][pawnrank-1] != BLANK) ){
        mult = 0.6;
    } //give a penalty if there's something in front of the passed pawn.
    if (endgameflag){
        int distside = MAX(abs((board->kingpos[color]/8)-pawnfile), abs((board->kingpos[color]%8)-pawnrank));
        int distopp = MAX(abs((board->kingpos[color^1]/8)-pawnfile), abs((board->kingpos[color^1]%8)-pawnrank));
        if (distside == distopp){
            ;
        }
        else if (distside > distopp && board->pnbrqcount[color][4] == 0){ //if there's no queen and the opponent's king is closer to the pawn than yours
            mult -= 0.15*(distside-distopp);
        }
        else if (distopp > distside && board->pnbrqcount[color^1][4] == 0){
            mult += 0.15*(distopp - distside);
        }
    }
    if (mult < 0){
        return 0;
    }
    if (mult > 1.4){
        return 1.4;
    }
    return mult;
}
int pstscore(struct board_info *board, int phase, short int *blockedpawns, char wbackwards[8], char bbackwards[8], int *values, int *values1){
    char revphase = MAXPHASE-phase;
    int wscore = 0, bscore = 0;
    char wadvanced[8], badvanced[8];
    int dummy_key = 0;
    for (char n = 0; n < 8; n++){
        bool wdblflag = false, bdblflag = false;
        wadvanced[n] = -1, badvanced[n] = 9;
        wbackwards[n] = 9, bbackwards[n] = -1;
        for (char i = 0; i < 8; i++){

            if (phase > 16 && n > 1 && n < 6 && i > 0 && i < 7){
            if (i < 4){
                if (board->board[n][i] != WPAWN && board->board[n+1][i+1] != BPAWN && board->board[n-1][i+1] != BPAWN){
                    (*values)++;
                    if ((board->board[n][i+1] == WPAWN || board->board[n][i+2] == WPAWN || board->board[n][i+3] == WPAWN) && safecapture(board, n, i, WHITE)){
                        (*values)++;
                    }
                }
            }
            else{
                if (board->board[n][i] != BPAWN && board->board[n+1][i-1] != WPAWN && board->board[n-1][i-1] != WPAWN){
                    (*values1)++;
                    if ((board->board[n][i-1] == BPAWN || board->board[n][i-2] == BPAWN || board->board[n][i-3] == BPAWN) && safecapture(board, n, i, BLACK)){
                        (*values1)++;
                    }
                }
            }
            }

            if (board->board[n][i]%2 == WHITE){
                if (board->board[n][i] == WPAWN){
                    if (i > wadvanced[n]){
                        wadvanced[n] = i;
                    }
                    if (i < wbackwards[n]){
                        wbackwards[n] = i;
                    }
                    wscore += (phase*pawntable[n][i] + (revphase)*pawntable2[n][i])/MAXPHASE;
                    if (wdblflag){
                        wscore += doubledpen;
                    }
                    wdblflag = true;
                    if (i < 6 && (board->board[n][i+1] == BPAWN || (n > 0 && n < 7 && board->board[n-1][i+2] == BPAWN && board->board[n+1][i+2] == BPAWN))){
                        (*blockedpawns)++;
                    }
                }
                else if (board->board[n][i] == WBISHOP){
                    bishop_moves((struct list *) 0, board, &dummy_key, n, i, WHITE, false, false);
                    if (isattacker){
                        attacksw++;
                    }
                    wscore += (bishopmobilitymg[current_mobility]*phase + bishopmobilityeg[current_mobility]*(revphase))/MAXPHASE;
                    wscore += (bishoptable[n][i]*phase + bishoptable2[n][i]*(revphase))/MAXPHASE;
                }
                else if (board->board[n][i] == WKNIGHT){
                    knight_moves((struct list *) 0, board, &dummy_key, n, i, WHITE, false, false);
                    if (isattacker){
                        attacksw++;
                    }
                    wscore += (knightmobilitymg[current_mobility]*phase + knightmobilityeg[current_mobility]*(revphase))/MAXPHASE;
                    wscore += (phase*knighttable[n][i] + (revphase)*knighttable2[n][i])/MAXPHASE;
                }
                else if (board->board[n][i] == WROOK){
                    rook_moves((struct list *) 0, board, &dummy_key, n, i, WHITE, false, false);
                    if (isattacker){
                        attacksw++;
                    }
                    wscore += (phase*rookmobilitymg[current_mobility] + (revphase)*rookmobilityeg[current_mobility])/MAXPHASE;
                    wscore += (phase*rooktable[n][i] + (revphase)*rooktable2[n][i])/MAXPHASE;
                }
                else if (board->board[n][i] == WQUEEN){
                    queen_moves((struct list *) 0, board, &dummy_key, n, i, WHITE, false, false);
                    if (isattacker){
                        attacksw+= 2;
                    }
                    wscore += (phase*queentable[n][i] + (revphase)*queentable2[n][i])/MAXPHASE;
                }
            }
            else if (board->board[n][i]%2 == BLACK){
                if (board->board[n][i] == BPAWN){
                    if (i < badvanced[n]){
                        badvanced[n] = i;
                    }
                    if (i > bbackwards[n]){
                        bbackwards[n] = i;
                    }
                    bscore += (phase*pawntable[n][7-i] + (revphase)*pawntable2[n][7-i])/MAXPHASE;
                    if (bdblflag){
                        bscore += doubledpen;
                    }
                    bdblflag = true;
                    if (i > 1 && (board->board[n][i-1] == WPAWN || (n > 0 && n < 7 && board->board[n-1][i-2] == WPAWN && board->board[n+1][i-2] == WPAWN))){
                        (*blockedpawns)++;
                    }
                }
                else if (board->board[n][i] == BBISHOP){
                    bishop_moves((struct list *) 0, board, &dummy_key, n, i, BLACK, false, false);
                    if (isattacker){
                        attacksb++;
                    }
                    bscore += (phase*bishopmobilitymg[current_mobility] + (revphase)*bishopmobilityeg[current_mobility])/MAXPHASE;
                    bscore += (phase*bishoptable[n][7-i] + (revphase)*bishoptable2[n][7-i])/MAXPHASE;
                }
                else if (board->board[n][i] == BKNIGHT){
                    
                    knight_moves((struct list *) 0, board, &dummy_key, n, i, BLACK, false, false);
                    if (isattacker){
                        attacksb++;
                    }
                    bscore += (phase*knightmobilitymg[current_mobility] + (revphase)*knightmobilityeg[current_mobility])/MAXPHASE;
                    bscore +=  (phase*knighttable[n][7-i] + (revphase)*knighttable2[n][7-i])/MAXPHASE;
                }
                else if (board->board[n][i] == BROOK){
                    rook_moves((struct list *) 0, board, &dummy_key, n, i, BLACK, false, false);
                    bscore += (phase*rookmobilitymg[current_mobility] + (revphase)*rookmobilityeg[current_mobility])/MAXPHASE;
                    if (isattacker){
                        attacksb++;
                    }
                    bscore += (phase*rooktable[n][7-i] + (revphase)*rooktable2[n][7-i])/MAXPHASE;
                }
                else if (board->board[n][i] == BQUEEN){
                    queen_moves((struct list *) 0, board, &dummy_key, n, i, BLACK, false, false);
                    if (isattacker){
                        attacksb+= 2;
                    }
                    bscore += (phase*queentable[n][7-i] + (revphase)*queentable2[n][7-i])/MAXPHASE;
                    
                }

            }
        }
    if (n == 7){
            if (wadvanced[7] != -1 && wadvanced[6] == -1){              //evaluates isolated pawns for h file + passed pawns for h file
                wscore += isopen;
            }
            else if (wbackwards[7] < wbackwards[6]){
                wscore += backwardspen;
            }
            if (badvanced[7] != 9 && badvanced[6] == 9){
                bscore += isopen;
            }
            else if (bbackwards[7] > bbackwards[6]){
                bscore += backwardspen;
            }

            if (wadvanced[7] != -1 && wadvanced[7] >= bbackwards[7] && wadvanced[7] >= bbackwards[6]){
                wscore += (phase*passedmgbonus[wadvanced[7]]*passedmult(board, 7, wadvanced[7], false, WHITE) +
                 (revphase)*passedegbonus[wadvanced[7]]*passedmult(board, 7, wadvanced[7], true, WHITE))/MAXPHASE;
            }
            if (badvanced[7] != 9 && badvanced[7] <= wbackwards[7] && badvanced[7] <= wbackwards[6]){
                bscore += (phase*passedmgbonus[7-badvanced[7]]*passedmult(board, 7, badvanced[7], false, BLACK) +
                 (revphase)*passedegbonus[7-badvanced[7]]*passedmult(board, 7, badvanced[7], true, BLACK))/MAXPHASE;
            }
        }
    if (n > 1){
            if (wadvanced[n] == -1 && wadvanced[n-1] != -1 &&  wadvanced[n-2] == -1){  //iso pawns for b-g files and passed pawns for b-g files
                wscore += isopen;
            }
            else if (wbackwards[n-1] < wbackwards[n-2] && wbackwards[n-1] < wbackwards[n]){
                wscore += backwardspen;
            }
            if (badvanced[n] == 9 && badvanced[n-1] != 9 && badvanced[n-2] == 9){
                bscore += isopen;
            }
            else if (bbackwards[n-1] > bbackwards[n-2] && bbackwards[n-1] > bbackwards[n]){
                bscore += backwardspen;
            }
            if (wadvanced[n-1] != -1 && wadvanced[n-1] >= bbackwards[n-1] && wadvanced[n-1] >= bbackwards[n-2] && wadvanced[n-1] >= bbackwards[n]){
                wscore += (phase*passedmgbonus[wadvanced[n-1]]*passedmult(board, n-1, wadvanced[n-1], false, WHITE) +
                 (revphase)*passedegbonus[wadvanced[n-1]]*passedmult(board, n-1, wadvanced[n-1], true, WHITE))/MAXPHASE;
                passed[WHITE][n-1] = wadvanced[n-1];
            }
            if (badvanced[n-1] != 9 && badvanced[n-1] <= wbackwards[n-1] && badvanced[n-1] <= wbackwards[n-2] && badvanced[n-1] <= wbackwards[n]){
                passed[BLACK][n-1] = badvanced[n-1];
                bscore += (phase*passedmgbonus[7-badvanced[n-1]]*passedmult(board, n-1, badvanced[n-1], false, BLACK) +
                 (revphase)*passedegbonus[7-badvanced[n-1]]*passedmult(board, n-1, badvanced[n-1], true, BLACK))/MAXPHASE;
            }
        }
        else if (n == 1){
            if (wadvanced[0] != -1 && wadvanced[1] == -1){    //iso pawns for a file and passed pawns for a file
                wscore += isopen;
            }
            else if (wbackwards[0] < wbackwards[1]){
                wscore += backwardspen;
            }
            if (badvanced[0] != 9 && badvanced[1] == 9){
                bscore += isopen;
            }   
            else if (bbackwards[0] > bbackwards[1]){
                bscore += backwardspen;   
            }
            if (wadvanced[0] != -1 && wadvanced[0] >= bbackwards[0] && wadvanced[0] >= bbackwards[1]){
                passed[WHITE][0] = wadvanced[0];
                wscore += (phase*passedmgbonus[wadvanced[0]]*passedmult(board, 0, wadvanced[0], false, WHITE) +
                 (revphase)*passedegbonus[wadvanced[0]]*passedmult(board, 0, wadvanced[0], true, WHITE))/MAXPHASE;
            }
            if (badvanced[0] != 9 && badvanced[0] <= wbackwards[0] && badvanced[0] <= wbackwards[1]){
                bscore += (phase*passedmgbonus[7-badvanced[0]]*passedmult(board, 0, badvanced[0], false, BLACK) +
                 (revphase)*passedegbonus[7-badvanced[0]]*passedmult(board, 0, badvanced[0], true, BLACK))/MAXPHASE;
                passed[BLACK][0] = badvanced[0];
            }           
        }
    }

        wscore += (phase*kingtable[board->kingpos[0]/8][board->kingpos[0]%8] + (revphase)*kingtable2[board->kingpos[0]/8][board->kingpos[0]%8])/MAXPHASE;
        

        bscore += (phase*kingtable[board->kingpos[1]/8][7-(board->kingpos[1]%8)] + (revphase)*kingtable2[board->kingpos[1]/8][7-(board->kingpos[1]%8)])/MAXPHASE;
        if (king_attack_count[WHITE] > 98){
            king_attack_count[WHITE] = 98;
        }
        if (king_attack_count[BLACK] > 98){
            king_attack_count[BLACK] = 98;
        }
    return wscore-bscore;
}
int space(struct board_info *board, short int *blockedpawns, int wspace, int bspace){

    int weight0 = 0, weight1 = 0;
    
    for (int i = 0; i < 5; i++){
            weight0 += board->pnbrqcount[WHITE][i];            
            weight1 += board->pnbrqcount[BLACK][i];

    }
    if ((*blockedpawns) > 9){(*blockedpawns) = 9;}
    weight0 = weight0 - 3 + 1 + *blockedpawns;
    weight1 = weight1 - 3 + 1 + *blockedpawns;
    if (weight0 < 0){weight0 = 0;}
    if (weight1 < 0){weight1 = 0;}
    return ((wspace * weight0 * weight0 / 16) - (bspace * weight1 * weight1 / 16))/2;
}
int kingsafety(struct board_info *board, char wbackwards[8], char bbackwards[8]){
    float pawnsw = 0, pawnsb = 0; //pawnsw is how many pawns are around the white king, pawnsb the black.
    int n = board->kingpos[WHITE]/8, n1 = board->kingpos[BLACK]/8, i = board->kingpos[WHITE]%8, i1 = board->kingpos[BLACK]%8;
    for (int a = n-1; a < n+2 && a < 8; a++){
        if (a < 0){
            continue;
        }
        if (wbackwards[a] <= 2 && wbackwards[a] >= i){ //if there was a king on g1, pawns on g2 and g3 would be counted
            pawnsw++;
        }
    }
    for (int a = n1-1; a < n1+2 && a < 8; a++){
        if (a < 0){
            continue;
        }
        if (bbackwards[a] >= 5 && bbackwards[a] <= i1){ //make sure though that we're not giving a bonus to a king on b6 and a pawn on b7!!!!
            pawnsb++;
        }
    }    
    int wval, bval;
    if (attacksw < 2){ //if there are no attackers, there is nothing to evaluate.                
        wval = 0;
    }
    else if (board->pnbrqcount[WHITE][4] == 0){
        wval = kingdangertable[king_attack_count[WHITE]]/2; //if there is no queen, we halve the king danger score. we do not scale based on pawns,
                                                            //because with little material kings can escape in the open, and in fact can get trapped
                                                            //into mates or tactics by their own pawns.
    }
    else{
        wval = kingdangertable[king_attack_count[WHITE]]*(1.4-(0.2*pawnsb)); //if there are queens, definitely scale by pawns. even a bare queen can
                                                                            //force a draw/pick up a bunch of pawns if the enemy king is wide open.
                                                                            //even if there is no queen attacking the king, it can quite easily manuever its
                                                                            //way over.
    }
    if (attacksb < 2){
        bval = 0;
    }
    else if (board->pnbrqcount[BLACK][4] == 0){
        bval = kingdangertable[king_attack_count[BLACK]]/2;
    }
    else{
        bval = kingdangertable[king_attack_count[BLACK]]*(1.4-(0.2*pawnsw));
    }
    return (wval-bval)*2/3; //this weight needs to be calibrated
}
int eval(struct board_info *board, char color){   
    evals++;
    if (evals%10000 == 0){
        clock_t rightnow = clock() - start_time;
        if ((float)rightnow/CLOCKS_PER_SEC > maximumtime || (float)rightnow/CLOCKS_PER_SEC > coldturkey-0.1){ //you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
        if (InputPending()){
            int a = com_uci(board, (struct movelist *) 0, (int *) 0, WHITE);
            if (a == 1){
                return TIMEOUT;
            }
        }

    }
    attacksw = 0, attacksb = 0;
    king_attack_count[WHITE] = 0, king_attack_count[BLACK] = 0;
    int phase = 0; //0 represents full phase, 40 represents complete middlegame
    short int blockedpawns = 0;
    int spacew = 0, spaceb = 0;
    int evl = material(board, &phase);

    if (phase == 0 && board->pnbrqcount[WHITE][0] + board->pnbrqcount[BLACK][0] == 1){  //for king and one pawn endgames
        if (board->pnbrqcount[WHITE][0] == 1){
            int n = board->kingpos[BLACK]/8, i = board->kingpos[BLACK]%8;
            if (n == 8 || n == 0){
                while (i > 1){
                    i--;
                    if (board->board[n][i] == WPAWN){
                        return LIKELYDRAW;
                    }
                }
            }
            else{
                if (i < 7 && (board->board[n][i-1] == WPAWN || board->board[n][i-2] == WPAWN)){
                    return LIKELYDRAW;
                }
            }
        }
        else{
            int n = board->kingpos[WHITE]/8, i = board->kingpos[WHITE]%8;
            if (n == 8 || n == 0){
                while (i < 6){
                    i++;
                    if (board->board[n][i] == BPAWN){
                        return LIKELYDRAW;
                    }
                }
            }   
            else{
                if (i > 1 && (board->board[n][i+1] == BPAWN || board->board[n][i+2] == BPAWN)){
                    return LIKELYDRAW;
                }
            }         
        }
    }

    int mat = evl;
    char wbackwards[8], bbackwards[8];
    evl += pstscore(board, phase, &blockedpawns, wbackwards, bbackwards, &spacew, &spaceb);
    if (phase > 16){evl += space(board, &blockedpawns, spacew, spaceb);}
    
    evl += kingsafety(board, wbackwards, bbackwards);
    king_attack_count[WHITE] = 0, king_attack_count[BLACK] = 0;
    attacksw = 0, attacksb = 0;

    if (board->pnbrqcount[WHITE][0] <= 1 && mat >= 0 && mat < 400 && phase != 0){ //if White is up material, we want to stop it from trading pawns
    if (board->pnbrqcount[WHITE][0] == 0){
        evl /= 4;
    }
    else evl /= 2;
    }
    if (board->pnbrqcount[BLACK][0] <= 1 && mat <= 0 && mat > -400 && phase != 0){
    if (board->pnbrqcount[BLACK][0] == 0){
        evl /= 4;
    }
    else evl /= 2;
    }

    if (color == WHITE){
        return evl+TEMPO;
    }
    else{
        return -evl + TEMPO;
    }
}
void movescore(struct board_info *board, struct list *list, int depth, char color, bool isque, char type, bool ispv){
    int evl;
    char mve[8];
    mve[0] = '\0';
    int i = 0; while (list[i].move[0] != '\0'){
        int evl;

         if ((type == '0' || type == '1') && !strcmp(TT[CURRENTPOS & _mask].bestmove, list[i].move)){            
            list[i].eval = 10000;
        }
        else if (ispv && (!strcmp(list[i].move, pvstack[depth]))){    
            list[i].eval = 9999;
        }
        else if (islower(list[i].move[0]) && list[i].move[5] == 'Q'){
            list[i].eval = 9000;
        }
        else if (strchr(list[i].move, 'x')){
            list[i].eval = see(board, list[i].move, color, isque);
        }
        else if (!strcmp(list[i].move, KILLERTABLE[depth][0])){
            list[i].eval = 199;
        }
        else if (!strcmp(list[i].move, KILLERTABLE[depth][1])){
            list[i].eval = 198;
        }
        
        else{
            int pos; if (islower(list[i].move[0])){
                pos = 0;
            }
            else{
                pos = 1;
            }

            if (list[i].move[0] == '0'){
                list[i].eval = 100;
            }
            else{

                list[i].eval = (int)round(pow(log(HISTORYTABLE[color][((list[i].move[pos]-97)*8) + atoi(&list[i].move[pos+1])-1]
                [((list[i].move[pos+3]-97)*8) + atoi(&list[i].move[pos+4])-1]), 1.9));
            }

        }

        i++;
    }
}
int quiesce(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depth, int maxdepth, char color, bool incheck){
    long long unsigned int original_pos = CURRENTPOS;
    int stand_pat = eval(board, color);
    if (depth > maxdepth || stand_pat == TIMEOUT){
        return stand_pat;
    }

    if (stand_pat >= beta && !incheck){
        return beta;
    }

    if (stand_pat > alpha && !incheck){
        alpha = stand_pat;
    }
    if (stand_pat + 1125 < alpha){
        return stand_pat;
    }
    struct list list[LISTSIZE];
    list[0].move[0] = '\0';
    
    if (incheck){
        movelist(board, list, movelst, key, color);
        //remove_illegal(board, list, color);
    }
    else{
        movelistq(board, list, movelst, key, color);
        //remove_illegal(board, list, color);
    }


    movescore(board, list, 0, color, true, 'n', false);

    int i = 0;
    bool ismove = false;
    while (list[i].move[0] != '\0'){
        selectionsort(list, i);
        if ((!incheck) && list[i].eval < 200){
            CURRENTPOS = original_pos;
            return alpha;
        }
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
        bool ischeck = check_check(board, color^1);
        if (stand_pat + 150 < alpha && (!ischeck || depth > 2)){
            int victim = 0; if (!strchr(list[i].move, 'x')){
                victim = 0;
            }
            else{
                if (list[i].move[5] == 'e'){
                    victim = VALUES[0];
                }
                else if (islower(list[i].move[0])){
                    victim = VALUES[(board->board[(int)list[i].move[3]-97] [atoi(&list[i].move[4])-1]-(color^1))/2];
                }
                else{
                    victim = VALUES[(board->board[(int)list[i].move[4]-97][atoi(&list[i].move[5])-1]-(color^1))/2];
                }
            }
            if (stand_pat + victim + 150 < alpha){
                movelst[*key-1].move[0] = '\0';
                *key = *key-1;
                CURRENTPOS = original_pos;
                i++;
                continue;
            }
        }
        list[i].eval = -quiesce(&board2, movelst, key, -beta, -alpha, depth+1, maxdepth, color^1, ischeck);
        if (abs(list[i].eval) == TIMEOUT){
            movelst[*key-1].move[0] = '\0';
            *key = *key-1;
            CURRENTPOS = original_pos;
            return TIMEOUT;
        }
        if (list[i].eval >= beta){
            movelst[*key-1].move[0] = '\0';
            *key = *key-1;
            CURRENTPOS = original_pos;
            return beta;
        }
        if (list[i].eval > alpha){
            alpha = list[i].eval;
        }
        movelst[*key-1].move[0] = '\0';
        *key = *key-1;
        CURRENTPOS = original_pos;
        i++;
    }
    if (incheck && !ismove){
        return -100000;
    }
    return alpha;
}
int alphabeta(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depthleft, int depth, char color, bool isnull, bool ispv, bool incheck){
    //printf("%i\n", depthleft);
    if (checkdraw1(board) || checkdraw2(movelst, key) == 2){     
        return 0;
    }
    if (depth > 0 && checkdraw2(movelst, key) > 0){
        return 0;
    }

    int evl;

    char type; if (CURRENTPOS == TT[CURRENTPOS & _mask].zobrist_key){
        type = TT[CURRENTPOS & _mask].type;
            evl = TT[CURRENTPOS & _mask].eval;
    }
    else{
        type = 'n';
        evl = -1024;
    }

    if (!ispv && type != 'n' && TT[CURRENTPOS & _mask].depth >= depthleft){
            if (type == '0'){
                if (evl >= beta){
                    return beta;
                }
                if (evl < alpha){
                    return alpha;
                }
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
                    return alpha;
                }
            }
        }

    if (depthleft <= 0){
        int b = quiesce(board, movelst, key, alpha, beta, 0, 10, color, incheck);      
        if (b == -100000){
            b += depth;
        } 
        if (b == 100000){
            b -= depth;
        }
        return b;        
    } 
        evl = eval(board, color);


    if (isnull == false && !ispv){       
        bool ispiecew = false, ispieceb = false;
        for (int i = 1; i < 5; i++){
            if (board->pnbrqcount[WHITE][i] > 0){
                ispiecew = true;
            }
            if (board->pnbrqcount[BLACK][i] > 0){
                ispieceb = true;
            }
        }
        if (ispiecew && ispieceb){
            if (!incheck && evl >= beta){
                unsigned long long int a = CURRENTPOS;
                CURRENTPOS ^= ZOBRISTTABLE[772];
                int R = 3 + (depthleft/8);
                int nullmove = -alphabeta(board, movelst, key, -beta, -beta+1, depthleft-1-R, depth+2, color^1, true, false, false);
                CURRENTPOS = a;
                if (abs(nullmove) == TIMEOUT){
                    return TIMEOUT;
                }
                
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
    //remove_illegal(board, list, color);
    movescore(board, list, depth, color, false, type, ispv);
    int i = 0;
    unsigned long long int original_pos = CURRENTPOS;
    bool raisedalpha = false;
    if (depth == 0){
        currentmove[0] = '\0';
    }
    int pvstart = pvptr;
    pvstack[pvptr++][0] = '\0';
    char bestmove[8];
    bool _fprune = false;
    if (depthleft < 5 && !incheck && !ispv && alpha > -1000000 && evl+(futility[depthleft-1]) < alpha && (depthleft < 4 || type == 'n')){
        _fprune = true;
    }
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
        bool ischeck = check_check(&board2, color^1);
        if (_fprune && !ischeck){
            int victim = 0; if (!strchr(list[i].move, 'x')){
                if (islower(list[i].move[0]) && list[i].move[5] == 'Q'){
                    victim = VALUES[4];
                }
                else{
                    victim = 0;
                }
            }
            else{
                if (list[i].move[5] == 'e'){
                    victim = VALUES[0];
                }
                else if (islower(list[i].move[0])){
                    victim = VALUES[(board->board[(int)list[i].move[3]-97] [atoi(&list[i].move[4])-1]-(color^1))/2];
                }
                else{
                    victim = VALUES[(board->board[(int)list[i].move[4]-97][atoi(&list[i].move[5])-1]-(color^1))/2];
                }
            }

            if ((evl + victim + futility[depthleft-1]) + 50 < alpha){
                firstmove = false;
                betacount++;
                CURRENTPOS = original_pos;
                i++;
                continue;
            }
        }
        move_add(&board2, movelst, key, list[i].move, color);

        if (ispv == true && firstmove){

                list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depthleft-1, depth+1, color^1, false, ispv, ischeck);
                if (abs(list[i].eval) == TIMEOUT){
                                movelst[*key-1].move[0] = '\0';
                *key = *key-1;
                CURRENTPOS = original_pos;
        
                return TIMEOUT;
                }
            }

        else{
            int R = 0;
            if (strchr(list[i].move, 'x') || depthleft < 3){
                R = 0;
            }
            else if (ischeck || incheck){
                int endgame = 0;
                int a = material(board, &endgame);
                if (endgame > 8){
                    R = 0;
                }
                else{
                    R = (int)round(log(depthleft)*log(betacount+1)/3);
                }
            }
            else{
                R = (int)round(log(depthleft)*log(betacount+1)/1.95);
            }
            list[i].eval = -alphabeta(&board2, movelst, key, -alpha-1, -alpha, depthleft-1-R, depth+1, color^1, false, false, ischeck);
                if (abs(list[i].eval) == TIMEOUT){
                                movelst[*key-1].move[0] = '\0';
            *key = *key-1;
            CURRENTPOS = original_pos;
    
            return TIMEOUT;
                }

            if (list[i].eval > alpha && R){
                list[i].eval = -alphabeta(&board2, movelst, key, -alpha-1, -alpha, depthleft-1, depth+1, color^1, false, ispv, ischeck);
                if (abs(list[i].eval) == TIMEOUT){
                                movelst[*key-1].move[0] = '\0';
            *key = *key-1;
            CURRENTPOS = original_pos;
    
            return TIMEOUT;
                }
            }

            if (list[i].eval > alpha && ispv){

                    list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depthleft-1, depth+1, color^1, false, ispv, ischeck);
                        if (abs(list[i].eval) == TIMEOUT){
                                movelst[*key-1].move[0] = '\0';
            *key = *key-1;
            CURRENTPOS = original_pos;
    
            return TIMEOUT;
                }
                }
        }

        if (list[i].eval >= beta){
            if (depth == 0){
                memcpy(currentmove, list[i].move, 8);
            }
            memcpy(bestmove, list[i].move, 8);
            //if (beta != 0){
                insert(original_pos, depthleft, beta, '1', bestmove);
            //}
            total++;
            if (betacount < 5){
                betas++;
            }
            else{
                //printf("%s %i\n", list[i].move, betacount);
            }

            if (!strchr(list[i].move, 'x') && depthleft > 1){
                if (strcmp(KILLERTABLE[depth][0], list[i].move)){
                    memcpy(KILLERTABLE[depth][0], list[i].move, 8);
                }
                else if (strcmp(KILLERTABLE[depth][1], list[i].move)){
                    memcpy(KILLERTABLE[depth][1], list[i].move, 8);
                }
                if (list[i].move[0] != '0'){
                    int pos; if (islower(list[i].move[0])){
                        pos = 0;
                    }
                    else{
                        pos = 1;
                    }
                    int c = depthleft*depthleft; if (c > 400){c = 400;}
                    HISTORYTABLE[color][((list[i].move[pos]-97)*8) + atoi(&list[i].move[pos+1])-1]
                    [((list[i].move[pos+3]-97)*8) + atoi(&list[i].move[pos+4])-1] += c;

                    if (HISTORYTABLE[color][((list[i].move[pos]-97)*8) + atoi(&list[i].move[pos+1])-1]
                    [((list[i].move[pos+3]-97)*8) + atoi(&list[i].move[pos+4])-1] > 1000000){
                        for (int a = 0; a < 64; a++){
                            for (int b = 0; b < 64; b++){
                                HISTORYTABLE[color][a][b] /= 2;
                            }
                        }
                    }
                }
            }
            movelst[*key-1].move[0] = '\0';
            *key = *key-1;
            CURRENTPOS = original_pos;
            pvptr = pvstart;
            return beta;
        }
        betacount++;
        movelst[*key-1].move[0] = '\0';
        *key = *key-1;
        if (list[i].eval > alpha){
            if (depth == 0){
                memcpy(currentmove, list[i].move, 8);
            }
            raisedalpha = true;
            alpha = list[i].eval;
            memcpy(bestmove, list[i].move, 8);            
            
            int p = pvptr;
            //printf("%i\n", p);
            pvptr = pvstart;
            uci_move(list[i].move, pvstack[pvptr], color);
            pvptr++;        
            while (p < 4999 && pvstack[p][0] != '\0'){ memcpy(pvstack[pvptr], pvstack[p], 8); p++; pvptr++;}
            pvstack[pvptr][0] = '\0';
             // the move we just searched is now the first of the new PV
        }

        else if (firstmove && depth == 0){
            CURRENTPOS = original_pos;
            pvptr = pvstart;
            return alpha;
        }
        firstmove = false;
        CURRENTPOS = original_pos;
        i++;
    }

    pvptr = pvstart;
    if (!ismove){
        if (check_check(board, color)){
            return -100000 + depth;
        }
        else{
            return 0;
        }
    }
    //printfull(board);
    if (raisedalpha){
        //if (alpha != 0){
        insert(original_pos, depthleft, alpha, '0', bestmove);
        //}
    }
    else{
        char j[8];
        j[0] = '\0';
        //if (alpha != 0){
        insert(original_pos, depthleft, alpha, '2', j);
        //}
    }
    return alpha;
}


float iid_time(struct board_info *board, struct movelist *movelst, float maxtime, int *key, char color, bool ismove){
    /*if (*key < 5 && opening_book(board, movelst, key, color)){
        return 1;
    }*/
    maximumtime = maxtime*3;
    clearTT(false);
    clearPV();
    //if (*key % 5 == 0){
        clearHistory();
    //}
    clearKiller();
    currentmove[0] = '\0';
    int alpha = -1000000, beta = 1000000;   
    long int totalevls = 0;
    start_time = clock();
    bool incheck = check_check(board, color);
    int g;
    int depth;
    char pvmove[8];
    //printf("%i %s %s\n", *key, movelst[*key-1].fen, movelst[*key-1].move);
    for (depth = 1; ; depth++){        
        int aspiration = 25;       
        int evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, true, incheck);  

        while (evl == alpha || evl == beta){

            if (evl == alpha){
                char temp[8];
                printf("info depth %i score cp %i nodes %li time %i pv %s\n ", depth, alpha, evals, (int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC, uci_move(pvmove, temp, color));
                alpha -= aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, true, incheck);
                        if (abs(evl) == TIMEOUT){
            if (currentmove[0] == '\0'){
                memcpy(currentmove, pvmove, 8);
                depth--;
            }
            break;
        }   
                
            }
            else if (evl == beta){
                char temp[8];
                printf("info depth %i score cp %i nodes %li time %i pv %s\n ", depth, beta, evals, (int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC, uci_move(currentmove, temp, color));
                if ((float)(clock()-start_time)/CLOCKS_PER_SEC > maxtime){
                    g = beta+1; break;
                }

                beta += aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, true, incheck);   
                        if (abs(evl) == TIMEOUT){
                if (currentmove[0] == '\0'){
                    memcpy(currentmove, pvmove, 8);
                    depth--;
                }
                break;
            }         


            }
        }
        if (abs(evl) == TIMEOUT){
                if (currentmove[0] == '\0'){
                    memcpy(currentmove, pvmove, 8);
                    depth--;
                }
                break;
                }

        clock_t time2 = clock()-start_time;
        g = evl;  
        char temp;
        memcpy(pvmove, currentmove, 8);
        printf("info depth %i score cp %i nodes %li time %i pv ", depth, g, evals, (int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC);

        for (int i = 0; i < depth && pvstack[i][0] != '\0'; i++){
            char temp[8];
            printf("%s ", pvstack[i]);
        }
        printf("\n");
        fflush(stdin);
        if ((float)time2/CLOCKS_PER_SEC > maxtime*0.7 || depth > 45){                      
            break;
        }
         if (depth > 5){
            alpha = evl-aspiration;
            beta = evl+aspiration;
        }   
    
    }
    char temp[8];
    printf("bestmove %s ponder %s\n", uci_move(currentmove, temp, color), pvstack[1]);
        
       

    if (ismove){
        move(board, currentmove, color);
        move_add(board, movelst, key, currentmove, color);

    } 
    evals = 0;
    return g;
}

void iid(struct board_info *board, struct movelist *movelst, int maxdepth, int *key, char color, bool ismove){
    clearTT(true);
    clearPV();
    clearHistory();
    clearKiller();
    evals = 0;
    maximumtime = 100000;
    long int alpha = -1000000, beta = 1000000;   
    char mve[8];
    start_time = clock();
    bool incheck = check_check(board, color);
    for (int depth = 1; depth <= maxdepth; depth++){
        int aspiration = 25;
        int evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, true, incheck);
        while (evl == alpha || evl == beta){
            if (evl == alpha){
                alpha -= aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, true, incheck);
            }
            else if (evl == beta){
                beta += aspiration;
                aspiration *= 2;
                evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, true, incheck);            
            }
        }
        clock_t time2 = clock()-start_time;
            printf("depth %i: %f %li %f secs\n", depth, (float)evl/100, evals,(float)time2/CLOCKS_PER_SEC);
            for (int i = 0; i < depth; i++){
                printf("%s ", pvstack[i]);
            }
            printf("\n");

        if (depth > 5){
            alpha = evl-aspiration;
            beta = evl+aspiration;
        }
    }
    if (ismove){
        move(board, pvstack[0], color);
        move_add(board, movelst, key, pvstack[0], color);
    }
    totals += evals;
}


void setfromfen(struct board_info *board, struct movelist *movelst, int *key, char *fenstring, char *color){
    int i = 7, n = 0;
    int fenkey = 0;
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
                board->board[n][i] = BLANK;
                n++;
            }
        }
        else{
                switch(fenstring[fenkey]){
                    case 'P':
                    board->board[n][i] = WPAWN; board->pnbrqcount[WHITE][0]++; break;
                    case 'N':
                    board->board[n][i] = WKNIGHT; board->pnbrqcount[WHITE][1]++; break;
                    case 'B':
                    board->board[n][i] = WBISHOP; board->pnbrqcount[WHITE][2]++; break;
                    case 'R':
                    board->board[n][i] = WROOK; board->pnbrqcount[WHITE][3]++; break;
                    case 'Q':
                    board->board[n][i] = WQUEEN; board->pnbrqcount[WHITE][4]++; break;
                    case 'K':
                    board->board[n][i] = WKING; board->kingpos[WHITE] = (n*8)+i; break;
                    case 'p':
                    board->board[n][i] = BPAWN; board->pnbrqcount[BLACK][0]++; break;
                    case 'n':
                    board->board[n][i] = BKNIGHT; board->pnbrqcount[BLACK][1]++; break;
                    case 'b':
                    board->board[n][i] = BBISHOP; board->pnbrqcount[BLACK][2]++; break;
                    case 'r':
                    board->board[n][i] = BROOK; board->pnbrqcount[BLACK][3]++; break;
                    case 'q':
                    board->board[n][i] = BQUEEN; board->pnbrqcount[BLACK][4]++; break;
                    default:
                    board->board[n][i] = BKING; board->kingpos[BLACK] = (n*8)+i; break;
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
    calc_pos(board);
    setmovelist(movelst, key);
    while (isblank(fenstring[fenkey])){
        fenkey++;
    }
    if (fenstring[fenkey] == '-'){
        board->castling[WHITE][0] = false, board->castling[WHITE][1] = false;
        fenkey++;
    }
    else if (isupper(fenstring[fenkey])){
        if (fenstring[fenkey] == 'K'){
            board->castling[WHITE][1] = true;
            fenkey++;
            if (fenstring[fenkey] == 'Q'){
                board->castling[WHITE][0] = true;
            }
        }
        else{
            board->castling[WHITE][0] = true;
        }
        fenkey++;
    }

    if (fenstring[fenkey] == '-'){
        board->castling[BLACK][0] = false, board->castling[WHITE][1] = false;
        fenkey++;
    }
    else{
        if (fenstring[fenkey] == 'k'){
            board->castling[BLACK][1] = true;
            fenkey++;
            if (fenstring[fenkey] == 'Q'){
                board->castling[BLACK][0] = true;
            }
        }
        else{
            board->castling[BLACK][0] = true;
        }
        fenkey++;
    }
    while (isblank(fenstring[fenkey])){
        fenkey++;
    }
    if (fenstring[fenkey] == '-'){
        board->epsquare = -1;
    }
    else{
        if (color == WHITE){
        board->epsquare = ((fenstring[fenkey]-97)*8) + atoi(&fenstring[fenkey+1])-2;
        }
        else{
        board->epsquare = ((fenstring[fenkey]-97)*8) + atoi(&fenstring[fenkey+1]);
        }
    }
    while (isblank(fenstring[fenkey])){
        fenkey++;
    }    
    movelst[*key].halfmoves = atoi(&fenstring[fenkey]);
    if (isdigit(fenstring[fenkey+1])){
        movelst[*key].halfmoves *= 10;
        movelst[*key].halfmoves += atoi(&fenstring[fenkey+1]);
    }

}

char setfrompgn(struct board_info *board, struct movelist *movelst, int *key, char *pgnline){ //takes a position from starting position and makes all the moves on it
 
    char color = WHITE;
    int pgnkey = 0; while (pgnline[pgnkey] != '\0'){
        char mve[8];
        while (!isalpha(pgnline[pgnkey])){  //moves it forward to the next word
            pgnkey++;
        }
        int i = 0;
        while (!isblank(pgnline[pgnkey])){
            mve[i] = pgnline[pgnkey];
            i++;
            pgnkey++;
        }
        mve[i] = '\0';
        char temp[8];
        move(board, move_to_uci(mve, temp, color, board), color);
        move_add(board, movelst, key, temp, color);
        pgnkey++;
        color ^= 1;
    }
    return color;
}

int com_uci( struct board_info *board, struct movelist *movelst, int *key, char *color) { //assumes board+movelist have been already set up under wraps

    char command[65536];
    getsafe(command, sizeof(command));
    //printf("%s\n", command);
    if (command[0] == '\0'){
        return 0;
    }
    if (!strcmp(command, "stop")){
        return 1;
    }
  if (!strcmp(command, "uci")) {
    printf("id name Willow 2.5\n");
    printf("id author Adam Kulju\n");
    // send options

    printf("uciok\n");
  }
    if (!strcmp(command, "isready")){
        printf("readyok\n");
    }
    if (!strcmp(command, "ucinewgame")) {
        clearTT(true);
        clearKiller();
        clearHistory();
        for (int i = 0; i < MOVESIZE; i++){
            movelst[i].halfmoves = 0;
            movelst[i].fen = 0;
            movelst[i].move[0] = '\0';
        }
        *key = 1;
        }

    if (!strcmp(command, "quit")){
        exit(0);
    }
    if (strstr(command, "position startpos moves")){
        *color = WHITE;
        setfull(board);
        setmovelist(movelst, key); 
        calc_pos(board);
        *key = 1;
        int i = 24;
        while (command[i] != '\0' && command[i] != '\n'){
            while (isblank(command[i]) && command[i] != '\0'){
                i++;
            }
            char buf[8];
            int a = 0;
            while (isalnum(command[i])){
                buf[a] = command[i];
                i++;
                a++;
            }
            buf[a] = '\0';
            char temp[8]; move_to_uci(buf, temp, *color, board);
            move(board, temp, *color);
            move_add(board, movelst, key, temp, *color);

            *color ^= 1;            
        }
        //printfull(board, *color);
    }
    if (strstr(command, "go")){
        float time;
        if (strstr(command, "infinite")){
            time = 1000000;
            coldturkey = 1000000;
        }
        else if (strstr(command, "movetime")){
            time = atoi(command+12)/1000;
            coldturkey = time+100;
        }

        else if (strstr(command, "wtime")){ //this will be for a game
            int k = 9;
            
            if (*color == BLACK){
                while (!isblank(command[k])){
                    k++;
                }
                while (isblank(command[k])){
                    k++;
                }
                while (!isblank(command[k])){
                    k++;
                }
                while (isblank(command[k])){
                    k++;
                }              //we need to skip past the "1000 btime part"
            }
            int milltime = atoi(&command[k]);
            coldturkey = (float)milltime/1000;
            int movesleft = MAX(20, 60-(*key/2));
            time = ((float)milltime/(1000*movesleft)) * 1.5;
            if (strstr(command, "winc")){
                while (command[k] != 'w'){
                    k++;
                } //brings it to the "winc" part

                while (!isblank(command[k])){
                    k++;
                }
                while (isblank(command[k])){
                    k++;
                }               //skips past the "winc" part to the numbers

                if (*color == BLACK){
                    while (!isblank(command[k])){
                        k++;
                    }
                    while (isblank(command[k])){
                        k++;
                    }
                    while (!isblank(command[k])){
                        k++;
                    }
                    while (isblank(command[k])){
                        k++;
                    }           //if we're playing as black, we need to skip again to past "binc". it's usually the same, but eventually it might not be so always good to plan ahead.
                }
                milltime = atoi(&command[k]);
                time += (float)(milltime/1000) * 0.8;
            }
        }
        iid_time(board, movelst, time, key, *color, false);
        printf("%f %f\n", time, coldturkey);
    }
    //fflush(hstdin);
    return 0;
}

int com() {
    unsigned long long init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    init_by_array64(init, 4);
    setzobrist();    
    struct board_info board;
    setfull(&board);
    struct movelist movelst[MOVESIZE];
    int key;
    calc_pos(&board);
    setmovelist(movelst, &key); 
    char color;
    for (;;){
            com_uci(&board, movelst, &key, &color);
        }
    return 0;
}


void game(int time){
    unsigned long long init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    clearTT(true);
    init_by_array64(init, 4);
    struct board_info board;
    struct movelist movelst[MOVESIZE];
    int key;
    setfull(&board);    
    //setempty(&board);
    setzobrist();
    calc_pos(&board);    
    clearHistory();
    char fen[65] = "RP----prNP----pnBP----pbQP----pqKP----pkBP----pbNP----pnRP----pr\0";
    //char fen[65] =   "-----------------------------Q--K------k------------------------\0";
    setmovelist(movelst, &key);   

    char color, temp;
    int game_end_flag = 0;
    printfull(&board, WHITE);
    printf("Welcome to WILLOW!\n");
    printf("What color would you like to play as? (W/B): ");
    if(scanf("%c", &temp)){;}
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
        printfull(&board, color);
        printf("COMPUTER MOVING: \n");
        iid_time(&board, movelst, time, &key, color^1, true);
        //move(&board, TT[CURRENTPOS & _mask].bestmove, color);
        //printf("%llu %llu %s\n", TT[CURRENTPOS & _mask].zobrist_key, CURRENTPOS, TT[CURRENTPOS & _mask].bestmove);

    }
    if (game_end_flag == 11){
        printf("Draw!\n");
    }
    else if (game_end_flag == 1000){
        printf("You won! Congratulations!\n");
    }
    else{
        printf("You lost. Better luck next time!\n");
    }
}



int main(void){
    init();
    com();
    return 0;
} 
