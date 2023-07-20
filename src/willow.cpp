#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "constants.h"
#include "globals.h"
#include "board.h"
#include "movegen.h"
#include "search.h"

char *getsafe(char *buffer, int count) // Gets a number of characters up to count (the size of the buffer in this case)
{
    char *result = buffer, *np;
    if ((buffer == NULL) || (count < 1)) // if we have neither a buffer or any space to put it, return null
    {
        result = NULL;
    }
    else
    {
        result = fgets(buffer, count, stdin);
        if (result == NULL) // a check for EOF (indicating that whatever is running Willow (OB etc.) has been killed and it's time to exit)
        {
            free(TT);
            exit(0);
        }
    }
    if ((np = strchr(buffer, '\n'))) // remove trailing newline
        *np = '\0';
    return result;
}

long long unsigned int perft(int depth, struct board_info *board, bool color, bool first) // Performs a perft search to the desired depth, displaying results for each move at the root.
{
    if (!depth)
    {
        return 1; // a terminal node
    }
    struct list list[LISTSIZE];
    int nmoves, i;
    long long unsigned int l = 0;
    nmoves = movegen(board, list, color, false);
    for (i = 0; i < nmoves; i++) // Loop through all of the moves, skipping illegal ones.
    {
        struct board_info board2 = *board;
        move(&board2, list[i].move, color);
        if (isattacked(&board2, board2.kingpos[color], color ^ 1))
        {
            nnue_state.pop();
            continue;
        }
        long long unsigned int b = perft(depth - 1, &board2, color ^ 1, false);
        if (first)
        {
            char temp[6];
            printf("%llu %s\n", b, conv(list[i].move, temp));
        }
        nnue_state.pop();
        l += b;
    }
    return l;
}

void move_uci(char *command, int i, struct board_info *board, struct movelist *movelst, int *key, bool *color)
{
    // parses a uci move such as "e2e4" internally and plays it.
    while (command[i] != '\0' && command[i] != '\n')
    {
        while (isblank(command[i]) && command[i] != '\0')
        {
            i++;
        }
        char buf[8];
        int a = 0;
        while (isalnum(command[i]))
        {
            buf[a] = command[i];
            i++;
            a++;
        }
        buf[a] = '\0';
        struct move temp;
        convto(buf, &temp, board);
        bool iscap = (temp.flags == 0xC || board->board[temp.move & 0xFF]); // captures reset the halfmove clock for 50-move rule draws, so it's important to check that the move is a capture.
        move(board, temp, *color);

        move_add(board, movelst, key, temp, *color, iscap);

        *color ^= 1;
    }
}

int com_uci(struct board_info *board, struct movelist *movelst, int *key, bool *color) // handles various UCI options.
{                                                                                      // assumes board+movelist have been already set up under wraps

    char command[65536];
    getsafe(command, sizeof(command));
    if (command[0] == '\0')
    {
        return 0;
    }
    if (!strcmp(command, "stop"))
    {
        return 1;
    }

    if (!strcmp(command, "uci"))
    {
        printf("id name Willow 2.9\n");
        printf("id author Adam Kulju\n");
        // send options
        printf("option name Hash type spin default 32 min 1 max 1028\n");
        printf("option name Threads type spin default 1 min 1 max 1\n");

        printf("uciok\n");
    }
    if (!strcmp(command, "isready"))
    {
        printf("readyok\n");
    }
    if (!strcmp(command, "ucinewgame"))
    {
        clearTT();
        clearKiller();
        clearCounters();
        clearHistory(true);
        setfull(board);
        setmovelist(movelst, key);
        search_age = 0;
    }

    if (!strcmp(command, "quit"))
    {
        free(TT);
        exit(0);
    }

    if (strstr(command, "setoption name Hash value"))
    {
        int a = atoi(&command[26]);
        int target = a * 1024 * 1024;
        int size = 0;
        while (sizeof(struct ttentry) * (1 << size) < target) // Set the hash to 2^n entires where n is the largest number that satisfies 2^n < the option that was set
        {
            size++;
        }
        size--;
        if (TT)
        {
            free(TT);
        }
        TT = (struct ttentry *)malloc(sizeof(struct ttentry) * (1 << size));
        TTSIZE = 1 << size;
        _mask = TTSIZE - 1;
    }

    if (strstr(command, "position startpos"))
    {
        *color = WHITE;
        setfull(board);
        nnue_state.reset_nnue(board);
        setmovelist(movelst, key);
        calc_pos(board, WHITE);
        *key = 1;
        if (strstr(command, "moves"))
        {
            move_uci(command, 24, board, movelst, key, color);
        }
    }
    if (strstr(command, ("position fen")))
    {
        setfromfen(board, movelst, key, command, color, 13);
        if (strstr(command, "moves"))
        {
            int i = 20;
            while (command[i] != 's')
            {
                i++;
            }
            i++;
            move_uci(command, i, board, movelst, key, color);
        }
    }
    if (strstr(command, "go"))
    {
        float time = 1;
        if (strstr(command, "infinite"))
        {
            time = 1000000;
            coldturkey = 1000000;
        }
        else if (strstr(command, "movetime"))
        {
            time = atoi(command + 12) / 1000;
            coldturkey = time;
        }

        else if (strstr(command, "wtime")) // the "wtime" command indicates that Willow is playing a game currently. In this case, we use information about
                                           // our time in order to get an optimal thinking time for the move before searching.
        {
            int k = 0;
            int movestogo = -1;
            if (strstr(command, "movestogo"))
            {
                int m = 0;
                while (command[m] != 'm' || command[m + 1] != 'o' || command[m + 2] != 'v' || command[m + 3] != 'e' || command[m + 4] != 's')
                {
                    m++;
                }
                while (!isdigit(command[m]))
                {
                    m++;
                }
                movestogo = atoi(&command[m]);
                while (command[k] != 'w')
                {
                    k++;
                }
                k += 6;
            }
            else
            {
                k = 9;
            }

            if (*color == BLACK)
            {
                while (!isblank(command[k]))
                {
                    k++;
                }
                while (isblank(command[k]))
                {
                    k++;
                }
                while (!isblank(command[k]))
                {
                    k++;
                }
                while (isblank(command[k]))
                {
                    k++;
                } // we need to skip past the "1000 btime part"
            }

            int milltime = atoi(&command[k]) - 50;
            if (milltime < 1)
            {
                time = 0.001;
                coldturkey = -0.001;
            }
            else
            {
                coldturkey = (float)milltime / 1000;

                if (movestogo != -1)
                {
                    time = ((float)milltime / (1000 * (movestogo + 2))) * 1.5;
                }
                else
                {
                    int movesleft = MAX(20, 70 - (*key / 2));
                    time = ((float)milltime / (1000 * movesleft)) * 1.5;
                }

                if (strstr(command, "winc"))
                {
                    while (command[k] != 'w')
                    {
                        k++;
                    } // brings it to the "winc" part

                    while (!isblank(command[k]))
                    {
                        k++;
                    }
                    while (isblank(command[k]))
                    {
                        k++;
                    } // skips past the "winc" part to the numbers

                    if (*color == BLACK)
                    {
                        while (!isblank(command[k]))
                        {
                            k++;
                        }
                        while (isblank(command[k]))
                        {
                            k++;
                        }
                        while (!isblank(command[k]))
                        {
                            k++;
                        }
                        while (isblank(command[k]))
                        {
                            k++;
                        } // if we're playing as black, we need to skip again to past "binc". it's usually the same, but eventually it might not be so always good to plan ahead.
                    }
                    milltime = atoi(&command[k]);
                    if (time + ((float)milltime / 1000 * 4) < coldturkey)
                    { // if you have at least four increments left over, it's safe to add half the increment to your move.
                        time += (float)milltime / 1000 * 0.5;
                    }
                }
            }
        }

        time = MAX(time, 0.001);
        printf("%f %f\n", coldturkey, time);
        iid_time(board, movelst, time, key, *color, false, true, nullmove);
    }
    // fflush(hstdin);
    return 0;
}

int bench() // Benchmarks Willow, printing total nodes and nodes per second.
{
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
        {"2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 23 93\0"}};
    unsigned long int t = 0;
    clock_t start = clock();
    for (int i = 0; i < 50; i++) // clear all game info between positions
    {

        clearTT();
        clearKiller();
        clearCounters();
        clearHistory(true);
        search_age = 0;

        struct board_info board;
        struct movelist movelst[MOVESIZE];
        int key;
        bool color;
        /*setfull(&board);
        nnue_state.reset_nnue(&board);
        printfull(&board);
        printf("%i\n", eval(&board, BLACK));
        exit(0);*/

        setfromfen(&board, movelst, &key, positions[i], &color, 0);

        printfull(&board);

        iid_time(&board, movelst, 1000000, &key, color, false, true, nullmove);
        t += nodes;
    }
    printf("Bench: %lu nodes %i nps\n", t, (int)(t / ((clock() - start) / (float)CLOCKS_PER_SEC)));
    return 1;
}

int com()
{
    struct board_info board;
    setfull(&board);
    struct movelist movelst[MOVESIZE];
    int key;
    calc_pos(&board, WHITE);
    setmovelist(movelst, &key);
    bool color;

    for (;;)
    {
        com_uci(&board, movelst, &key, &color);
    }
    return 0;
}

int init() // sets up I/O and all the global variables/lookup tables
{

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    MAXDEPTH = 99;
    initglobals();
    unsigned long long init[4] = {0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    init_by_array64(init, 4);
    setzobrist();
    return 0;
}

int main(int argc, char *argv[])
{

    init();
    if (argc > 1)
    {

        if (!strcmp(argv[1], "bench"))
        {
            bench();
            // printf("%f\n", (float)betas / total);
            exit(0);
        }
        else if (!strcmp(argv[1], "perft"))
        {
            struct board_info board;
            setfull(&board);
            nnue_state.reset_nnue(&board);
            int depth;
            if (argc == 2)
            {
                depth = 6;
            }
            else
            {
                depth = atoi(argv[2]);
            }
            clock_t start = clock();
            long long unsigned int a = perft(depth, &board, WHITE, true);
            float time_elapsed = (float)(clock() - start) / CLOCKS_PER_SEC;
            printf("%llu nodes %i nps\n", a, (int)(a / ((clock() - start) / (float)CLOCKS_PER_SEC)));
            exit(0);
        }
    }

    com();
    return 0;
}
