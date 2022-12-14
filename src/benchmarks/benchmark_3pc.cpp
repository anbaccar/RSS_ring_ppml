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

#include "../include/benchmark_3pc.h"

void benchmark_3pc(NodeNetwork *nodeNet, NodeConfiguration *nodeConfig, char *protocol, uint size, uint batch_size, uint num_iterations) {

    int pid = nodeConfig->getID();
    int ring_size = nodeNet->RING;


    vector<string> exp_names = {"multiply", "randbit", "edabit", "msb_rb", "msb_eda", "mat_mul"};

    vector<vector<double>> exp_times(exp_names.size());
    vector<vector<double>> exp_ratios(exp_names.size());

    // checking if we can access this built-in function
    // uint aaa = _pext_u32((unsigned) ring_size, 0x55555555);

    struct timeval start;
    struct timeval end;
    unsigned long timer;

    // printf("hello, I am %d\n", pid);

    // setup prg key (will be used by all parties, only for data makeup)
    __m128i *key_prg;
    uint8_t key_raw[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    key_prg = offline_prg_keyschedule(key_raw);
    // setup prg seed(k1, k2, k3)
    uint8_t k1[] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    uint8_t k2[] = {0xa2, 0x34, 0x6f, 0x67, 0x10, 0x1b, 0x13, 0xa3, 0x56, 0x45, 0x90, 0xb2, 0x13, 0xe3, 0x23, 0x24};

    Lint **a = new Lint *[2];
    Lint **b = new Lint *[2];
    Lint **c = new Lint *[2];
    Lint **d = new Lint *[2];

    for (size_t i = 0; i < 2; i++) {
        c[i] = new Lint[(size * batch_size)];
        memset(c[i], 0, sizeof(Lint) * (size * batch_size));
        d[i] = new Lint[(size * batch_size)];
        memset(d[i], 0, sizeof(Lint) * (size * batch_size));
    }

    Lint **Data1;
    Lint **Data2;

    Data1 = new Lint *[3];
    for (int i = 0; i < 3; i++) {
        Data1[i] = new Lint[(size * batch_size)];
        memset(Data1[i], 0, sizeof(Lint) * (size * batch_size));
    }

    Data2 = new Lint *[3];
    for (int i = 0; i < 3; i++) {
        Data2[i] = new Lint[(size * batch_size)];
        memset(Data2[i], 0, sizeof(Lint) * (size * batch_size));
    }

    for (int i = 0; i < (size * batch_size); i++) {
        prg_aes_ni(Data1[0] + i, k1, key_prg);
        prg_aes_ni(Data1[1] + i, k1, key_prg);
        prg_aes_ni(Data2[0] + i, k2, key_prg);
        prg_aes_ni(Data2[1] + i, k2, key_prg);
        Data1[2][i] = i - Data1[0][i] - Data1[1][i];
        Data2[2][i] = i - Data2[0][i] - Data2[1][i];
    }
    free(key_prg);

    // assigning data
    switch (pid) {
    case 1:
        a[0] = Data1[1];
        a[1] = Data1[2];
        b[0] = Data2[1];
        b[1] = Data2[2];
        break;
    case 2:
        a[0] = Data1[2];
        a[1] = Data1[0];
        b[0] = Data2[2];
        b[1] = Data2[0];
        break;
    case 3:
        a[0] = Data1[0];
        a[1] = Data1[1];
        b[0] = Data2[0];
        b[1] = Data2[1];
        break;
    }
    if (!strcmp(protocol, "mult")) {

        gettimeofday(&start, NULL); // start timer here
        for (size_t j = 0; j < num_iterations; j++) {
            Rss_Mult(c, a, b, (size * batch_size), ring_size, nodeNet);
        }
        gettimeofday(&end, NULL); // stop timer here
        timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
        printf("[%s_3pc] [%u, %i, %u, %u] [%.6lf ms,  %.6lf ms/size,  %lu bytes] \n", protocol, ring_size, size, batch_size, num_iterations, (double)(timer * 0.001) / num_iterations, (double)(timer * 0.001 / size) / num_iterations, nodeNet->getCommunicationInBytes() / num_iterations);
        

    } else if (!strcmp(protocol, "randbit")) {

        gettimeofday(&start, NULL); // start timer here
        for (size_t j = 0; j < num_iterations; j++) {
            Rss_RandBit(c, (size * batch_size), ring_size, nodeNet);
        }
        gettimeofday(&end, NULL); // stop timer here
        timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
        printf("[%s_3pc] [%u, %i, %u, %u] [%.6lf ms,  %.6lf ms/size,  %lu bytes] \n", protocol, ring_size, size, batch_size, num_iterations, (double)(timer * 0.001) / num_iterations, (double)(timer * 0.001 / size) / num_iterations, nodeNet->getCommunicationInBytes() / num_iterations);
        

    } else if (!strcmp(protocol, "edabit")) {

        gettimeofday(&start, NULL); // start timer here
        for (size_t j = 0; j < num_iterations; j++) {
            Rss_edaBit(c, d, (size * batch_size), ring_size, nodeNet);
        }
        gettimeofday(&end, NULL); // stop timer here
        timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
        printf("[%s_3pc] [%u, %i, %u, %u] [%.6lf ms,  %.6lf ms/size,  %lu bytes] \n", protocol, ring_size, size, batch_size, num_iterations, (double)(timer * 0.001) / num_iterations, (double)(timer * 0.001 / size) / num_iterations, nodeNet->getCommunicationInBytes() / num_iterations);
        

    } else if (!strcmp(protocol, "msb_rb")) {

        gettimeofday(&start, NULL); // start timer here
        for (size_t j = 0; j < num_iterations; j++) {
            Rss_MSB(c, a, (size * batch_size), ring_size, nodeNet);
        }
        gettimeofday(&end, NULL); // stop timer here
        timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
        printf("[%s_3pc] [%u, %i, %u, %u] [%.6lf ms,  %.6lf ms/size,  %lu bytes] \n", protocol, ring_size, size, batch_size, num_iterations, (double)(timer * 0.001) / num_iterations, (double)(timer * 0.001 / size) / num_iterations, nodeNet->getCommunicationInBytes() / num_iterations);
        

    } else if (!strcmp(protocol, "msb_eda")) {

        gettimeofday(&start, NULL); // start timer here
        for (size_t j = 0; j < num_iterations; j++) {
            new_Rss_MSB(c, a, (size * batch_size), ring_size, nodeNet);
        }
        gettimeofday(&end, NULL); // stop timer here
        timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
        printf("[%s_3pc] [%u, %i, %u, %u] [%.6lf ms,  %.6lf ms/size,  %lu bytes] \n", protocol, ring_size, size, batch_size, num_iterations, (double)(timer * 0.001) / num_iterations, (double)(timer * 0.001 / size) / num_iterations, nodeNet->getCommunicationInBytes() / num_iterations);
        
    }else if (!strcmp(protocol, "mat_mult")) {

        gettimeofday(&start, NULL); // start timer here
        for (size_t j = 0; j < num_iterations; j++) {
            Rss_MatMultArray_batch(c, a, b, int(sqrt(size)), int(sqrt(size)), int(sqrt(size)), ring_size, batch_size, 1, 1, nodeNet);
        }
        gettimeofday(&end, NULL); // stop timer here
        timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
        printf("[%s_3pc] [%u, %i x %i, %u, %u] [%.6lf ms,  %.6lf ms/size,  %lu bytes] \n", protocol, ring_size, int(sqrt(size)), int(sqrt(size)), batch_size, num_iterations, (double)(timer * 0.001) / num_iterations, (double)(timer * 0.001 / size) / num_iterations, nodeNet->getCommunicationInBytes() / num_iterations);
        
    } else{

        printf("Invalid protocol name\n");
        printf("Protocols: mult, randbit, edabit, msb_rb, msb_eda, mat_mult\n");
    }


    //     for (size_t i = 0; i < sizes.size(); i++) {

    //         exp_ctr = 0;

    //         total = 100;
    //         gettimeofday(&start, NULL); // start timer here
    //         for (size_t j = 0; j < total; j++) {
    //             Rss_Mult(c, a, b, sizes.at(i), ring_size, nodeNet);
    //         }
    //         gettimeofday(&end, NULL); // stop timer here
    //         timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    //         exp_times.at(exp_ctr).push_back((double)(timer * 0.001) / total);
    //         exp_ratios.at(exp_ctr).push_back((double)(timer * 0.001 / sizes.at(i)) / total);

    //         exp_ctr++;

    //         total = 100;
    //         gettimeofday(&start, NULL); // start timer here
    //         for (size_t j = 0; j < total; j++) {
    //             Rss_RandBit(c, sizes.at(i), ring_size, nodeNet);
    //         }
    //         gettimeofday(&end, NULL); // stop timer here
    //         timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    //         exp_times.at(exp_ctr).push_back((double)(timer * 0.001) / total);
    //         exp_ratios.at(exp_ctr).push_back((double)(timer * 0.001 / sizes.at(i)) / total);

    //         exp_ctr++;

    //         total = 100;
    //         gettimeofday(&start, NULL); // start timer here
    //         for (size_t j = 0; j < total; j++) {
    //             Rss_edaBit(c, d, sizes.at(i), ring_size, nodeNet);
    //         }
    //         gettimeofday(&end, NULL); // stop timer here
    //         timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    //         exp_times.at(exp_ctr).push_back((double)(timer * 0.001) / total);
    //         exp_ratios.at(exp_ctr).push_back((double)(timer * 0.001 / sizes.at(i)) / total);

    //         exp_ctr++;

    //         total = 100;
    //         gettimeofday(&start, NULL); // start timer here
    //         for (size_t j = 0; j < total; j++) {
    //             Rss_MSB(c, a, sizes.at(i), ring_size, nodeNet);
    //         }
    //         gettimeofday(&end, NULL); // stop timer here
    //         timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    //         exp_times.at(exp_ctr).push_back((double)(timer * 0.001) / total);
    //         exp_ratios.at(exp_ctr).push_back((double)(timer * 0.001 / sizes.at(i)) / total);

    //         exp_ctr++;

    //         total = 100;
    //         gettimeofday(&start, NULL); // start timer here
    //         for (size_t j = 0; j < total; j++) {
    //             new_Rss_MSB(c, a, sizes.at(i), ring_size,  nodeNet);
    //         }
    //         gettimeofday(&end, NULL); // stop timer here
    //         timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    //         exp_times.at(exp_ctr).push_back((double)(timer * 0.001) / total);
    //         exp_ratios.at(exp_ctr).push_back((double)(timer * 0.001 / sizes.at(i)) / total);

    //         exp_ctr++;
    //     }

    //   for (size_t i = 0; i < matrix_sizes.size(); i++) {

    //         total = 100;
    //         gettimeofday(&start, NULL); // start timer here
    //         for (size_t j = 0; j < total; j++) {
    //             Rss_MatMultArray(c, a, b, matrix_sizes.at(i), matrix_sizes.at(i),matrix_sizes.at(i), ring_size, nodeNet);
    //         }
    //         gettimeofday(&end, NULL); // stop timer here
    //         timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    //         exp_times.at(exp_ctr).push_back((double)(timer * 0.001) / total);
    //         exp_ratios.at(exp_ctr).push_back((double)(timer * 0.001 / matrix_sizes.at(i)) / total);

    //     }

    //     write_benchmark(out_fname, exp_names, exp_times, exp_ratios, ring_size);

    for (int i = 0; i < 3; i++) {

        delete[] Data1[i];
        delete[] Data2[i];
    }

    delete[] Data1;
    delete[] Data2;

    delete[] a;
    delete[] b;

    for (size_t i = 0; i < 2; i++) {
        delete[] c[i];
        delete[] d[i];
    }

    delete[] c;
    delete[] d;
}
