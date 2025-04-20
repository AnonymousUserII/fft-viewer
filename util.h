#pragma once

#include <SDL.h>

Uint32 max(Uint32 a, Uint32 b);
Uint32 min(Uint32 a, Uint32 b);
double mind(double a, double b);
double maxd(double a, double b);

// Turn really large doubles to zero
double temperDouble(double z);
