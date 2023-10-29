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
#include <random>

long int num_fens = 0;

char *export_fen(struct board_info *board, bool color, struct movelist *movelst, int *key, char *fen)
{
    int pos = 0x70;
    int fenkey = 0;
    while (pos >= 0)
    {
        if (pos & 0x88)
        {
            pos -= 0x18;
            if (pos >= 0)
            {
                fen[fenkey++] = '/';
            }
            continue;
        }
        if (board->board[pos])
        {
            switch (board->board[pos])
            {
            case WPAWN:
                fen[fenkey++] = 'P';
                break;
            case WKNIGHT:
                fen[fenkey++] = 'N';
                break;
            case WBISHOP:
                fen[fenkey++] = 'B';
                break;
            case WROOK:
                fen[fenkey++] = 'R';
                break;
            case WQUEEN:
                fen[fenkey++] = 'Q';
                break;
            case WKING:
                fen[fenkey++] = 'K';
                break;
            case BPAWN:
                fen[fenkey++] = 'p';
                break;
            case BKNIGHT:
                fen[fenkey++] = 'n';
                break;
            case BBISHOP:
                fen[fenkey++] = 'b';
                break;
            case BROOK:
                fen[fenkey++] = 'r';
                break;
            case BQUEEN:
                fen[fenkey++] = 'q';
                break;
            case BKING:
                fen[fenkey++] = 'k';
                break;
            default:
                printf("Error parsing board!");
                exit(1);
                break;
            }
        }
        else
        {
            int empty = 1;
            pos++;
            while (!board->board[pos] && !(pos & 0x88))
            {
                empty++;
                pos++;
            }
            fen[fenkey++] = empty + '0';
            pos--;
        }
        pos++;
    }

    fen[fenkey++] = ' ';
    if (color)
    {
        fen[fenkey++] = 'b';
    }
    else
    {
        fen[fenkey++] = 'w';
    }
    fen[fenkey++] = ' ';

    bool castlerights = false;

    if (board->castling[WHITE][1])
    {
        fen[fenkey++] = 'K';
        castlerights = true;
    }
    if (board->castling[WHITE][0])
    {
        fen[fenkey++] = 'Q';
        castlerights = true;
    }

    if (board->castling[BLACK][1])
    {
        fen[fenkey++] = 'k';
        castlerights = true;
    }
    if (board->castling[BLACK][0])
    {
        fen[fenkey++] = 'q';
        castlerights = true;
    }

    if (!castlerights)
    {
        fen[fenkey++] = '-';
    }
    fen[fenkey++] = ' ';
    if (board->epsquare)
    {
        int tempsquare = board->epsquare + (color ? SOUTH : NORTH);
        fen[fenkey++] = (char)(tempsquare % 16 + 97);
        fen[fenkey++] = tempsquare / 16 + 1 + '0';
    }
    else
    {
        fen[fenkey++] = '-';
    }
    fen[fenkey++] = ' ';

    sprintf(fen + fenkey, "%d", movelst[*key - 1].halfmoves);
    while (fen[fenkey] != '\0')
    {
        fenkey++;
    }
    fen[fenkey++] = ' ';

    sprintf(fen + fenkey, "%d", (*key) / 2);
    while (fen[fenkey] != '\0')
    {
        fenkey++;
    }
    fen[fenkey++] = '\0';

    return fen;
}

struct move random_move(struct board_info *board, bool color, bool incheck, ThreadInfo *thread_info)
{
    struct list list[LISTSIZE];
    int movelen = movegen(board, list, color, incheck);
    struct list legalmovelist[LISTSIZE];
    int legalmoves = 0;
    for (int i = 0; i < movelen; i++)
    {
        struct board_info board2 = *board;
        long long unsigned int temp = thread_info->CURRENTPOS;
        move(&board2, list[i].move, color, thread_info);
        // nnue_state.pop();
        thread_info->CURRENTPOS = temp;
        if (!isattacked(&board2, board2.kingpos[color], color ^ 1))
        {
            legalmovelist[legalmoves] = list[i];
            legalmoves++;
        }
    }
    if (!legalmoves)
    {
        return nullmove;
    }
    int randIndex = rand() % (legalmoves);
    return legalmovelist[randIndex].move;
}

float game(const std::string &filename, ThreadInfo *thread_info)
{
    clearTT();
    clearHistory(true, thread_info);
    clearKiller(thread_info);
    search_age = 0;
    srand(std::random_device()());
    struct board_info board;
    struct movelist movelst[MOVESIZE];
    setfull(&board);
    thread_info->nnue_state.reset_nnue(&board);
    calc_pos(&board, WHITE, thread_info);
    int key;
    setmovelist(movelst, &key, thread_info);

    char fen[100];

    bool color = WHITE;
    int moves = 8 + (rand() % 2);

    for (int i = 0; i < moves; i++)
    {
        struct move mve = random_move(&board, color, isattacked(&board, board.kingpos[color], color ^ 1), thread_info);
        if (ismatch(mve, nullmove))
        {
            return 0;
        }
        int piecetype = board.board[mve.move >> 8] - 1;
        move(&board, mve, color, thread_info);
        move_add(&board, movelst, &key, mve, color, (mve.flags == 0xC || board.board[mve.move & 0xFF]), thread_info, piecetype);
        color ^= 1;
    }

    bool game_end = false;
    int decisive_flag = 0;
    char fens[1000][150];
    int fkey = 0;
    float res = 0.5;

    memset(fens, '\0', sizeof(fens));
    std::ofstream fr;
    fr.open(filename, std::ios::out | std::ios::app);

    while (!game_end && res == 0.5 && key < 900 && !checkdraw1(&board) && !checkdraw2(movelst, &key))
    {
        char fen[100];
        export_fen(&board, color, movelst, &key, fen);

        struct list list[LISTSIZE];
        int movelen = movegen(&board, list, color, isattacked(&board, board.kingpos[color], color ^ 1));
        struct list legalmovelist[LISTSIZE];
        int legalmoves = 0;
        for (int i = 0; i < movelen; i++)
        {
            struct board_info board2 = board;
            long long unsigned int temp = thread_info->CURRENTPOS;
            if (!move(&board2, list[i].move, color, thread_info)){
                legalmovelist[legalmoves] = list[i];
                legalmoves++;
                thread_info->nnue_state.pop();
            }
            // nnue_state.pop();
            thread_info->CURRENTPOS = temp;
        }
        if (!legalmoves)
        {
            break;
        }

        if (movelst[2].move.move == 0)
        {
            exit(0);
        }

        thread_info->currentmove = nullmove;
        start_time = std::chrono::steady_clock::now();
        int g = iid_time(&board, movelst, 5000, &key, color, true, false, nullmove, thread_info);


        if (color)
        {
            g = -g;
        }
        if (abs(g) > 1000)
        {
            if (g > 0)
            {
                res = 1;
            }
            else
            {
                res = 0;
            }
            break;
        }

        if (thread_info->currentmove.move == 0)
        {
            printfull(&board);
            g = iid_time(&board, movelst, 5000, &key, color, true, true, nullmove, thread_info);
            printf("%i\n", g);
            exit(0);
        }

        bool isnoisy = (thread_info->currentmove.flags == 0xC || thread_info->currentmove.flags == 0x7 || board.board[thread_info->currentmove.move & 0xFF]);
        bool incheck = isattacked(&board, board.kingpos[color], color ^ 1);

        bool legalcheck = false;
        for (int i = 0; i < legalmoves; i++){
            if (ismatch(legalmovelist[i].move, thread_info->currentmove)){
                legalcheck = true;
                break;
            }
        }
        if (!legalcheck){
            printf("%x %i\n", thread_info->currentmove.move, color);
            for (int i = 0; i < key; i++){
                printf("%x ", movelst[i].move.move);
            }
            printf("\n");
            printfull(&board);
            start_time = std::chrono::steady_clock::now();
            g = iid_time(&board, movelst, 5000, &key, color, true, true, nullmove, thread_info);
            exit(0);
        }
        int piecetype = board.board[thread_info->currentmove.move >> 8] - 1;
        move(&board, thread_info->currentmove, color, thread_info);
        move_add(&board, movelst, &key, thread_info->currentmove, color, isnoisy, thread_info, piecetype);
        bool ischeck = isattacked(&board, board.kingpos[color ^ 1], color);

        if (!(isnoisy || incheck))
        {
            sprintf(fens[fkey++], "%s | %d | ", fen, g);
        }
        color ^= 1;
    }
    char result[8];
    sprintf(result, "%.1f", res);
    for (int i = 0; i < fkey; i++)
    {
        num_fens++;
        if (num_fens % 100000 == 0)
        {
            printf("%li fens written to file %s\n", num_fens, filename.c_str());
        }
        fr << fens[i] << result << "\n";
    }
    fr.close();
    return 0;
}

void run_game(const std::string &filename, ThreadInfo &thread_info)
{
    printf("Writing data into file %s\n", filename.c_str());
    while (1)
    {

        game(filename, &thread_info);
    }
}

int main(int argc, char *argv[])
{
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    MAXDEPTH = 99;
    initglobals();
    unsigned long long init[4] = {0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    init_by_array64(init, 4);
    setzobrist();
    NODES_IID = 12000;
    int target = 32 * 1024 * 1024;
    int size = 0;
    auto thread_info = std::make_unique<ThreadInfo>();
    thread_info->nnue_state.m_accumulator_stack.reserve(MOVESIZE);
    while (sizeof(struct ttentry) * (1 << size) < target)
    {
        size++;
    }
    TT = (struct ttentry *)malloc(sizeof(struct ttentry) * (1 << size));
    TTSIZE = 1 << size;
    _mask = TTSIZE - 1;
    maximumtime = 1000, coldturkey = 1000;
    run_game(argv[1], *thread_info);
}
