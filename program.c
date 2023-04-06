#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>     
#include <string.h>
#include <time.h>
#include <math.h>
#include <Windows.h>
#include <io.h>
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
    char board[0x80];
    char pnbrqcount[2][5];
    bool castling[2][2];
    unsigned char kingpos[2];
    char epsquare;
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
    struct move bestmove;
    int eval;
    char depth;
    char age;
};
struct move currentmove;
struct move nullmove = {0,0};
struct ttentry TT[TTSIZE];

const int boardsize = sizeof(struct board_info);
const int listsize = sizeof(struct list);
const int movesize = sizeof(struct movelist);

int LMRTABLE[50][LISTSIZE];

bool CENTERWHITE[0x88];
bool CENTERBLACK[0x88];

struct move KILLERTABLE[50][2];
struct move COUNTERMOVES[6][64];
unsigned long int HISTORYTABLE[2][0x80][0x80]; //allows for faster lookups
struct move pvstack[5000];

unsigned long int nodes;
long int totals;
int betas, total;
 const int VALUES[5] = {70, 348, 364, 473, 1024};
 const int VALUES2[5] = {91, 285, 295, 502, 932};
short int pstbonusesm[6][0x80] = {
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        -28,-1,-13,5,7,37,36,-17,0,0,0,0,0,0,0,0,
        -22,-10,1,5,18,13,28,-12,0,0,0,0,0,0,0,0,
        -29,-13,-3,8,12,12,-2,-26,0,0,0,0,0,0,0,0,
        -18,2,-2,8,11,12,6,-23,0,0,0,0,0,0,0,0,
        -16,-2,13,13,53,52,21,-23,0,0,0,0,0,0,0,0,
        70,104,35,68,43,98,36,-7,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    },
    {
        -104,-18,-57,-32,-14,-25,-12,-23,0,0,0,0,0,0,0,0,
        -28,-52,-12,7,8,16,-13,-16,0,0,0,0,0,0,0,0,
        -25,-9,3,10,19,10,25,-20,0,0,0,0,0,0,0,0,
        -14,4,12,15,23,18,21,-6,0,0,0,0,0,0,0,0,
        -9,10,19,50,28,47,13,23,0,0,0,0,0,0,0,0,
        -47,57,36,64,84,109,73,43,0,0,0,0,0,0,0,0,
        -73,-40,70,35,23,62,7,-17,0,0,0,0,0,0,0,0,
        -166,-89,-49,-49,-49,-97,-49,-107,0,0,0,0,0,0,0,0
    },
    {
        -33,-13,1,-24,-7,0,-37,-20,0,0,0,0,0,0,0,0,
        4,24,14,8,18,22,39,4,0,0,0,0,0,0,0,0,
        3,16,13,14,10,26,17,8,0,0,0,0,0,0,0,0,
        -5,11,8,23,32,4,9,5,0,0,0,0,0,0,0,0,
        -1,1,30,46,34,33,5,-4,0,0,0,0,0,0,0,0,
        -12,34,40,37,34,50,36,-4,0,0,0,0,0,0,0,0,
        -24,42,-20,-10,29,55,16,-47,0,0,0,0,0,0,0,0,
        -35,2,-82,-35,-26,-42,6,-8,0,0,0,0,0,0,0,0
    },

    {
        -15,-10,5,15,21,7,-29,-14,0,0,0,0,0,0,0,0,
        -39,-16,-19,-9,0,7,-5,-69,0,0,0,0,0,0,0,0,
        -41,-25,-11,-2,8,-7,5,-23,0,0,0,0,0,0,0,0,
        -36,-25,-11,-2,8,-7,5,-23,0,0,0,0,0,0,0,0,
        -23,-10,6,25,22,34,-5,-19,0,0,0,0,0,0,0,0,
        -6,18,25,35,17,44,60,16,0,0,0,0,0,0,0,0,
        23,28,54,58,76,64,24,43,0,0,0,0,0,0,0,0,
        28,39,27,46,59,9,30,42,0,0,0,0,0,0,0,0
    },
    {
    -1,-11,2,18,-4,-22,-31,-47,0,0,0,0,0,0,0,0,
    -31,-2,15,17,24,17,0,1,0,0,0,0,0,0,0,0,
    -15,6,-9,-1,3,1,11,2,0,0,0,0,0,0,0,0,
    -7,-25,-5,-13,-0,-8,-4,-7,0,0,0,0,0,0,0,0,
    -27,-24,-15,-19,-6,10,-7,-13,0,0,0,0,0,0,0,0,
    -14,-17,5,4,25,51,41,47,0,0,0,0,0,0,0,0,
    -21,-37,-5,1,-16,54,23,51,0,0,0,0,0,0,0,0,
    -30,-3,25,9,55,42,41,41,0,0,0,0,0,0,0,0

    },
    {
    -15,39,12,-59,-6,-32,31,8,0,0,0,0,0,0,0,0,
    2,8,-11,-64,-45,-18,17,9,0,0,0,0,0,0,0,0,
    -15,-13,-22,-45,-42,-28,-11,-31,0,0,0,0,0,0,0,0,
    -49,0,-27,-39,-45,-43,-33,-55,0,0,0,0,0,0,0,0,
    -17,-20,-12,-26,-30,-24,-14,-39,0,0,0,0,0,0,0,0,
    -9,25,2,-15,-19,7,23,-22,0,0,0,0,0,0,0,0,
    29,-1,-20,-7,-8,-4,-37,-29,0,0,0,0,0,0,0,0,
    -65,23,16,-15,-56,-34,2,13,0,0,0,0,0,0,0,0
    }

};


short int pstbonusese[6][0x80] = {
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        16,4,8,5,5,-5,-10,-7,0,0,0,0,0,0,0,0,
        7,3,-3,-6,-5,-5,-13,-8,0,0,0,0,0,0,0,0,
        20,9,-2,-7,-7,-7,-2,1,0,0,0,0,0,0,0,0,
        32,20,9,-5,-5,-1,10,12,0,0,0,0,0,0,0,0,
        74,77,56,35,25,31,62,64,0,0,0,0,0,0,0,0,
        179,154,144,116,126,114,148,178,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    },
    {
        -28,-41,-19,-10,-16,-14,-46,-65,0,0,0,0,0,0,0,0,
        -41,-18,-9,-4,-2,-19,-22,-41,0,0,0,0,0,0,0,0,
        -22,-4,-2,12,10,-3,-18,-22,0,0,0,0,0,0,0,0,
        -16,-3,15,19,18,16,3,-14,0,0,0,0,0,0,0,0,
        -15,2,19,18,19,43,8,-13,0,0,0,0,0,0,0,0,
        -21,-23,7,4,-6,-13,-19,-3,0,0,0,0,0,0,0,0,
        -24,-6,-27,-5,-11,-28,-24,-52,0,0,0,0,0,0,0,0,
        -57,-37,-16,-28,-28,-27,-67,-108,0,0,0,0,0,0,0,0
    },
    {
        -17,-5,-8,4,-2,-6,-2,-16,0,0,0,0,0,0,0,0,
        -14,-17,-8,-2,-2,-5,-14,-24,0,0,0,0,0,0,0,0,
        -10,-2,7,9,12,0,-6,-13,0,0,0,0,0,0,0,0,
        -6,0,10,12,3,8,-6,-9,0,0,0,0,0,0,0,0,
        -1,7,4,1,10,6,3,0,0,0,0,0,0,0,0,0,
        2,-7,-3,-5,-4,3,-2,3,0,0,0,0,0,0,0,0,
        -7,-8,5,-12,-6,-17,-5,-14,0,0,0,0,0,0,0,0,
        -13,-21,-10,-9,-8,-9,-18,-24,0,0,0,0,0,0,0,0
    },
    {
        1,4,4,-1,-8,-7,8,-16,0,0,0,0,0,0,0,0,
        0,-5,2,1,-8,-12,-12,-1,0,0,0,0,0,0,0,0,
        1,3,-1,1,-9,-11,-10,-13,0,0,0,0,0,0,0,0,
        7,10,10,3,-5,-3,-8,-7,0,0,0,0,0,0,0,0,
        13,9,13,1,0,1,3,6,0,0,0,0,0,0,0,0,
        9,7,6,0,2,-4,-9,-4,0,0,0,0,0,0,0,0,
        7,8,1,0,-11,-4,5,0,0,0,0,0,0,0,0,0,
        6,4,8,3,3,10,7,1,0,0,0,0,0,0,0,0
    },
    {
        -32,-25,-19,-36,-3,-30,-20,-40,0,0,0,0,0,0,0,0,
        -20,-20,-28,-12,-14,-23,-35,-32,0,0,0,0,0,0,0,0,
        -16,-24,18,9,13,17,9,2,0,0,0,0,0,0,0,0,
        -15,31,23,49,33,31,35,28,0,0,0,0,0,0,0,0,
        6,25,27,46,56,36,53,29,0,0,0,0,0,0,0,0,
        -19,7,10,47,44,29,15,2,0,0,0,0,0,0,0,0,
        -13,22,33,41,59,22,27,-3,0,0,0,0,0,0,0,0,
        -11,19,18,24,21,16,7,14,0,0,0,0,0,0,0,0
    },
    {
        -54,-40,-23,-8,-22,-10,-36,-56,0,0,0,0,0,0,0,0,
        -27,-8,6,19,21,11,-3,-18,0,0,0,0,0,0,0,0,
        -22,-3,12,25,27,20,7,-12,0,0,0,0,0,0,0,0,
        -19,-5,19,23,29,25,10,-13,0,0,0,0,0,0,0,0,
        -9,20,21,27,25,34,27,-1,0,0,0,0,0,0,0,0,
        8,18,20,19,23,46,45,13,0,0,0,0,0,0,0,0,
        -13,17,15,15,18,38,27,10,0,0,0,0,0,0,0,0,
        -74,-35,-19,-19,-11,15,3,-18,0,0,0,0,0,0,0,0
    }
};



short int bishop_pair[9] = { 10,  16,  21,  22,  22,  20,  12,  11,  16};
short int passedmgbonus[8] = {  0,   3,   4,   6,  26,  73, 140,   0};
short int passedegbonus[8] = { 0,   3,   6,  28,  57,  98, 184,   0};


short int mobilitybonusesmg[3][15] = {
    {-27,  -3,   5,   6,  13,  18,  22,  24,  21},
    {-18,  -7,   4,  11,  16,  18,  20,  22,  26,  29,  37,  38,  45,  49},
    {-17, -10,  -7,  -2,  -3,   3,   7,  12,  18,  19,  19,  24,  27,  26,  26}
};

short int mobilitybonuseseg[3][15] = {
    {-40, -22,   2,  16,  15,  19,  19,  20,  14},
    {-32, -13,  -7,   1,  13,  20,  24,  27,  30,  28,  31,  33,  44,  44},
    {-33,  -3,  13,  25,  41,  48,  56,  57,  59,  64,  68,  73,  76,  72,  74}
};
short int doubledpen = -14;
short int isopen =  -11;
short int backwardspen = -5;

char attacknums[4] = {2,2,3,5};
int king_attack_count[2];

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
short int futility[4] = {125, 125, 300, 300};

int com_uci( struct board_info *board, struct movelist *movelst, int *key, char *color);

int pipe;
HANDLE hstdin;

     int InputPending()
     {  // checks for waiting input in pipe
     //return 0;
	static int init; static HANDLE inp; DWORD cnt;
	if(!init) inp = GetStdHandle(STD_INPUT_HANDLE);
    if (!PeekNamedPipe(inp, NULL, 0, NULL, &cnt, NULL)){
        return 0;
    }
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
    DWORD dw;
    hstdin = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(hstdin, &dw);

    if (!pipe) {
        SetConsoleMode(hstdin,dw&~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(hstdin);
    } else {
        setvbuf(stdin,NULL,_IONBF,0);
        setvbuf(stdout,NULL,_IONBF,0);
    }
    DWORD n;
    if (!PeekNamedPipe(hstdin, NULL, 0, NULL, &n, NULL)){
        //exit(0);
    }

    printf("Willow 2.7, by Adam Kulju\n");
    return 0;
}
void initglobals(){
    for (unsigned char i = 0; i < 0x80; i++){
        if (i & 0x88){
            i += 7; continue;
        }
        for (char n = 0; n < 8; n++){
            if (!((i+vector[4][n]) & 0x88)){                    //this will give us a value of 2 for side (i.e. rook checks) and 1 for diagonals
                KINGZONES[0][i][i+vector[4][n]] = (n&1) + 1;
                KINGZONES[1][i][i+vector[4][n]] = (n&1) + 1;    
            }   
        }
        if (i + 31 < 0x80){
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
    for (int i = 0; i < 50; i++){
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
    CURRENTPOS = 0^ZOBRISTTABLE[772];
    int i;
    for (i = 0; i < 0x80; i++){
            if (!(i & 0x88) && board->board[i]){
                CURRENTPOS ^= ZOBRISTTABLE[(((board->board[i]-2)<<6))+i-((i>>4)<<3)];
                //board->board[i]-2<<6: gives us the base value for each piece*64.
                //i-((i>>4)<<3): converts i into a number from 1 to 64. it does that by subtracting 8 for every rank it's on - on rank 0 doesn't get subtracted, on rank 7 gets -56
            }
    }
}

void insert(unsigned long long int position, int depthleft, int eval, char type, struct move bestmove){
    int index = position & _mask;
    if (TT[index].zobrist_key == position && TT[index].depth >= depthleft && TT[index].age < 2){
        //return;
    }
    TT[index].zobrist_key = position;
    TT[index].depth = depthleft;
    TT[index].eval = eval;
    TT[index].type = type;
    TT[index].age = 0;
    TT[index].bestmove = bestmove;
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
    /*int i; for (i = 0; i < 5000; i++){
        pvstack[i].move = 0;
    }*/
    pvptr = 0;
}

void clearHistory(bool delete){
    int n, i;
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
    for (int i = 0; i < 50; i++){
        KILLERTABLE[i][0].move = 0;
        KILLERTABLE[i][1].move = 0;
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
    if (from & 0x88 || to & 0x88){
        printf("out of board index! %4x %x %i %x\n", move.move, (board->epsquare), color, move.flags);
        printfull(board);    
        return 1;
    }

    CURRENTPOS ^= ZOBRISTTABLE[(((board->board[from]-2)<<6))+from-((from>>4)<<3)]; //xor out the piece to be moved
    

    if (flag != 3){ //handle captures and en passant
            if (board->board[to]){  
                board->pnbrqcount[color^1][((board->board[to]-2-(color^1))>>1)]--;
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
    if ((mve.flags>>2) == 1 || mve.flags == 0xC || board->board[(mve.move&0xFF)] == WPAWN+color || iscap){ //if the move is a capture, or a promotion, or a pawn move, half move clock is reset.
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

bool isattacked(struct board_info *board, char pos, bool encolor){
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
    char d, f;
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
char isattacked_mv(struct board_info *board, char pos, bool encolor){
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
    char d, f;
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
        char diff = pos+NORTH;

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
        if ((board->board[diff]&1) && !(diff&0x88)){ //capture to the right

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
        if ((board->board[diff]&1) && !(diff&0x88)){ 
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
        char diff = pos+SOUTH;

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
        if (board->board[diff] && !(diff&0x88) && !(board->board[diff]&1)){ //capture to the right
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
        if (board->board[diff] && !(diff&0x88) && !(board->board[diff]&1)){
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
            char piecetype = (board->board[i]>>1)-2;
            if (piecetype == -1){ //if it is a pawn
                pawnmoves(board, list, &key, i, color);
            }

            else{
                for (char dir = 0; dir < vectors[piecetype]; dir++){
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

int piece_mobility(struct board_info *board, char i, bool color, char piecetype){
    
    
    isattacker = false;
    char mobility = 0;
    for (int dir = 0; dir < vectors[piecetype]; dir++){
        for (unsigned char pos = i;;){
            pos += vector[piecetype][dir];

                if ((pos & 0x88) || (board->board[pos] && (board->board[pos]&1) == color)){break;}

                    if (piecetype != 3){ //queen
                        if (color){
                            if ((((pos+SE) & 0x88) || (board->board[pos+SE] != WPAWN)) && (((pos+SW) & 0x88) || (board->board[pos+SW] != WPAWN))){
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
    
                                        king_attack_count[color] += 6;
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
            rep++;
            if (rep > 1){
                return 2;
            }
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
    char spacew = 0, spaceb = 0;
    int mgscore = 0, egscore = 0;
    int blockedpawns = 0;

    char wbackwards[8], wadvanced[8], bbackwards[8], badvanced[8];

    for (char i = 0; i < 8; i++){
    wadvanced[i] = -1, badvanced[i] = 9;
    wbackwards[i] = 9, bbackwards[i] = -1;
    }
    for (unsigned char i = 0; i < 0x7a; i++){
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
                    if ((board->board[i+SOUTH] == BPAWN || board->board[i+(SOUTH<<1)] == BPAWN || board->board[i+(SOUTH*3)] == BPAWN) && !isattacked(board, i, WHITE)){
                        spaceb++;
                    }
                } 

            }

        }
        if (board->board[i]){
            bool mobilitybonus = false;
            int moves = 0;
            char piecetype = (board->board[i]>>1)-1, piececolor = (board->board[i]&1);
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

    for (char i = 0; i < 8; i++){
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
        int pawnshield = 0;
        if (board->kingpos[WHITE] < 0x40){
        if (board->board[board->kingpos[WHITE]+WEST] == WPAWN || board->board[board->kingpos[WHITE]+NW] == WPAWN){
            pawnshield++;
        }
        if (((board->board[board->kingpos[WHITE]+NORTH]>>1) == PAWN) || board->board[board->kingpos[WHITE]+NORTH+NORTH] == WPAWN || board->board[board->kingpos[WHITE]+NORTH+NORTH+NORTH] == WPAWN){
            pawnshield++;
        }
        if (board->board[board->kingpos[WHITE]+EAST] == WPAWN || board->board[board->kingpos[WHITE]+NE] == WPAWN){
            pawnshield++;
        }

        }
        wk -= kingdangertable[king_attack_count[BLACK]]*(1.6-(0.2*pawnshield));
    }
    
    if (attackers[WHITE] > 1){
        int pawnshield = 0;
        if (board->kingpos[BLACK] > 0x37){
        if (board->board[board->kingpos[BLACK]+WEST] == BPAWN || board->board[board->kingpos[BLACK]+SW] == BPAWN){
            pawnshield++;
        }
        if (((board->board[board->kingpos[BLACK]+SOUTH]>>1) == PAWN) || board->board[board->kingpos[BLACK]+SOUTH+SOUTH] == BPAWN || board->board[board->kingpos[BLACK]+SOUTH+SOUTH+SOUTH] == BPAWN){
            pawnshield++;
        }
        if (board->board[board->kingpos[BLACK]+EAST] == BPAWN || board->board[board->kingpos[BLACK]+SE] == BPAWN){
            pawnshield++;
        }
        }
        wk += kingdangertable[king_attack_count[WHITE]]*(1.6-(0.2*pawnshield));
    }
    score += (wk>>1);
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

    int i;
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

void movescore(struct board_info *board, struct list *list, int depth, bool color, char type, bool ispv, struct move lastmove, int movelen){
    int evl;
    int position = -1;
    int i = 0; while (i < movelen){
        list[i].eval = 1000000;
         if ((type != 'n') && ismatch(TT[CURRENTPOS & _mask].bestmove, list[i].move)){            
            list[i].eval += 100000;
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
        else if (ismatch(list[i].move, COUNTERMOVES[(board->board[lastmove.move&0xFF]>>1)][lastmove.move&0xFF])){
            list[i].eval += 197;
        }
        
        else{
                //list[i].eval = 0;

                list[i].eval = HISTORYTABLE[color][list[i].move.move>>8][list[i].move.move&0xFF];
                if (list[i].eval != 0){
                    char b[6];
                    //printf("#%s\n", conv(list[i].move, b));
                    //exit(0);
                }

            }
            
            i++;
        }

}
int quiesce(struct board_info *board, int alpha, int beta, int depth, int depthleft, bool color, bool incheck){
    if (depth > maxdepth){
        maxdepth = depth;
    }
    nodes++;
    if (!(nodes & CHECKTIME)){
        clock_t rightnow = clock() - start_time;
        if ((float)rightnow/CLOCKS_PER_SEC > maximumtime || (float)rightnow/CLOCKS_PER_SEC > coldturkey){ //you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
        if (InputPending() > 0){
            int a = com_uci(board, (struct movelist *) 0, (int *) 0, WHITE);
            if (a == 1){
                return TIMEOUT;
            }
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

    int bestscore = stand_pat;

    movescore(board, list, 0, color, 'n', false, nullmove, listlen);
    int i = 0;
    bool ismove = false;
    while (i < listlen){
        selectionsort(list, i, listlen);
        
        if ((!incheck) && list[i].eval < 1000200){
            CURRENTPOS = original_pos;
            return bestscore;
        }
        struct board_info board2 = *board;
        bool iscap = (list[i].move.flags == 0xC || board->board[list[i].move.move & 0xFF]);
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


int alphabeta(struct board_info *board, struct movelist *movelst, int *key, int alpha, int beta, int depthleft, int depth, char color, bool isnull, bool incheck){
    nodes++;
    if (depth == 0){
        maxdepth = 0;
    }
    if (!(nodes & CHECKTIME)){
        clock_t rightnow = clock() - start_time;
        if ((float)rightnow/CLOCKS_PER_SEC > maximumtime || (float)rightnow/CLOCKS_PER_SEC > coldturkey){ //you MOVE if you're down to 0.1 seconds!
            return TIMEOUT;
        }
        if (InputPending() > 0){
            int a = com_uci(board, (struct movelist *) 0, (int *) 0, WHITE);
            if (a == 1){
                return TIMEOUT;
            }
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

    char type; if (CURRENTPOS == TT[CURRENTPOS & _mask].zobrist_key){
        type = TT[CURRENTPOS & _mask].type;
            evl = TT[CURRENTPOS & _mask].eval;
    }
    else{
        type = 'n';
        evl = -1024;
    }
    bool ispv = (beta != alpha+1);
    if (!ispv && type != 'n' && TT[CURRENTPOS & _mask].depth >= depthleft){
            if (type == '0'){
                /*if (evl > beta){
                    return beta;
                }
                if (evl < alpha){
                    return alpha;
                }*/
                return evl;
            }
            else if (type == '1'){ //a move that caused a beta cutoff
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
    else if (type != 'n'){
        evl = TT[CURRENTPOS & _mask].eval;
    }
    else{
        evl = eval(board, color);
    }
    movelst[*key-1].staticeval = evl;
    bool improving = (*key > 2 && movelst[*key-3].staticeval != -1000000 && movelst[*key-1].staticeval > movelst[*key-3].staticeval);
    int rfpmargin = (depthleft*70);
    if (depthleft < 9 && evl - rfpmargin > beta){
        return evl;
    }
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
            if (evl >= beta){
                unsigned long long int a = CURRENTPOS;
                unsigned char tempep = board->epsquare;
                board->epsquare = 0;
                if (tempep){
                    CURRENTPOS ^= ZOBRISTTABLE[773];
                }
                CURRENTPOS ^= ZOBRISTTABLE[772];
                int R = 4 + depthleft/6 + MIN(((evl-beta)/200), 3);
                int nullmove = -alphabeta(board, movelst, key, -beta, -beta+1, depthleft-R, depth+2, color^1, true, false);
                CURRENTPOS = a;
                if (tempep){
                    CURRENTPOS ^= ZOBRISTTABLE[773];
                }
                board->epsquare = tempep;
                if (abs(nullmove) == TIMEOUT){
                    return TIMEOUT;
                }
                
                if (nullmove >= beta){
                    return evl;
                }
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
    movescore(board, list, depth, color, type, ispv, movelst[*key-2].move, movelen);
    int i = 0;
    unsigned long long int original_pos = CURRENTPOS;
    bool raisedalpha = false;
    if (depth == 0){
        currentmove.move = 0;
    }
    int pvstart = pvptr;
    pvstack[pvptr++].move = 0;
    struct move bestmove;
    bool _fprune = false;
    if (depthleft < 5 && !incheck && !ispv && alpha > -1000000 && evl+(futility[depthleft-1]) < alpha && (depthleft < 4 || type == 'n')){
        _fprune = true;
    }


    int futility_move_count = (3+depthleft*depthleft);
    int numquiets = 0;
    bool movecountprune = false;

    int bestscore = -1000000;

    while (i < movelen && !movecountprune){      
        selectionsort(list, i, movelen);
        struct board_info board2 = *board;


        bool iscap = (list[i].move.flags == 0xC || board->board[list[i].move.move & 0xFF]);

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
        ismove = true;
        
        if (!iscap && !ispv && depthleft < 4){
            numquiets++;
            if (numquiets >= futility_move_count && depth > 0){
                movecountprune = true;
            }
        }
        bool ischeck = isattacked(&board2, board2.kingpos[color^1], color);
        if (_fprune && !ischeck && !iscap){
            if ((evl + futility[depthleft-1]) < alpha){
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
                                movelst[*key-1].move.flags = 0;
                *key = *key-1;
                CURRENTPOS = original_pos;
        
                return TIMEOUT;
                }
            }

        else{
            int R;
            if (list[i].eval > 1000197 || depthleft < 3){
                R = 0;
            }

            else{
                R = LMRTABLE[depthleft-1][betacount];
                if (ischeck || incheck || iscap){
                    R >>= 1;
                }
                if (!ispv){
                    R += 1;
                }
            }

            

            list[i].eval = -alphabeta(&board2, movelst, key, -alpha-1, -alpha, depthleft-1-R, depth+1, color^1, false, ischeck);
                if (abs(list[i].eval) == TIMEOUT){
                                movelst[*key-1].move.flags = 0;
            *key = *key-1;
            CURRENTPOS = original_pos;
    
            return TIMEOUT;
                }

            if (list[i].eval > alpha && R > 0){
                list[i].eval = -alphabeta(&board2, movelst, key, -alpha-1, -alpha, depthleft-1, depth+1, color^1, false, ischeck);
                if (abs(list[i].eval) == TIMEOUT){
                                movelst[*key-1].move.flags = 0;
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
                                movelst[*key-1].move.flags = 0;
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
            insert(original_pos, depthleft, list[i].eval, '1', bestmove);
            total++;
            if (betacount < 1){
                betas++;
            }
            else{
                //printf("%s %i\n", list[i].move, betacount);
            }
            int pos;
            if (!iscap && depthleft > 1){
                if (!ismatch(KILLERTABLE[depth][0], list[i].move)){
                    KILLERTABLE[depth][0] = list[i].move;
                }
                else if (!ismatch(KILLERTABLE[depth][1], list[i].move)){
                    KILLERTABLE[depth][1] = list[i].move;
                }

                    int c = depthleft*depthleft; if (c > 400){c = 400;}
                    char b[6];
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


        if (depth > 0 && !isnull && !iscap){  //key-1 is the move you made, key-2 is the move the opponent made

                COUNTERMOVES[(board->board[movelst[(*key)-2].move.move&0xFF]>>2)][movelst[(*key)-2].move.move&0xFF] = list[i].move;
            }
            movelst[(*key)-1].move.flags = 0;
            *key = *key-1;
            CURRENTPOS = original_pos;
            pvptr = pvstart;
            return bestscore;
        }
        
        movelst[*key-1].move.flags = 0;
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
            insert(original_pos, depthleft, bestscore, '2', list[i].move);
            CURRENTPOS = original_pos;
            pvptr = pvstart;
            return bestscore;
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
        insert(original_pos, depthleft, bestscore, '0', bestmove);
        //}
    }
    else{

        insert(original_pos, depthleft, bestscore, '2', bestmove);
        //}
    }

    return bestscore;
}

float iid_time(struct board_info *board, struct movelist *movelst, float maxtime, int *key, char color, bool ismove){
    /*if (*key < 5 && opening_book(board, movelst, key, color)){
        return 1;
    }*/
    maximumtime = maxtime*2;
    printf("%f\n", maximumtime);
    //clearTT(false);
    start_time = clock();
    clearPV();
    clearHistory(false);
    clearKiller();
    currentmove.move = 0;
    int alpha = -1000000, beta = 1000000;   
    bool incheck = isattacked(board, board->kingpos[color], color^1);
    int g;
    int depth;
    struct move pvmove;
    char pvreply[6];
    //printf("%i %s %s\n", *key, movelst[*key-1].fen, movelst[*key-1].move);
    for (depth = 1; ; depth++){        
        int asphigh = 25, asplow = 25;       
        int evl = alphabeta(board, movelst, key, alpha, beta, depth, 0, color, false, incheck); 

        while (abs(evl) != TIMEOUT && (evl <= alpha || evl >= beta)){

            if (evl <= alpha){
                char temp[8];
                printf("info depth %i seldepth %i score cp %i nodes %li time %i pv %s\n", depth, maxdepth, alpha, nodes, (int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC, conv(pvmove, temp));
                alpha -= asplow;    
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
                char temp[8];
                printf("info depth %i seldepth %i score cp %i nodes %li time %i pv %s\n", depth, maxdepth, beta, nodes, (int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC, conv(currentmove, temp));
                pvmove = currentmove;
                beta += asphigh;
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
        char temp;
        pvmove = currentmove;
        printf("info depth %i seldepth %i score cp %i nodes %lu time %i pv ", depth, maxdepth, g, nodes, (int)((float)clock()-start_time)*1000/CLOCKS_PER_SEC);
        

        for (int i = 0; i < depth && pvstack[i].move != 0; i++){
            char temp[8];
            printf("%s ", conv(pvstack[i], temp));

        }
        printf("\n");
        printf("%f\n", (float)betas/total*100);

        if ((float)time2/CLOCKS_PER_SEC > maxtime*0.6 || depth > 48){                      
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
    nodes = 0;
    return g;
}
int com_uci( struct board_info *board, struct movelist *movelst, int *key, char *color) { //assumes board+movelist have been already set up under wraps

    char command[65536];
    getsafe(command, sizeof(command));
    //memcpy(command, "position startpos moves d2d4 d7d5 g1f3 c7c6 c2c4 g8f6 b1c3 e7e6 e2e3 b8d7 f1d3 f8d6 e1g1 e8g8 c4c5 d6c7 b2b4 b7b6 c1b2 a7a5 a2a3 c8b7 h2h3 d8e7 f1e1 f8e8 d1d2 h7h6 a1d1 e8d8 d2e2 d8e8 d3c2 b6c5 b4c5 e6e5 c2f5 e5e4 f3h2 b7a6 e2c2 g7g6 f5g4 c7h2 g1h2 f6g4 h3g4 e7h4 h2g1 h4g4 c2a4 e8c8 d1d2 d7f6 c3e2 a6e2 d2e2 g4h4 e1c1 f6g4\0", 325);
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

    printf("uciok\n");
  }
    if (!strcmp(command, "isready")){
        printf("readyok\n");
    }
    if (!strcmp(command, "ucinewgame")) {
        clearTT(true);
        clearKiller();
        clearHistory(true);
        for (int i = 0; i < MOVESIZE; i++){
            movelst[i].halfmoves = 0;
            movelst[i].fen = 0;
            movelst[i].move.move = 0;
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
        calc_pos(board, WHITE);
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
            struct move temp; convto(buf, &temp, board);
            bool iscap = (temp.flags == 0xC || board->board[temp.move & 0xFF]); 
            move(board, temp, *color);
            move_add(board, movelst, key, temp, iscap, *color);

            *color ^= 1;            
        }
    }
    if (strstr(command, "go")){
        float time;
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

int com() {
    initglobals();
        clearHistory(true);
    unsigned long long init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    init_by_array64(init, 4);
    setzobrist();    
    struct board_info board;
    setfull(&board);
    struct movelist movelst[MOVESIZE];
    int key;
    calc_pos(&board, WHITE);
    setmovelist(movelst, &key); 
    char color;
    for (;;){
            com_uci(&board, movelst, &key, &color);
        }
    return 0;
}

int main(void){
    /*struct board_info board;
    setfull(&board);
    struct move mve;
    convto("d2d4\0", &mve, &board);
    move(&board, mve, WHITE);
    printfull(&board);
    eval(&board, WHITE);
    exit(0);*/
    init();
    com();
    return 0;
} 
