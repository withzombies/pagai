#include "pagai_assert.h"

int main()
{
    int x = 0;
    for (int i = 0; i < 50; ++i) {
        x = 2 * i - x;
    }
    assert(x <= 50);
    return 0;
}
