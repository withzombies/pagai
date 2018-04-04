int main()
{
    for (/* reachable */
         int i = 0; i < 50; ++i) {
        (void) i;
    /* invariant:
    49-i >= 0
    i >= 0
    */
    }
    /* reachable */
    return 0;
}
