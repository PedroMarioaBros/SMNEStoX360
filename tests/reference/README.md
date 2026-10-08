# Independent host validation

Run `python3 tools/compare_reference.py` from the repository. Requires Python 3,
C compiler (tested GCC/x86-64), pthreads and libm. No network or desktop required.
The vendored source is the exact subset needed from binji/binjnes at
`e2f5871a28ff189daa82b70be9324a42e2aaf9fd`, MIT, copyright Ben Smith.
LICENSE is inside the source archive. Manifest verifies every included byte.
No reference code is compiled into the canonical runtime or Xbox product.

The tool fingerprints ROM/PRG/CHR before either executable runs, checks archive
and member hashes, builds separate executables, captures complete frames and all
2 KiB CPU RAM, and fails on any difference or incomplete capture. Direct C adapter
invocation is diagnostic only: use the Python entry point for the hash gate.

Defaults: 600 captures idle, 6000 captures scripted. Each capture is one completed
image at VBlank. Both runtimes start with zero RAM and their own unmodified reset.
Canonical captures just after scanline 241 dot 1 (instruction boundary); binjnes
signals its NEW_FRAME event at dot 2. Its printed PPU frame counter is one higher;
the comparison uses ordinal capture 0 against capture 0, with NO offset search,
image cropping, palette replacement, RAM exclusions, or game-specific patches.
Reference emphasis bits cause a failure rather than being silently discarded.
This alignment was validated for the tested ROM, not for every power-on scenario.

Input for capture intervals: Start 100–101; A+Right 220–269; Right 270–359;
otherwise zero. Changes occur immediately after the preceding capture (before its
NMI polling), with port 2 released. This explicitly differs from the original
run_canonical.py frame-start scheduling. Do not interchange their log baselines.
The rest of a 6000-capture run releases buttons; it is not 6000 frames of active
human play. Physical two-player interaction remains untested.

All local canonical compilation uses -Wall -Wextra -Werror. Unmodified upstream
source needs unused/implicit-fallthrough warning suppressions and
-Wno-error=format-security because its disassembler uses a nonliteral snprintf
format. The warning remains visible in compile.log. This is not permission to
weaken warnings in product sources. A first strict build failed on that warning;
a preliminary capture command then failed because the binary did not yet exist.
The successful captures were regenerated after building the reference.

S006 comparison exposed the $4017 read/write alias bug: phantom port-2 input
changed RAM $06fd and $074b while pixels remained identical. Fixing the controller
read path removed all RAM differences in the measured sequences. Sources for
controller protocol: https://www.nesdev.org/wiki/Standard_controller and
https://www.nesdev.org/wiki/Controller_reading . Open bus is still incomplete.

No claim of complete NES fidelity: APU/audio, PPU microtiming, CPU microcycles,
power-up races, other input scripts, and Xbox hardware require further work.
