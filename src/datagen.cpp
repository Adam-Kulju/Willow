#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "constants.h"
#include "globals.h"
#include "board.h"
#include "movegen.h"
#include "search.h"
#include <thread>
#include <iostream>
#include <fstream>
#include <string>


long int num_fens = 0;

char* export_fen(struct board_info *board, bool color, struct movelist *movelst, int *key, char *fen){
    int pos = 0x70;
    int fenkey = 0;
    while (pos >= 0){
        if (pos & 0x88){
            pos -= 0x18;
            if (pos >= 0){
                fen[fenkey++] = '/';
            }
            continue;
        }
        if (board->board[pos]){
            switch (board->board[pos]){
                case WPAWN:
                fen[fenkey++] = 'P'; break;
                case WKNIGHT:
                fen[fenkey++] = 'N'; break;
                case WBISHOP:
                fen[fenkey++] = 'B'; break;
                case WROOK:
                fen[fenkey++] = 'R'; break;
                case WQUEEN:
                fen[fenkey++] = 'Q'; break;
                case WKING:
                fen[fenkey++] = 'K'; break;
                case BPAWN:
                fen[fenkey++] = 'p'; break;
                case BKNIGHT:
                fen[fenkey++] = 'n'; break;
                case BBISHOP:
                fen[fenkey++] = 'b'; break;
                case BROOK:
                fen[fenkey++] = 'r'; break;
                case BQUEEN:
                fen[fenkey++] = 'q'; break;
                case BKING:
                fen[fenkey++] = 'k'; break;
                default:
                printf("Error parsing board!");
                exit(1); 
                break;
            }
        }
        else{
            int empty = 1;
            pos++;
            while (!board->board[pos] && !(pos & 0x88)){
                empty++;
                pos++;
            }
            fen[fenkey++] = empty + '0';
            pos--;
        }
            pos++;
    }

    fen[fenkey++] = ' ';
    if (color){
        fen[fenkey++] = 'b';
    }
    else{
        fen[fenkey++] = 'w';
    }
    fen[fenkey++] = ' ';

    bool castlerights = false;

    if (board->castling[WHITE][1]){
        fen[fenkey++] = 'K'; castlerights = true;
    }
    if (board->castling[WHITE][0]){
            fen[fenkey++] = 'Q'; castlerights = true; 
    }


    if (board->castling[BLACK][1]){
        fen[fenkey++] = 'k'; castlerights = true;
    }
    if (board->castling[BLACK][0]){
            fen[fenkey++] = 'q'; castlerights = true;
    }

    if (!castlerights){
        fen[fenkey++] = '-';
    }
    fen[fenkey++] = ' ';
    if (board->epsquare){
        int tempsquare = board->epsquare + (color ? SOUTH : NORTH);
        fen[fenkey++] = (char) (tempsquare % 16 + 97);
        fen[fenkey++] = tempsquare / 16 + 1 + '0';
    }
    else{
        fen[fenkey++] = '-';
    }
    fen[fenkey++] = ' ';

    sprintf(fen + fenkey, "%d", movelst[*key-1].halfmoves);
    while (fen[fenkey] != '\0'){fenkey++;}
    fen[fenkey++] = ' ';

    sprintf(fen + fenkey, "%d", (*key)/2);
    while (fen[fenkey] != '\0'){fenkey++;}
    fen[fenkey++] = '\0';

    return fen;
}

struct move random_move(struct board_info *board, bool color, bool incheck){
    struct list list[LISTSIZE];
    int movelen = movegen(board, list, color, incheck);
    struct list legalmovelist[LISTSIZE];
    int legalmoves = 0;
    for (int i = 0; i < movelen; i++){
        struct board_info board2 = *board;
        long long unsigned int temp = CURRENTPOS;
        move(&board2, list[i].move, color);
        //nnue_state.pop();
        CURRENTPOS = temp;
        if (!isattacked(&board2, board2.kingpos[color], color ^ 1)){
            legalmovelist[legalmoves] = list[i];
            legalmoves++;
        }
    }
    if (!legalmoves){
        return nullmove;
    }
    int randIndex = rand() % (legalmoves);
    return legalmovelist[randIndex].move;
}

float game(std::ofstream& file){
    clearTT();
    clearKiller();
    clearCounters();
    clearHistory(true);
    search_age = 0;
    srand(clock());
    struct board_info board;
    struct movelist movelst[MOVESIZE];
    setfull(&board);
    nnue_state.reset_nnue(&board);
    calc_pos(&board, WHITE);
    int key;
    setmovelist(movelst, &key);

    char fen[100];

    bool color = WHITE;
    int moves = 8 + (rand() % 2);

    for (int i = 0; i < moves; i++){
        struct move mve = random_move(&board, color, isattacked(&board, board.kingpos[color], color^1));
        if (ismatch(mve, nullmove)){return 0;}
        move(&board, mve, color);
        move_add(&board, movelst, &key, mve, color, (mve.flags == 0xC || board.board[mve.move & 0xFF]));
        color ^= 1;
    }

    bool game_end = false;
    int decisive_flag = 0;
    char fens[1000][150];
    int fkey = 0;
    float res = 0.5;
    memset(fens, '\0', sizeof(fens));
    while (!game_end && res == 0.5 && key < 900 && !checkdraw1(&board) && !checkdraw2(movelst, &key)){

        struct list list[LISTSIZE];
        int movelen = movegen(&board, list, color, isattacked(&board, board.kingpos[color], color^1));
        struct list legalmovelist[LISTSIZE];
        int legalmoves = 0;
        for (int i = 0; i < movelen; i++){
            struct board_info board2 = board;
            long long unsigned int temp = CURRENTPOS;
            move(&board2, list[i].move, color);
            //nnue_state.pop();
            CURRENTPOS = temp;
            if (!isattacked(&board2, board2.kingpos[color], color ^ 1)){
                legalmovelist[legalmoves] = list[i];
                legalmoves++;
            }
        }
        if (!legalmoves){
            break;
        }

        if (movelst[2].move.move == 0){
            exit(0);
        }

        int g = 0;

        int r = rand() % 100;
        if (r == 99){
            currentmove = random_move(&board, color, isattacked(&board, board.kingpos[color], color^1));
        }
        else if (r >= 94){                                                                  //Play the second best move if it (a) exists and (b) is not "much" worse than the best move.
            g = iid_time(&board, movelst, 5000, &key, color, true, false, nullmove);

        if (legalmoves > 1){

            struct move tmp = currentmove;
            int d = iid_time(&board, movelst, 5000, &key, color, true, false, tmp);
            if (d + 75 < g){
                currentmove = tmp;
                d = g;
            }
        }

        }
        else{
            g = iid_time(&board, movelst, 5000, &key, color, true, false, nullmove);
        }

        if (color){
            g = -g;
        }
        if (abs(g) > 1000){
                if (g > 0){
                    res = 1;
                }
                else{
                    res = 0;
                }
                break;
        }

        if (currentmove.move == 0){
            printfull(&board);
            g = iid_time(&board, movelst, 5000, &key, color, true, true, nullmove);
            printf("%i\n", g);
            exit(0);
        }

        bool isnoisy = (currentmove.flags == 0xC || currentmove.flags == 0x7 || board.board[currentmove.move & 0xFF]);
        bool incheck = isattacked(&board, board.kingpos[color], color^1);
        move(&board, currentmove, color);
        move_add(&board, movelst, &key, currentmove, color, isnoisy);
        bool ischeck = isattacked(&board, board.kingpos[color^1], color);

        if (!(isnoisy || incheck || ischeck || decisive_flag || r == 99)){
            char fen[100]; export_fen(&board, color, movelst, &key, fen);
            sprintf(fens[fkey++], "%s | %d | ", fen, g);
        }
        color ^= 1;
    }
    char result[8];
    sprintf(result, "%.1f", res);
    for (int i = 0; i < fkey; i++){
        num_fens++;
        if (num_fens % 100000 == 0){
            printf("%li fens generated\n", num_fens);
        }
        file << fens[i] << result << "\n";
    }
    return 0;
}

void run_game(){
    srand(clock());
    std::string filename = "data" + std::to_string(rand()) + ".txt";
    printf("Writing data into file %s\n", filename.c_str());
    while (1){
        std::ofstream fr;
        fr.open(filename, std::ios::out | std::ios::app);
        game(fr);
        fr.close();
    }
}

int main(int argc, char *argv[]){
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    MAXDEPTH = 99;
    initglobals();
    unsigned long long init[4] = {0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    init_by_array64(init, 4);
    setzobrist();
    NODES_IID = 5000;
    int target = 32 * 1024 * 1024;
    int size = 0;
    nnue_state.m_accumulator_stack.reserve(MOVESIZE);
    while (sizeof(struct ttentry) * (1 << size) < target)
    {
        size++;
    }
    TT = (struct ttentry *)malloc(sizeof(struct ttentry) * (1 << size));
    TTSIZE = 1 << size;
    _mask = TTSIZE - 1;
    
  std::vector<std::thread> threads;
  for (int i = 0; i < atoi(argv[1]); ++i)
    threads.push_back(std::thread(run_game));

  // do some other stuff

  // loop again to join the threads
  for (auto& t : threads)
    t.join();  
    


}
