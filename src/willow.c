#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>     
#include <string.h>
#include <time.h>
#include <math.h>

#define WHITE 0
#define BLACK 1
#define BLANK 0
#define WPAWN 2
#define BPAWN 3
#define WKNIGHT 4
#define BKNIGHT 5
#define WBISHOP 6
#define BBISHOP 7
#define WROOK 8
#define BROOK 9
#define WQUEEN 10
#define BQUEEN 11
#define WKING 12
#define BKING 13
//all we have to do is (board[i] & 1) to see if the color is white or black - if that is 1 then it's black if not then it's white

#define NORTH 16
#define SOUTH -16
#define EAST 1
#define WEST -1
#define NE 17
#define SE -15
#define NW 15
#define SW -17
#define SSW -33
#define SSE -31
#define WSW -18
#define ESE -14
#define WNW 14
#define ENE 18
#define NNW 31
#define NNE 33
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6


#define LISTSIZE 150
#define MOVESIZE 1000
#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL /* Least significant 31 bits */
//#define TTSIZE 1 << 20
//#define _mask (1 << 20) - 1
#define CHECKTIME (1 << 10)-1
#define TIMEOUT 111111
#define TEMPO 5
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAXPHASE 24
#define LIKELYDRAW -111110
clock_t start_time;
float maximumtime;
float coldturkey = 10000000;
bool isattacker;
char attacksw;
char attacksb;

int maxdepth;

bool slide[5] = { 0, 1, 1, 1, 0 };
char vectors[5] = { 8, 4, 4, 8, 8 };
char vector[5][8] = {
    { SSW, SSE, WSW, ESE, WNW, ENE, NNW, NNE },
    { SW, SE, NW, NE},
    {SOUTH, WEST, EAST, NORTH},
  {SW, SOUTH, SE, WEST, NW, EAST, NE, NORTH},
  {SW, SOUTH, SE, WEST, NW, EAST, NE, NORTH}
};



char KINGZONES[2][0x80][0x80];


/* The array for the state vector */
static unsigned long long mt[NN]; 
/* mti==NN+1 means mt[NN] is not initialized */
static int mti=NN+1; 
char passed[2][8];
unsigned long long int ZOBRISTTABLE[774];
//to access zobrist number for say white knight on a6 we go zobristtable[(64*piece)+(file-1*8) + rank-1]
//768-771 is castling
//color key is 772
//ep possible is 773
unsigned long long int CURRENTPOS;
int attackers[2];
struct move{
    unsigned short int move;
    //use >>8 to get SQUAREFROM and &FF to get SQUARETO
    unsigned char flags;
    //in form 0000 xx yy
    //xx = flags - 00 normal 01 promotion 10 castling 11 ep - if = 1 then yy flags - 00 knight 01 bishop 10 rook 11 queen
    //get xxflags with >>2, and yyflags with &11
};

struct board_info{
    unsigned char board[0x80];
    unsigned char pnbrqcount[2][5];
    bool castling[2][2];
    unsigned char kingpos[2];
    unsigned char epsquare;
};

char current_mobility;

struct movelist{
    struct move move;
    //char fen[65];
    long long unsigned int fen;
    char halfmoves;
    int staticeval;
};
struct list{
    struct move move;
    int eval;
};

struct ttentry{
    unsigned long long int zobrist_key;
    char type;
    //come in three types:
    //2: EXACT, 2: FAIL-HIGH, 1: FAIL-LOW
    struct move bestmove;
    int eval;
    char depth;
    short int age;
};

struct move currentmove;
struct move nullmove = {0,0};
struct ttentry nullTT = {0,0,{0,0},0,0,0};

//struct ttentry TT[TTSIZE];
struct ttentry *TT;
long int TTSIZE;
long int _mask;

short int search_age;

const int boardsize = sizeof(struct board_info);
const int listsize = sizeof(struct list);
const int movesize = sizeof(struct move);

short int MAXDEPTH;

int LMRTABLE[100][LISTSIZE];

bool CENTERWHITE[0x88];
bool CENTERBLACK[0x88];

struct move KILLERTABLE[100][2];
struct move COUNTERMOVES[6][128];
long int HISTORYTABLE[2][0x80][0x80]; //allows for faster lookups

unsigned long int nodes;
long int totals;
int betas, total;
 const int VALUES[5] = {75,  299,  297,  409,  819};
 const int VALUES2[5] = {82,  338,  349,  584, 1171};
short int pstbonusesm[6][0x80] = {
    {
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
 -25,  -13,  -11,   -6,    1,   18,   14,  -20,    0,    0,    0,    0,    0,    0,    0,    0, 
 -28,  -13,   -7,   -3,   10,    3,    7,  -13,    0,    0,    0,    0,    0,    0,    0,    0, 
 -28,  -15,   -8,    4,    4,    3,  -15,  -20,    0,    0,    0,    0,    0,    0,    0,    0, 
 -23,   -8,   -7,   -9,    9,   11,    1,  -14,    0,    0,    0,    0,    0,    0,    0,    0, 
  -2,   10,   25,   29,   60,   87,   46,    7,    0,    0,    0,    0,    0,    0,    0,    0, 
  67,  112,   46,   74,   55,  142,   32,  -29,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    },
    {
 -99,  -11,  -36,  -14,    3,   -5,   -4,   -7,    0,    0,    0,    0,    0,    0,    0,    0, 
 -16,  -42,   -9,   10,   10,   19,   -3,    5,    0,    0,    0,    0,    0,    0,    0,    0, 
 -21,   -8,    5,    8,   24,   15,   18,   -3,    0,    0,    0,    0,    0,    0,    0,    0, 
  -4,    4,   13,   20,   26,   23,   22,    5,    0,    0,    0,    0,    0,    0,    0,    0, 
 -12,   11,   10,   46,   26,   50,   17,   23,    0,    0,    0,    0,    0,    0,    0,    0, 
 -45,   37,   14,   41,   59,   93,   60,   45,    0,    0,    0,    0,    0,    0,    0,    0, 
 -80,  -63,   52,   -2,   -6,   36,  -18,  -15,    0,    0,    0,    0,    0,    0,    0,    0, 
-167, -101,  -93,  -47,   -5,  -96,  -37, -105,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
 -15,   27,   10,    8,   14,   12,  -13,   -1,    0,    0,    0,    0,    0,    0,    0,    0, 
  22,   22,   22,   10,   24,   30,   44,   18,    0,    0,    0,    0,    0,    0,    0,    0, 
   8,   23,   20,   21,   21,   28,   22,   19,    0,    0,    0,    0,    0,    0,    0,    0, 
   4,   14,   15,   29,   37,    9,   11,   14,    0,    0,    0,    0,    0,    0,    0,    0, 
  -3,   19,   12,   44,   26,   27,   14,    1,    0,    0,    0,    0,    0,    0,    0,    0, 
 -10,   23,   28,   29,   22,   49,   37,   18,    0,    0,    0,    0,    0,    0,    0,    0, 
 -35,   -1,  -24,  -43,   13,   18,   -6,  -42,    0,    0,    0,    0,    0,    0,    0,    0, 
 -17,  -29, -121, -113,  -51,  -70,  -23,  -12,    0,    0,    0,    0,    0,    0,    0,    0
    },

    {
   0,    5,   17,   25,   30,   23,   19,    3,    0,    0,    0,    0,    0,    0,    0,    0, 
 -24,   -7,   -1,    9,   19,   19,   17,  -34,    0,    0,    0,    0,    0,    0,    0,    0, 
 -25,  -13,    0,   -4,   13,   13,   18,   -9,    0,    0,    0,    0,    0,    0,    0,    0, 
 -31,  -26,   -5,    0,   11,   -2,   15,  -20,    0,    0,    0,    0,    0,    0,    0,    0, 
 -26,  -11,    6,   21,   19,   20,    1,  -13,    0,    0,    0,    0,    0,    0,    0,    0, 
 -22,    3,   14,   20,   -2,   34,   58,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   4,   10,   39,   42,   58,   62,   13,   27,    0,    0,    0,    0,    0,    0,    0,    0, 
   5,   28,  -21,   34,   35,  -25,  -15,   -3,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
   8,   10,   13,   23,   16,    1,    1,  -41,    0,    0,    0,    0,    0,    0,    0,    0, 
 -13,    7,   17,   24,   24,   30,   23,   23,    0,    0,    0,    0,    0,    0,    0,    0, 
 -15,   13,    3,    6,    7,   12,   17,    4,    0,    0,    0,    0,    0,    0,    0,    0, 
   4,  -25,    0,   -4,    4,    0,    6,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
 -34,  -14,  -16,  -27,  -25,   -7,  -12,  -10,    0,    0,    0,    0,    0,    0,    0,    0, 
  -6,  -19,   -2,  -23,   -8,   30,    6,   20,    0,    0,    0,    0,    0,    0,    0,    0, 
 -20,  -44,  -24,  -14,  -47,   18,  -15,   40,    0,    0,    0,    0,    0,    0,    0,    0, 
 -29,  -44,   -7,  -52,   48,   33,   16,   34,    0,    0,    0,    0,    0,    0,    0,    0, 

    },
    {
 -12,   53,   28,  -64,   -7,  -33,   31,   22,    0,    0,    0,    0,    0,    0,    0,    0, 
  10,   18,  -21,  -60,  -45,  -26,   11,   11,    0,    0,    0,    0,    0,    0,    0,    0, 
 -38,   11,  -28,  -49,  -59,  -44,  -14,  -43,    0,    0,    0,    0,    0,    0,    0,    0, 
 -68,   12,  -21,  -88,  -90,  -57,  -75, -122,    0,    0,    0,    0,    0,    0,    0,    0, 
 -28,   -7,    8,   -8,  -26,  -25,  -18, -122,    0,    0,    0,    0,    0,    0,    0,    0, 
 -15,   36,   60,    8,   33,   67,   97,  -50,    0,    0,    0,    0,    0,    0,    0,    0, 
  31,   24,    4,   33,   -6,   -6,  -46,  -58,    0,    0,    0,    0,    0,    0,    0,    0, 
 -72,   33,   23,  -11,  -73,  -53,   10,    9,    0,    0,    0,    0,    0,    0,    0,    0
    }

};


short int pstbonusese[6][0x80] = {
    {
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
  34,   31,   26,   24,   35,   25,   11,   17,    0,    0,    0,    0,    0,    0,    0,    0, 
  33,   30,   21,   23,   27,   20,   15,   18,    0,    0,    0,    0,    0,    0,    0,    0, 
  37,   36,   22,   17,   16,   18,   21,   19,    0,    0,    0,    0,    0,    0,    0,    0, 
  54,   46,   31,   13,   17,   17,   31,   34,    0,    0,    0,    0,    0,    0,    0,    0, 
  72,   66,   37,    5,  -13,    4,   37,   50,    0,    0,    0,    0,    0,    0,    0,    0, 
 103,   77,   77,   42,   46,   25,   73,  108,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
 -12,  -32,  -16,   -6,  -19,  -15,  -34,  -41,    0,    0,    0,    0,    0,    0,    0,    0, 
 -40,  -13,  -16,  -18,  -15,  -28,  -20,  -43,    0,    0,    0,    0,    0,    0,    0,    0, 
 -25,  -12,  -13,    8,    4,  -16,  -28,  -19,    0,    0,    0,    0,    0,    0,    0,    0, 
  -9,  -10,   14,   19,   18,    9,    1,   -7,    0,    0,    0,    0,    0,    0,    0,    0, 
  -6,   -2,   25,   19,   21,   10,    5,  -11,    0,    0,    0,    0,    0,    0,    0,    0, 
 -19,  -22,   12,    7,   -8,  -14,  -26,  -44,    0,    0,    0,    0,    0,    0,    0,    0, 
  -9,    3,  -34,    1,   -6,  -31,  -20,  -49,    0,    0,    0,    0,    0,    0,    0,    0, 
 -32,  -31,   -1,  -31,  -13,  -28,  -40,  -81,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
  -9,   -7,   -3,   -7,   -7,   -1,   -2,  -10,    0,    0,    0,    0,    0,    0,    0,    0, 
 -11,  -19,  -13,   -7,   -7,  -12,  -16,  -25,    0,    0,    0,    0,    0,    0,    0,    0, 
  -9,   -2,    4,    3,   11,   -5,   -4,   -8,    0,    0,    0,    0,    0,    0,    0,    0, 
  -9,   -4,    6,   14,    1,    5,   -9,  -10,    0,    0,    0,    0,    0,    0,    0,    0, 
  -2,    0,    7,    8,   10,    6,   -4,    3,    0,    0,    0,    0,    0,    0,    0,    0, 
   7,   -8,   -5,   -7,   -4,   -3,   -4,    8,    0,    0,    0,    0,    0,    0,    0,    0, 
   6,   -6,    3,   -7,   -9,   -9,   -3,    3,    0,    0,    0,    0,    0,    0,    0,    0, 
 -13,  -12,    4,    6,   -2,   -2,   -7,  -21,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
  -9,   -9,   -7,  -11,  -17,  -14,   -7,  -23,    0,    0,    0,    0,    0,    0,    0,    0, 
  -5,   -7,   -2,   -5,  -19,  -21,  -18,   -2,    0,    0,    0,    0,    0,    0,    0,    0, 
   3,    2,   -5,    0,  -12,  -17,  -14,  -11,    0,    0,    0,    0,    0,    0,    0,    0, 
  11,   15,   10,    6,   -1,   -1,   -6,    1,    0,    0,    0,    0,    0,    0,    0,    0, 
  19,   14,   16,    3,    2,    4,    4,   12,    0,    0,    0,    0,    0,    0,    0,    0, 
  18,   16,   11,    7,   12,   -4,   -6,    3,    0,    0,    0,    0,    0,    0,    0,    0, 
  13,   15,   11,    7,   -8,   -3,   13,    8,    0,    0,    0,    0,    0,    0,    0,    0, 
  14,    6,   26,    6,    6,   20,   15,   13,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
 -27,  -38,  -15,  -23,   -4,  -38,  -21,  -27,    0,    0,    0,    0,    0,    0,    0,    0, 
  -9,  -17,  -28,  -26,  -21,  -31,  -55,  -52,    0,    0,    0,    0,    0,    0,    0,    0, 
  13,  -27,   31,   11,   22,   20,    5,    9,    0,    0,    0,    0,    0,    0,    0,    0, 
 -12,   52,   33,   62,   45,   23,   19,    7,    0,    0,    0,    0,    0,    0,    0,    0, 
  34,   36,   38,   74,   79,   35,   48,   21,    0,    0,    0,    0,    0,    0,    0,    0, 
  -9,   17,   18,   65,   66,   16,    6,  -12,    0,    0,    0,    0,    0,    0,    0,    0, 
   2,   42,   51,   63,   79,   27,   37,   -3,    0,    0,    0,    0,    0,    0,    0,    0, 
 -12,   44,   31,   51,   -5,    5,   -9,  -11,    0,    0,    0,    0,    0,    0,    0,    0, 
    },
    {
 -70,  -54,  -27,   -9,  -29,  -15,  -42,  -72,    0,    0,    0,    0,    0,    0,    0,    0, 
 -38,  -18,    7,   18,   15,    7,   -8,  -29,    0,    0,    0,    0,    0,    0,    0,    0, 
 -23,   -8,   15,   28,   33,   20,    3,  -12,    0,    0,    0,    0,    0,    0,    0,    0, 
 -22,   -7,   24,   46,   47,   34,   21,    3,    0,    0,    0,    0,    0,    0,    0,    0, 
 -16,   16,   29,   34,   38,   41,   31,   17,    0,    0,    0,    0,    0,    0,    0,    0, 
   2,   14,   14,   25,   23,   40,   35,   12,    0,    0,    0,    0,    0,    0,    0,    0, 
 -18,   14,   19,   13,   22,   41,   32,   20,    0,    0,    0,    0,    0,    0,    0,    0, 
 -76,  -36,  -35,  -17,    0,   18,    1,  -16,    0,    0,    0,    0,    0,    0,    0,    0
    }
};



short int bishop_pair[9] = {55,   70,   46,   47,   41,   35,   32,   35,   39};
short int passedmgbonus[8] = {4,   -4,  -10,   -8,   14,    0,   -6,    0};
short int passedegbonus[8] = {58,   10,   13,   42,   70,  153,  186,    0};

short int blockedmgbonus[8] = {4,  -16,  -16,  -18,   -4,   -5,    6,    0};
short int blockedegbonus[8] = {58,   11,   21,   32,   40,   64,   60,    0};


short int mobilitybonusesmg[4][28] = {
    {-30,  -19,   -8,   -2,    4,    9,   17,   24,   36},
    {-29,  -18,   -8,   -1,    7,   12,   16,   17,   18,   21,   34,   56,   35,   63},
    {-33,  -20,  -16,  -12,  -13,   -5,    0,    7,   13,   18,   24,   28,   35,   37,   15},
    {-1,  -12,  -24,  -23,  -22,  -19,  -16,  -14,  -11,   -9,   -8,   -5,   -3,   -3,   -1,    2,   -8,   -7,    1,   10,   16,   43,   30,   20,   24,    9,    2,    0}
};

short int mobilitybonuseseg[4][28] = {
    {-67,  -13,    9,   20,   27,   35,   35,   32,   21},
    {-45,  -21,   -4,   10,   20,   30,   35,   39,   43,   42,   35,   32,   46,   32},
    {-19,   14,   28,   34,   44,   48,   54,   58,   62,   67,   70,   74,   76,   74,   87},
    {0,   -2,   -6,   -9,  -13,  -17,  -16,  -12,  -10,   -7,   -3,    0,    0,    4,    4,    5,   27,   25,   21,   14,   15,    2,    5,   13,   16,   12,    3,   -1}
};
short int doubledpen = -16;
short int isopen =  -12;
short int backwardspen = -4;

char attacknums[4] = {2,2,3,5};
int king_attack_count[2];
int attacking_pieces[2];

short int kingdangertablemg[4][100] = {

{
   0,    0,    1,    2,   44,   39,   54,   77,   88,   65, 
  77,  114,   95,  137,  139,   93,  188,  150,  218,  162, 
 144,  212,  185,  243,  199,  146,  246,  193,  229,  181, 
 242,  246,  229,  235,  248,  308,  274,  232,  251,  277, 
 283,  267,  273,  261,  294,  246,  234,  249,  258,  324, 
 367,  294,  302,  359,  365,  404,  418,  421,  437,  451, 
 493,  494,  492,  472,  490,  486,  491,  492,  498,  480, 
 496,  500,  495,  500,  495,  498,  500,  497,  500,  489, 
 500,  498,  499,  500,  492,  500,  500,  500,  500,  495, 
 500,  498,  500,  500,  500,  500,  500,  500,  500,  500, 
},
{
   0,    0,    1,    2,   22,   11,   33,   30,   56,   46, 
  26,   78,   55,   87,   73,   39,  105,   72,  115,   85, 
  60,  131,   84,  152,  118,   84,  150,  120,  158,  135, 
 167,  174,  207,  175,  226,  270,  216,  242,  208,  204, 
 285,  228,  232,  234,  235,  231,  252,  251,  221,  340, 
 346,  353,  319,  338,  341,  417,  391,  437,  446,  444, 
 464,  492,  469,  486,  489,  488,  490,  491,  491,  491, 
 492,  494,  498,  500,  496,  498,  498,  500,  500,  499, 
 500,  500,  499,  498,  500,  500,  495,  500,  500,  500, 
 500,  500,  500,  501,  500,  500,  500,  497,  500,  501, 
},
{
   0,    0,    1,    2,   -2,   -3,    5,    4,   15,   16, 
   4,   26,   13,   42,   22,   15,   46,   32,   63,   45, 
  23,   74,   50,   97,   62,   72,   84,  106,  123,  103, 
 104,  110,  129,  122,  100,  164,  148,  194,  165,  140, 
 162,  220,  190,  180,  229,  260,  263,  272,  295,  314, 
 371,  361,  347,  360,  390,  437,  427,  447,  445,  454, 
 495,  494,  485,  483,  497,  493,  489,  496,  500,  500, 
 500,  500,  500,  500,  500,  500,  496,  500,  500,  499, 
 500,  500,  499,  500,  500,  500,  500,  500,  500,  500, 
 500,  498,  500,  500,  500,  500,  500,  500,  500,  500, 
},
{
   0,    0,    1,    2,  -13,  -10,  -13,  -11,  -13,   -7, 
  -1,   -1,   -4,    2,    0,    1,   11,   11,   15,    5, 
  -3,   19,   17,   14,    9,   22,   12,   28,   41,   24, 
  15,   45,   45,   79,   85,  132,  132,  127,  231,  211, 
 183,  216,  206,  278,  263,  289,  330,  301,  320,  338, 
 387,  377,  359,  386,  395,  437,  446,  460,  460,  460, 
 495,  494,  491,  490,  497,  492,  493,  496,  500,  500, 
 500,  500,  500,  500,  500,  500,  500,  500,  500,  499, 
 500,  500,  499,  500,  500,  500,  500,  500,  500,  500, 
 500,  500,  500,  500,  500,  500,  500,  500,  500,  500, 
}
};

short int kingdangertableeg[4][100] = {

{
   0,    0,    1,    2,   -8,   35,  -12,    2,  -17,    1, 
  15,   -5,   -4,  -34,   -5,   12,  -38,  -10,  -48,   -3, 
  35,  -53,   37,  -43,  -13,   45,    1,   25,   26,   78, 
  11,   43,   44,  118,   93,  -24,   28,  101,  145,  144, 
 111,   96,   90,  202,  256,  208,  190,  190,  212,  310, 
 351,  289,  292,  318,  350,  409,  398,  423,  440,  442, 
 493,  494,  488,  481,  488,  484,  490,  493,  498,  488, 
 498,  500,  497,  500,  494,  498,  500,  499,  500,  492, 
 500,  498,  499,  500,  496,  500,  500,  500,  500,  497, 
 500,  498,  500,  500,  500,  500,  500,  500,  500,  500, 
},
{
   0,    0,    1,    2,    2,   17,  -10,   11,   -9,   -9, 
  16,  -10,   -7,  -12,   10,   21,  -20,   24,  -25,   20, 
  45,  -17,   75,  -10,    3,   50,   17,   57,   34,   59, 
  26,   76,   44,  105,   17,  -34,   57,   59,  123,  167, 
 102,  118,  120,  199,  221,  204,  231,  245,  227,  326, 
 345,  352,  314,  333,  352,  426,  407,  444,  449,  435, 
 477,  493,  474,  486,  491,  489,  492,  493,  493,  494, 
 494,  498,  498,  500,  496,  498,  498,  500,  500,  499, 
 500,  500,  499,  499,  500,  500,  497,  500,  500,  500, 
 500,  500,  500,  500,  500,  500,  500,  499,  500,  500, 
},
{
   0,    0,    1,    2,    4,    4,  -10,    7,  -10,   -6, 
  -1,   -7,   -6,  -19,    7,   10,  -28,   22,  -27,   11, 
  12,    0,   16,   18,   19,   18,   43,   49,   74,   54, 
  63,   85,   93,  139,  112,  125,  123,  157,  222,  163, 
 181,  230,  213,  233,  266,  282,  306,  295,  320,  341, 
 386,  372,  354,  375,  391,  437,  440,  456,  457,  463, 
 495,  494,  490,  488,  497,  492,  491,  496,  500,  500, 
 500,  500,  500,  500,  500,  500,  498,  500,  500,  499, 
 500,  500,  499,  500,  500,  500,  500,  500,  500,  500, 
 500,  499,  500,  500,  500,  500,  500,  500,  500,  500, 
},
{
   0,    0,    1,    2,    4,    2,   -2,    7,   -6,   -2, 
  -8,   -4,   -1,   -7,   -2,  -17,    1,  -13,  -30,    3, 
   7,   43,   -5,   15,   28,   21,   72,   96,   65,   28, 
 128,   79,  138,  146,  129,  132,  141,  143,  234,  212, 
 197,  239,  211,  277,  277,  290,  325,  304,  320,  350, 
 388,  377,  359,  386,  394,  437,  446,  460,  460,  462, 
 495,  494,  491,  490,  497,  492,  493,  496,  500,  500, 
 500,  500,  500,  500,  500,  500,  500,  500,  500,  499, 
 500,  500,  499,  500,  500,  500,  500,  500,  500,  500, 
 500,  500,  500,  500,  500,  500,  500,  500,  500,  500, 
}
};


const int pieceattacksbonus[4][4] = {
    {31, 46, 31, 29},
    {0, 16, 13, 7},
    {9, 0,  14, 38},
    {0, 0, 0, 41}
};
const int multattacksbonus = 27;


char *getsafe(char *buffer, int count)
{
    	char *result = buffer, *np;
	if ((buffer == NULL) || (count < 1)){
        result =  NULL;
    }

	else{
        result = fgets(buffer, count, stdin);
        if (result == NULL){
            free(TT);
            exit(0);
        }
    } 
	if ((np = strchr(buffer, '\n')))
		*np = '\0';
	return result;
}



void initglobals(){
    for (unsigned char i = 0; i < 0x80; i++){
        if (i & 0x88){
            i += 7; continue;
        }
        for (unsigned char n = 0; n < 8; n++){
            if (!((i+vector[4][n]) & 0x88)){                    //this will give us a value of 2 for side (i.e. rook checks) and 1 for diagonals
                KINGZONES[0][i][i+vector[4][n]] = (n&1) + 1;
                KINGZONES[1][i][i+vector[4][n]] = (n&1) + 1;    
            }   
        }
        if (i + 33 < 0x80){
        KINGZONES[0][i][i+31] = 3, KINGZONES[0][i][i+32] = 3, KINGZONES[0][i][i+33] = 3;   //and 3 for advanced squares
        }
        if (i - 33 > 0x0){
        KINGZONES[1][i][i-31] = 3, KINGZONES[1][i][i-32] = 3, KINGZONES[1][i][i-33] = 3;
        }

        if ((i&7) > 1 && (i&7) < 6 && (i>>4) > 0 && (i>>4) < 7){
            if ((i>>4) >= 4){
                CENTERBLACK[i] = true;
            }
            else{
                CENTERWHITE[i] = true;
            }
        }
    }
    for (int i = 0; i < 100; i++){
        for (int n = 0; n < LISTSIZE; n++){
            LMRTABLE[i][n] = (int)round(log(i+1)*log(n+1)/1.95);
        }
    }

}



bool ismatch(struct move move1, struct move move2){    
    if (move1.move == move2.move && move1.flags == move2.flags){
        return true;
    }
    return false;
}

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
    for (int i = 0; i < 774; i++){
        ZOBRISTTABLE[i] = genrand64_int64();
    }
}

void calc_pos(struct board_info *board, bool color){
    CURRENTPOS = 0;
    int i;
    for (i = 0; i < 0x80; i++){
            if (!(i & 0x88) && board->board[i]){
                CURRENTPOS ^= ZOBRISTTABLE[(((board->board[i]-2)<<6))+i-((i>>4)<<3)];
                //board->board[i]-2<<6: gives us the base value for each piece*64.
                //i-((i>>4)<<3): converts i into a number from 1 to 64. it does that by subtracting 8 for every rank it's on - on rank 0 doesn't get subtracted, on rank 7 gets -56
            }
    }
    if (color){
        CURRENTPOS ^= ZOBRISTTABLE[772];
    }
}

void insert(unsigned long long int position, int depthleft, int eval, char type, struct move bestmove){
    int index = (position) & (_mask);

    if (TT[index].zobrist_key == position && !(type == 3 && TT[index].type != 3)){
        int agediff = search_age - TT[index].age;
        int newentryb = depthleft + type + ((agediff*agediff)>>2);
        int oldentryb = TT[index].depth + TT[index].type;
        if (oldentryb * 2 > newentryb * 3){
            return;
        }
    }
    TT[index].zobrist_key = position;
    TT[index].depth = depthleft;
    TT[index].eval = eval;
    TT[index].type = type;
    TT[index].age = search_age;
    TT[index].bestmove = bestmove;
}


void clearTT(){
    int i; for (i = 0; i < TTSIZE; i++){
        TT[i] = nullTT;
        }
        
    }

void clearHistory(bool delete){
    if (!delete){
        for (int i = 0; i < 0x80; i++){
        for (int n = 0; n < 0x80; n++){
            HISTORYTABLE[WHITE][i][n] = (HISTORYTABLE[WHITE][i][n] >> 2);
            HISTORYTABLE[BLACK][i][n] = (HISTORYTABLE[BLACK][i][n] >> 2);
        }
    }
    }
    else{

    for (int i = 0; i < 0x80; i++){
        for (int n = 0; n < 0x80; n++){
            HISTORYTABLE[WHITE][i][n] = 0;
            HISTORYTABLE[BLACK][i][n] = 0;
        }
    }
    }
}
void clearKiller(){
    for (int i = 0; i < 100; i++){
        KILLERTABLE[i][0].move = 0;
        KILLERTABLE[i][1].move = 0;
    }
}

void clearCounters(){
    for (int i = 0; i < 6; i++){
        for (int n = 0; n < 128; n++){
            COUNTERMOVES[i][n] = nullmove;
        }
    }
}

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

void setmovelist(struct movelist *movelst, int *key){
    
    movelst[0].fen = CURRENTPOS;
    *key = 1;
    movelst[0].move.move = 0;
    movelst[1].move.move = 0;
    movelst[0].halfmoves = 0;
    return;
}

char *conv(struct move move, char *temp){
    temp[0] = ((move.move>>8) & 7) + 97, temp[1] = ((move.move>>8) >> 4) + 1 + '0', temp[2] = ((move.move & 0xFF) & 7) + 97, temp[3] = ((move.move & 0xFF) >> 4) + 1 + '0';
    if (move.flags >> 2 == 1){
        switch(move.flags){
            case 4:
            temp[4] = 'N'; break;
            case 5:
            temp[4] = 'B'; break;
            case 6:
            temp[4] = 'R'; break;
            default:
            temp[4] = 'Q'; break;
        }
        temp[5] = '\0';
    }
    else{
        temp[4] = '\0';
    }
    return temp;
}

void convto(char *mve, struct move *to_move, struct board_info *board){
    unsigned short int from = ((atoi(&mve[1])-1)<<4) + (mve[0]-97), to = ( (atoi(&mve[3])-1)<<4) + (mve[2]-97);
    
    to_move->move = (from<<8) + to;
    if ((board->board[from]>>1) == KING && abs(from-to) == 2){
        to_move->flags = 8;
    }
    else if (board->board[from]>>1 == PAWN && (from&7) != (to&7) && !board->board[to]){
        to_move->flags = 0xC;
    }
    else if (isalpha(mve[4])){
        switch (toupper(mve[4])){
            case 'Q':
            to_move->flags = 7; break;
            case 'R':
            to_move->flags = 6; break;
            case 'B':
            to_move->flags = 5; break;
            case 'N':
            to_move->flags = 4; break;
            default:
            printf("error\n!");
            exit(1);
        }
    }
    else{
        to_move->flags = 0;
    }
    
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

void pawnmoves(struct board_info *board, struct list *list, int *key, unsigned char pos, bool color){
        
    if (!color){

        unsigned char diff = pos+NORTH;

        if (!board->board[diff]){   //if the square in front is empty, go forwards
            unsigned short int t = pos<<8;
            if (((diff)>>4) == 7){  //we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;

            }
            else{ 
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 0; //go forwards and see if we can go forwards again

                if ((pos>>4) == 1 && !board->board[diff + NORTH]){

                
                    list[*key].move.move = t + diff + NORTH, list[(*key)++].move.flags = 0;

                }
            }
        }
        diff++;
        if (!(diff&0x88) && board->board[diff] && (board->board[diff]&1)){ //capture to the right

            unsigned short int t = pos<<8;
            if (((diff)>>4) == 7){  //we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;

            }
            else{
            list[*key].move.move = t + diff, list[(*key)++].move.flags = 0;
            }
      
        }
        diff-=2;
        if (!(diff&0x88) && board->board[diff] && (board->board[diff]&1)){ 
            unsigned short int t = pos<<8;
            if (((diff)>>4) == 7){  //we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;

            }
            else{
            list[*key].move.move = t + diff, list[(*key)++].move.flags = 0;
            }
        }
    }
    else{
        unsigned char diff = pos+SOUTH;

        if (!board->board[diff]){   //if the square in front is empty, go forwards
            unsigned short int t = pos<<8;
            if (((diff)>>4) == 0){  //we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;

            }
            else{ 
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 0; //go forwards and see if we can go forwards again

                if ((pos>>4) == 6 && !board->board[diff + SOUTH]){

                
                    list[*key].move.move = t + diff + SOUTH, list[(*key)++].move.flags = 0;

                }
            }
        }
        diff++;
        if (!(diff&0x88) && board->board[diff] && !(board->board[diff]&1)){ //capture to the right
            unsigned short int t = pos<<8;
            if (((diff)>>4) == 0){  //we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;

            }
            else{
            list[*key].move.move = t + diff, list[(*key)++].move.flags = 0;
            }
        }
        diff-=2;
        if ( !(diff&0x88) && board->board[diff] && !(board->board[diff]&1)){
            unsigned short int t = pos<<8;
            if (((diff)>>4) == 0){  //we have a promotion.
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 4;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 5;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 6;
                list[*key].move.move = t + diff, list[(*key)++].move.flags = 7;

            }
            else{
            list[*key].move.move = t + diff, list[(*key)++].move.flags = 0;  
            }
        }
    }

}

int movegen(struct board_info *board, struct list *list, bool color, bool incheck){  //a full pseudolegal move gen
    int key = 0;
    for (unsigned char i = 0; i < 0x80; i++){
        if (i & 0x88){
            i += 7; continue;
        }
        if (board->board[i] && (board->board[i]&1) == color){
            int piecetype = (board->board[i]>>1)-2;
            if (piecetype == -1){ //if it is a pawn
                pawnmoves(board, list, &key, i, color);
            }

            else{
                for (unsigned char dir = 0; dir < vectors[piecetype]; dir++){
                    for (int pos = i;;){
                        pos += vector[piecetype][dir];
                        if ((pos & 0x88) || (board->board[pos] && (board->board[pos]&1) == color)){break;}
                        list[key].move.move = (i<<8) + pos, list[key++].move.flags = 0;
                        if (board->board[pos] || !slide[piecetype]){
                            break;
                        }                         
                    }
                }
            }
        }
    }
    if (board->epsquare){
        if ( !((board->epsquare+1)&0x88)  && board->board[board->epsquare+1] == WPAWN+color){
            if (!color){
                list[key].move.move = ((board->epsquare+1)<<8) + board->epsquare + NORTH, list[key++].move.flags = 0xC;
            }
            else{
                list[key].move.move = ((board->epsquare+1)<<8) + board->epsquare + SOUTH, list[key++].move.flags = 0xC;
            }
            
        }
        if ( !((board->epsquare-1)&0x88) && board->board[board->epsquare-1] == WPAWN+color){
            if (!color){
                list[key].move.move = ((board->epsquare-1)<<8) + board->epsquare + NORTH, list[key++].move.flags = 0xC;
            }
            else{
                list[key].move.move = ((board->epsquare-1)<<8) + board->epsquare + SOUTH, list[key++].move.flags = 0xC;
            }
        }
    }
    if (incheck){
        return key;
    }

    if (board->castling[color][0]){
        char x = board->kingpos[color];
        if (board->board[x-4]-color == WROOK && !board->board[x-3] && !board->board[x-2] && !board->board[x-1] && !isattacked(board, x-1, color^1)){
            list[key].move.move = (x<<8) + x + WEST + WEST, list[key++].move.flags = 0x8;
        }
    }
    if (board->castling[color][1]){
        char x = board->kingpos[color];
        if (board->board[x+3]-color == WROOK && !board->board[x+2] && !board->board[x+1] && !isattacked(board, x+1, color^1)){
            list[key].move.move = ((board->kingpos[color])<<8) + x + EAST + EAST, list[key++].move.flags = 0x8;
        }       
    }
    return key;
}

int piece_mobility(struct board_info *board, unsigned char i, bool color, unsigned char piecetype, int *score){
    
    
    isattacker = false;
    unsigned char mobility = 0;
    for (unsigned char dir = 0; dir < vectors[piecetype]; dir++){
        for (int pos = i;;){
            pos += vector[piecetype][dir];

                if ((pos & 0x88) || (board->board[pos] && (board->board[pos]&1) == color)){break;}

                        if (color){
                            if ( (((pos+SE) & 0x88) || (board->board[pos+SE] != WPAWN))  && (((pos+SW) & 0x88) || (board->board[pos+SW] != WPAWN))){
                                mobility++;
                            }                           
                        }
                        else{
                            if ((((pos+NE) & 0x88) || (board->board[pos+NE] != BPAWN)) && ( ((pos+NW) & 0x88) || (board->board[pos+NW] != BPAWN))){
                                mobility++;
                            }      
                        }                        
                    
                    if (KINGZONES[color^1][board->kingpos[color^1]][pos]){
                        attackers[color] += (!isattacker);          
                        king_attack_count[color] += attacknums[piecetype];
                        if (piecetype == 2 && KINGZONES[color^1][board->kingpos[color^1]][pos] == 2){ //ROOK
                            char temp = board->board[i];
                            board->board[i] = BLANK;   
                            if (isattacked(board, pos, color) && isattacked_mv(board, pos, color^1) != 2){
    
                                        king_attack_count[color] += 2;
                                    }
                                     
                            board->board[i] = temp;
                        }
                        else if (piecetype == 3 && KINGZONES[color^1][board->kingpos[color^1]][pos] != 3){ //QUEEN
                            attackers[color] += (!isattacker);
                            char temp = board->board[i];
                            board->board[i] = BLANK;   
                            if (isattacked(board, pos, color) && isattacked_mv(board, pos, color^1) != 2){
    
                                        king_attack_count[color] += 6;
                                    }
                                     
                            board->board[i] = temp;
                        }
                        isattacker = true;
                    }
                    if (board->board[pos] || !slide[piecetype]){
                        if (piecetype == 0){    //knight
                            if (board->board[pos] > BKNIGHT && board->board[pos]/2 != KING){   //knights get bonuses for attacking queens, rooks, and bishops.
                                *score -= pieceattacksbonus[1][board->board[pos]/2 - 2];
                            }
                        }
                        else if (piecetype == 1){ //bishops get bonuses for attacking queens, rooks, and knights.
                            if ((board->board[pos] > BBISHOP || board->board[pos]/2 == KNIGHT) && board->board[pos]/2 != KING){
                                *score -= pieceattacksbonus[2][board->board[pos]/2 - 2];
                            }
                        }
                        else if (piecetype == 2){
                            if (board->board[pos]/2 == QUEEN){ //rooks get bonuses for attacking queens.
                                *score -= pieceattacksbonus[3][board->board[pos]/2 - 2];
                            }
                        }
                        break;
                    }                         
                }
            }    
    return mobility;
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

long long unsigned int perft(int depth, struct board_info *board, bool color, bool first){
    if (!depth){
        return 1;
    }
    struct list list[LISTSIZE];
    int nmoves, i;
    long long unsigned int l = 0;
    nmoves = movegen(board, list, color, isattacked(board, board->kingpos[color], color^1));
    for (i = 0; i < nmoves; i++){
        struct board_info board2 = *board;
        move(&board2, list[i].move, color);
        if (isattacked(&board2, board2.kingpos[color], color^1)){
            continue;
        }
        long long unsigned int b = perft(depth-1, &board2, color^1, false);
        if (first){
            char temp[6];
            printf("%llu %s\n", b, conv(list[i].move, temp));
        }
        l += b;
    }
    return l;
}


int material(struct board_info *board, int *phase){
    int mgscore = 0, egscore = 0;
    int val = 0;
    for (int i = 0; i < 5; i++){
        if (i == 4){
                *phase += board->pnbrqcount[WHITE][i]<<2;
                *phase += board->pnbrqcount[BLACK][i]<<2;
        }
        else{
            *phase += ((i+1)>>1)*board->pnbrqcount[WHITE][i];
            *phase += ((i+1)>>1)*board->pnbrqcount[BLACK][i];
        }
        mgscore += VALUES[i]*board->pnbrqcount[WHITE][i];
        mgscore -= VALUES[i]*board->pnbrqcount[BLACK][i];
        egscore += VALUES2[i]*board->pnbrqcount[WHITE][i];
        egscore -= VALUES2[i]*board->pnbrqcount[BLACK][i];        
    }
    if (*phase > MAXPHASE){
        *phase = MAXPHASE;
    }
    
    val = ((*phase)*mgscore + (24-(*phase))*egscore)/24;
    if (board->pnbrqcount[WHITE][2] > 1){
        val += bishop_pair[board->pnbrqcount[WHITE][0]];
    }
    if (board->pnbrqcount[BLACK][2] > 1){
        val -= bishop_pair[board->pnbrqcount[BLACK][0]];
    }
    return val;
}

int pst(struct board_info *board, int phase){

    attacking_pieces[0] = 0, attacking_pieces[1] = 0;

    int score = 0;
    unsigned char spacew = 0, spaceb = 0;
    int mgscore = 0, egscore = 0;
    int blockedpawns = 0;

    short int wbackwards[8], wadvanced[8], bbackwards[8], badvanced[8];

    for (unsigned char i = 0; i < 8; i++){
    wadvanced[i] = -1, badvanced[i] = 9;
    wbackwards[i] = 9, bbackwards[i] = -1;
    }
    for (unsigned char i = 0; i < 0x80; i++){
        if ((i&0x88)){
            i += 7; continue;
        }
        if (phase > 16){
            if (CENTERWHITE[i]){
                
                if (board->board[i] != WPAWN && ((((i+NW)&0x88) || board->board[i+NW] != BPAWN) && (((i+NE)&0x88) || board->board[i+NE] != BPAWN))){
                    
                    spacew++;
                    if ((board->board[i+NORTH] == WPAWN || board->board[i+(NORTH<<1)] == WPAWN || board->board[i+(NORTH*3)] == WPAWN) && !isattacked(board, i, BLACK)){
                        spacew++;
                    }
                }
            }
            else if (CENTERBLACK[i]){
                if (board->board[i] != BPAWN && ((((i+SW)&0x88) || board->board[i+SW] != WPAWN) && (((i+SE)&0x88) || board->board[i+SE] != WPAWN))){
                    spaceb++;
                    if ((board->board[i+SOUTH] == BPAWN || board->board[i+(SOUTH*1)] == BPAWN || board->board[i+(SOUTH*3)] == BPAWN) && !isattacked(board, i, WHITE)){
                        spaceb++;
                    }
                } 

            }

        }
        
        if (board->board[i]){
            bool mobilitybonus = false;
            int moves = 0;
            unsigned char piecetype = (board->board[i]>>1)-1, piececolor = (board->board[i]&1);
            if (piecetype != 0 && piecetype != 5){ //pawns or kings
                moves = piece_mobility(board, i, piececolor, piecetype-1, &score);
                mobilitybonus = true;
            }
            if ((piececolor)){   //black piece
                mgscore -= pstbonusesm[piecetype][i^112];
                egscore -= pstbonusese[piecetype][i^112];
                if (mobilitybonus){
                    //printf("%i %i\n", moves, i);
                    mgscore -= mobilitybonusesmg[piecetype-1][moves];
                    egscore -= mobilitybonuseseg[piecetype-1][moves];
                }
                if (!piecetype && (board->board[i+SOUTH] == WPAWN || ( (((i+SSW)&0x88) || board->board[i+SSW] == WPAWN) && (((i+SSE)&0x88) || board->board[i+SSE] == WPAWN)) )){
                    blockedpawns++;
                }
                if (piecetype == 0){
                    if (!((i + SW) & 0x88) && board->board[i+SW] > BPAWN && board->board[i+SW] < WKING && !(board->board[i+SW]&1)){
                            attacking_pieces[BLACK]++;
                            score -= pieceattacksbonus[0][board->board[i+SW]/2-2];
                    }
                   if (!((i + SE) & 0x88) && board->board[i+SE] > BPAWN && board->board[i+SE] < WKING && !(board->board[i+SE]&1)){
                            attacking_pieces[BLACK]++;
                            score -= pieceattacksbonus[0][board->board[i+SE]/2-2];
                    }
                    if (badvanced[(i&7)] == 9){
                        badvanced[(i&7)] = (i>>4);
                    }
                    if ((i>>4) > bbackwards[(i&7)]){
                        if (bbackwards[(i&7)] != -1){

                            mgscore -= doubledpen;
                            egscore -= doubledpen;
                        }
                        bbackwards[(i&7)] = (i>>4);
                        
                    }
                }
            }
            else{
                mgscore += pstbonusesm[piecetype][i];
                egscore += pstbonusese[piecetype][i];
                if (mobilitybonus){
                    mgscore += mobilitybonusesmg[piecetype-1][moves];
                    egscore += mobilitybonuseseg[piecetype-1][moves];
                }
                if (!piecetype && (board->board[i+NORTH] == BPAWN || ( (((i+NNW)&0x88) || board->board[i+NNW] == BPAWN) && (((i+NNE)&0x88) || board->board[i+NNE] == BPAWN)) )){
                    blockedpawns++;
                }
                if (piecetype == 0){
                    if (!((i + NW) & 0x88) && board->board[i+NW] > BPAWN && board->board[i+NW] < WKING && (board->board[i+NW]&1)){
                            attacking_pieces[WHITE]++;
                            score -= pieceattacksbonus[0][board->board[i+NW]/2-2];
                    }
                   if (!((i + NE) & 0x88) && board->board[i+NE] > BPAWN &&  board->board[i+NE] < WKING && (board->board[i+NE]&1)){
                            attacking_pieces[WHITE]++;
                            score += pieceattacksbonus[0][board->board[i+NE]/2-2];
                    }
                    if (wbackwards[(i&7)] == 9){
                        wbackwards[(i&7)] = (i>>4);
                    }
                    if ((i>>4) > wadvanced[(i&7)]){
                        if (wadvanced[(i&7)] != -1){
                            egscore += doubledpen;
                            mgscore += doubledpen;
                        }
                        wadvanced[(i&7)] = (i>>4);
                        
                    }
                }
            }

        }
    }


    
    for (unsigned char i = 0; i < 8; i++){
        if (i == 7){
            if (wadvanced[7] != -1){
                if ( wadvanced[6] == -1){              //evaluates isolated pawns for h file + passed pawns for h file
                    score += isopen;
                }
                else if (wbackwards[7] < wbackwards[6]){
                    
                    score += backwardspen;
                }
            }

            if (badvanced[7] != 9){
                if (badvanced[6] == 9){
                score -= isopen;
            }
            else if (bbackwards[7] > bbackwards[6]){
                score -= backwardspen;
            }
            }

            if (wadvanced[7] != -1 && wadvanced[7] >= bbackwards[7] && wadvanced[7] >= bbackwards[6]){
                int pos = ((wadvanced[7]<<4)) + 7;
                 if (board->board[pos+NORTH]){
                    mgscore += blockedmgbonus[wadvanced[7]], egscore += blockedegbonus[wadvanced[7]];
                    }
                else{
                    mgscore += passedmgbonus[wadvanced[7]], egscore += passedegbonus[wadvanced[7]];
                }
            }
            if (badvanced[7] != 9 && badvanced[7] <= wbackwards[7] && badvanced[7] <= wbackwards[6]){
                int pos = ((badvanced[7]<<4)) + 7;
                 if (board->board[pos+SOUTH]){
                    mgscore -= blockedmgbonus[7-badvanced[7]], egscore -= blockedegbonus[7-badvanced[7]];
                 }
                else{
                    mgscore -= passedmgbonus[7-badvanced[7]], egscore -= passedegbonus[7-badvanced[7]];
                }
            }
        }
        else if (i == 0){
            if (wadvanced[0] != -1){

                if (wadvanced[1] == -1){              //evaluates isolated pawns for a+passed
                    score += isopen;
                }
                else if (wbackwards[0] < wbackwards[1]){
                    score += backwardspen;
                }
            }
            if (badvanced[0] != 9){
            if (badvanced[6] == 9){
                score -= isopen;
            }
            else if (bbackwards[0] > bbackwards[1]){
                score -= backwardspen;
            }
            }

            if (wadvanced[0] != -1 && wadvanced[0] >= bbackwards[0] && wadvanced[0] >= bbackwards[1]){
                int pos = ((wadvanced[0]<<4));
                 if (board->board[pos+NORTH]){
                    mgscore += blockedmgbonus[wadvanced[0]], egscore += blockedegbonus[wadvanced[0]];
                    }
                else{
                    mgscore += passedmgbonus[wadvanced[0]], egscore += passedegbonus[wadvanced[0]];
                }
            }
            if (badvanced[0] != 9 && badvanced[0] <= wbackwards[0] && badvanced[0] <= wbackwards[1]){
                int pos = ((badvanced[0]<<4));
                 if (board->board[pos+SOUTH]){
                    mgscore -= blockedmgbonus[7-badvanced[0]], egscore -= blockedegbonus[7-badvanced[0]];
                 }
                else{
                    mgscore -= passedmgbonus[7-badvanced[0]], egscore -= passedegbonus[7-badvanced[0]];
                }
            }
        }
        else{
            if (wadvanced[i] != -1){
                if( wadvanced[i-1] == -1 &&  wadvanced[i+1] == -1){              //evaluates isolated pawns for b-g files
                score += isopen;
            }
            else if (wbackwards[i] < wbackwards[i+1] && wbackwards[i] < wbackwards[i-1]){
                score += backwardspen;
            }
            }

            if (badvanced[i] != 9){

             if (badvanced[i-1] == 9 &&  badvanced[i+1] == 9){
                score -= isopen;
            }
            else if (bbackwards[i] > bbackwards[i+1] && bbackwards[i] > bbackwards[i-1]){
                score -= backwardspen;
            }
            }

            if (wadvanced[i] != -1 && wadvanced[i] >= bbackwards[i] && wadvanced[i] >= bbackwards[i-1] && wadvanced[i] >= bbackwards[i+1]){
                int pos = ((wadvanced[i]<<4)) + i;
                
                 if (board->board[pos+NORTH]){
                    mgscore += blockedmgbonus[wadvanced[i]], egscore += blockedegbonus[wadvanced[i]];
                    }
                else{
                    mgscore += passedmgbonus[wadvanced[i]], egscore += passedegbonus[wadvanced[i]];
                }
            }
            if (badvanced[i] != 9 && badvanced[i] <= wbackwards[i-1] && badvanced[i] <= wbackwards[i] && badvanced[i] <= wbackwards[i+1]){
                int pos = ((badvanced[i]<<4)) + i;
                
                 if (board->board[pos+SOUTH]){
                    mgscore -= blockedmgbonus[7-badvanced[i]], egscore -= blockedegbonus[7-badvanced[i]];
                 }
                else{
                    mgscore -= passedmgbonus[7-badvanced[i]], egscore -= passedegbonus[7-badvanced[i]];
                }
            }
        }
    }
    
    if (phase > 16){
        int weight0 = 0, weight1 = 0;
    
        for (int i = 0; i < 5; i++){
            weight0 += board->pnbrqcount[WHITE][i];            
            weight1 += board->pnbrqcount[BLACK][i];
        }
        if ((blockedpawns) > 9){(blockedpawns) = 9;}
        weight0 = weight0 - 3 + 1 + blockedpawns;
        weight1 = weight1 - 3 + 1 + blockedpawns;
        if (weight0 < 0){weight0 = 0;}
        if (weight1 < 0){weight1 = 0;}
        int space = ((((spacew * weight0 * weight0)>>4) - ((spaceb * weight1 * weight1)>>4)))>>1;
        score += space * 5 / 10;
    }
    
    if (attackers[BLACK] > 1){
        if (king_attack_count[BLACK] > 99){
            king_attack_count[BLACK] = 99;
        }
        int pawnshield = 0;
        if (board->kingpos[WHITE] < 0x40){
        if ((!((board->kingpos[WHITE]+WEST) & 0x88)) && (board->board[board->kingpos[WHITE]+WEST] == WPAWN || board->board[board->kingpos[WHITE]+NW] == WPAWN)){
            pawnshield++;
        }
        if ((((board->board[board->kingpos[WHITE]+NORTH])>>1) == PAWN) || board->board[board->kingpos[WHITE]+NORTH+NORTH] == WPAWN || board->board[board->kingpos[WHITE]+NORTH+NORTH+NORTH] == WPAWN){
            pawnshield++;
        }
        if ((!((board->kingpos[WHITE]+EAST) & 0x88)) && (board->board[board->kingpos[WHITE]+EAST] == WPAWN || board->board[board->kingpos[WHITE]+NE] == WPAWN)){
            pawnshield++;
        }

        }
        mgscore -= kingdangertablemg[pawnshield][king_attack_count[BLACK]], egscore -= kingdangertableeg[pawnshield][king_attack_count[BLACK]];


    }
    
    if (attackers[WHITE] > 1){
        if (king_attack_count[WHITE] > 99){
            king_attack_count[WHITE] = 99;
        }
        int pawnshield = 0;
        if (board->kingpos[BLACK] > 0x3a){
        if (!((board->kingpos[BLACK]+WEST) & 0x88) && (board->board[board->kingpos[BLACK]+WEST] == BPAWN || board->board[board->kingpos[BLACK]+SW] == BPAWN)){
            pawnshield++;
        }
        if ((((board->board[board->kingpos[BLACK]+SOUTH])>>1) == PAWN) || board->board[board->kingpos[BLACK]+SOUTH+SOUTH] == BPAWN || board->board[board->kingpos[BLACK]+SOUTH+SOUTH+SOUTH] == BPAWN){
            pawnshield++;
        }
        if (!((board->kingpos[BLACK]+EAST) & 0x88) && (board->board[board->kingpos[BLACK]+EAST] == BPAWN || board->board[board->kingpos[BLACK]+SE] == BPAWN)){
            pawnshield++;
        }
        }
        mgscore += kingdangertablemg[pawnshield][king_attack_count[WHITE]], egscore += kingdangertableeg[pawnshield][king_attack_count[WHITE]];


    }
    score += (phase*mgscore + (24-phase)*egscore)/24;

    if (attacking_pieces[WHITE] > 1){
        score += multattacksbonus * (attacking_pieces[WHITE] - 1);
    }
    if (attacking_pieces[BLACK] > 1){
        score -= multattacksbonus * (attacking_pieces[BLACK] - 1);
    }

    return score;
}

int eval(struct board_info *board, bool color){
    attackers[0] = 0, attackers[1] = 0;
    king_attack_count[0] = 0, king_attack_count[1] = 0;
    int phase = 0;
    int evl = material(board, &phase);

    evl += pst(board, phase);
    if (board->pnbrqcount[WHITE][0] <= 1 && evl >= 0 && evl < 400 && phase != 0){ //if White is up material, we want to stop it from trading pawns
    if (board->pnbrqcount[WHITE][0] == 0){
        evl >>= 2;
    }
    else{ evl >>= 1;}
    }
    if (board->pnbrqcount[BLACK][0] <= 1 && evl <= 0 && evl > -400 && phase != 0){
    if (board->pnbrqcount[BLACK][0] == 0){
        evl >>= 2;
    }
    else {evl >>= 1;}
    }
    if (color == BLACK){
        evl = -evl;
    }
    return evl + TEMPO;
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

int SEEVALUES[7] = {0, 100, 450, 450, 650, 1250, 10000};

bool static_exchange_evaluation(struct board_info *board, struct move mve, bool color, int threshold){
    if (mve.flags && mve.flags >> 2 != 1){  //castling and en passant both come out neutral; thus we can return immediately.
        return (threshold <= 0);
    }
    int to = mve.move&0xFF;
    
    int gain = SEEVALUES[board->board[mve.move & 0xFF] >> 1];   //what you get from taking the piece
    int risk = SEEVALUES[board->board[mve.move >> 8] >> 1];     //what you lose if your piece gets taken!

    struct board_info board2 = *board;

    board2.board[mve.move >> 8] = BLANK;

    unsigned int attacker_pos = 0;

    int totalgain = gain - threshold;   //how much you have gained after the first capture you made.

    if (totalgain < 0){
        return false;
    }

    while (totalgain - risk < 0){   //an example with RxB BxR RxB

        int i = get_cheapest_attacker(&board2, to, &attacker_pos, color^1);
        if (i == 10){
            return true;    //if opponent had no attackers, we just took a free piece!
        }

        totalgain -= risk;      //first we subtract the value of the piece that you just had taken (since this is from opponent's point of view). in our RXB case, that was a bishop, so we're now at -200.
        risk = SEEVALUES[i];    //if the bishop gets taken, he loses 300 (a minor piece).
        if (totalgain + risk < 0){  //if the piece he took with was low enough value that you can't get the material back by taking it (2 pawns for a bishop), return. not true here.
            return false;
        }
        board2.board[attacker_pos] = BLANK; //remove the piece to account for batteries. 
                                            //and yeah this fails to discovered checks, but a discovered check will quickly cause a beta cutoff anyways so no big deal.

        i = get_cheapest_attacker(&board2, to, &attacker_pos, color);  //now we look at it from our point of view.
        if (i == 10){   //if we have no attackers, it is losing, since the condition in the while loop guarantees at least equal if we do something like BxB.
            return false;
        }
        totalgain += risk;  //we were at -200 before, now we're at 100.
        risk = SEEVALUES[i];

        board2.board[attacker_pos] = BLANK;

    }
    //another example: RxB QxR PxQ
    //RxB is not winning in and of itself, so we get the smallest attacker which turns out to be Q.
    //this sets totalgain to -300, but risk to 900.
    //we can take it, and it's with a pawn. Total gain becomes 600, risk becomes 100, and we return true.
    return true;
}


int see(struct board_info *board, struct move mve, bool color, int threshold){

    if (mve.flags == 0xC){
        return 2000000 + 100*10;
    }

    int attacker = SEEVALUES[board->board[(mve.move>>8)]>>1], victim = SEEVALUES[board->board[(mve.move&0xFF)]>>1]; 

    int v = 2000000;
    if (victim - attacker < threshold && !static_exchange_evaluation(board, mve, color, threshold)){
        v = -v;
    }
    return v + victim*10 - attacker;

}

void selectionsort(struct list *list, int k, int t){
    int temp = k;
    int i = k; while (i < t){
        if (list[i].eval > list[temp].eval){
            temp = i;          
        }
        i++;
    }
    struct list tempmove = list[temp];
    list[temp] = list[k];
    list[k] = tempmove;
}

int movescore(struct board_info *board, struct list *list, int depth, bool color, char type, struct move lastmove, int movelen, int threshold){
    int i = 0; while (i < movelen){
        list[i].eval = 1000000;

        if (depth > 1 && lastmove.move != 0 && (board->board[lastmove.move&0xFF]>>1)-1 < 0){
            return 1;
        }

        if (type != 'n' && ismatch(TT[(CURRENTPOS) & (_mask)].bestmove, list[i].move)){

            if (TT[(CURRENTPOS) & (_mask)].bestmove.move == list[i].move.move){

                if (TT[(CURRENTPOS) & (_mask)].bestmove.flags == list[i].move.flags){

                    list[i].eval += 10000000;
                }
            }
        }    
            
        else if (list[i].move.flags == 7){
            list[i].eval += 5000000 + SEEVALUES[board->board[(list[i].move.move&0xFF)]>>1];
            }
        else if (board->board[list[i].move.move & 0xFF]){
            list[i].eval = see(board, list[i].move, color, threshold);
        }
        else if (ismatch(list[i].move, KILLERTABLE[depth][0])){
            list[i].eval += 199;
        }
        else if (ismatch(list[i].move, KILLERTABLE[depth][1])){
            list[i].eval += 198;
        }
        else if (depth > 1 && lastmove.move != 0 && ismatch(list[i].move, COUNTERMOVES[(board->board[lastmove.move&0xFF]>>1)-1][lastmove.move&0xFF])){
                                                    //the piece that the opponent moved     the square it is on

            list[i].eval += 197;
        }
        
        else{
                //list[i].eval = 0;

                list[i].eval = HISTORYTABLE[color][list[i].move.move>>8][list[i].move.move&0xFF];

            }
            
            i++;
        }
    return 0;
}


int quiesce(struct board_info *board, int alpha, int beta, int depth, int depthleft, bool color, bool incheck){
    if (depth > maxdepth){
        maxdepth = depth;
    }
    nodes++;
    if (depthleft <= 0){
        return incheck ? 0 : eval(board, color);
    }

    if (!((nodes) & (CHECKTIME))){
        float rightnow = ((float)(clock() - start_time))/CLOCKS_PER_SEC;
        if (rightnow > maximumtime || rightnow > coldturkey){ //you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
    }
    int evl = 0;
    char type; if (CURRENTPOS == TT[(CURRENTPOS) & (_mask)].zobrist_key){
        type = TT[(CURRENTPOS) & (_mask)].type;
        evl = TT[(CURRENTPOS) & (_mask)].eval;
    }
    else{
        type = 'n';
        evl = 0;
    }
    if (type != 'n'){
            if (type == 3){
                return evl;
            }
            else if (type == 2){ //a move that caused a beta cutoff
                if (evl >= beta){
                    //don't eval any further
                    return evl;
                }

            }
            else{ //a move that didn't raise alpha
                if (evl < alpha){
                    return evl;
                }
            }
        }
    long long unsigned int original_pos = CURRENTPOS;

    int stand_pat;
    if (incheck){
        stand_pat = -100000;
    }
    else{
        stand_pat = eval(board, color);
    }
    
    
    int bestscore = stand_pat;
    int futility = -100000;

    if (!incheck){

        if (stand_pat >= beta){
            return stand_pat;
        }
        if (stand_pat > alpha){
            alpha = stand_pat;
        }
        futility = stand_pat + 60;

    }

    int falpha = alpha;
    
    struct list list[LISTSIZE];
    int listlen = 0;

    if (incheck){
        listlen = movegen(board, list, color, true);
    }
    else{
        listlen = movegen(board, list, color, false);
    }


    if (movescore(board, list, 99, color, (type != 'n' && TT[CURRENTPOS & _mask].depth == 0) ? type : 'n', nullmove, listlen, -108)){
        printfull(board);
        exit(1);
    }

    struct move bestmove = nullmove;
    int i = 0;

    while (i < listlen){
        selectionsort(list, i, listlen);

        if (!incheck){
            if (list[i].eval < 1000200){
                break;
            }
            if (board->board[list[i].move.move & 0xFF] && futility + VALUES2[(board->board[list[i].move.move & 0xFF]>>1) - 1] <= alpha){
                    bestscore = MAX(bestscore, futility + VALUES2[(board->board[list[i].move.move & 0xFF]>>1) - 1]);
                    i++;
                    continue;
                }

        }
        struct board_info board2 = *board;

        if (move(&board2, list[i].move, color)){
            exit(1);
        }       

        if (isattacked(&board2, board2.kingpos[color], color^1)){
            CURRENTPOS = original_pos;
            i++;
            continue;
        }

        list[i].eval = -quiesce(&board2, -beta, -alpha, depth+1, depthleft-1, color^1, isattacked(board, board->kingpos[color^1], color));
        
       
        
        if (abs(list[i].eval) == TIMEOUT){
            CURRENTPOS = original_pos;
            return TIMEOUT;
        }
        if (list[i].eval > bestscore){
            bestmove = list[i].move;
            bestscore = list[i].eval;
        }
        if (list[i].eval >= beta){
            CURRENTPOS = original_pos;
            insert(original_pos, 0, list[i].eval, 2, list[i].move);
            return list[i].eval;
        }
        if (list[i].eval > alpha){
            alpha = list[i].eval;
        }
        CURRENTPOS = original_pos;
        i++;
    }
    
    if (incheck && bestscore == -100000){
        return -100000;
    }
    if (falpha != alpha){
        insert(original_pos, 0, bestscore, 3, bestmove);
    }
    else{
        insert(original_pos, 0, bestscore, 1, bestmove);
    }
    return bestscore;
}


int alphabeta(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depthleft, int depth, bool color, bool isnull, bool incheck){
    nodes++;
    if (depth == 0){
        maxdepth = 0;
    }
    if (!((nodes) & (CHECKTIME))){
        float rightnow = ((float)(clock() - start_time))/CLOCKS_PER_SEC;
        if (rightnow > maximumtime || rightnow > coldturkey){ //you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
    }
    if (depth > 0){
        if (checkdraw2(movelst, key) > 0 || checkdraw1(board)){
            return (nodes & 3) - 2;
        }
        int mate_distance = 100000 - depth;
        if (mate_distance < beta){
            beta = mate_distance;
            if (alpha >= beta){
                return beta;
            }
        }
    }
    int evl;

    char type; if (CURRENTPOS == TT[(CURRENTPOS) & (_mask)].zobrist_key){
        type = TT[(CURRENTPOS) & (_mask)].type;
            evl = TT[(CURRENTPOS) & (_mask)].eval;
    }
    else{
        type = 'n';
        evl = -1024;
    }
    bool ispv = (beta != alpha+1);
    if (!ispv && type != 'n' && TT[(CURRENTPOS) & (_mask)].depth >= depthleft){
            if (type == 3){
                return evl;
            }
            else if (type == 2){ //a move that caused a beta cutoff
                if (evl >= beta){
                    //don't eval any further
                    return evl;
                }

            }
            else{ //a move that didn't raise alpha
                if (evl < alpha){
                    return evl;
                }
            }
        }

    if (depthleft <= 0){
        int b = quiesce(board, alpha, beta, depth, 15, color, incheck);      
        if (b == -100000){
            b += depth;
        } 
        if (b == 100000){
            b -= depth;
        }
        return b;  
    } 
    if (incheck){
        evl = -1000000;
    }
    else{
        evl = eval(board, color);
    }
    movelst[*key-1].staticeval = evl;

    bool improving = (depth > 1 && !incheck && movelst[*key-1].staticeval > movelst[*key-3].staticeval);

    if (type != 'n'){
        evl = TT[(CURRENTPOS) & (_mask)].eval;
    }

    if (!ispv && !incheck && depthleft < 9 && evl - ((depthleft-improving)*80) >= beta){
        return evl;
    }

    if (isnull == false && !ispv && !incheck && depthleft > 2 && 
    (   evl >= beta)){

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
                unsigned long long int a = CURRENTPOS;
                struct board_info board2 = *board;
                board2.epsquare = 0;
                if (board->epsquare){
                    CURRENTPOS ^= ZOBRISTTABLE[773];
                }
                CURRENTPOS ^= ZOBRISTTABLE[772];
                move_add(&board2, movelst, key, nullmove, color, false);
                int R = 4 + (depthleft/6) + MIN((evl-beta)/200, 3);
                int nm = -alphabeta(&board2, movelst, key, -beta, -beta+1, depthleft-R, depth+1, color^1, true, false);

                CURRENTPOS = a;

                movelst[*key-1].move = nullmove;
                *key = *key-1;
                if (abs(nm) == TIMEOUT){
                    return TIMEOUT;
                }
                
                if (nm >= beta){
                    return evl;
                }
            }

    }

    struct list list[LISTSIZE];

    bool ismove = false;
    int betacount = 0;
    int movelen = movegen(board, list, color, incheck);
    if (movescore(board, list, depth, color, type, depth > 1 ? movelst[*key-1].move : nullmove, movelen, 0)){
        printfull(board);
        printf("%i\n", depth);
        for (int i = 0; i < *key; i++){
            printf("%x\n", movelst[i].move.move);
        }
        exit(1);
    }

    if (ispv && type == 'n' && depthleft > 3){
        alphabeta(board, movelst, key, alpha, beta, depthleft-2, depth, color, false, incheck);
        type = TT[CURRENTPOS & _mask].type;
    }

    int i = 0;
    unsigned long long int original_pos = CURRENTPOS;
    bool raisedalpha = false;
    if (depth == 0){
        currentmove.move = 0;
    }
    struct move bestmove = nullmove;
    int futility_move_count = (3+depthleft*depthleft/(1+(!improving)));
    int numquiets = 0;
    bool quietsprune = false;
    int bestscore = -100000;

    while (i < movelen){     

        selectionsort(list, i, movelen);
        bool iscap = (list[i].move.flags == 0xC || board->board[list[i].move.move & 0xFF]);    
        if (quietsprune && !iscap){
            i++;
            continue;
        }
        struct board_info board2 = *board;

        if (move(&board2, list[i].move, color)){
            printfull(board);
            for (int b = 0; b < *key; b++){
                char a[6];
                printf("%s *%x ", conv(movelst[b].move, a), movelst[b].move.flags);
            }
            printf("\n");
            exit(0);
        }      
        if (isattacked(&board2, board2.kingpos[color], color^1)){
            CURRENTPOS = original_pos;
            i++;
            continue;
        }
        if (!ismove){
            bestmove = list[i].move;
        }
        ismove = true;

        if (depth > 0 && !iscap && !ispv){
            if (depthleft < 4){
            numquiets++;
            if (numquiets >= futility_move_count && depth > 0){
                quietsprune = true;
            }
            }

            if ((depthleft < 6 && list[i].eval < 1000200 && movelst[*key-1].staticeval + 90*(depthleft) + (improving*40) < alpha)){
                quietsprune = true;
            }

        }
        if (depth && list[i].eval < 1000200 && bestscore > -50000 && depthleft < 9 && 
            !static_exchange_evaluation(board, list[i].move, color, depthleft * ((iscap || list[i].move.flags>>2 == 1) ? -90 : -50))){
                CURRENTPOS = original_pos;
                i++;
                continue;
        }
        bool ischeck = isattacked(&board2, board2.kingpos[color^1], color);
        move_add(&board2, movelst, key, list[i].move, color, iscap);

        if (ispv == true && !betacount){
                list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depthleft-1, depth+1, color^1, false, ischeck);
                if (abs(list[i].eval) == TIMEOUT){
                movelst[*key-1].move = nullmove;
                *key = *key-1;
                CURRENTPOS = original_pos;
        
                return TIMEOUT;
                }
            }

        else{
            int R;
            if (list[i].eval > 1000200 || depthleft < 3){
                R = 0;
            }

            else{
                R = LMRTABLE[depthleft-1][betacount];
                if (iscap){
                    R >>= 1;
                }
                if (ischeck || incheck || list[i].eval > 1000190){
                    R--;
                }
                if (list[i].eval > 1000190){
                    R--;
                }
                if (!ispv && type != 3){
                    R++;
                }
                if (improving){
                    R--;
                }
                if (type != 'n' && (list[0].move.flags == 0xC || board->board[list[0].move.move & 0xFF])){
                    R++;
                }
                    R = MAX(R, 0);
                
            }

            

            list[i].eval = -alphabeta(&board2, movelst, key, -alpha-1, -alpha, depthleft-1-R, depth+1, color^1, false, ischeck);
                if (abs(list[i].eval) == TIMEOUT){
                                movelst[*key-1].move = nullmove;
            *key = *key-1;
            CURRENTPOS = original_pos;
    
            return TIMEOUT;
                }

            if (list[i].eval > alpha && R > 0){
                list[i].eval = -alphabeta(&board2, movelst, key, -alpha-1, -alpha, depthleft-1, depth+1, color^1, false, ischeck);
                if (abs(list[i].eval) == TIMEOUT){
                                movelst[*key-1].move = nullmove;
            *key = *key-1;
            CURRENTPOS = original_pos;
    
            return TIMEOUT;
                }
            }

            if (list[i].eval > alpha && ispv){

                    list[i].eval = -alphabeta(&board2, movelst, key, -beta, -alpha, depthleft-1, depth+1, color^1, false, ischeck);
                        if (abs(list[i].eval) == TIMEOUT){
                                movelst[*key-1].move = nullmove;
            *key = *key-1;
            CURRENTPOS = original_pos;
    
            return TIMEOUT;
                }
                }
        }

        if (list[i].eval > bestscore){
            bestmove = list[i].move;  
            bestscore = list[i].eval;
        }
        if (list[i].eval >= beta){
            if (depth == 0){
                currentmove = list[i].move;
            }
            bestmove = list[i].move;
            insert(original_pos, depthleft, bestscore, 2, bestmove);
            total++;
            betas += betacount+1;

            int c = depthleft*depthleft+depthleft-1;
            if (!iscap){
                if (!ismatch(KILLERTABLE[depth][0], list[i].move)){
                    KILLERTABLE[depth][0] = list[i].move;
                }
                else if (!ismatch(KILLERTABLE[depth][1], list[i].move)){
                    KILLERTABLE[depth][1] = list[i].move;
                }

                    //printf("%s\n", conv(list[i].move, b));
                    HISTORYTABLE[color][(list[i].move.move>>8)][list[i].move.move&0xFF] += c;

                    if (HISTORYTABLE[color][(list[i].move.move>>8)][(list[i].move.move&0xFF)] > 1000000){
                        for (int a = 0; a < 0x80; a++){
                            for (int b = 0; b < 0x80; b++){
                                HISTORYTABLE[color][a][b] = (HISTORYTABLE[color][a][b]>>1);
                            }
                        }
                    }

                    for (int a = 0; a < i; a++){

                        if (!(list[a].move.flags == 0xC || board->board[list[a].move.move & 0xFF])){

                            HISTORYTABLE[color][(list[a].move.move>>8)][list[a].move.move&0xFF] -= c;


                            if (HISTORYTABLE[color][(list[a].move.move>>8)][(list[a].move.move&0xFF)] < -1000000){
                                for (int c = 0; c < 0x80; c++){
                                    for (int b = 0; b < 0x80; b++){
                                        HISTORYTABLE[color][c][b] = (HISTORYTABLE[color][c][b]>>1);
                                    }
                                }
                            }


                        }
                    }
            }

        if (depth > 1 && !isnull && !iscap){  //key-1 is the move you made, key-2 is the move the opponent made

                COUNTERMOVES[(board->board[movelst[(*key)-2].move.move&0xFF]>>1)-1][movelst[(*key)-2].move.move&0xFF] = list[i].move;
            }
            movelst[(*key)-1].move.flags = 0;
            *key = *key-1;
            CURRENTPOS = original_pos;
            return beta;
        }
        
        movelst[*key-1].move = nullmove;
        *key = *key-1;
        if (list[i].eval > alpha){
            if (depth == 0){
                currentmove = list[i].move;
            }
            raisedalpha = true;
            alpha = list[i].eval;     

        }

        else if (!betacount && depth == 0){
            insert(original_pos, depthleft, list[i].eval, 1, list[i].move);
            CURRENTPOS = original_pos;
            return list[i].eval;
        }

        CURRENTPOS = original_pos;
        betacount++;
        i++;
    }

    if (!ismove){
        if (incheck){
            return -100000 + depth;
        }
        else{
            return 0;
        }
    }
    //printfull(board);
    if (raisedalpha){
        //if (alpha != 0){
        insert(original_pos, depthleft, alpha, 3, bestmove);
        //}
    }
    else{

        insert(original_pos, depthleft, bestscore, 1, bestmove);
        //}
    }

    return bestscore;
}

bool verifypv(struct board_info *board, struct move pvmove, bool incheck, bool color){
    struct list list[LISTSIZE];
    int movelen = movegen(board, list, color, incheck);
    for (int i = 0; i < movelen; i++){
        if (ismatch(pvmove, list[i].move)){
            unsigned long long int c = CURRENTPOS;
            struct board_info board2 = *board;
            move(&board2, pvmove, color);
            CURRENTPOS = c;
            if (isattacked(&board2, board2.kingpos[color], color^1)){
                return false;
            }
            return true;
        }
    }
    return false;
}


float iid_time(struct board_info *board, struct movelist *movelst, float maxtime, int *key, bool color, bool ismove){
    /*if (*key < 5 && opening_book(board, movelst, key, color)){
        return 1;
    }*/
    nodes = 0;    
    maximumtime = maxtime*2;
    //printf("%f\n", maximumtime);
    //clearTT(false);
    start_time = clock();
    clearHistory(false);
    clearKiller();
    currentmove.move = 0;
    int alpha = -1000000, beta = 1000000;   
    bool incheck = isattacked(board, board->kingpos[color], color^1);
    int g = 0;
    int depth;
    struct move pvmove;
    //printf("%i %s %s\n", *key, movelst[*key-1].fen, movelst[*key-1].move);
    for (depth = 1; ; depth++){
        int delta = 12;
        int tempdepth = depth;    
        int asphigh = 25, asplow = 25;       
        int evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color, false, incheck); 

        while (abs(evl) != TIMEOUT && (evl <= alpha || evl >= beta)){
            if (evl <= alpha){
                char temp[6];
                    printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv %s\n", depth, maxdepth, alpha, nodes, (long int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC, conv(pvmove, temp));
                alpha -= delta;
                beta = (alpha + 3 * beta) / 4;         
                delta += delta * 2 / 3;
                evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color, false, incheck);

                        if (abs(evl) == TIMEOUT){
            if (currentmove.move == 0){
                currentmove = pvmove;
                depth--;
            }
            break;
        }   
                
            }
            else if (evl >= beta){
                char temp[6];
                printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv %s\n", depth, maxdepth, beta, nodes, (long int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC, conv(currentmove, temp));
                pvmove = currentmove;
                beta += delta;
                delta += delta * 2 / 3;
                tempdepth = MAX(tempdepth-1, depth-3);
                evl = alphabeta(board, movelst, key, alpha, beta, tempdepth, 0, color, false, incheck);   
            if (abs(evl) == TIMEOUT){
                currentmove = pvmove;
                break;
            }         


            }
        }
        if (abs(evl) == TIMEOUT){
                if (currentmove.move == 0){
                    currentmove = pvmove;
                    depth--;
                }
                break;
                }

        clock_t time2 = clock()-start_time;
        g = evl;  
        pvmove = currentmove;

        if (g > 99900){
            printf("info depth %i seldepth %i score mate %i nodes %lu time %li pv ", depth, maxdepth, (100001-g)/2, nodes, (long int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC);
        }
        else if (g < -99900){
            printf("info depth %i seldepth %i score mate %i nodes %lu time %li pv ", depth, maxdepth, (-100001-g)/2, nodes, (long int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC);
        }
        else{
            printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv ", depth, maxdepth, g, nodes, (long int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC);
        }
                
        int d = depth;
        unsigned long long int op = CURRENTPOS;
        struct board_info board2 = *board; 
        bool c = color;
        while (d > 0){
            if (TT[CURRENTPOS & _mask].zobrist_key != CURRENTPOS || ismatch(TT[CURRENTPOS & _mask].bestmove, nullmove) ||
             !verifypv(&board2, TT[CURRENTPOS & _mask].bestmove, false, c)){
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

        if ((float)time2/CLOCKS_PER_SEC > maxtime*0.6 || depth >= MAXDEPTH){                      
            break;
        }
         if (depth > 6){
            alpha = evl-12;
            beta = evl+12;
        }   
    
    }
    char temp[8], temp2[8];
    printf("bestmove %s\n", conv(currentmove, temp));
    
    

    if (ismove){
        bool iscap = (currentmove.flags == 0xC || board->board[currentmove.move & 0xFF]);
        move(board, currentmove, color);
        move_add(board, movelst, key, currentmove, color, iscap);

    } 
    search_age++;
    return g;
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

void move_uci(char *command, int i, struct board_info *board, struct movelist *movelst, int *key, bool *color){
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
            struct move temp; convto(buf, &temp, board);
            bool iscap = (temp.flags == 0xC || board->board[temp.move & 0xFF]); 
            move(board, temp, *color);

            move_add(board, movelst, key, temp, *color, iscap);

            

            *color ^= 1;            
        }
}

int com_uci( struct board_info *board, struct movelist *movelst, int *key, bool *color) { //assumes board+movelist have been already set up under wraps

    char command[65536];
    getsafe(command, sizeof(command));
    if (command[0] == '\0'){
        return 0;
    }
    if (!strcmp(command, "stop")){
        return 1;
    }
    
  if (!strcmp(command, "uci")) {
    printf("id name Willow 2.8\n");
    printf("id author Adam Kulju\n");
    // send options
    printf("option name Hash type spin default 32 min 1 max 1028\n");
    printf("option name Threads type spin default 1 min 1 max 1\n");

    printf("uciok\n");
    
  }
    if (!strcmp(command, "isready")){
        printf("readyok\n");
    }
    if (!strcmp(command, "ucinewgame")) {
        clearTT();
        clearKiller();
        clearCounters();
        clearHistory(true);
        setfull(board);
        setmovelist(movelst, key); 
        search_age = 0;
        }

    if (!strcmp(command, "quit")){
        free(TT);
        exit(0);
    }

    if (strstr(command, "setoption name Hash value")){
        int a = atoi(&command[26]);
        int target = a * 1024 * 1024;
        int size = 0; while (sizeof(struct ttentry) * (1<<size) < target){
            size++;
        }
        size--;
        if (TT){
            free(TT);
        }
        TT = (struct ttentry *) malloc(sizeof(struct ttentry) * (1<<size));
        TTSIZE = 1<<size;
        _mask = TTSIZE - 1;
    }

    if (strstr(command, "position startpos")){
        *color = WHITE;
        setfull(board);
        setmovelist(movelst, key); 
        calc_pos(board, WHITE);
        *key = 1;
        if (strstr(command, "moves")){
            move_uci(command, 24, board, movelst, key, color);
        }


    }
    if (strstr(command, ("position fen"))){
        setfromfen(board, movelst, key, command, color, 13);
        if (strstr(command, "moves")){
            int i = 20; while (command[i] != 's'){
                i++;
            }
            i++;
            move_uci(command, i, board, movelst, key, color);
        }
    }
    if (strstr(command, "go")){
        float time = 100;
        if (strstr(command, "infinite")){
            time = 1000000;
            coldturkey = 1000000;
        }
        else if (strstr(command, "movetime")){
            time = atoi(command+12)/1000;
            coldturkey = time;
        }

        else if (strstr(command, "wtime")){ //this will be for a game
        int k = 0;
        int movestogo = -1;
        if (strstr(command, "movestogo")){
            int m = 0;
            while (command[m] != 'm' || command[m+1] != 'o' || command[m+2] != 'v' || command[m+3] != 'e' || command[m+4] != 's'){
                m++;
            }
            while (!isdigit(command[m])){
                m++;
            }
            movestogo = atoi(&command[m]);
            while (command[k] != 'w'){
                k++;
            }
            k += 6;
        }          
        else{
            k = 9;
        }
  
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

            int milltime = atoi(&command[k]) - 50;
            if (milltime < 1){
                time = 0.001; coldturkey = -0.001;
            }
            else{
            coldturkey = (float)milltime/1000;

            if (movestogo != -1){
                time = ((float)milltime/(1000*(movestogo+2))) * 1.5;
            }
            else{
                int movesleft = MAX(20, 70-(*key/2));
                time = ((float)milltime/(1000*movesleft)) * 1.5;
            }

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
                if (time + ((float)milltime/1000 * 4) < coldturkey){ //if you have at least four increments left over, it's safe to add half the increment to your move.
                time += (float)milltime/1000 * 0.5;
                }
            }
            }

            
        }

        time = MAX(time, 0.001);
        printf("%f %f\n", coldturkey, time);
        iid_time(board, movelst, time, key, *color, false);
    }
    //fflush(hstdin);
    return 0;
}


int bench(){
    MAXDEPTH = 14;
    char positions[50][1024] = {
            {"2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 42\0"},
        	{"2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 55\0"},
            {"6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 1 44\0"},
            {"6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 1 36\0"},
        	{"4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 2 24\0"},
            {"3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 23\0"},
            {"r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 14\0"},
		    {"r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 16 42\0"},
		    {"6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 4 44\0"},
		    {"8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 3 54\0"},
		    {"7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 36\0"},
		    {"r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 2 10\0"},
		    {"3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 3 87\0"},		    
		    {"4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 37\0"},
		    {"2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 2 34\0"},
		    {"1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 1 37\0"},
		    {"r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 17\0"},
		    {"8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 16 42\0"},
		    {"1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 8 29\0"},
		    {"8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 29 68\0"},
		    {"3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 2 20\0"},
		    {"5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 1 49\0"},
		    {"1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 1 51\0"},
		    {"q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 2 34\0"},
		    {"r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 22\0"},
		    {"r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 5\0"},
		    {"r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 1 12\0"},
		    {"r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 2 19\0"},
		    {"r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 2 25\0"},
		    {"r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 32\0"},
		    {"r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 1 5\0"},
		    {"3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 12\0"},
		    {"5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 2 20\0"},
		    {"8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 3 27\0"},
		    {"8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 1 33\0"},
		    {"8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 1 38\0"},
		    {"8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 15 45\0"},
		    {"8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 5 49\0"},
		    {"8/8/1p4p1/p1p2k1p/P2npP1P/4K1P1/1P6/3R4 w - - 6 54\0"},
		    {"8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 6 59\0"},
		    {"8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 14 63\0"},
		    {"8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 67\0"},
		    {"1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 4 23\0"},
		    {"4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 3 22\0"},
		    {"r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 4 12\0"},
		    {"2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 3 25\0"},
		    {"r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 2 14\0"},
		    {"rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2\0"},
		    {"2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 20\0"},
		    {"2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 23 93\0"}
    };
    unsigned long int t = 0;
    clock_t start = clock();
    int target = 32 * 1024 * 1024;
    int size = 0; while (sizeof(struct ttentry) * (1<<size) < target){
        size++;
    }
    TT = (struct ttentry *) malloc(sizeof(struct ttentry) * (1<<size));
    TTSIZE = 1<<size;
    _mask = TTSIZE - 1;
    for (int i = 0; i < 50; i++){

        clearTT();
        clearKiller();
        clearCounters();
        clearHistory(true);
        search_age = 0;

        
        struct board_info board;
        struct movelist movelst[MOVESIZE];
        int key;
        bool color;
        setfromfen(&board, movelst, &key, positions[i], &color, 0);
        
        printfull(&board);

        iid_time(&board, movelst, 1000000, &key, color, false);
        t += nodes;
    }
    printf("Bench: %lu nodes %i nps\n", t, (int)(t / ((clock()-start)/(float)CLOCKS_PER_SEC)) );
    return 1;
}

int com() {
    struct board_info board;
    setfull(&board);
    struct movelist movelst[MOVESIZE];
    int key;
    calc_pos(&board, WHITE);
    setmovelist(movelst, &key); 
    bool color;
    
    for (;;){
            com_uci(&board, movelst, &key, &color);
        }
    return 0;
}

int init(){

        setvbuf(stdin,NULL,_IONBF,0);
        setvbuf(stdout,NULL,_IONBF,0);
        MAXDEPTH = 99;
    initglobals();
    unsigned long long init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    init_by_array64(init, 4);
    setzobrist();    
    return 0;
}

int main(int argc, char *argv[]){

    init();

    printf("%i\n", argc);
    if (argc > 1 && !strcmp(argv[1], "bench")){
        bench();
        printf("%f\n", (float)betas/total);
        exit(0);
    }
    
    com();
    return 0;
} 
