#ifndef __globals__
#define __globals__
#include "constants.h"
#include "nnue.h"
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <thread>

clock_t start_time; // stores the time that the engine started a search
float maximumtime;  // the time that the engine has gotten for time management purposes
float coldturkey;   // the total amount of time that the engine has in the game
bool isattacker;    // a temp global for if a piece is an attacker on king

int maxdepth; // used for selective depth

int attackers[2]; // the number of attackers on king

int NODES_IID = 0;


short int search_age;    // search age for TT purposes

short int MAXDEPTH; // The maximum depth of a position (set to 14 for bench and 99 normally)

unsigned long int nodes;  // self explanatory
long int totals;          // total number of nodes spent on a search
int betas, total;         // move ordering trackers
int king_attack_count[2]; // attacks on king for White and Black
int attacking_pieces[2];  // number of attacks on pieces for threats purposes

int LMRTABLE[100][LISTSIZE];   // LMR constant table
char KINGZONES[2][0x80][0x80]; // a quick lookup to tell if a square is in the kingzone of a king at a particular square

bool CENTERWHITE[0x88]; // lookup table for White's center
bool CENTERBLACK[0x88]; // lookup table for Black's center


struct ThreadInfo{
    struct move KILLERTABLE[100][2];      // Stores killer moves
    struct move COUNTERMOVES[6][128];     // Stores countermoves
    int HISTORYTABLE[2][0x80][0x80]; // The History table
    int CONTHIST[6][128][6][128];
    unsigned long long int CURRENTPOS;
    NNUE_State nnue_state{};
    struct move currentmove; // The engine's current best move at root
    int id;
    struct board_info *board;
    struct movelist *movelst;
    int *key;
    bool stop;
};

std::vector<std::thread> threads; 
std::vector<ThreadInfo> thread_infos;
//ThreadInfo thread_info;

static unsigned long long mt[NN];
static int mti = NN + 1;
unsigned long long int ZOBRISTTABLE[774];
// to access zobrist number for say white knight on a6 we go zobristtable[(64*piece)+(file-1*8) + rank-1]
// 768-771 is castling
// color key is 772
// ep possible is 773

struct ttentry *TT;
size_t TTSIZE;
unsigned long long _mask;

void initglobals() // Initialize all our global variable stuff.
{

    int target = 32 * 1024 * 1024;
    int size = 0;
    while (sizeof(struct ttentry) * (1 << size) < target)
    {
        size++;
    }
    TT = (struct ttentry *)malloc(sizeof(struct ttentry) * (1 << size));
    TTSIZE = 1 << size;
    _mask = TTSIZE - 1;

    for (unsigned char i = 0; i < 0x80; i++)
    {
        if (i & 0x88)
        {
            i += 7;
            continue;
        }
        for (unsigned char n = 0; n < 8; n++)
        {
            if (!((i + vector[4][n]) & 0x88))
            { // set king attacks. this will give us a value of 2 for side (i.e. rook checks) and 1 for diagonals
                KINGZONES[0][i][i + vector[4][n]] = (n & 1) + 1;
                KINGZONES[1][i][i + vector[4][n]] = (n & 1) + 1;
            }
        }
        if (i + 33 < 0x80)
        {
            KINGZONES[0][i][i + 31] = 3, KINGZONES[0][i][i + 32] = 3, KINGZONES[0][i][i + 33] = 3; // and 3 for advanced squares
        }
        if (i - 33 > 0x0)
        {
            KINGZONES[1][i][i - 31] = 3, KINGZONES[1][i][i - 32] = 3, KINGZONES[1][i][i - 33] = 3;
        }

        if ((i & 7) > 1 && (i & 7) < 6 && (i >> 4) > 0 && (i >> 4) < 7) // set center squares
        {
            if ((i >> 4) >= 4)
            {
                CENTERBLACK[i] = true;
            }
            else
            {
                CENTERWHITE[i] = true;
            }
        }
    }
    for (int i = 0; i < 100; i++) // initialize LMR table.
    {
        for (int n = 0; n < LISTSIZE; n++)
        {
            LMRTABLE[i][n] = (int)round(log(i + 1) * log(n + 1) / 1.95);
        }
    }
    coldturkey = 1000000;
}

// This copyright notice below applies to the three functions below them. Together they make a Mersenne Twister random number generator.
// I have little idea how it works, but it does. so.

/*
   A C-program for MT19937-64 (2004/9/29 version).
   Coded by Takuji Nishimura and Makoto Matsumoto.

   This is a 64-bit version of Mersenne Twister pseudorandom number
   generator.

   Before using, initialize the state by using init_genrand64(seed)
   or init_by_array64(init_key, key_length).

   Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   References:
   T. Nishimura, ``Tables of 64-bit Mersenne Twisters''
     ACM Transactions on Modeling and
     Computer Simulation 10. (2000) 348--357.
   M. Matsumoto and T. Nishimura,
     ``Mersenne Twister: a 623-dimensionally equidistributed
       uniform pseudorandom number generator''
     ACM Transactions on Modeling and
     Computer Simulation 8. (Jan. 1998) 3--30.

   Any feedback is very welcome.
   http://www.math.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove spaces)
*/

void init_genrand64(unsigned long long seed)
{
    mt[0] = seed;
    for (mti = 1; mti < NN; mti++)
        mt[mti] = (6364136223846793005ULL * (mt[mti - 1] ^ (mt[mti - 1] >> 62)) + mti);
}

unsigned long long genrand64_int64(void)
{
    int i;
    unsigned long long x;
    static unsigned long long mag01[2] = {0ULL, MATRIX_A};

    if (mti >= NN)
    {
        if (mti == NN + 1)
            init_genrand64(5489ULL);

        for (i = 0; i < NN - MM; i++)
        {
            x = (mt[i] & UM) | (mt[i + 1] & LM);
            mt[i] = mt[i + MM] ^ (x >> 1) ^ mag01[(int)(x & 1ULL)];
        }
        for (; i < NN - 1; i++)
        {
            x = (mt[i] & UM) | (mt[i + 1] & LM);
            mt[i] = mt[i + (MM - NN)] ^ (x >> 1) ^ mag01[(int)(x & 1ULL)];
        }
        x = (mt[NN - 1] & UM) | (mt[0] & LM);
        mt[NN - 1] = mt[MM - 1] ^ (x >> 1) ^ mag01[(int)(x & 1ULL)];

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
    i = 1;
    j = 0;
    k = (NN > key_length ? NN : key_length);
    for (; k; k--)
    {
        mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 62)) * 3935559000370003845ULL)) + init_key[j] + j;
        i++;
        j++;
        if (i >= NN)
        {
            mt[0] = mt[NN - 1];
            i = 1;
        }
        if (j >= key_length)
            j = 0;
    }
    for (k = NN - 1; k; k--)
    {
        mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 62)) * 2862933555777941757ULL)) - i;
        i++;
        if (i >= NN)
        {
            mt[0] = mt[NN - 1];
            i = 1;
        }
    }

    mt[0] = 1ULL << 63;
}

void setzobrist() // Fills the table of Zobrist keys with numbers.
{
    for (int i = 0; i < 774; i++)
    {
        ZOBRISTTABLE[i] = genrand64_int64();
    }
}

void calc_pos(struct board_info *board, bool color, ThreadInfo *thread_info) // Calculates the Zobrist Key for a particular position.
{
    thread_info->CURRENTPOS = 0;
    int i;
    for (i = 0; i < 0x80; i++)
    {
        if (!(i & 0x88) && board->board[i])
        {
            thread_info->CURRENTPOS ^= ZOBRISTTABLE[(((board->board[i] - 2) << 6)) + i - ((i >> 4) << 3)];
            // board->board[i]-2<<6: gives us the base value for each piece*64.
            // i-((i>>4)<<3): converts i into a number from 1 to 64. it does that by subtracting 8 for every rank it's on - on rank 0 doesn't get subtracted, on rank 7 gets -56
        }
    }
    if (color)
    {
        thread_info->CURRENTPOS ^= ZOBRISTTABLE[772];
    }
}

void clearTT() // Clears the Transposition table.
{
    int i;
    for (i = 0; i < TTSIZE; i++)
    {
        TT[i] = nullTT;
    }
}

void clearHistory(bool del, ThreadInfo *thread_info) // Either divides the entries in the history table by 4 (between searches), or resets it entirely (between games)
{
    if (!del)
    {
        /*for (int i = 0; i < 0x80; i++)
        {
            for (int n = 0; n < 0x80; n++)
            {
                HISTORYTABLE[WHITE][i][n] = (HISTORYTABLE[WHITE][i][n] / 2);
                HISTORYTABLE[BLACK][i][n] = (HISTORYTABLE[BLACK][i][n] / 2);
            }
        }
        for (int i = 0; i < 6; i++){
            for (int n = 0; n < 128; n++){
                for (int a = 0; a < 6; a++){
                    for (int b = 0; b < 128; b++){
                        CONTHIST[i][n][a][b] /= 4;
                    }
                }
            }
        }*/
    }
    else
    {

     memset(thread_info->CONTHIST, 0, sizeof(thread_info->CONTHIST));
     memset(thread_info->HISTORYTABLE, 0, sizeof(thread_info->HISTORYTABLE));
          
    }
}
void clearKiller(ThreadInfo *thread_info) // Clears the Killer Table
{
    for (int i = 0; i < 100; i++)
    {
        thread_info->KILLERTABLE[i][0].move = 0;
        thread_info->KILLERTABLE[i][1].move = 0;
    }
}

void clearCounters(ThreadInfo *thread_info) // Clears the countermoves table
{
    for (int i = 0; i < 6; i++)
    {
        for (int n = 0; n < 128; n++)
        {
            thread_info->COUNTERMOVES[i][n] = nullmove;
        }
    }
}

bool ismatch(struct move move1, struct move move2) // Compares two move structs to see if they are equal. Perhaps memcmp() would be faster.
{
    if (move1.move == move2.move && move1.flags == move2.flags)
    {
        return true;
    }
    return false;
}

void insert(unsigned long long int position, int depthleft, int eval, char type, struct move bestmove, int search_age) // Inserts an entry into the transposition table.
{
    int index = (position) & (_mask);

    if (TT[index].zobrist_key == position && !(type == 3 && TT[index].type != 3)) // Overwrite entries of same positions depending on depth, type, and age.
    {
        int agediff = search_age - TT[index].age;
        int newentryb = depthleft + type + ((agediff * agediff) >> 2);
        int oldentryb = TT[index].depth + TT[index].type;
        if (oldentryb * 2 > newentryb * 3)
        {
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

char *conv(struct move move, char *temp) // Converts a internally encoded move to a UCI string.
{
    temp[0] = ((move.move >> 8) & 7) + 97, temp[1] = ((move.move >> 8) >> 4) + 1 + '0', temp[2] = ((move.move & 0xFF) & 7) + 97, temp[3] = ((move.move & 0xFF) >> 4) + 1 + '0';
    if (move.flags >> 2 == 1)
    {
        switch (move.flags)
        {
        case 4:
            temp[4] = 'N';
            break;
        case 5:
            temp[4] = 'B';
            break;
        case 6:
            temp[4] = 'R';
            break;
        default:
            temp[4] = 'Q';
            break;
        }
        temp[5] = '\0';
    }
    else
    {
        temp[4] = '\0';
    }
    return temp;
}

void convto(char *mve, struct move *to_move, struct board_info *board) // Converts a UCI string to an internally coded move.
{
    unsigned short int from = ((atoi(&mve[1]) - 1) << 4) + (mve[0] - 97), to = ((atoi(&mve[3]) - 1) << 4) + (mve[2] - 97);

    to_move->move = (from << 8) + to;
    if ((board->board[from] >> 1) == KING && abs(from - to) == 2)
    {
        to_move->flags = 8;
    }
    else if (board->board[from] >> 1 == PAWN && (from & 7) != (to & 7) && !board->board[to])
    {
        to_move->flags = 0xC;
    }
    else if (isalpha(mve[4]))
    {
        switch (toupper(mve[4]))
        {
        case 'Q':
            to_move->flags = 7;
            break;
        case 'R':
            to_move->flags = 6;
            break;
        case 'B':
            to_move->flags = 5;
            break;
        case 'N':
            to_move->flags = 4;
            break;
        default:
            printf("error\n!");
            exit(1);
        }
    }
    else
    {
        to_move->flags = 0;
    }
}

#endif