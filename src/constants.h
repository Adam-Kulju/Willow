#ifndef __constants__
#define __constants__

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
// all we have to do is (board[i] & 1) to see if the color is white or black - if that is 1 then it's black if not then it's white

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
#define LM 0x7FFFFFFFULL         /* Least significant 31 bits */
// #define TTSIZE 1 << 20
// #define _mask (1 << 20) - 1
#define CHECKTIME (1 << 10) - 1
#define TIMEOUT 111111
#define TEMPO 5
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
const int VALUES[5] = {75, 299, 297, 409, 819}; //Material middlegame
const int VALUES2[5] = {82, 338, 349, 584, 1171};   //Material endgame


const int slide[5] = {0, 1, 1, 1, 0};   //Does the piece in question (knight bishop etc) slide?
const int vectors[5] = {8, 4, 4, 8, 8}; //number of directions a piece can move in
const int vector[5][8] = {              //list of directions it can move in
    {SSW, SSE, WSW, ESE, WNW, ENE, NNW, NNE},
    {SW, SE, NW, NE},
    {SOUTH, WEST, EAST, NORTH},
    {SW, SOUTH, SE, WEST, NW, EAST, NE, NORTH},
    {SW, SOUTH, SE, WEST, NW, EAST, NE, NORTH}};

struct move //An internally coded move in Willow.
{
    unsigned short int move;
    // use >>8 to get SQUAREFROM and &FF to get SQUARETO
    unsigned char flags;
    // in form 0000 xx yy
    // xx = flags - 00 normal 01 promotion 10 castling 11 ep - if = 1 then yy flags - 00 knight 01 bishop 10 rook 11 queen
    // get xxflags with >>2, and yyflags with &11
};

struct board_info
{
    unsigned char board[0x80];      //Stores the board itself
    unsigned char pnbrqcount[2][5]; //Stores material
    bool castling[2][2];            //Stores castling rights
    unsigned char kingpos[2];       //Stores King positions
    unsigned char epsquare;         //stores ep square
};

struct movelist     //A in game move representation
{
    struct move move;   //The move that was just played
    // char fen[65];
    long long unsigned int fen;     //The resulting zobrist key for the board
    char halfmoves;                 //Halfmoves
    int staticeval;                 //Static evaluation of the position at that time
    int piecetype;
    bool wascap;
};
struct list     //Stores a move and ordering/eval score. Used for search.
{
    struct move move;
    int eval;
};

struct ttentry  //Transposition table entry
{
    unsigned long long int zobrist_key;
    char type;
    // come in three types:
    // 3: EXACT, 2: FAIL-HIGH, 1: FAIL-LOW
    struct move bestmove;
    int eval;
    char depth;
    short int age;
};

const struct move nullmove = {0, 0};
const struct ttentry nullTT = {0, 0, {0, 0}, 0, 0, 0};

const int boardsize = sizeof(struct board_info);
const int listsize = sizeof(struct list);
const int movesize = sizeof(struct move);
const int attacknums[4] = {2, 2, 3, 5};


const int SEEVALUES[7] = {0, 100, 450, 450, 650, 1250, 10000};  //move ordering purposes

enum EntryType {None = 0, UBound = 1, LBound = 2, Exact = 3};

#endif