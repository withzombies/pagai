int main(int argc, char ** argv) {
    int x;
    int y;
    int i;

    x = 2;
    y = /* reachable */
        x + 50;

    for (i = 0; i < y; i++) {
        x = /* invariant:
            51-i >= 0
            i >= 0
            x-2-2*i = 0
            */
            x + 2;
    }

    /* invariant:
    x-106 = 0
    */
    return x;
}
