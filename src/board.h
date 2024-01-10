#ifndef __board__
#define __board__

#include "constants.h"
#include "globals.h"
#include "nnue.h"
#include <stdio.h>

void printfull(struct board_info *board) // Prints the board
{
  int i = 0x70;
  while (i >= 0) {

    printf("+---+---+---+---+---+---+---+---+\n");
    for (int n = i; n != i + 8; n++) {
      printf("| ");
      if (board->board[n] == BLANK) {
        printf("  ");
      } else {

        switch (board->board[n]) {
        case WPAWN:
          printf("P ");
          break;
        case WKNIGHT:
          printf("N ");
          break;
        case WBISHOP:
          printf("B ");
          break;
        case WROOK:
          printf("R ");
          break;
        case WQUEEN:
          printf("Q ");
          break;
        case WKING:
          printf("K ");
          break;
        case BPAWN:
          printf("p ");
          break;
        case BKNIGHT:
          printf("n ");
          break;
        case BBISHOP:
          printf("b ");
          break;
        case BROOK:
          printf("r ");
          break;
        case BQUEEN:
          printf("q ");
          break;
        default:
          printf("k ");
          break;
        }
      }
    }
    printf("|\n");
    i -= 0x10;
  }
  printf("+---+---+---+---+---+---+---+---+\n\n");
}
void setfull(
    struct board_info *board) // Sets up the board for the start of the game.
{
  char brd[0x80] = {
      WROOK,   WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WROOK,  0,
      0,       0,       0,       0,      0,     0,       0,       WPAWN,  WPAWN,
      WPAWN,   WPAWN,   WPAWN,   WPAWN,  WPAWN, WPAWN,   0,       0,      0,
      0,       0,       0,       0,      0,     0,       0,       0,      0,
      0,       0,       0,       0,      0,     0,       0,       0,      0,
      0,       0,       0,       0,      0,     0,       0,       0,      0,
      0,       0,       0,       0,      0,     0,       0,       0,      0,
      0,       0,       0,       0,      0,     0,       0,       0,      0,
      0,       0,       0,       0,      0,     0,       0,       0,      0,
      0,       0,       0,       0,      0,     0,       0,       0,      0,
      0,       0,       0,       0,      0,     0,       BPAWN,   BPAWN,  BPAWN,
      BPAWN,   BPAWN,   BPAWN,   BPAWN,  BPAWN, 0,       0,       0,      0,
      0,       0,       0,       0,      BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING,
      BBISHOP, BKNIGHT, BROOK,   0,      0,     0,       0,       0,      0,
      0,       0,
  };
  memcpy(board->board, brd, 0x80);
  char count[2][5] = {// the number of pieces on the board
                      {8, 2, 2, 2, 1},
                      {8, 2, 2, 2, 1}};
  memcpy(board->pnbrqcount, count, 10);
  board->castling[0][0] = true, board->castling[0][1] = true,
  board->castling[1][0] = true, board->castling[1][1] = true; // castling data
  board->kingpos[0] = 0x4, board->kingpos[1] = 0x74; // king position data
  board->rookstartpos[0][0] = 0, board->rookstartpos[0][1] = 7,
  board->rookstartpos[1][0] = 0x70, board->rookstartpos[1][1] = 0x77;
  board->epsquare = 0; // en passant square
}

void setdfrc(struct board_info *board,
             int indexNumber) { // sets up a board given a particular DFRC key.
  int indexes[2] = {indexNumber % 960, indexNumber / 960};
  char brd[0x80] = {
      0,     0,     0,     0,     0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, WPAWN, WPAWN, WPAWN, WPAWN,
      WPAWN, WPAWN, WPAWN, WPAWN, 0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, BPAWN, BPAWN, BPAWN, BPAWN,
      BPAWN, BPAWN, BPAWN, BPAWN, 0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, 0,     0,     0,     0,
      0,     0,     0,     0,     0, 0, 0,     0,
  };
  memcpy(board->board, brd, 0x80);
  char count[2][5] = {{8, 2, 2, 2, 1}, {8, 2, 2, 2, 1}};
  memcpy(board->pnbrqcount, count, 10);
  board->castling[0][0] = true, board->castling[0][1] = true,
  board->castling[1][0] = true, board->castling[1][1] = true;
  board->epsquare = 0;

  for (int color = WHITE; color <= BLACK; color++) {
    int rank_base = 0x70 * color;

    int index = indexes[color],
        remainder = indexes[color] %
                    4; // Step 1: given an index N, divide it by 4 and take the
                       // remainder N1; use that to place the LSB.
    index /= 4;
    board->board[rank_base + 1 + remainder * 2] =
        WBISHOP +
        color; //(0, 1, 2, 3) remainders correspond to (b1, d1, f1, h1)

    remainder = index % 4, index /= 4; // Step 2: repeat step 1 to get the DSB
                                       // positions (0 = a1, 1 = c1 etc)
    board->board[rank_base + remainder * 2] = WBISHOP + color;

    remainder = index % 6,
    index /=
        6; // Step 3: divide N by 6 and take the remainder N3; this is the queen
           // position, where 0 = first available square, 1 = second, etc.
    int empty_squares = 0, piece_index;
    for (piece_index = rank_base; piece_index < rank_base + 8; piece_index++) {
      if (!board->board[piece_index]) {
        empty_squares++;
        if (empty_squares > remainder) {
          break;
        }
      }
    }
    board->board[piece_index] = WQUEEN + color;

    empty_squares = 0; // Step 4: place the knights in the same way as the
                       // queen, but by looking up the N5N table.
    bool placed = false;
    for (piece_index = rank_base; piece_index < rank_base + 8; piece_index++) {
      if (!board->board[piece_index]) {
        empty_squares++;
        if ((empty_squares > N5NTABLE[index][0] && !placed) ||
            empty_squares > N5NTABLE[index][1]) {
          board->board[piece_index] = WKNIGHT + color;
          placed = true;
          if (empty_squares > N5NTABLE[index][1]) {
            break;
          }
        }
      }
    }

    empty_squares = 0;
    for (piece_index = rank_base; piece_index < rank_base + 8; piece_index++) {
      if (!board->board[piece_index]) {
        empty_squares++;
        if (empty_squares % 2) {
          board->board[piece_index] = WROOK + color;
          board->rookstartpos[color][(empty_squares - 1) / 2] = piece_index;
        } else {
          board->board[piece_index] = WKING + color;
          board->kingpos[color] = piece_index;
        }
      }
    }
  }
}

int eval(struct board_info *board, int color, ThreadInfo *thread_info) {
  int material = 0;
  for (int i = 1; i < 5; i++) {
    material +=
        SEEVALUES[i + 1] * (board->pnbrqcount[0][i] + board->pnbrqcount[1][i]);
  }
  material = 700 + material / 32;
  return thread_info->nnue_state.evaluate(color) * material /
         1024; // trying to get this material scaling back in order because oops
}

void setmovelist(
    struct movelist *movelst, int *key,
    ThreadInfo *thread_info) // Sets up the list of moves in the game.
{

  movelst[0].fen = thread_info->CURRENTPOS;
  *key = 1;
  movelst[0].move.move = 0;
  movelst[1].move.move = 0;
  movelst[0].halfmoves = 0;
  movelst[1].piecetype = -1, movelst[0].piecetype = -1;
  return;
}

void setfromfen(
    struct board_info *board, struct movelist *movelst, int *key,
    char *fenstring, bool *color, int start,
    ThreadInfo *thread_info) // Given an FEN, sets up the board to it.
{
  int i = 7, n = 0;
  int fenkey = start;
  // Set default board parameters, edit them later on as we add pieces to the
  // board/read more info from the FEN.
  board->castling[WHITE][0] = false, board->castling[BLACK][0] = false,
  board->castling[WHITE][1] = false, board->castling[BLACK][1] = false;
  for (int i = 0; i < 5; i++) {
    board->pnbrqcount[WHITE][i] = 0;
    board->pnbrqcount[BLACK][i] = 0;
  }
  while (!isblank(fenstring[fenkey])) {
    if (fenstring[fenkey] == '/') // Go to the next rank down.
    {
      i--;
      n = 0;
    } else if (isdigit(fenstring[fenkey])) // A number in the FEN (of form P3pp)
                                           // means there's three empty squares,
                                           // so we go past them
    {
      for (int b = 0; b < atoi(&fenstring[fenkey]); b++) {
        board->board[n + (i << 4)] = BLANK;
        n++;
      }
    } else // For each piece, add it to the board and add it to the material
           // count. If it's a king, update king position.
    {
      switch (fenstring[fenkey]) {
      case 'P':
        board->board[n + (i << 4)] = WPAWN;
        board->pnbrqcount[WHITE][0]++;
        break;
      case 'N':
        board->board[n + (i << 4)] = WKNIGHT;
        board->pnbrqcount[WHITE][1]++;
        break;
      case 'B':
        board->board[n + (i << 4)] = WBISHOP;
        board->pnbrqcount[WHITE][2]++;
        break;
      case 'R':
        board->board[n + (i << 4)] = WROOK;
        board->pnbrqcount[WHITE][3]++;
        break;
      case 'Q':
        board->board[n + (i << 4)] = WQUEEN;
        board->pnbrqcount[WHITE][4]++;
        break;
      case 'K':
        board->board[n + (i << 4)] = WKING;
        board->kingpos[WHITE] = n + (i << 4);
        break;
      case 'p':
        board->board[n + (i << 4)] = BPAWN;
        board->pnbrqcount[BLACK][0]++;
        break;
      case 'n':
        board->board[n + (i << 4)] = BKNIGHT;
        board->pnbrqcount[BLACK][1]++;
        break;
      case 'b':
        board->board[n + (i << 4)] = BBISHOP;
        board->pnbrqcount[BLACK][2]++;
        break;
      case 'r':
        board->board[n + (i << 4)] = BROOK;
        board->pnbrqcount[BLACK][3]++;
        break;
      case 'q':
        board->board[n + (i << 4)] = BQUEEN;
        board->pnbrqcount[BLACK][4]++;
        break;
      default:
        board->board[n + (i << 4)] = BKING;
        board->kingpos[BLACK] = n + (i << 4);
        break;
      }
      n++;
    } // sets up the board based on fen
    fenkey++;
  }
  board->rookstartpos[0][0] = 0, board->rookstartpos[0][1] = 7,
  board->rookstartpos[1][0] = 0x70,
  board->rookstartpos[1][1] = 0x77; // someday, I may supprt DFRC fen parsing,
                                    // but that day is not today.
  for (int i = 0; i < 8; i++) {
    for (int n = 8; n < 16; n++) {
      board->board[i * 16 + n] = BLANK;
    }
  }

  while (isblank(fenstring[fenkey])) {
    fenkey++;
  }
  if (fenstring[fenkey] == 'w') // Gets the color to move.
  {
    *color = WHITE;
  } else {
    *color = BLACK;
  }
  fenkey++;

  calc_pos(board, *color,
           thread_info); // Gets the Zobrist Hash of the position (it doesn't
                         // matter that we do it before castling stuff)
  thread_info->nnue_state.reset_nnue(board);

  setmovelist(movelst, key, thread_info);
  *key = 0;

  while (isblank(fenstring[fenkey])) {
    fenkey++;
  }

  if (fenstring[fenkey] == '-') {
    fenkey++;
  }

  else {
    if (fenstring[fenkey] == 'K') // Get castling rights information
    {
      board->castling[WHITE][1] = true;
      fenkey++;
    }
    if (fenstring[fenkey] == 'Q') {
      board->castling[WHITE][0] = true;
      fenkey++;
    }
    if (fenstring[fenkey] == 'k') {
      board->castling[BLACK][1] = true;
      fenkey++;
    }
    if (fenstring[fenkey] == 'q') {
      board->castling[BLACK][0] = true;
    }
    fenkey++;
  }

  while (isblank(fenstring[fenkey])) {
    fenkey++;
  }

  if (fenstring[fenkey] == '-') // Get en passant square information
  {
    board->epsquare = 0;
  } else {
    board->epsquare =
        (atoi(&fenstring[fenkey + 1]) - 1) * 16 + ((fenstring[fenkey] - 97));
    // In Willow, the en passant square is the square of the piece to be taken
    // en passant, and not the square behind.
    if (*color) {
      board->epsquare += NORTH;
    } else {
      board->epsquare += SOUTH;
    }
    fenkey++;
  }
  fenkey++;

  while (isblank(fenstring[fenkey])) {
    fenkey++;
  }

  movelst[*key].halfmoves =
      atoi(&fenstring[fenkey]); // Get halfmoves information.
  *key += 1;
}

bool isattacked(
    struct board_info *board, unsigned char pos,
    bool encolor) // Is a particular square attacked by an enemy piece?
{
  // pawns
  if (!encolor) {
    if ((!((pos + SW) & 0x88) && board->board[pos + SW] == WPAWN) ||
        (!((pos + SE) & 0x88) && board->board[pos + SE] == WPAWN)) {
      return true;
    }
  } else {
    if ((!((pos + NE) & 0x88) && board->board[pos + NE] == BPAWN) ||
        (!((pos + NW) & 0x88) && board->board[pos + NW] == BPAWN)) {
      return true;
    }
  }
  // knights, kings, and sliders. Work outwards from the square until we bump
  // into a piece/edge of board.
  unsigned char d, f;
  for (f = 0; f < 8; f++) {
    d = pos + vector[0][f];

    if (!(d & 0x88) && board->board[d] - encolor ==
                           WKNIGHT) // if we bump into a knight on its vector,
                                    // we're attacked by it and return true
    {
      return true;
    }

    char vec = vector[4][f];
    d = pos + vec;

    if ((d & 0x88)) {
      continue;
    }

    if (board->board[d] - encolor ==
        WKING) // if we hit a king on our first square we're attacked by it
    {
      return true;
    }
    do {
      if (board->board[d]) {
        // Queens attack a piece on all diagonals/sides, rooks attack only
        // orthogonally, bishops attack only diagonally
        if ((board->board[d] & 1) == encolor &&
            (board->board[d] - encolor == WQUEEN ||
             (((f & 1) && board->board[d] - encolor == WROOK) ||
              (!(f & 1) && board->board[d] - encolor == WBISHOP)))) {
          return true;
        }
        break;
      }
      d += vec;
    } while (!(d & 0x88));
  }

  return false;
}

int move(struct board_info *board, struct move move, bool color,
         ThreadInfo *thread_info) // Perform a move on the board.
{
  if (!move.move) {
    if (board->epsquare) {
      thread_info->CURRENTPOS ^=
          ZOBRISTTABLE[773]; // if en passant was possible last move, xor it so
                             // it is not.
    }
    thread_info->CURRENTPOS ^= ZOBRISTTABLE[772]; // xor for turn
    board->epsquare = 0;
    __builtin_prefetch(&TT[(thread_info->CURRENTPOS) & (_mask)]);
    return 0;
  }
  unsigned char from = move.move >> 8,
                to = move.move & 0xFF; // get the indexes of the move
  unsigned char flag = move.flags >> 2;
  if ((from & 0x88) || (to & 0x88)) {
    printf("out of board index! %4x %x %i %x\n", move.move, (board->epsquare),
           color, move.flags);
    printfull(board);
    exit(0);
  }

  struct board_info board2 = *board;

  if (flag != 3) { // handle captures and en passant - this is for normal moves
    if (board2.board[to]) {
      board2.pnbrqcount[color ^ 1][((board2.board[to] >> 1) - 1)]--;
    }
  } else { // and this branch handles en passant
    board2.pnbrqcount[color ^ 1][0]--;
    board2.board[board2.epsquare] = BLANK;
  }

  if (flag == 1) { // handle promotions
    board2.pnbrqcount[color][0]--;
    board2.board[from] = (((move.flags & 3) + 2) << 1) + color;
    board2.pnbrqcount[color][((move.flags & 3) + 1)]++;
  }

  if (from == board2.kingpos[color]) { // handle king moves
    board2.castling[color][0] = false;
    board2.castling[color][1] = false;
    board2.kingpos[color] = to;
  }

  if (board2.board[from] == WROOK + color) { // handle rook moves

    if (from == board->rookstartpos[color][0]) { // turn off queenside castling
      board2.castling[color][0] = false;
    } else if (from ==
               board->rookstartpos[color][1]) { // turn off kingside castling
      board2.castling[color][1] = false;
    }
  }

  if (!(IS_DFRC && flag == 2)) {
    board2.board[to] = board2.board[from];
    board2.board[from] = BLANK;
  }

  if (flag == 2) { // castle

    if (to > board->kingpos[color]) { // to = g file, meaning kingside
      if (IS_DFRC) {
        board2.board[to] = BLANK;
        board2.board[from] = BLANK;

        board2.board[to / 16 * 16 + 6] = board->board[from];
        board2.board[to / 16 * 16 + 5] = board->board[to];
      } else {
        board2.board[to - 1] = board2.board[to + 1];
        board2.board[to + 1] = BLANK;
      }

    } else {
      if (IS_DFRC) {
        board2.board[from] = BLANK;
        board2.board[to] = BLANK;

        board2.board[to / 16 * 16 + 2] = board->board[from];
        board2.board[to / 16 * 16 + 3] = board->board[to];

      } else {
        board2.board[to + 1] = board2.board[to - 2];
        board2.board[to - 2] = BLANK;
      }
    }
  }
  if (isattacked(&board2, board2.kingpos[color], color ^ 1)) {
    return 1;
  }
  thread_info->nnue_state.push();

  int wking = buckets[WHITE][board2.kingpos[WHITE]], bking = buckets[BLACK][board2.kingpos[BLACK]];

  if (board2.kingpos[color] != board->kingpos[color]){  //if we've moved the king, make sure we didn't move it into another bucket!
    int current_king = color ? bking : wking;
    if (current_king != buckets[color][board->kingpos[color]]){
      thread_info->nnue_state.reset_nnue_color(board, color, current_king);
    }
  }

  if (board->epsquare) {
    thread_info->CURRENTPOS ^=
        ZOBRISTTABLE[773]; // if en passant was possible last move, xor it so it
                           // is not.
  }
  thread_info->CURRENTPOS ^= ZOBRISTTABLE[772]; // xor for turn

  if (!board->board[from]) {
    printfull(board);
    printf("%x\n", move.move);
    exit(0);
  }
  thread_info->CURRENTPOS ^=
      ZOBRISTTABLE[(((board->board[from] - 2) << 6)) + from -
                   ((from >> 4) << 3)]; // xor out the piece to be moved
  thread_info->nnue_state.update_feature<false>(board->board[from],
                                                MAILBOX_TO_STANDARD[from], wking, bking);

  if (flag != 3) { // handle captures and en passant - this is for normal moves
    if (board->board[to]) {
      thread_info->CURRENTPOS ^=
          ZOBRISTTABLE[(((board->board[to] - 2) << 6)) + to - ((to >> 4) << 3)];
      thread_info->nnue_state.update_feature<false>(board->board[to],
                                                    MAILBOX_TO_STANDARD[to], wking, bking);
    }
  } else { // and this branch handles en passant
    thread_info->CURRENTPOS ^=
        ZOBRISTTABLE[(((board->board[board->epsquare] - 2) << 6)) +
                     board->epsquare - ((board->epsquare >> 4) << 3)];
    thread_info->nnue_state.update_feature<false>(
        board->board[board->epsquare], MAILBOX_TO_STANDARD[board->epsquare], wking, bking);
  }

  if (!(IS_DFRC && flag == 2)) {
    thread_info->CURRENTPOS ^=
        ZOBRISTTABLE[(((board2.board[to] - 2) << 6)) + to -
                     ((to >> 4) << 3)]; // xor in the piece that has been moved
    thread_info->nnue_state.update_feature<true>(board2.board[to],
                                                 MAILBOX_TO_STANDARD[to], wking, bking);
  }

  if (flag == 2) { // castle
    if (to > board->kingpos[color]) {

      if (IS_DFRC) {
        int oldpos = to, newpos = to / 16 * 16 + 5;

        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board2.board[newpos] - 2) << 6)) + newpos -
                         ((newpos >> 4)
                          << 3)]; // xor in the piece that has been moved
        thread_info->nnue_state.update_feature<true>(
            board2.board[newpos], MAILBOX_TO_STANDARD[newpos], wking, bking);

        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board->board[oldpos] - 2) << 6)) + oldpos -
                         (((oldpos) >> 4) << 3)]; // xor out the rook on hfile
                                                  // and xor it in on ffile
        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board2.board[newpos] - 2) << 6)) + newpos -
                         (((newpos) >> 4) << 3)];
        thread_info->nnue_state.update_feature<true>(
            board2.board[newpos], MAILBOX_TO_STANDARD[newpos], wking, bking);
        thread_info->nnue_state.update_feature<false>(
            board->board[oldpos], MAILBOX_TO_STANDARD[oldpos], wking, bking);
      }

      else {
        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board->board[to + 1] - 2) << 6)) + to + 1 -
                         (((to + 1) >> 4) << 3)]; // xor out the rook on hfile
                                                  // and xor it in on ffile
        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board2.board[to - 1] - 2) << 6)) + to - 1 -
                         (((to - 1) >> 4) << 3)];
        thread_info->nnue_state.update_feature<true>(
            board2.board[to - 1], MAILBOX_TO_STANDARD[to - 1], wking, bking);
        thread_info->nnue_state.update_feature<false>(
            board->board[to + 1], MAILBOX_TO_STANDARD[to + 1], wking, bking);
      }

    } else {
      if (IS_DFRC) {
        int oldpos = to, newpos = to / 16 * 16 + 3;

        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board2.board[newpos] - 2) << 6)) + newpos -
                         ((newpos >> 4)
                          << 3)]; // xor in the piece that has been moved
        thread_info->nnue_state.update_feature<true>(
            board2.board[newpos], MAILBOX_TO_STANDARD[newpos], wking, bking);

        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board->board[oldpos] - 2) << 6)) + oldpos -
                         (((oldpos) >> 4) << 3)]; // xor out the rook on afile
                                                  // and xor it in on dfile
        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board2.board[newpos] - 2) << 6)) + newpos -
                         (((newpos) >> 4) << 3)];
        thread_info->nnue_state.update_feature<false>(
            board->board[oldpos], MAILBOX_TO_STANDARD[oldpos], wking, bking);
        thread_info->nnue_state.update_feature<true>(
            board2.board[newpos], MAILBOX_TO_STANDARD[newpos], wking, bking);
      } else {
        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board->board[to - 2] - 2) << 6)) + to - 2 -
                         (((to - 2) >> 4) << 3)]; // xor out the rook on afile
                                                  // and xor it in on dfile
        thread_info->CURRENTPOS ^=
            ZOBRISTTABLE[(((board2.board[to + 1] - 2) << 6)) + to + 1 -
                         (((to + 1) >> 4) << 3)];
        thread_info->nnue_state.update_feature<false>(
            board->board[to - 2], MAILBOX_TO_STANDARD[to - 2], wking, bking);
        thread_info->nnue_state.update_feature<true>(
            board2.board[to + 1], MAILBOX_TO_STANDARD[to + 1], wking, bking);
      }
    }
  }

  *board = board2;
  board->epsquare = 0;
  if (!flag && abs(to - from) == 32 && board->board[to] == WPAWN + color) {
    board->epsquare = to;
    thread_info->CURRENTPOS ^= ZOBRISTTABLE[773];
  }
  // printfull(board);
  __builtin_prefetch(&TT[(thread_info->CURRENTPOS) & (_mask)]);
  return 0;
}

void move_add(struct board_info *board, struct movelist *movelst, int *key,
              struct move mve, bool color, bool iscap, ThreadInfo *thread_info,
              int piecetype) // Add a move to the list of moves in the game.
{
  int k = *key;
  movelst[k].move = mve;
  movelst[k].fen = thread_info->CURRENTPOS;
  if ((mve.flags >> 2) == 1 ||
      (board->board[(mve.move & 0xFF)] == WPAWN + color) ||
      iscap) { // if the move is a capture, or a promotion, or a pawn move, half
               // move clock is reset.
    movelst[k].halfmoves = 0;
  } else {
    movelst[k].halfmoves =
        movelst[k - 1].halfmoves + 1; // otherwise increment it
  }
  movelst[k].piecetype = piecetype;
  movelst[k].wascap = iscap;
  *key = k + 1;
}

char isattacked_mv(struct board_info *board, unsigned char pos, bool encolor)
// Same as above, but ignores kings. Slight speed boost for eval purposes.
{
  char flag = 0;
  // pawns
  if (!encolor) {
    if ((!((pos + SW) & 0x88) && board->board[pos + SW] == WPAWN) ||
        (!((pos + SE) & 0x88) && board->board[pos + SE] == WPAWN)) {
      return 2;
    }
  } else {
    if ((!((pos + NE) & 0x88) && board->board[pos + NE] == BPAWN) ||
        (!((pos + NW) & 0x88) && board->board[pos + NW] == BPAWN)) {
      return 2;
    }
  }
  // knights, kings, and sliders
  unsigned char d, f;
  for (f = 0; f < 8; f++) {
    d = pos + vector[0][f];

    if (!(d & 0x88) && board->board[d] - encolor == WKNIGHT) {
      return 2;
    }

    char vec = vector[4][f];
    d = pos + vec;

    if ((d & 0x88)) {
      continue;
    }

    if (board->board[d] - encolor == WKING) {
      flag = 1;
      continue;
    }
    do {
      if (board->board[d]) {
        if ((board->board[d] & 1) == encolor &&
            (board->board[d] - encolor == WQUEEN ||
             (((f & 1) && board->board[d] - encolor == WROOK) ||
              (!(f & 1) && board->board[d] - encolor == WBISHOP)))) {
          return 2;
        }
        break;
      }
      d += vec;
    } while (!(d & 0x88));
  }

  return flag;
}

bool checkdraw1(struct board_info *board) // checks for material draws.
{
  if (board->pnbrqcount[0][0] || board->pnbrqcount[0][3] ||
      board->pnbrqcount[0][4] || board->pnbrqcount[1][0] ||
      board->pnbrqcount[1][3] || board->pnbrqcount[1][4]) {
    return false;
  }
  if (board->pnbrqcount[0][2] > 1 || board->pnbrqcount[1][2] > 1) {
    return false;
  }
  if (board->pnbrqcount[0][1] > 2 || board->pnbrqcount[1][1] > 2) {
    return false;
  }
  if ((board->pnbrqcount[0][1] && board->pnbrqcount[0][2]) ||
      (board->pnbrqcount[0][1] && board->pnbrqcount[0][2])) {
    return false;
  }
  return true;
}
int checkdraw2(struct movelist *movelst,
               int *key) // checks for repetition draws.
{
  if (movelst[*key - 1].halfmoves > 99) {
    return 2;
  }
  int lmove = *key - 1;
  int k = lmove - 2;
  int rep = 0;

  while (k >= 1 && k >= lmove - 100) {
    if (movelst[k].fen == movelst[lmove].fen) {
      return 1;
    }
    k -= 2;
  }
  return rep;
}

int get_cheapest_attacker(struct board_info *board, unsigned int pos,
                          unsigned int *attacker,
                          bool encolor) { // returns 0 - 6 from blank to king
  char flag = 10;
  *attacker = 0;
  // pawns
  if (!encolor) {
    if (!((pos + SW) & 0x88) && board->board[pos + SW] == WPAWN) {
      *attacker = pos + SW; // immediately return because we're not hitting
                            // anything less valuable than a pawn!
      return 1;
    } else if (!((pos + SE) & 0x88) && board->board[pos + SE] == WPAWN) {
      *attacker = pos + SE;
      return 1;
    }
  } else {
    if (!((pos + NE) & 0x88) && board->board[pos + NE] == BPAWN) {
      *attacker = pos + NE;
      return 1;
    } else if (!((pos + NW) & 0x88) && board->board[pos + NW] == BPAWN) {
      *attacker = pos + NW;
      return 1;
    }
  }
  // knights, kings, and sliders
  unsigned char d, f;
  for (f = 0; f < 8; f++) {
    d = pos + vector[0][f];

    if (!(d & 0x88) && board->board[d] - encolor == WKNIGHT) {
      *attacker = d;
      return 2; // return if we hit a knight because we already checked for
                // pawns.
    }

    char vec = vector[4][f];
    d = pos + vec;

    if ((d & 0x88)) {
      continue;
    }

    if (board->board[d] - encolor == WKING) {
      if (flag > 6) {
        *attacker = d;
        flag = 6;
      }
      continue;
    }
    do {
      if (board->board[d]) {
        if ((board->board[d] & 1) == encolor &&
            (board->board[d] - encolor == WQUEEN ||
             (((f & 1) && board->board[d] - encolor == WROOK) ||
              (!(f & 1) && board->board[d] - encolor == WBISHOP)))) {
          if ((board->board[d] >> 1) < flag) {
            flag = (board->board[d] >> 1);
            *attacker = d;
          }
        }
        break;
      }
      d += vec;
    } while (!(d & 0x88));
  }

  return flag;
}



#endif