#ifndef CONVERT_H_
#define CONVERT_H_

#include "NodeNetwork.h"
#include "Mult.h"
#include "Open.h"
#include "mpc_util.h"
#include "randBit.h"
#include "edaBit.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void new_Rss_Convert(Lint **res, Lint **a, uint size, uint ring_size, uint ring_size_prime, NodeNetwork *nodeNet);
void Rss_Convert(Lint **res, Lint **a, uint size, uint ring_size, uint ring_size_prime, NodeNetwork *nodeNet);

#endif
