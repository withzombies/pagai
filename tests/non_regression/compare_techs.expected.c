#include <stdlib.h>

int f(int x) {
	return /* invariant:
	        0: -i + 49 >= 0
	        0: i
	        1: i >= 0
	       array of constraints of size 2
	       environment: dim = (1,0), count = 25
	       polyhedron of dim (1,0)
	       */
	       x + 1;
}

int main()
{
	int x;
	int z = /* reachable */
	        rand() % 2;
	if (z == 0) {
		for (int i = 0; i < 50; ++i) {
			x = f(i);
		}
	} else {
		x = f(60);
	}
    /* invariant:
     0: -x + 61 >= 0
     0: x
     1: x - 50 >= 0
    array of constraints of size 2
    environment: dim = (1,0), count = 2
    polyhedron of dim (1,0)
    */
    return x;
}
#include <stdlib.h>

int f(int x) {
	return /* invariant:
	       49-i >= 0
	       i >= 0
	       */
	       x + 1;
}

int main()
{
	int x;
	int z = /* reachable */
	        rand() % 2;
	if (z == 0) {
		for (int i = 0; i < 50; ++i) {
			x = f(i);
		}
	} else {
		x = f(60);
	}
    /* invariant:
    61-x >= 0
    x-50 >= 0
    */
    return x;
}
#include <stdlib.h>

int f(int x) {
	return /* invariant:
	       49-i >= 0
	       i >= 0
	       */
	       x + 1;
}

int main()
{
	int x;
	int z = /* reachable */
	        rand() % 2;
	if (z == 0) {
		for (int i = 0; i < 50; ++i) {
			x = f(i);
		}
	} else {
		x = f(60);
	}
    /* invariant:
    61-x >= 0
    x-50 >= 0
    */
    return x;
}
#include <stdlib.h>

int f(int x) {
	return /* invariant:
	       49-i >= 0
	       i >= 0
	       */
	       x + 1;
}

int main()
{
	int x;
	int z = /* reachable */
	        rand() % 2;
	if (z == 0) {
		for (int i = 0; i < 50; ++i) {
			x = f(i);
		}
	} else {
		x = f(60);
	}
    /* invariant:
    61-x >= 0
    x-50 >= 0
    */
    return x;
}
#include <stdlib.h>

int f(int x) {
	return /* invariant:
	       49-i >= 0
	       i >= 0
	       */
	       x + 1;
}

int main()
{
	int x;
	int z = /* reachable */
	        rand() % 2;
	if (z == 0) {
		for (int i = 0; i < 50; ++i) {
			x = f(i);
		}
	} else {
		x = f(60);
	}
    /* invariant:
    61-x >= 0
    x-50 >= 0
    */
    return x;
}
#include <stdlib.h>

int f(int x) {
	return /* invariant:
	       49-i >= 0
	       i >= 0
	       */
	       x + 1;
}

int main()
{
	int x;
	int z = /* reachable */
	        rand() % 2;
	if (z == 0) {
		for (int i = 0; i < 50; ++i) {
			x = f(i);
		}
	} else {
		x = f(60);
	}
    /* invariant:
    Disjunct 0
    Disjunct 1
    x-50 = 0
    x-61 = 0
    */
    return x;
}
#include <stdlib.h>

int f(int x) {
	return /* invariant:
	       49-i >= 0
	       i >= 0
	       */
	       x + 1;
}

int main()
{
	int x;
	int z = /* reachable */
	        rand() % 2;
	if (z == 0) {
		for (int i = 0; i < 50; ++i) {
			x = f(i);
		}
	} else {
		x = f(60);
	}
    /* invariant:
    61-x >= 0
    x-50 >= 0
    */
    return x;
}
[0;1;34m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m[0;1;35m#include <stdlib.h>

int f(int x) {
	return /* invariant:
	       49-i >= 0
	       i >= 0
	       */
	       x + 1;
}

int main()
{
	int x;
	int z = /* reachable */
	        rand() % 2;
	if (z == 0) {
		for (int i = 0; i < 50; ++i) {
			x = f(i);
		}
	} else {
		x = f(60);
	}
    /* invariant:
    61-x >= 0
    x-50 >= 0
    */
    return x;
}
