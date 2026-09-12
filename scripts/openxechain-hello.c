int main(void) {
    volatile unsigned long heartbeat = 0;
    for (;;) {
        heartbeat++;
    }
    return 0;
}
