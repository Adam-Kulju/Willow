#ifndef __movegen__
#define __movegen__
#include "constants.h"
#include "board.h"


int SEEVALUES[7] = {0, 100, 450, 450, 650, 1250, 10000};


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

bool static_exchange_evaluation(struct board_info *board, struct move mve, bool color, int threshold){
    if (mve.flags && mve.flags >> 2 != 1){  //castling and en passant both come out neutral; thus we can return immediately.
        return (threshold <= 0);
    }
    int to = mve.move&0xFF;
    
    int gain = SEEVALUES[board->board[mve.move & 0xFF] >> 1];   //what you get from taking the piece
    int risk = SEEVALUES[board->board[mve.move >> 8] >> 1];     //what you lose if your piece gets taken!

    struct board_info board2 = *board;

    board2.board[mve.move >> 8] = BLANK;

    unsigned int attacker_pos = 0;

    int totalgain = gain - threshold;   //how much you have gained after the first capture you made.

    if (totalgain < 0){
        return false;
    }

    while (totalgain - risk < 0){   //an example with RxB BxR RxB

        int i = get_cheapest_attacker(&board2, to, &attacker_pos, color^1);
        if (i == 10){
            return true;    //if opponent had no attackers, we just took a free piece!
        }

        totalgain -= risk;      //first we subtract the value of the piece that you just had taken (since this is from opponent's point of view). in our RXB case, that was a bishop, so we're now at -200.
        risk = SEEVALUES[i];    //if the bishop gets taken, he loses 300 (a minor piece).
        if (totalgain + risk < 0){  //if the piece he took with was low enough value that you can't get the material back by taking it (2 pawns for a bishop), return. not true here.
            return false;
        }
        board2.board[attacker_pos] = BLANK; //remove the piece to account for batteries. 
                                            //and yeah this fails to discovered checks, but a discovered check will quickly cause a beta cutoff anyways so no big deal.

        i = get_cheapest_attacker(&board2, to, &attacker_pos, color);  //now we look at it from our point of view.
        if (i == 10){   //if we have no attackers, it is losing, since the condition in the while loop guarantees at least equal if we do something like BxB.
            return false;
        }
        totalgain += risk;  //we were at -200 before, now we're at 100.
        risk = SEEVALUES[i];

        board2.board[attacker_pos] = BLANK;

    }
    //another example: RxB QxR PxQ
    //RxB is not winning in and of itself, so we get the smallest attacker which turns out to be Q.
    //this sets totalgain to -300, but risk to 900.
    //we can take it, and it's with a pawn. Total gain becomes 600, risk becomes 100, and we return true.
    return true;
}


int see(struct board_info *board, struct move mve, bool color, int threshold){

    if (mve.flags == 0xC){
        return 2000000 + 100*10;
    }

    int attacker = SEEVALUES[board->board[(mve.move>>8)]>>1], victim = SEEVALUES[board->board[(mve.move&0xFF)]>>1]; 

    int v = 2000000;
    if (victim - attacker < threshold && !static_exchange_evaluation(board, mve, color, threshold)){
        v = -v;
    }
    return v + victim*10 - attacker;

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

int movescore(struct board_info *board, struct list *list, int depth, bool color, char type, struct move lastmove, int movelen, int threshold){
    int i = 0; while (i < movelen){
        list[i].eval = 1000000;

        if (depth > 1 && lastmove.move != 0 && (board->board[lastmove.move&0xFF]>>1)-1 < 0){
            return 1;
        }

        if (type != 'n' && ismatch(TT[(CURRENTPOS) & (_mask)].bestmove, list[i].move)){

            if (TT[(CURRENTPOS) & (_mask)].bestmove.move == list[i].move.move){

                if (TT[(CURRENTPOS) & (_mask)].bestmove.flags == list[i].move.flags){

                    list[i].eval += 10000000;
                }
            }
        }    
            
        else if (list[i].move.flags == 7){
            list[i].eval += 5000000 + SEEVALUES[board->board[(list[i].move.move&0xFF)]>>1];
            }
        else if (board->board[list[i].move.move & 0xFF]){
            list[i].eval = see(board, list[i].move, color, threshold);
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

#endif