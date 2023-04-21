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
#define TTSIZE 1 << 20
#define _mask (1 << 20) - 1
#define CHECKTIME (1 << 12)-1
#define TIMEOUT 111111
#define TEMPO 1
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
int pvptr = 0;
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
struct ttentry TT[TTSIZE];

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
unsigned long int HISTORYTABLE[2][0x80][0x80]; //allows for faster lookups
struct move pvstack[10000];

unsigned long int nodes;
long int totals;
int betas, total;
 const int VALUES[5] = {80,  354,  344,  471,  969};
 const int VALUES2[5] = {70,  264,  273,  476,  953};
short int pstbonusesm[6][0x80] = {
    {
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
 -25,   -1,  -15,    5,    9,   41,   34,  -10,    0,    0,    0,    0,    0,    0,    0,    0, 
 -21,   -9,   -1,    3,   18,   15,   27,   -6,    0,    0,    0,    0,    0,    0,    0,    0, 
 -27,  -12,   -5,    6,   12,   12,   -2,  -22,    0,    0,    0,    0,    0,    0,    0,    0, 
 -20,    0,   -4,    8,    8,   12,    5,  -20,    0,    0,    0,    0,    0,    0,    0,    0, 
  -2,    9,   20,   19,   61,   85,   36,   -7,    0,    0,    0,    0,    0,    0,    0,    0, 
  45,   92,   20,   52,   34,  108,   13,  -31,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
-101,   -5,  -40,  -18,    5,  -10,   -1,  -10,    0,    0,    0,    0,    0,    0,    0,    0, 
 -16,  -48,  -10,   16,   16,   21,   -8,    3,    0,    0,    0,    0,    0,    0,    0,    0, 
 -18,   -9,   11,    8,   22,   20,   27,   -8,    0,    0,    0,    0,    0,    0,    0,    0, 
  -7,    2,   10,   12,   24,   22,   23,    1,    0,    0,    0,    0,    0,    0,    0,    0, 
 -15,   13,    1,   44,   26,   53,   18,   23,    0,    0,    0,    0,    0,    0,    0,    0, 
 -45,   41,   13,   44,   67,   98,   64,   41,    0,    0,    0,    0,    0,    0,    0,    0, 
 -80,  -62,   63,   11,    5,   43,  -11,  -18,    0,    0,    0,    0,    0,    0,    0,    0, 
-167,  -93,  -68,  -46,  -25,  -96,  -42, -103,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
 -23,   20,   10,    7,   13,    9,  -23,   -8,    0,    0,    0,    0,    0,    0,    0,    0, 
  18,   31,   21,   15,   24,   30,   48,   12,    0,    0,    0,    0,    0,    0,    0,    0, 
   8,   21,   23,   19,   20,   38,   21,   16,    0,    0,    0,    0,    0,    0,    0,    0, 
   3,   17,   15,   24,   36,    7,    9,   13,    0,    0,    0,    0,    0,    0,    0,    0, 
  -6,   13,   10,   45,   27,   25,   15,    1,    0,    0,    0,    0,    0,    0,    0,    0, 
 -21,   24,   27,   28,   22,   46,   32,   -2,    0,    0,    0,    0,    0,    0,    0,    0, 
 -33,    3,  -24,  -34,   19,   27,   -1,  -44,    0,    0,    0,    0,    0,    0,    0,    0, 
 -23,  -21, -104,  -76,  -39,  -59,   -8,  -12,    0,    0,    0,    0,    0,    0,    0,    0
    },

    {
  -3,    5,   21,   30,   36,   24,   -7,    1,    0,    0,    0,    0,    0,    0,    0,    0, 
 -31,   -8,   -8,    7,   21,   24,    8,  -47,    0,    0,    0,    0,    0,    0,    0,    0, 
 -33,  -16,   -2,   -7,   15,   16,   12,  -19,    0,    0,    0,    0,    0,    0,    0,    0, 
 -39,  -26,   -6,   -3,   10,    0,   14,  -25,    0,    0,    0,    0,    0,    0,    0,    0, 
 -28,  -14,    7,   24,   21,   26,   -1,  -18,    0,    0,    0,    0,    0,    0,    0,    0, 
 -20,    6,   20,   24,   -2,   40,   58,    2,    0,    0,    0,    0,    0,    0,    0,    0, 
  11,   12,   44,   46,   66,   67,   17,   32,    0,    0,    0,    0,    0,    0,    0,    0, 
   9,   30,   -9,   35,   41,   -4,   10,   12,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
  11,   11,   17,   28,   13,   -4,   -8,  -41,    0,    0,    0,    0,    0,    0,    0,    0, 
 -19,    5,   23,   25,   35,   33,   13,   16,    0,    0,    0,    0,    0,    0,    0,    0, 
 -15,   14,    1,   10,    8,   12,   20,    7,    0,    0,    0,    0,    0,    0,    0,    0, 
   2,  -31,    1,   -7,    2,    3,    6,    1,    0,    0,    0,    0,    0,    0,    0,    0, 
 -37,  -21,  -18,  -28,  -16,   -2,   -9,   -5,    0,    0,    0,    0,    0,    0,    0,    0, 
 -12,  -20,   -1,  -18,   10,   35,   17,   29,    0,    0,    0,    0,    0,    0,    0,    0, 
 -23,  -52,  -17,   -3,  -35,   35,   -5,   44,    0,    0,    0,    0,    0,    0,    0,    0, 
 -35,  -25,    6,  -25,   46,   33,   20,   30,    0,    0,    0,    0,    0,    0,    0,    0

    },
    {
 -21,   51,   21,  -69,    0,  -31,   41,   23,    0,    0,    0,    0,    0,    0,    0,    0, 
   3,   20,  -20,  -69,  -46,  -22,   18,   20,    0,    0,    0,    0,    0,    0,    0,    0, 
 -22,    8,  -21,  -41,  -50,  -36,   -8,  -40,    0,    0,    0,    0,    0,    0,    0,    0, 
 -59,   10,  -22,  -65,  -74,  -46,  -51,  -98,    0,    0,    0,    0,    0,    0,    0,    0, 
 -22,  -13,    2,  -20,  -27,  -25,  -12,  -77,    0,    0,    0,    0,    0,    0,    0,    0, 
  -8,   28,   29,   -9,   -2,   24,   50,  -35,    0,    0,    0,    0,    0,    0,    0,    0, 
  28,   12,  -13,   12,   -8,  -10,  -41,  -42,    0,    0,    0,    0,    0,    0,    0,    0, 
 -68,   27,   18,  -15,  -64,  -45,    3,   12,    0,    0,    0,    0,    0,    0,    0,    0
    }

};


short int pstbonusese[6][0x80] = {
    {
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
  29,   20,   25,   15,   21,   10,    0,    2,    0,    0,    0,    0,    0,    0,    0,    0, 
  23,   23,   13,   14,   15,   12,    1,    4,    0,    0,    0,    0,    0,    0,    0,    0, 
  33,   29,   17,   11,   10,   10,   12,   12,    0,    0,    0,    0,    0,    0,    0,    0, 
  45,   34,   24,    7,   13,   15,   24,   27,    0,    0,    0,    0,    0,    0,    0,    0, 
  67,   61,   36,    9,   -8,    6,   34,   69,    0,    0,    0,    0,    0,    0,    0,    0, 
 122,  108,  102,   73,   78,   60,   97,  157,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
 -12,  -33,  -17,   -8,  -20,  -19,  -37,  -46,    0,    0,    0,    0,    0,    0,    0,    0, 
 -39,  -14,  -14,  -16,  -11,  -24,  -20,  -45,    0,    0,    0,    0,    0,    0,    0,    0, 
 -25,   -8,   -6,   12,    6,   -9,  -29,  -22,    0,    0,    0,    0,    0,    0,    0,    0, 
 -13,  -11,   17,   22,   13,   10,   -5,  -13,    0,    0,    0,    0,    0,    0,    0,    0, 
 -10,   -2,   27,   17,   20,    8,   -1,  -19,    0,    0,    0,    0,    0,    0,    0,    0, 
 -24,  -22,   20,   10,   -9,  -11,  -28,  -45,    0,    0,    0,    0,    0,    0,    0,    0, 
 -15,   -2,  -35,   -6,  -14,  -35,  -26,  -48,    0,    0,    0,    0,    0,    0,    0,    0, 
 -30,  -33,  -10,  -31,  -16,  -27,  -52,  -81,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
  -7,   -6,   -4,   -5,   -6,   -7,   -1,   -7,    0,    0,    0,    0,    0,    0,    0,    0, 
 -13,  -20,  -11,   -7,   -6,  -13,  -20,  -24,    0,    0,    0,    0,    0,    0,    0,    0, 
 -10,   -4,    4,    5,    9,   -6,   -6,  -11,    0,    0,    0,    0,    0,    0,    0,    0, 
 -11,   -5,    9,   16,    1,    4,  -12,   -7,    0,    0,    0,    0,    0,    0,    0,    0, 
  -2,    1,    7,    6,   10,    4,   -9,   -1,    0,    0,    0,    0,    0,    0,    0,    0, 
   5,   -9,   -3,   -5,   -6,   -4,   -7,    3,    0,    0,    0,    0,    0,    0,    0,    0, 
   3,   -5,    3,  -12,   -9,  -11,   -3,   -4,    0,    0,    0,    0,    0,    0,    0,    0, 
  -9,  -15,   -5,   -5,   -5,   -6,  -14,  -22,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
  -3,    1,   -1,   -6,  -12,  -10,   -2,  -21,    0,    0,    0,    0,    0,    0,    0,    0, 
   2,   -2,    1,    0,  -13,  -17,  -14,    4,    0,    0,    0,    0,    0,    0,    0,    0, 
   4,    5,   -4,    0,  -11,  -15,  -13,   -7,    0,    0,    0,    0,    0,    0,    0,    0, 
  12,   12,    8,    4,   -5,   -7,  -12,   -2,    0,    0,    0,    0,    0,    0,    0,    0, 
  14,    8,   11,   -3,   -2,   -2,   -7,    9,    0,    0,    0,    0,    0,    0,    0,    0, 
  12,    9,    2,    3,    3,  -11,  -14,   -3,    0,    0,    0,    0,    0,    0,    0,    0, 
  10,   11,    4,    2,  -17,   -8,    5,    1,    0,    0,    0,    0,    0,    0,    0,    0, 
  14,    6,   21,    9,    6,   11,    4,    6,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
 -24,  -34,  -14,  -22,    1,  -34,  -11,  -26,    0,    0,    0,    0,    0,    0,    0,    0, 
 -10,  -12,  -24,  -23,  -20,  -25,  -44,  -43,    0,    0,    0,    0,    0,    0,    0,    0, 
   7,  -27,   30,   12,   21,   17,    5,    5,    0,    0,    0,    0,    0,    0,    0,    0, 
  -6,   49,   30,   58,   36,   20,   19,   10,    0,    0,    0,    0,    0,    0,    0,    0, 
  32,   35,   30,   62,   63,   24,   41,   22,    0,    0,    0,    0,    0,    0,    0,    0, 
  -5,   17,   16,   51,   48,    6,    1,  -14,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,   39,   41,   50,   59,   12,   24,   -7,    0,    0,    0,    0,    0,    0,    0,    0, 
  -8,   28,   13,   26,   -7,   -1,  -22,  -11,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
 -59,  -49,  -23,    2,  -22,   -6,  -37,  -63,    0,    0,    0,    0,    0,    0,    0,    0, 
 -33,  -12,   15,   26,   25,   15,   -4,  -26,    0,    0,    0,    0,    0,    0,    0,    0, 
 -19,   -3,   17,   28,   34,   26,   11,   -6,    0,    0,    0,    0,    0,    0,    0,    0, 
 -17,   -6,   25,   36,   39,   31,   18,    1,    0,    0,    0,    0,    0,    0,    0,    0, 
 -11,   19,   26,   27,   28,   35,   29,   10,    0,    0,    0,    0,    0,    0,    0,    0, 
   7,   10,   16,   13,   17,   40,   39,   11,    0,    0,    0,    0,    0,    0,    0,    0, 
 -21,    9,   16,   12,   14,   34,   33,   14,    0,    0,    0,    0,    0,    0,    0,    0, 
 -79,  -48,  -42,  -27,   -8,    8,    0,  -16,    0,    0,    0,    0,    0,    0,    0,    0
    }
};



short int bishop_pair[9] = {7,   41,   50,   47,   42,   41,   37,   42,  50};
short int passedmgbonus[8] = {4,    4,   -7,   -8,    8,   16,   50,    0};
short int passedegbonus[8] = {58,   10,   15,   35,   61,  125,  129,    0};


short int mobilitybonusesmg[3][15] = {
    {-23,   -8,   -1,    2,   11,   15,   21,   25,   43},
    {-26,  -13,   -2,    5,   11,   14,   19,   21,   23,   30,   42,   62,   37,   48},
    {-39,  -29,  -24,  -18,  -16,   -8,   -2,    5,   17,   21,   33,   42,   51,   50,   58}
};

short int mobilitybonuseseg[3][15] = {
    {-59,  -11,    7,   15,   17,   24,   20,   17,   -8},
    {-41,  -18,   -1,   11,   21,   27,   32,   31,   36,   30,   24,   17,   32,   17},
    {-23,    6,   27,   33,   43,   52,   58,   57,   57,   59,   59,   60,   61,   59,   55}
};
short int doubledpen = -14;
short int isopen =  -12;
short int backwardspen = -4;

char attacknums[4] = {2,2,3,5};
int king_attack_count[2];

short int kingdangertable[100] = {
   0,    0,    1,    2,    1,    8,   -2,   13,    8,   13, 
  11,   16,   12,    9,   17,   18,   25,   21,   23,   43, 
  48,   51,   82,   83,   66,   68,  109,  105,  113,  119, 
  85,  124,  151,  189,  175,  120,  126,  181,  234,  225, 
 221,  251,  224,  283,  290,  299,  316,  308,  324,  352, 
 374,  376,  374,  398,  408,  436,  439,  459,  465,  476, 
 494,  497,  496,  495,  498,  496,  496,  498,  500,  500, 
 500,  500,  498,  500,  500,  500,  500,  500,  500,  496, 
 500,  500,  496,  500,  500,  498,  500,  500,  500,  500, 
 500,  500,  500,  500,  500,  500,  500,  500,  500,  500
};
short int futility[4] = {125, 125, 300, 300};


char *getsafe(char *buffer, int count)
{
    	char *result = buffer, *np;
	if ((buffer == NULL) || (count < 1)){
        result =  NULL;
    }

	else if (count == 1)
		*result = '\0';

	else if ((result = fgets(buffer, count, stdin)) != NULL)
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


void clearPV(){
    int i; for (i = 0; i < 10000; i++){
        pvstack[i].move = 0;
        pvstack[i].flags = 0;
    }
    pvptr = 0;
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

int piece_mobility(struct board_info *board, unsigned char i, bool color, unsigned char piecetype){
    
    
    isattacker = false;
    unsigned char mobility = 0;
    for (unsigned char dir = 0; dir < vectors[piecetype]; dir++){
        for (int pos = i;;){
            pos += vector[piecetype][dir];

                if ((pos & 0x88) || (board->board[pos] && (board->board[pos]&1) == color)){break;}

                    if (piecetype != 3){ //queen
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
                moves = piece_mobility(board, i, piececolor, piecetype-1);
                if (piecetype != 4){ //queens
                    mobilitybonus = true;
                }
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


    
    int score = 0;
    
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
                int mg = passedmgbonus[wadvanced[7]], eg = passedegbonus[wadvanced[7]];
                 if (board->board[pos+NORTH]){mg >>=1; eg >>=1;}
                mgscore += mg;
                egscore += eg;
            }
            if (badvanced[7] != 9 && badvanced[7] <= wbackwards[7] && badvanced[7] <= wbackwards[6]){
                int pos = ((badvanced[7]<<4)) + 7;
                int mg = passedmgbonus[7-badvanced[7]], eg = passedegbonus[7-badvanced[7]];
                 if (board->board[pos+SOUTH]){mg>>=1; eg >>= 1;}
                mgscore -= mg;
                egscore -= eg;
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
                int mg = passedmgbonus[wadvanced[0]], eg = passedegbonus[wadvanced[0]];
                 if (board->board[pos+NORTH]){mg >>=1; eg >>=1;}
                mgscore += mg;
                egscore += eg;
            }
            if (badvanced[0] != 9 && badvanced[0] <= wbackwards[0] && badvanced[0] <= wbackwards[1]){
                int pos = ((badvanced[0]<<4));
                int mg = passedmgbonus[7-badvanced[0]], eg = passedegbonus[7-badvanced[0]];
                 if (board->board[pos+SOUTH]){mg>>=1; eg >>= 1;}
                mgscore -= mg;
                egscore -= eg;
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
                int mg = passedmgbonus[wadvanced[i]], eg = passedegbonus[wadvanced[i]];
                 if (board->board[pos+NORTH]){mg >>=1; eg >>=1;}
                mgscore += mg;
                egscore += eg;
            }
            if (badvanced[i] != 9 && badvanced[i] <= wbackwards[i-1] && badvanced[i] <= wbackwards[i] && badvanced[i] <= wbackwards[i+1]){
                int pos = ((badvanced[i]<<4)) + i;
                int mg = passedmgbonus[7-badvanced[i]], eg = passedegbonus[7-badvanced[i]];
                 if (board->board[pos+SOUTH]){mg>>=1; eg >>= 1;}
                mgscore -= mg;
                egscore -= eg;
            }
        }
    }
    
    score += (phase*mgscore + (24-phase)*egscore)/24;
    
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
        score += space;
    }
    int wk = 0;
    
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
        wk -= (kingdangertable[king_attack_count[BLACK]]*(8-pawnshield))/10;


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
        wk += (kingdangertable[king_attack_count[WHITE]]*(8-pawnshield))/10;


    }
    score += wk;


    return score;
}

int eval(struct board_info *board, bool color){
    attackers[0] = 0, attackers[1] = 0;
    king_attack_count[0] = 0, king_attack_count[1] = 0;
    int phase = 0;
    int evl = material(board, &phase);
    int mat = evl;

    evl += pst(board, phase);
    if (board->pnbrqcount[WHITE][0] <= 1 && mat >= 0 && mat < 400 && phase != 0){ //if White is up material, we want to stop it from trading pawns
    if (board->pnbrqcount[WHITE][0] == 0){
        evl >>= 2;
    }
    else{ evl >>= 1;}
    }
    if (board->pnbrqcount[BLACK][0] <= 1 && mat <= 0 && mat > -400 && phase != 0){
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


int see(struct board_info *board, struct move mve, bool color){

    int attacker, victim;

    switch(board->board[(mve.move>>8)]>>1){
        case PAWN:
        attacker = 1000; break;
        case KING:
        attacker = 9500; break;
        case ROOK:
        attacker = 5000; break;
        case QUEEN:
        attacker = 9000; break;
        default:
        attacker = 3000; break;
    }

    if (mve.flags == 0xC){
        return 1000;
    }

    switch(board->board[(mve.move&0xFF)]>>1){
        case PAWN:
        victim = 1000; break;
        case ROOK:
        victim = 5000; break;
        case QUEEN:
        victim = 9000;  break;
        case BLANK:
        return 0;
        default:
        victim = 3000; break;
    }

    char a = isattacked_mv(board, mve.move&0xFF, color^1);
    if (!a){
        return (victim-(attacker/1000));
    }
    if (a == 1){
        char temp = board->board[(mve.move>>8)];

        board->board[(mve.move>>8)] = BLANK;
        
        a = isattacked_mv(board, mve.move&0xFF, color);

        board->board[(mve.move>>8)] = temp;
        if (a != 2){ //this means that the opponent can't take that piece with his king. In this case the higher material the better.
            //printfull(board, color);
            //printf("%s\n", mve);
            return (victim+(attacker/1000));
        }
    }
    if (victim-attacker >= 0){
        return (victim-(attacker/100));
    }

    return 160+((victim-attacker)/1000);
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

int movescore(struct board_info *board, struct list *list, int depth, bool color, char type, struct move lastmove, int movelen){
    int i = 0; while (i < movelen){
        list[i].eval = 1000000;

        if (depth > 1 && lastmove.move != 0 && (board->board[lastmove.move&0xFF]>>1)-1 < 0){
            return 1;
        }

        if (type != 'n' && ismatch(TT[(CURRENTPOS) & (_mask)].bestmove, list[i].move)){

            if (TT[(CURRENTPOS) & (_mask)].bestmove.move == list[i].move.move){

                if (TT[(CURRENTPOS) & (_mask)].bestmove.flags == list[i].move.flags){

                    list[i].eval += 100000;
                }
            }
        }    
            
        else if (list[i].move.flags == 7){
            list[i].eval += (9900 + see(board, list[i].move, color));
        }
        else if (board->board[list[i].move.move & 0xFF]){
            list[i].eval += see(board, list[i].move, color);
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
    if (!((nodes) & (CHECKTIME))){
        clock_t rightnow = clock() - start_time;
        if ((float)rightnow/CLOCKS_PER_SEC > maximumtime || (float)rightnow/CLOCKS_PER_SEC > coldturkey){ //you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
    }
    long long unsigned int original_pos = CURRENTPOS;
    int stand_pat;
    if (!incheck || depthleft <= 0){
        stand_pat = eval(board, color);
    }
    else{
        stand_pat = -100000;
    }
    if (depthleft <= 0 || stand_pat == TIMEOUT){
        return stand_pat;
    }
    
    if (stand_pat >= beta && !incheck){
        return stand_pat;
    }

    if (stand_pat > alpha){
        alpha = stand_pat;
    }
    if (stand_pat + 1125 < alpha && !incheck){
        return stand_pat;
    }
    struct list list[LISTSIZE];
    int listlen = 0;
    if (incheck){
        listlen = movegen(board, list, color, true);
    }
    else{
        listlen = movegen(board, list, color, false);
    }


    if (movescore(board, list, 0, color, 'n', nullmove, listlen)){
        printfull(board);
        exit(1);
    }
    int i = 0;
    bool ismove = false;

    int bestscore = stand_pat;

    while (i < listlen){
        selectionsort(list, i, listlen);
        
        if ((!incheck) && list[i].eval < 1000200){
            CURRENTPOS = original_pos;
            return bestscore;
        }
        struct board_info board2 = *board;

        if (move(&board2, list[i].move, color)){
            //printf("%i %i\n", depth, color);
        }         
        if (isattacked(&board2, board2.kingpos[color], color^1)){
            CURRENTPOS = original_pos;
            i++;
            continue;
        }
        ismove = true;   
        
        bool ischeck = isattacked(board, board->kingpos[color^1], color);

        list[i].eval = -quiesce(&board2, -beta, -alpha, depth+1, depthleft-1, color^1, ischeck);
        
       
        
        if (abs(list[i].eval) == TIMEOUT){
            CURRENTPOS = original_pos;
            return TIMEOUT;
        }
        if (list[i].eval > bestscore){
            bestscore = list[i].eval;
        }
        if (list[i].eval >= beta){
            CURRENTPOS = original_pos;
            return list[i].eval;
        }
        if (list[i].eval > alpha){
            alpha = list[i].eval;
        }
        CURRENTPOS = original_pos;
        i++;
    }
    
    if (incheck && !ismove){
        return -100000;
    }
    return bestscore;
}


int alphabeta(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depthleft, int depth, bool color, bool isnull, bool incheck){
    nodes++;
    if (depth == 0){
        maxdepth = 0;
        clearPV();
    }
    if (!((nodes) & (CHECKTIME))){
        clock_t rightnow = clock() - start_time;
        if ((float)rightnow/CLOCKS_PER_SEC > maximumtime || (float)rightnow/CLOCKS_PER_SEC > coldturkey){ //you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
    }
    if (depth > 0){
        if (checkdraw2(movelst, key) > 0 || checkdraw1(board)){
            return 0;
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

    bool improving = (depth > 2 && !incheck && movelst[*key-1].staticeval > movelst[*key-3].staticeval);

    if (type != 'n'){
        evl = TT[(CURRENTPOS) & (_mask)].eval;
    }

    if (!ispv && !incheck && depthleft < 9 && evl - (depthleft*70) + (improving*50) >= beta){
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
    
    if (ispv && type == 'n'){
        depthleft--;
    }

    struct list list[LISTSIZE];

    bool ismove = false;
    int betacount = 0;
    int movelen = movegen(board, list, color, incheck);
    //remove_illegal(board, list, color);
    if (movescore(board, list, depth, color, type, depth > 1 ? movelst[*key-1].move : nullmove, movelen)){
        printfull(board);
        printf("%i\n", depth);
        for (int i = 0; i < *key; i++){
            printf("%x\n", movelst[i].move.move);
        }
        exit(1);
    }
    int i = 0;
    unsigned long long int original_pos = CURRENTPOS;
    bool raisedalpha = false;
    if (depth == 0){
        currentmove.move = 0;
    }
    int pvstart = pvptr;
    pvstack[pvptr++].move = 0;
    struct move bestmove = nullmove;
    bool _fprune = false;

    if (depthleft < 5 && !incheck && !ispv && alpha > -1000000 && evl+(futility[depthleft-1]) < alpha && (depthleft < 4 || type == 'n')){
        _fprune = true;
    }


    int futility_move_count = (3+depthleft*depthleft/(1+(!improving)));
    int numquiets = 0;
    bool movecountprune = false;
    int bestscore = -1000000;

    while (i < movelen && !movecountprune){     

        selectionsort(list, i, movelen);
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
        bool iscap = (list[i].move.flags == 0xC || board->board[list[i].move.move & 0xFF]);        
        if (!iscap && !ispv && depthleft < 4){
            numquiets++;
            if (numquiets >= futility_move_count && depth > 0){
                movecountprune = true;
            }
        }
        bool ischeck = isattacked(&board2, board2.kingpos[color^1], color);
        if (_fprune && !ischeck && !iscap){
            if ((evl + futility[depthleft-1] + (improving*40)) < alpha){
                betacount++;
                CURRENTPOS = original_pos;
                i++;
                continue;
            }
        }
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
            if (depthleft < 3){
                R = 0;
            }

            else{
                R = LMRTABLE[depthleft-1][betacount];
                if (list[i].eval > 1000000){
                    R--;
                }
                if (ischeck || incheck){
                    R--;
                }
                if (list[i].eval > 100195){
                    R--;
                }
                if (!ispv && type != 3){
                    R++;
                }
                if (!improving){
                    R++;
                }
                
            }
            R = MAX(R, 0);

            

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
                if (R > 0){
                    R = 0;
                }
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
            if (betacount < 1){
                betas++;
            }
            else{
                //printf("%s %i\n", list[i].move, betacount);
            }
            if (!iscap && depthleft > 1){
                if (!ismatch(KILLERTABLE[depth][0], list[i].move)){
                    KILLERTABLE[depth][0] = list[i].move;
                }
                else if (!ismatch(KILLERTABLE[depth][1], list[i].move)){
                    KILLERTABLE[depth][1] = list[i].move;
                }

                    int c = depthleft*depthleft; if (c > 400){c = 400;}
                    //printf("%s\n", conv(list[i].move, b));
                    HISTORYTABLE[color][(list[i].move.move>>8)][list[i].move.move&0xFF] += c;

                    if (HISTORYTABLE[color][(list[i].move.move>>8)][(list[i].move.move&0xFF)] > 1000000){
                        for (int a = 0; a < 0x80; a++){
                            for (int b = 0; b < 0x80; b++){
                                HISTORYTABLE[color][a][b] = (HISTORYTABLE[color][a][b]>>1)+1;
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
            pvptr = pvstart;
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
            
            int p = pvptr;
            //printf("%i\n", p);
            pvptr = pvstart;
            pvstack[pvptr] = list[i].move;
            pvptr++;        
            while (p < 4999 && pvstack[p].move != 0){ pvstack[pvptr] = pvstack[p]; p++; pvptr++;}
            pvstack[pvptr].move = 0;
             // the move we just searched is now the first of the new PV
        }

        else if (!betacount && depth == 0){
            insert(original_pos, depthleft, list[i].eval, 1, list[i].move);
            CURRENTPOS = original_pos;
            pvptr = pvstart;
            return list[i].eval;
        }

        CURRENTPOS = original_pos;
        betacount++;
        i++;
    }

    pvptr = pvstart;
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


float iid_time(struct board_info *board, struct movelist *movelst, float maxtime, int *key, bool color, bool ismove){
    /*if (*key < 5 && opening_book(board, movelst, key, color)){
        return 1;
    }*/
    nodes = 0;    
    maximumtime = maxtime*2;
    //printf("%f\n", maximumtime);
    //clearTT(false);
    start_time = clock();
    clearPV();
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
        int asphigh = 25, asplow = 25;       
        int evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, incheck); 

        while (abs(evl) != TIMEOUT && (evl <= alpha || evl >= beta)){

            if (evl <= alpha){
                char temp[6];
                    printf("info depth %i seldepth %i score cp %i nodes %lu time %li pv %s\n", depth, maxdepth, alpha, nodes, (long int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC, conv(pvmove, temp));
                
                alpha = evl - asplow;    
                asplow *= 3;            
                evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, incheck);
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
                beta = evl + asphigh;
                asphigh *= 3;
                evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, incheck);   
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
        
        

        for (int i = 0; i < depth && pvstack[i].move != 0; i++){
            char temp[8];
            printf("%s ", conv(pvstack[i], temp));

        }
        printf("\n");
        //printf("%f\n", (float)betas/total*100);

        if ((float)time2/CLOCKS_PER_SEC > maxtime*0.6 || depth >= MAXDEPTH){                      
            break;
        }
         if (depth > 5){
            alpha = evl-25;
            beta = evl+25;
        }   
    
    }
    char temp[8], temp2[8];
    printf("bestmove %s ponder %s\n", conv(currentmove, temp), conv(pvstack[1], temp2));
    
    

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
    printf("id name Willow 2.7\n");
    printf("id author Adam Kulju\n");
    // send options
    printf("option name Hash type spin default 32 min 32 max 32\n");
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
        clearPV();
        setfull(board);
        setmovelist(movelst, key); 
        search_age = 0;
        }

    if (!strcmp(command, "quit")){
        exit(0);
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
            int milltime = atoi(&command[k]) - 400;
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
                if ((float)milltime/250 < coldturkey){ //if you're not already literally running on increment
                time += (float)milltime/1000 * 0.5;
                }
            }

            
        }
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
    for (int i = 0; i < 50; i++){

        clearTT();
        clearKiller();
        clearCounters();
        clearHistory(true);
        clearPV();
        search_age = 0;

        printf("%s\n", positions[i]);
        
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
        exit(0);
    }
    
    com();
    return 0;
} 
