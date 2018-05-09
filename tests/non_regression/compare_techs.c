#include <stdlib.h>

int f(int x) {
	return x + 1;
}

int main()
{
	int x;
	int z = rand() % 2;
	if (z == 0) {
		for (int i = 0; i < 50; ++i) {
			x = f(i);
		}
	} else {
		x = f(60);
	}
    return x;
}
