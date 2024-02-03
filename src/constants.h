#pragma once
#include <string>
#include <iostream>
#include <vector>
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
// all we have to do is (board[i] & 1) to see if the color is white or black -
// if that is 1 then it's black if not then it's white

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

#define LISTSIZE 216
#define MOVESIZE 10000
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
const int VALUES[5] = {75, 299, 297, 409, 819};   // Material middlegame
const int VALUES2[5] = {82, 338, 349, 584, 1171}; // Material endgame

const int slide[5] = {
    0, 1, 1, 1, 0}; // Does the piece in question (knight bishop etc) slide?
const int vectors[5] = {8, 4, 4, 8,
                        8}; // number of directions a piece can move in
const int vector[5][8] = {  // list of directions it can move in
    {SSW, SSE, WSW, ESE, WNW, ENE, NNW, NNE},
    {SW, SE, NW, NE},
    {SOUTH, WEST, EAST, NORTH},
    {SW, SOUTH, SE, WEST, NW, EAST, NE, NORTH},
    {SW, SOUTH, SE, WEST, NW, EAST, NE, NORTH}};

struct move // An internally coded move in Willow.
{
  unsigned short int move;
  // use >>8 to get SQUAREFROM and &FF to get SQUARETO
  unsigned char flags;
  // in form 0000 xx yy
  // xx = flags - 00 normal 01 promotion 10 castling 11 ep - if = 1 then yy
  // flags - 00 knight 01 bishop 10 rook 11 queen get xxflags with >>2, and
  // yyflags with &11
};

struct board_info {
  unsigned char board[0x80];      // Stores the board itself
  unsigned char pnbrqcount[2][5]; // Stores material
  bool castling[2][2];            // Stores castling rights
  unsigned char rookstartpos[2][2];
  unsigned char kingpos[2]; // Stores King positions
  unsigned char epsquare;   // stores ep square
};

struct movelist // A in game move representation
{
  struct move move; // The move that was just played
  // char fen[65];
  long long unsigned int fen; // The resulting zobrist key for the board
  char halfmoves;             // Halfmoves
  int staticeval;             // Static evaluation of the position at that time
  int piecetype;
  bool wascap;
};
struct list // Stores a move and ordering/eval score. Used for search.
{
  struct move move;
  int eval;
};

struct ttentry // Transposition table entry
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

const int N5NTABLE[10][2] = {{0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 2},
                             {1, 3}, {1, 4}, {2, 3}, {2, 4}, {3, 4}};

enum EntryType { None = 0, UBound = 1, LBound = 2, Exact = 3 };

constexpr int buckets[2][0x80] = {
  {
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
  },
  {
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    2,2,2,2,3,3,3,3,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,       
  }
};

constexpr int zobristkeys[128] = {
  0, 1, 2, 3, 4, 5, 6, 7, 0,0,0,0,0,0,0,0,
  8, 9, 10, 11, 12, 13, 14, 15, 0,0,0,0,0,0,0,0,
  16, 17, 18, 19, 20, 21, 22, 23, 0,0,0,0,0,0,0,0,
  24, 25, 26, 27, 28, 29, 30, 31, 0,0,0,0,0,0,0,0,
  32, 33, 34, 35, 36, 37, 38, 39, 0,0,0,0,0,0,0,0,
  40, 41, 42, 43, 44, 45, 46, 47, 0,0,0,0,0,0,0,0,
  48, 49, 50, 51, 52, 53, 54, 55, 0,0,0,0,0,0,0,0,
  56, 57, 58, 59, 60, 61, 62, 63, 0,0,0,0,0,0,0,0,
};


struct Parameter {
    std::string name;
    int &value;
    int min, max;
};

std::vector <Parameter> params;

// trick to be able to create options
struct CreateParam {
    int _value;
    CreateParam(std::string name, int value, int min, int max) : _value(value) { params.push_back({ name, _value, min, max }); }

    operator int() const {
        return _value;
    }
};

#define TUNE_FLAG       //uncomment this line when running SPSA

#ifndef TUNE_FLAG

#define TUNE_PARAM(name, value, min, max) constexpr int name = value;

#else

#define TUNE_PARAM(name, value, min, max) CreateParam name(#name, value, min, max);

#endif


// search constants
TUNE_PARAM(QsearchFutilityThreshold, 61, 30, 100);
TUNE_PARAM(MaxQsearchDepth, 16, 10, 25);
TUNE_PARAM(MaxRfpDepth, 10, 6, 12);
TUNE_PARAM(RfpThreshold, 83, 50, 110);
TUNE_PARAM(RfpImprovingBonus, 77, 50, 110);
TUNE_PARAM(NmpThreshold, 38, 0, 60);
TUNE_PARAM(NmpThresholdDepth, 5, 2, 6);
TUNE_PARAM(NmpBase, 5, 3, 6);
TUNE_PARAM(NmpDepthDiv, 5, 3, 8);
TUNE_PARAM(NmpEvalDiv, 183, 100, 300);
TUNE_PARAM(NmpEvalMax, 4, 2, 5);
TUNE_PARAM(IIRMinDepth, 2, 1, 5);
TUNE_PARAM(LmpDepth, 4, 3, 8);
TUNE_PARAM(LmpBase, 2, 1, 5);
TUNE_PARAM(FpDepth, 10, 6, 14);
TUNE_PARAM(FpMargin1, 85, 50, 200);
TUNE_PARAM(FpMargin2, 156, 100, 200);
TUNE_PARAM(SeePruningDepth, 9, 5, 12);
TUNE_PARAM(SeePruningQuietMargin, 78, 60, 100);
TUNE_PARAM(SeePruningNoisyMargin, 30, 20, 40);
TUNE_PARAM(SeDepth, 7, 4, 8);
TUNE_PARAM(SeMargin, 70, 32, 96);
TUNE_PARAM(SeDoubleExtMargin, 20, 10, 30);
TUNE_PARAM(LmrHistDiv, 8288, 6000, 10000);
TUNE_PARAM(LmrNoisyHistDiv, 7874, 6000, 10000);
TUNE_PARAM(LmrBase, 0, -2, 10);
TUNE_PARAM(LmrRatio, 24, 16, 30);
TUNE_PARAM(LmrNoisyDiv, 33, 24, 40);
TUNE_PARAM(HistBonusMargin, 316, 250, 500);
TUNE_PARAM(HistBonusMax, 2520, 1800, 3000);
TUNE_PARAM(OptTimeRatio, 60, 50, 80);
TUNE_PARAM(AspStartingWindow, 13, 8, 20);
TUNE_PARAM(MaxAspDepthReduction, 4, 2, 6);
TUNE_PARAM(SeeValPawn, 100, 80, 120);
TUNE_PARAM(SeeValKnight, 450, 400, 500);
TUNE_PARAM(SeeValBishop, 450, 400, 500);
TUNE_PARAM(SeeValRook, 650, 600, 700);
TUNE_PARAM(SeeValQueen, 1250, 1150, 1350);
TUNE_PARAM(HardTimeLimit, 20, 10, 50);
TUNE_PARAM(NodeTmCoeff1, 162, 120, 200);
TUNE_PARAM(NodeTmCoeff2, 148, 110, 190);
TUNE_PARAM(MaterialBase, 700, 500, 900);
TUNE_PARAM(MaterialDiv, 32, 24, 40);

void print_params_for_ob() {
    for (auto& param : params) {
        std::cout << param.name << ", int, " << param.value << ", " << param.min << ", " << param.max << ", " << std::max(0.5, (param.max - param.min) / 20.0) << ", 0.002\n";
    }
}


int SEEVALUES[7] = {0,   SeeValPawn,  SeeValKnight,  SeeValBishop,
                    SeeValRook, SeeValQueen, 10000}; // move ordering purposes


void change_see_values(std::string name, int value){
  int key;
  if (name == "SeeValPawn"){
    key = 1;
  }
  else if (name == "SeeValKnight"){
    key = 2;
  }
  else if (name == "SeeValBishop"){
    key = 3;
  }
  else if (name == "SeeValRook"){
    key = 4;
  }
  else if (name == "SeeValQueen"){
    key = 5;
  }
  SEEVALUES[key] = value;

}
