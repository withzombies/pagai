#include "pagai_assert.h"

int main()
{
    int i = 50;
    /* reachable */
    /* invariant:
    50-i >= 0
    i-1 >= 0
    */
    while (i > 0) {
        --i;
    }
    /* assert OK */
    assert(i == 0);
    /* reachable */
    return 0;
}
