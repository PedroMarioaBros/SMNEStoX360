/* S017: explicitly entered, CRT-free Xbox 360 title compatibility probe.
 * The previous S015 and S016 linked Newlib startup/stdio/file I/O even
 * where the application was empty.  This XEX does not need Newlib:
 * only KeDelayExecutionThread, XamLoaderTerminateTitle and
 * ExTerminateThread are allowed imports.
 *
 * This is Xbox-OS PowerPC code, not Xenon bare-metal/libxenon code.
 * No unsafe framebuffer I/O, memory-mapped hardware, threads,
 * ROM, heap, stack buffers exceeding 8 bytes, printf or stdio.
 * A boot/crash verdict still REQUIRES a real console.
 */
#include <stdint.h>
#include <xecore/xboxkrnl_threads.h>
#include <xecore/xam_loader.h>

/* Tell the PE linker to point the XEX entry directly here.
 * Do not define 'main': that would trigger xecore/Newlib CRT startup.
 */
__attribute__((noreturn)) void smb360_entry(void) {
    int64_t relative_100ns = -50000000ll;
    (void)KeDelayExecutionThread(0u, 0u, &relative_100ns);
    /* Xbox dashboard owns process teardown.  Exit via documented
     * XAM title-termination export (rather than returning to a
     * missing CRT or guessing kernel MMIO). */
    XamLoaderTerminateTitle();
    /* The Xbox OS may not return from terminate.  Defensive
     * termination of this thread if XAM unexpectedly returns.
     * This is unreachable during the intended exit path. */
    (void)ExTerminateThread(0u);
    for (;;) {
        /* No second launch and no writes to unknown memory. */
    }
}
