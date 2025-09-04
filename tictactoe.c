#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>

#define dimension 3

// make game board struct

typedef struct
{
   uint16_t gameBoard;
   uint16_t playerXBoard;
   uint16_t playerOBoard;
   bool gameOver;
   char currPlayer;
} gameState;

void printBoard(gameState *pgameState)
{
   for (int8_t t = (dimension * dimension) - 1; t >= 0; t--)
   {
      uint16_t bitVal = 0;
      uint16_t playerOBitVal = 0;

      bitVal = (pgameState->gameBoard & (1 << t)) >> t;
      if (bitVal == 0)
      {
         // spot is not occupied by any player yet
         printf("|   ");
      }
      else if (bitVal == 1)
      {
         playerOBitVal = (pgameState->playerOBoard & (1 << t)) >> t; // 1 is current spot is occupied by player O, otherwise 0
         if (playerOBitVal == 1)
         {
            printf("| O ");
         }
         else
         {
            printf("| X ");
         }
      }

      if ((t) % dimension == 0 && t != 0)
      {
         printf("|\n-------------\n");
      }
   }
   printf("|\n\n\n");
}

bool checkWin(uint16_t playerBoard)
{
   // static to store mask in stack to reuse, without static it would be created every time the function is called and destroyed after
   // const so it's unchangable
   // array of winning boards
   static const uint16_t winningMasks[8] = {
       0b111000000,
       0b000111000,
       0b000000111,
       0b100100100,
       0b010010010,
       0b001001001,
       0b100010001,
       0b001010100};

   for (int i = 0; i < 8; i++)
   {
      if ((playerBoard & winningMasks[i]) == winningMasks[i])
      {
         return true;
      }
   }
   return false;
}

bool checkDraw(uint16_t gameBoard)
{
   static const uint16_t drawBoard = 0b111111111;

   if (gameBoard == drawBoard)
   {
      return true;
   }
   return false;
}

bool isMoveValid(gameState *pgameState, uint16_t spotBit)
{
   if (pgameState->gameBoard & spotBit)
   {
      // move is invalid
      // printf("Move is not valid, spot is already occupied\n");
      return false;
   }
   return true;
}

uint16_t spotToBit(uint16_t spot)
{
   if (spot < 1 || spot > 9)
   {
      printf("Invalid Spot selected\n");
      return 0;
   }
   return (1 << (9 - spot));
}

uint16_t computerMove(gameState *pgameState)
{
   int spot = rand() % 9 + 1;
   uint16_t spotBit = spotToBit(spot);
   int maxPicks = 1000;
   int pickCount = 0;

   printf("Computer has played: \n");

   while (!isMoveValid(pgameState, spotBit) && pickCount < maxPicks)
   {
      spot = rand() % 9 + 1;
      spotBit = spotToBit(spot);
   }

   return spot;
}

uint16_t humanMove(gameState *pgameState)
{
   int spot;
   uint16_t spotBit = 0;

   printf("Enter your move (1-9): ");
   scanf("%d", &spot);

   spotBit = spotToBit(spot);
   while (!isMoveValid(pgameState, spotBit))
   {
      printf("Move was invalid, Enter your move (1-9): ");
      scanf("%d", &spot);
      spotBit = spotToBit(spot);
   }

   return spot;
}

void playMove(gameState *pgameState, uint16_t spot, char player)
{
   uint16_t spotBit = spotToBit(spot);
   // check validity of move
   pgameState->gameBoard |= spotBit;
   if (player == 'X')
   {
      pgameState->playerXBoard |= spotBit;
      printBoard(pgameState);

      if (checkWin(pgameState->playerXBoard))
      {
         pgameState->gameOver = true;
         printf("Player X wins!\n\n");
      }
   }
   else
   {
      pgameState->playerOBoard |= spotBit;
      printBoard(pgameState);
      if (checkWin(pgameState->playerOBoard))
      {
         pgameState->gameOver = true;
         printf("Player O wins!\n\n");
      }
   }

   if (pgameState->gameOver == false && checkDraw(pgameState->gameBoard))
   {
      pgameState->gameOver = true;
      printf("Game ended in a draw\n\n");
   }
}

void switchPlayer(gameState *pgameState)
{
   if (pgameState->currPlayer == 'X')
   {
      pgameState->currPlayer = 'O';
   }
   else
   {
      pgameState->currPlayer = 'X';
   }
}

int main(int argc, char *argv[])
{
   gameState pgameState;
   pgameState.gameBoard = 0;
   pgameState.playerOBoard = 0;
   pgameState.playerXBoard = 0;
   pgameState.gameOver = false;
   pgameState.currPlayer = 'X';
   printBoard(&pgameState);

   while (!pgameState.gameOver)
   {
      uint16_t moveBit;

      if (pgameState.currPlayer == 'X')
      {
         // Human move
         moveBit = humanMove(&pgameState);
         playMove(&pgameState, moveBit, pgameState.currPlayer);
      }
      else
      {
         // Computer move
         printf("Computer is making a move...\n");
         moveBit = computerMove(&pgameState);
         playMove(&pgameState, moveBit, pgameState.currPlayer);
      }

      // Switch player only if game is not over
      if (!pgameState.gameOver)
      {
         switchPlayer(&pgameState);
      }
   }

   return 0;
}
