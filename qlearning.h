#ifndef QLEARNING_H
#define QLEARNING_H

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

using namespace std;

typedef struct
{
   unordered_map<uint32_t, array<double, 9>> qTable;
   // double qValues[MAX_STATES][9];
} QTable;

// ---- Q-learning functions ----
uint16_t agentMove(gameState *pgameState, QTable *qtable, double epsilon, bool greedy);
void updateQTable(QTable *qtable, uint32_t prevState, int action, double reward,
                  uint32_t afterState, double alpha, double gamma);

#endif