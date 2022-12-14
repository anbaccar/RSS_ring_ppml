/*   
   Multi-Party Replicated Secret Sharing over a Ring 
   ** Copyright (C) 2022 Alessandro Baccarini, Marina Blanton, and Chen Yuan
   ** Department of Computer Science and Engineering, University of Buffalo (SUNY)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NEURAL_OPS_H_
#define NEURAL_OPS_H_
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <sstream>

#include "NodeNetwork.h"
#include "Trunc.h"
#include "LT.h"
#include "sys/time.h"

void Rss_avg_pool_trunc(Lint **res, Lint **x, uint in_channels, uint *n, uint kernel, uint stride, uint batch_size, uint ring_size,  NodeNetwork *nodeNet, unsigned long &timer) ;

// neural network operations

// original
void im2col(Lint** res, Lint** a, uint channels, uint height, uint width, uint ksize, uint stride, uint batch_size,  NodeNetwork* nodeNet);

// tensorflow version
void extract_patch(Lint** res, Lint** data_im, int channels, int height, int width, int ksize, int stride, uint batch_size, int pad_flag, NodeNetwork* nodeNet);

void pad_original(Lint** res, Lint** data_im, int channels, int height, int width, int ksize, int stride, uint batch_size, NodeNetwork *nodeNet);




void ReLU(Lint** res, Lint** a,  uint size, uint ring_size,  NodeNetwork* nodeNet, uint flag);
void old_ReLU(Lint** res, Lint** a,  uint size, uint ring_size,  NodeNetwork* nodeNet);
void eda_ReLU(Lint** res, Lint** a,  uint size, uint ring_size,  NodeNetwork* nodeNet);


void ReLU_6_alpha(Lint** res, Lint** x, Lint** upper, uint size, uint ring_size, uint layer,  NodeNetwork* nodeNet);
void ReLU_6_alpha_timer(Lint** res, Lint** x, Lint** upper, uint size, uint ring_size, uint layer,  NodeNetwork* nodeNet, unsigned long &timer);

void add_zp(Lint **res, Lint **x, Lint **z_p, uint size, uint batch_size, uint layer_id, NodeNetwork *nodeNet);
void sub_zp(Lint **res, Lint **x, Lint **z_p, uint size, uint batch_size, uint layer_id, NodeNetwork *nodeNet);



void MaxPool(Lint** res, Lint** a, uint c, uint m, uint n, uint batch_size, uint ring_sze,  NodeNetwork* nodeNet,uint flag);
void old_MaxPool(Lint** res, Lint** a, uint c, uint m, uint n, uint batch_size, uint ring_sze,  NodeNetwork* nodeNet);
void eda_MaxPool(Lint** res, Lint** a, uint c, uint m, uint n, uint batch_size, uint ring_sze,  NodeNetwork* nodeNet);

void add_biases(Lint** res, Lint **a, Lint **b, uint m, uint n, uint batch_size, NodeNetwork *nodeNet);
void pw_add_biases(Lint **res, Lint **x, Lint **b, uint height, uint width, uint channels, uint batch_size, NodeNetwork *nodeNet);



#endif
