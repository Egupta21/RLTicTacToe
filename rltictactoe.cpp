#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>
#include <bits/stdc++.h>
#include <cstdlib>
#include <random>

#include "game.h"
#include "qlearning.h"

using namespace std;

#define dimension 3
#define MAX_STATES 10
#define actions 9

#define ALPHA 0.1
#define GAMMA 0.95
#define epsilon 0.1

#define EPISODES 10000
#define GENERATIONS 100

#define EVAL_INTERVAL 10000
#define EVAL_GAMES 100

// make game board struct

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

   // printf("Computer has played: \n");

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

// returns the move the agent want's to play from 1-9
uint16_t agentMove(gameState *pgameState, QTable *qtable, double e, bool greedy)
{
   uint32_t currentStateKey = createKey(pgameState);

   double r = ((double)std::rand() / (RAND_MAX));
   uint16_t action = 1;

   double maxActionProb = -1e9;

   // allow for exploration if greedy if false
   if (r < e && greedy == false)
   {
      action = (std::rand() % 9 + 1);
      while (!isMoveValid(pgameState, spotToBit(action)))
      {
         action = (std::rand() % 9 + 1);
      }
      // playMove(pgameState, std::rand() % 9 + 1, pgameState->currPlayer);
   }
   else if (qtable->qTable.find(currentStateKey) == qtable->qTable.end())
   {
      array<double, 9> qValues = {0.0};
      qtable->qTable.insert({currentStateKey, qValues});
      action = computerMove(pgameState);
   }
   else
   {
      for (size_t t = 0; t < actions; t++)
      {
         if (qtable->qTable.at(currentStateKey)[t] > maxActionProb && isMoveValid(pgameState, spotToBit(((uint16_t)(t + 1)))))
         {
            action = t + 1;
            maxActionProb = qtable->qTable.at(currentStateKey)[t];
         }
      }
      // agent to compute best move using q learning
   }
   return action;
}

// takes in gameState, spot of the move 1-9, and current player
void playMove(gameState *pgameState, uint16_t spot, char player)
{
   uint16_t spotBit = spotToBit(spot);
   // check validity of move
   pgameState->gameBoard |= spotBit;
   if (player == 'X')
   {
      pgameState->playerXBoard |= spotBit;

      // printBoard(pgameState);

      if (checkWin(pgameState->playerXBoard))
      {
         pgameState->playerXGameWin = true;
         // printf("Player X wins!\n\n");
      }
   }
   else
   {
      pgameState->playerOBoard |= spotBit;
      // printBoard(pgameState);
      if (checkWin(pgameState->playerOBoard))
      {
         pgameState->playerOGameWin = true;
         // printf("Player O wins!\n\n");
      }
   }

   if (pgameState->playerXGameWin == false && pgameState->playerOGameWin == false && checkDraw(pgameState->gameBoard))
   {
      pgameState->gameDraw = true;
      // printf("Game ended in a draw\n\n");
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
   gameState->playerXGameWin = false;
   gameState->gameDraw = false;
   gameState->playerOGameWin = false;
   int r = std::rand() % 100 + 1;
   if (r > 50)
   {
      gameState->currPlayer = 'X';
   }
   else
   {
      gameState->currPlayer = 'O';
   }
}

void updateQTable(QTable *qtable, uint32_t prevState, uint16_t action, double reward,
                  uint32_t afterState, double alpha, double gamma)
{
   auto it = qtable->qTable.find(prevState);
   if (it != qtable->qTable.end())
   {
      auto &qValues = it->second;
      auto afterIt = qtable->qTable.find(afterState);
      if (afterIt == qtable->qTable.end())
      {
         array<double, 9> zeroesQValues = {0.0};
         qtable->qTable.insert({afterState, zeroesQValues});
      }
      double maxQafterState = *max_element(qtable->qTable.at(afterState).begin(), qtable->qTable.at(afterState).end());
      qtable->qTable.at(prevState)[action - 1] += (alpha * (reward + gamma * maxQafterState - qtable->qTable.at(prevState)[action - 1]));
   }
}

uint32_t createKey(const gameState *pgameState)
{
   uint32_t key = 0;
   key |= (pgameState->playerXBoard << 16);
   key |= (pgameState->playerOBoard << 0);
   if (pgameState->currPlayer == 'X')
   {
      key |= (1 << 31);
   }
   return key;
}

void logEpisode(FILE *logFile, int episode, int wins, int loss, int draws)
{
   printf("Episode %d: Wins=%d, Loss=%d, Draws=%d\n", episode, wins, loss, draws);
   fprintf(logFile, "%d,%d,%d,%d\n", episode, wins, loss, draws);
   fflush(logFile);
}

void saveQTable(QTable *QTable, const char *filename)
{
   FILE *file = fopen(filename, "w");
   if (!file)
   {
      printf("Error opening Q-table file for writing!\n");
      return;
   }

   for (auto &entry : QTable->qTable)
   {
      uint32_t key = entry.first;
      fprintf(file, "%u", key);
      for (double qval : entry.second)
      {
         fprintf(file, ",%.6f", qval);
      }
      fprintf(file, "\n");
   }
   fclose(file);
}

void runEvaluation(QTable *QTable, int episode, FILE *logFile)
{
   int wins = 0, loss = 0, draws = 0;
   gameState evalGame;
   char Agent = 'X';

   for (int g = 0; g < EVAL_GAMES; g++)
   {
      resetBoard(&evalGame);
      if (evalGame.currPlayer == 'X')
      {
         Agent = 'O';
      }
      else
      {
         Agent = 'X';
      }
      // char Agent = evalGame.currPlayer;
      while (!evalGame.playerXGameWin && !evalGame.playerOGameWin && !evalGame.gameDraw)
      {
         if (evalGame.currPlayer != Agent)
         {
            playMove(&evalGame, computerMove(&evalGame), evalGame.currPlayer);
         }
         else
         {
            uint16_t move = agentMove(&evalGame, QTable, epsilon, true);
            playMove(&evalGame, move, evalGame.currPlayer);
         }
         switchPlayer(&evalGame);
      }

      if (evalGame.gameDraw)
      {
         draws++;
      }
      else if ((Agent == 'X' && evalGame.playerXGameWin) || (Agent == 'O' && evalGame.playerOGameWin))
      {
         wins++;
      }
      else
      {
         loss++;
      }
   }
   logEpisode(logFile, episode, wins, loss, draws);
}

void loadQTable(QTable *QTable, const char *filename)
{
   FILE *file = fopen(filename, "r");
   // file doesn't exist nothing to load from
   if (!file)
      return;

   QTable->qTable.clear();
   char line[256];
   while (fgets(line, sizeof(line), file))
   {
      uint32_t key;
      double qvals[9];
      if (sscanf(line, "%u,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                 &key, &qvals[0], &qvals[1], &qvals[2], &qvals[3],
                 &qvals[4], &qvals[5], &qvals[6], &qvals[7], &qvals[8]) == 10)
      {
         std::array<double, 9> qArray;
         for (int i = 0; i < 9; i++)
         {
            qArray[i] = qvals[i];
         }
         QTable->qTable[key] = qArray;
      }
   }
   fclose(file);
}

int main(int argc, char *argv[])
{
   std::srand(std::time(nullptr));

   gameState gameState;
   gameState.gameBoard = 0;
   gameState.playerOBoard = 0;
   gameState.playerXBoard = 0;
   gameState.playerXGameWin = false;
   gameState.gameDraw = false;
   gameState.playerOGameWin = false;
   gameState.currPlayer = 'X';
   // printBoard(&gameState);

   QTable qtable;

   loadQTable(&qtable, "qtable.csv");

   FILE *logFile = fopen("training_log.csv", "a");
   if (!logFile)
   {
      printf("Error opening log file!\n");
      return 1;
   }

   uint32_t lastStateX = 0;
   uint32_t lastStateO = 0;
   uint16_t lastActionX = 1;
   uint16_t lastActionO = 1;
   for (int gen = 0; gen < GENERATIONS; gen++)
   {
      for (int episode = 0; episode < EPISODES; episode++)
      {
         resetBoard(&gameState);

         if (episode % EVAL_INTERVAL == 0)
         {
            // runEval call
            saveQTable(&qtable, "qtable.csv");
            printf("Generation: %d\n", gen);
            runEvaluation(&qtable, episode, logFile);
            printf("\n");
         }
         while (!gameState.playerXGameWin && !gameState.playerOGameWin && !gameState.gameDraw)
         {
            uint16_t move;

            move = agentMove(&gameState, &qtable, epsilon, false);
            uint32_t prevState = createKey(&gameState);
            char lastPlayer = gameState.currPlayer;

            playMove(&gameState, move, gameState.currPlayer);
            uint32_t afterState = createKey(&gameState);

            // reward for current player (player who played the game ending move)
            double reward = 0;
            if (lastPlayer == 'X' && gameState.playerXGameWin)
               reward = 1;
            else if (lastPlayer == 'O' && gameState.playerOGameWin)
               reward = 1;
            else if (gameState.gameDraw)
               reward = 0;

            // reward current player
            updateQTable(&qtable, prevState, move, reward, afterState, ALPHA, GAMMA);

            // give reward to non active player
            if (lastPlayer == 'X' && gameState.playerXGameWin)
               updateQTable(&qtable, lastStateO, lastActionO, -1, prevState, ALPHA, GAMMA);
            else if (lastPlayer == 'O' && gameState.playerOGameWin)
               updateQTable(&qtable, lastStateX, lastActionX, -1, prevState, ALPHA, GAMMA);
            else if (gameState.gameDraw)
            {
               reward = 0;
               updateQTable(&qtable, lastStateX, lastActionX, 0, prevState, ALPHA, GAMMA);
               updateQTable(&qtable, lastStateO, lastActionO, 0, prevState, ALPHA, GAMMA);
            }

            // Save last move and state of each player
            if (gameState.currPlayer == 'X')
            {
               lastStateX = prevState;
               lastActionX = move;
            }
            else
            {
               lastStateO = prevState;
               lastActionO = move;
            }

            switchPlayer(&gameState);
         }
      }
   }
}
