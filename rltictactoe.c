#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>

#define dimension 3
#define MAX_STATES 10

#define REWARD 1
#define ALPHA 0.01
#define GAMMA 0.01
#define epsilon 0.1

#define EPISODES 10000
#define GENERATIONS 100

// make game board struct

// game status struct
typedef struct
{
   uint16_t gameBoard;
   uint16_t playerXBoard;
   uint16_t playerOBoard;
   bool gameWin;
   bool gameDraw;
   bool gameLoss;
   char currPlayer;
} gameState;

typedef struct
{
   double qValues[MAX_STATES][9];
} updateRule;

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

uint16_t agentMove(gameState *pgameState, updateRule *qTable, double e)
{

   // agent to compute best move using q learning
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
         pgameState->gameWin = true;
         printf("Player X wins!\n\n");
      }
   }
   else
   {
      pgameState->playerOBoard |= spotBit;
      printBoard(pgameState);
      if (checkWin(pgameState->playerOBoard))
      {
         pgameState->gameWin = true;
         printf("Player O wins!\n\n");
      }
   }

   if (pgameState->gameWin == false && checkDraw(pgameState->gameBoard))
   {
      pgameState->gameDraw = true;
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

void resetBoard(gameState *gameState)
{
   gameState->gameBoard = 0;
   gameState->playerOBoard = 0;
   gameState->playerXBoard = 0;
   gameState->gameWin = false;
   gameState->gameDraw = false;
   gameState->gameLoss = false;
   gameState->currPlayer = 'X';
}

void updateQTable(updateRule *qTable, int prevState, uint16_t move, double reward, int newState)
{
}

int main(int argc, char *argv[])
{
   gameState gameState;
   gameState.gameBoard = 0;
   gameState.playerOBoard = 0;
   gameState.playerXBoard = 0;
   gameState.gameWin = false;
   gameState.gameDraw = false;
   gameState.gameLoss = false;
   gameState.currPlayer = 'X';
   printBoard(&gameState);

   updateRule qTable;

   // only 1 agent being trained
   for (int episode = 0; episode < EPISODES; episode++)
   {
      resetBoard(&gameState);
      while (!gameState.gameWin && !gameState.gameLoss && !gameState.gameDraw)
      {
         uint16_t move;

         move = agentMove(&gameState, &qTable, epsilon);
         playMove(&gameState, move, gameState.currPlayer);
         updateQTable(&qTable, prevState, move, REWARD, newState);
         switchPlayer(&gameState);
      }
   }
}
