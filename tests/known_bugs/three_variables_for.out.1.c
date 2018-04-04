int main(int argc, char ** argv) {
    int x;
    int y;
    int i;

    x = 2;
    y = /* reachable */
        x + 50;

    for (i = 0; i < y; i++) {
        x = /* invariant:
            104-x >= 0
            2+2*i-x = 0
            x-2 >= 0
            */
            x + 2;
    }

    /* invariant:
    x-106 = 0
    */
    return x;
}
