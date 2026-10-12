/* S016 boot isolation, stage 01: Xbox OS kernel only.
 * NO ROM, NO graphics/MMIO, NO direct XAM calls, NO stdio/printf,
 * but the linked OpenXeChain CRT still imports xam.xex.
 * NO dynamic allocations, NO threads. Return after a 5s delay.
 * This can still crash if the PE/XEX/startup/kernel import is broken.
 * It is NOT the Mario game. */
#include <stdint.h>
#include <xecore/xboxkrnl.h>
int main(void) {
    int64_t five_seconds = -50000000ll;
    KeDelayExecutionThread(0u, 0u, &five_seconds);
    return 0;
}
