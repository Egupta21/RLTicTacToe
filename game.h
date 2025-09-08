#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>

// Represents the state of the game
typedef struct
{
   uint16_t gameBoard;
   uint16_t playerXBoard;
   uint16_t playerOBoard;
   bool playerXGameWin;
   bool gameDraw;
   bool playerOGameWin;
   char currPlayer;
} gameState;

// ---- Game functions ----
uint16_t spotToBit(uint16_t spot);
bool isMoveValid(const gameState *pgameState, uint16_t spotBit);
void playMove(gameState *pgameState, uint16_t spot, char player);
uint32_t createKey(const gameState *pgameState);

#endif // GAME_H