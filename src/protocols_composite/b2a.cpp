#include "b2a.h"

void Rss_b2a(Lint **res, Lint **a, uint ring_size, uint size, NodeNetwork *nodeNet) {
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i;
    Lint **b = new Lint *[numShares];
    Lint **b_2 = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];

    Lint *c = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
    }
    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
        if (pid == 1) {
            ai[0] = 1; // party 1's share 1
        } else if (pid == 3) {
            ai[1] = 1; // party 3's share 2
        }


    Rss_RandBit(b, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            b_2[s][i] = GET_BIT(b[s][i], Lint(0));
            // b_2[1][i] = GET_BIT(b[1][i], Lint(0));

            sum[s][i] = (a[s][i] ^ b_2[s][i]);
            // sum[1][i] = (a[1][i] ^ b_2[1][i]);
        }
    }

    Rss_Open_Bitwise(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = ai[s] * c[i] + b[s][i] - 2 * c[i] * b[s][i];
        }
        // res[1][i] = a2 * c[i] + b[1][i] - 2 * c[i] * b[1][i];
    }

    delete[] c;
    delete[] ai;

    for (i = 0; i < numShares; i++) {
        delete[] b[i];
        delete[] b_2[i];
        delete[] sum[i];
    }
    delete[] b;
    delete[] b_2;
    delete[] sum;
}