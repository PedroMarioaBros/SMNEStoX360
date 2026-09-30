#include <stdint.h>
#include <xecore/xboxkrnl.h>

/*
 * SMB360 hardware boot probe.
 * Deliberately contains no SMB core, framebuffer access, XAudio or XInput.
 * If this XEX stays alive on the physical Xbox 360, the OpenXeChain
 * startup/link/XEX path is proven on that console before adding subsystems.
 */
static uint64_t read_timebase(void) {
    uint32_t hi0, hi1, lo;
    do {
        __asm__ volatile("mftbu %0" : "=r"(hi0));
        __asm__ volatile("mftb %0" : "=r"(lo));
        __asm__ volatile("mftbu %0" : "=r"(hi1));
    } while (hi0 != hi1);
    return ((uint64_t)hi0 << 32) | lo;
}

int main(void) {
    volatile uint64_t heartbeat = 0;
    volatile uint64_t last = read_timebase();

    /*
     * Do not call any Xbox API here. Even KeDelayExecutionThread is omitted:
     * this probe tests the generated executable/startup itself.
     */
    for (;;) {
        uint64_t now = read_timebase();
        if (now - last >= 49875000ull) {
            ++heartbeat;
            last = now;
        }
        __asm__ volatile("" : : "r"(heartbeat) : "memory");
    }
}
