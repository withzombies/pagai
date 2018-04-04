#include "pagai_assert.h"

int main()
{
    int i = 50;
    while (i > 0) {
        --i;
    }
    assert(i == 0);
    return 0;
}
