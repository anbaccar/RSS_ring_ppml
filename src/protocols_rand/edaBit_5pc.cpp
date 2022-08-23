#include "edaBit_5pc.h"

void Rss_edaBit_mp(Lint **r, Lint **b_2, uint size, uint ring_size, NodeNetwork *nodeNet) {

    // int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // this is the number of shares we need to add (t+1)
    uint new_size = (threshold + 1) * size;
    // printf("new_size: %llu\n", new_size);

    Lint **r_bitwise = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(b_2[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares_mp(r, r_bitwise, ring_size, size, nodeNet);

    Rss_nBitAdd_mp(b_2, r_bitwise, ring_size, size, nodeNet);

    for (i = 0; i < numShares; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

void Rss_GenerateRandomShares_mp(Lint **res, Lint **res_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet) {
    // printf("start\n");
    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    uint new_size = 2 * size; // DO NOT CHANGE, IT IS IRRELEVANT for n>3
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // printf("threshold : %u \n", threshold);
    // printf("numParties : %u \n", numParties);

    Lint **recvbuf = new Lint *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new Lint[new_size];
        memset(recvbuf[i], 0, sizeof(Lint) * new_size);
    }

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    // printf("new_size : %u \n", new_size);

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[numShares];
    for (i = 0; i < numShares; i++) {
        r_values[i] = new Lint *[(threshold + 1)];
        for (j = 0; j < (threshold + 1); j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size); // NECESSARY FOR n>3
        }
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);

    uint8_t *r_buffer = new uint8_t[bytes * size];

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[(threshold + 1) * bytes * new_size]; // we may not use all of these random bytes, but
        nodeNet->prg_getrandom(s, bytes, (threshold + 1) * new_size, buffer[s]);
    }

    int *index_map = new int[threshold+1];

    uint reset_value;
    switch (pid) {
    case 1:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = 3;
        index_map[2] = 5;
        break;
    case 2:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = 3;
        break;
    case 3:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 4:
        reset_value = 1;
        index_map[0] = 5;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 5:
        reset_value = 0;
        index_map[0] = 3;
        index_map[1] = 5;
        index_map[2] = -1;
        break;
    }

    bool prg_bools[4][6] = {
        {1, 1, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 0},
        {0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1},
    };

    if (pid < threshold + 2) {
        // p1, p2, p3, choosing random values

        nodeNet->prg_getrandom(bytes, size, r_buffer);
        // memcpy(r_bits, r_buffer, size * bytes);

        for (i = 0; i < size; i++) {
            memcpy(r_bits + i, r_buffer + i * bytes, bytes);

            // printf("hi1\n");
            for (size_t s = 1; s < numShares; s++) {
                // memcpy(r_values[s][p_index], buffer[s], new_size * bytes);
                memcpy(r_values[s][p_index] + 2 * i, buffer[s] + (2 * i) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 1, buffer[s] + (2 * i + 1) * bytes, bytes);
            }

            // r_values[0][p_index][2 * i] = Lint(5 + i);
            // r_values[0][p_index][2 * i + 1] = Lint(5 + i);
            r_values[0][p_index][2 * i] = r_bits[i];
            r_values[0][p_index][2 * i + 1] = r_bits[i];

            for (size_t s = 1; s < numShares; s++) {
                r_values[0][p_index][2 * i] -= r_values[s][p_index][2 * i];
                r_values[0][p_index][2 * i + 1] ^= r_values[s][p_index][2 * i + 1];
            }
        }
    }

    // num parties - 1

    // printf("indices\n");
    // int n = numParties - 1;
    // printf("n: %llu\n", n);

    for (size_t i = 0; i < size; i++) {

        int index = ((pid - 2) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
        for (size_t t = 0; t < threshold + 1; t++) {
            if (p_index != t) {
                // int index = (((pid - 2 - t) % ((numParties - 1))) + ((numParties - 1)) )  % ((numParties - 1) );

                // printf("%i : bool_index = %i\n", t, index);
                // loop through num_shares
                // if we're supposed to generate, then memcpy from buffer[j]
                for (size_t s = 0; s < numShares; s++) {
                    if (prg_bools[index][s]) {
                        // printf("copying: %llu\n", copying);
                        memcpy(r_values[s][t] + (2 * i), buffer[s] + (2 * i) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 1), buffer[s] + (2 * i + 1) * bytes, bytes);
                    }
                }
                // printf("subtracting\(numParties - 1)");
                index = ((index - 1) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
            }
        }
    }

    nodeNet->SendAndGetDataFromPeer_Eda(r_values[0][p_index], recvbuf, new_size, ring_size);

    // extracting from buffer
    for (size_t i = 0; i < size; i++) {
        j = reset_value;
        for (size_t t = 0; t < threshold + 1; t++) {

            if (index_map[t] > 0) {
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i], j, index_map[t]);
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i + 1], j, index_map[t]);
                memcpy(r_values[index_map[t]][t] + 2 * i, recvbuf[j] + 2 * i, sizeof(Lint));
                memcpy(r_values[index_map[t]][t] + 2 * i + 1, recvbuf[j] + 2 * i + 1, sizeof(Lint));
                j++;
            }
        }
    }

    for (size_t i = 0; i < size; i++) {
        for (uint p = 0; p < threshold + 1; p++) {
            for (size_t s = 0; s < numShares; s++) {
                res_bitwise[s][p * (size) + i] = r_values[s][p][2 * i + 1];
            }
        }
    }

    for (i = 0; i < size; i++) {
        // this is so we only have t+1 parties generating shares
        for (j = 0; j < threshold + 1; j++) {
            // adding all the parties arithmetic shares together
            for (size_t s = 0; s < numShares; s++) {
                res[s][i] += r_values[s][j][2 * i];
                // res[1][i] += r_values[1][j][1 * i];
            }
        }
    }

    delete[] index_map;
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;

    for (i = 0; i < numShares; i++) {
        for (j = 0; j < (threshold + 1); j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
    delete[] r_buffer;
    delete[] r_bits;
}

void Rss_edaBit_trunc_mp(Lint **r, Lint **r_prime, Lint **r_km1, uint size, uint ring_size, uint m, NodeNetwork *nodeNet) {

    // int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // this is the number of shares we need to add (t+1)
    uint new_size = (threshold + 1) * size;
    // printf("new_size: %llu\n", new_size);

    uint b2a_size = 3 * size;

    Lint **carry = new Lint *[numShares];
    Lint **b_2 = new Lint *[numShares];
    Lint **r_bitwise = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);
        // carry will hold both kth and m-1th bits, in succession
        carry[i] = new Lint[b2a_size];
        memset(carry[i], 0, sizeof(Lint) * b2a_size);

        b_2[i] = new Lint[size];
        memset(b_2[i], 0, sizeof(Lint) * size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(r_prime[i], 0, sizeof(Lint) * size);
        memset(r_km1[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares_trunc_mp(r, r_prime, r_bitwise, ring_size, m, size, nodeNet);

    Rss_nBitAdd_trunc_mp(b_2, carry, r_bitwise, ring_size, m, size, nodeNet);

    Rss_b2a_mp(carry, carry, ring_size, b2a_size, nodeNet);
    for (size_t s = 0; s < numShares; s++) {
        memcpy(r_km1[s], carry[s] + 2 * (size), size * sizeof(Lint));
    }

    // adding m-1 and subtracting k carries
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {

            r_prime[s][i] = r_prime[s][i] + carry[s][i] - ((carry[s][size + i]) << Lint(ring_size - m));
        }
    }

    for (i = 0; i < numShares; i++) {
        delete[] r_bitwise[i];
        delete[] carry[i];
        delete[] b_2[i];
    }
    delete[] r_bitwise;
    delete[] carry;
    delete[] b_2;
}

void Rss_GenerateRandomShares_trunc_mp(Lint **res, Lint **res_prime, Lint **res_bitwise, uint ring_size, uint m, uint size, NodeNetwork *nodeNet) {
    // printf("start\n");
    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    uint new_size = 3 * size; // DO NOT CHANGE, IT IS IRRELEVANT for n>3
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // printf("threshold : %u \n", threshold);
    // printf("numParties : %u \n", numParties);

    int *index_map = new int[3];

    uint reset_value;
    switch (pid) {
    case 1:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = 3;
        index_map[2] = 5;
        break;
    case 2:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = 3;
        break;
    case 3:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 4:
        reset_value = 1;
        index_map[0] = 5;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 5:
        reset_value = 0;
        index_map[0] = 3;
        index_map[1] = 5;
        index_map[2] = -1;
        break;
    }

    bool prg_bools[4][6] = {
        {1, 1, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 0},
        {0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1},
    };

    Lint **recvbuf = new Lint *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new Lint[new_size];
        memset(recvbuf[i], 0, sizeof(Lint) * new_size);
    }

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    // printf("new_size : %u \n", new_size);

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[numShares];
    for (i = 0; i < numShares; i++) {
        r_values[i] = new Lint *[(threshold + 1)];
        for (j = 0; j < (threshold + 1); j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size); // NECESSARY FOR n>3
        }
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);
    Lint *r_prime = new Lint[size];
    memset(r_prime, 0, sizeof(Lint) * size);

    uint8_t *r_buffer = new uint8_t[bytes * size];

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[(threshold + 1) * bytes * new_size]; // we may not use all of these random bytes, but
        nodeNet->prg_getrandom(s, bytes, (threshold + 1) * new_size, buffer[s]);
    }

    if (pid < 4) {
        // p1, p2, p3, choosing random values

        nodeNet->prg_getrandom(bytes, size, r_buffer);
        // memcpy(r_bits, r_buffer, size * bytes);

        for (i = 0; i < size; i++) {
            memcpy(r_bits + i, r_buffer + i * bytes, bytes);

            r_bits[i] = r_bits[i] & nodeNet->SHIFT[ring_size];
            r_prime[i] = (r_bits[i] >> Lint(m));

            // printf("hi1\n");
            for (size_t s = 1; s < numShares; s++) {
                // memcpy(r_values[s][p_index], buffer[s], new_size * bytes);
                memcpy(r_values[s][p_index] + 2 * i, buffer[s] + (2 * i) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 1, buffer[s] + (2 * i + 1) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 2, buffer[s] + (2 * i + 2) * bytes, bytes);
            }

            // r_values[0][p_index][2 * i] = Lint(5 + i);
            // r_values[0][p_index][2 * i + 1] = Lint(5 + i);
            r_values[0][p_index][2 * i] = r_bits[i];
            r_values[0][p_index][2 * i + 1] = r_bits[i];
            r_values[0][p_index][2 * i + 2] = r_bits[i];

            for (size_t s = 1; s < numShares; s++) {
                r_values[0][p_index][2 * i] -= r_values[s][p_index][2 * i];
                r_values[0][p_index][2 * i + 1] ^= r_values[s][p_index][2 * i + 1];
                r_values[0][p_index][2 * i + 2] -= r_values[s][p_index][2 * i + 2];
            }
        }
    }



    for (size_t i = 0; i < size; i++) {

        int index = ((pid - 2) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
        for (size_t t = 0; t < threshold + 1; t++) {
            if (p_index != t) {
                // int index = (((pid - 2 - t) % ((numParties - 1))) + ((numParties - 1)) )  % ((numParties - 1) );

                // printf("%i : bool_index = %i\n", t, index);
                // loop through num_shares
                // if we're supposed to generate, then memcpy from buffer[j]
                for (size_t s = 0; s < numShares; s++) {
                    if (prg_bools[index][s]) {
                        // printf("copying: %llu\n", copying);
                        memcpy(r_values[s][t] + (2 * i), buffer[s] + (2 * i) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 1), buffer[s] + (2 * i + 1) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 2), buffer[s] + (2 * i + 2) * bytes, bytes);
                    }
                }
                // printf("subtracting\(numParties - 1)");
                index = ((index - 1) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
            }
        }
    }

    nodeNet->SendAndGetDataFromPeer_Eda(r_values[0][p_index], recvbuf, new_size, ring_size);

    // extracting from buffer
    for (size_t i = 0; i < size; i++) {
        j = reset_value;
        for (size_t t = 0; t < threshold + 1; t++) {

            if (index_map[t] > 0) {
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i], j, index_map[t]);
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i + 1], j, index_map[t]);
                memcpy(r_values[index_map[t]][t] + 2 * i, recvbuf[j] + 2 * i, sizeof(Lint));
                memcpy(r_values[index_map[t]][t] + 2 * i + 1, recvbuf[j] + 2 * i + 1, sizeof(Lint));
                memcpy(r_values[index_map[t]][t] + 2 * i + 2, recvbuf[j] + 2 * i + 2, sizeof(Lint));
                j++;
            }
        }
    }

    for (size_t i = 0; i < size; i++) {
        for (uint p = 0; p < threshold + 1; p++) {
            for (size_t s = 0; s < numShares; s++) {
                res_bitwise[s][p * (size) + i] = r_values[s][p][2 * i + 1];
            }
        }
    }

    for (i = 0; i < size; i++) {
        // this is so we only have t+1 parties generating shares
        for (j = 0; j < threshold + 1; j++) {
            // adding all the parties arithmetic shares together
            for (size_t s = 0; s < numShares; s++) {
                res[s][i] += r_values[s][j][2 * i];

                res_prime[s][i] += r_values[s][j][2 * i + 2];
                // res[1][i] += r_values[1][j][1 * i];
            }
        }
    }

    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;
    delete[] index_map;

    for (i = 0; i < numShares; i++) {
        for (j = 0; j < (threshold + 1); j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_buffer;
    delete[] r_prime;
    delete[] r_bits;
}
