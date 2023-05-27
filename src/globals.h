#ifndef __globals__
#define __globals__
#include "constants.h"
#include <math.h>
#include <ctype.h>
#include <stdio.h>

clock_t start_time;
float maximumtime;
float coldturkey;
bool isattacker;
char attacksw;
char attacksb;

int maxdepth;

int attackers[2];

char current_mobility;

struct move currentmove;
short int search_age;

short int MAXDEPTH;

unsigned long int nodes;
long int totals;
int betas, total;
int king_attack_count[2];
int attacking_pieces[2];

int LMRTABLE[100][LISTSIZE];
char KINGZONES[2][0x80][0x80];

bool CENTERWHITE[0x88];
bool CENTERBLACK[0x88];

struct move KILLERTABLE[100][2];
struct move COUNTERMOVES[6][128];
long int HISTORYTABLE[2][0x80][0x80]; //allows for faster lookups

static unsigned long long mt[NN]; 
static int mti=NN+1; 
unsigned long long int ZOBRISTTABLE[774];
//to access zobrist number for say white knight on a6 we go zobristtable[(64*piece)+(file-1*8) + rank-1]
//768-771 is castling
//color key is 772
//ep possible is 773
unsigned long long int CURRENTPOS;


struct ttentry *TT;
long int TTSIZE;
long int _mask;

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
    coldturkey = 1000000;
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

void clearTT(){
    int i; for (i = 0; i < TTSIZE; i++){
        TT[i] = nullTT;
        }
        
    }

void clearHistory(bool del){
    if (!del){
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

bool ismatch(struct move move1, struct move move2){    
    if (move1.move == move2.move && move1.flags == move2.flags){
        return true;
    }
    return false;
}

void insert(unsigned long long int position, int depthleft, int eval, char type, struct move bestmove, int search_age){
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




#endif