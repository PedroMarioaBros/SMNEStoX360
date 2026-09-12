# SMB360 — verified acceptance gates

A gate is marked PASS only when there is inspectable or executable evidence. The gate count is a checklist, not an estimate of remaining engineering effort.

| # | Acceptance gate | Status | Evidence |
|---:|---|---|---|
| 1 | Clean GCC Release configure/build/test | PASS | host verification matrix |
| 2 | Clean Clang Release configure/build/test | PASS | host verification matrix |
| 3 | ASan + UBSan host suite | PASS | host verification matrix |
| 4 | Project sources compile with warnings-as-errors | PASS | strict-warning builds |
| 5 | Source/release contains no ROM/FDS payload | PASS | source/release scans |
| 6 | Core isolated from Xenon platform headers | PASS | boundary audit |
| 7 | iNES parsing/bounds validation | PASS | unit tests |
| 8 | SMB1 ROM structural validation | PASS | unit tests |
| 9 | Platform-neutral input mapping | PASS | unit/replay tests |
| 10 | Framebuffer/tile/scale primitives | PASS | unit tests |
| 11 | Audio buffering/timing primitives | PASS | unit tests |
| 12 | Deterministic replay/state hashing infrastructure | PASS | replay/hash tests |
| 13 | LibXenon bootstrap/input/storage/audio/video/gameplay API contracts | PASS | Xenon contract tests + diagnostic mode |
| 14 | Licensed-core adapter + exact upstream public API contract | PASS | nathsou adapter tests including save/load-state ABI |
| 15 | Common runner supports licensed core RGB888 output path | PASS | RGB/core tests |
| 16 | Complete real Apache-2.0 nathsou/smb checkout compiles in SMB360 | PASS | exact commit `2143a91e84a7aa888a31f0def9ed5f747fc3da26`, 19/19 Git blobs verified, real host/Xenon compilation |
| 17 | User-supplied legal SMB1 ROM accepted by exact verified hash | PASS | exact allowlist validation; ROM remains outside repository |
| 18 | First real SMB1 frame produced on host through licensed core | PASS | real-core 256x240 title frame generated from owner ROM |
| 19 | World 1-1 controllable on host | PASS | deterministic START then RIGHT+B run; player-region framebuffer movement observed |
| 20 | LibXenon cross-build produces SMB360 `.elf32` | PASS | official Free60 image produced verified 32-bit MSB PowerPC static ELF |
| 21 | Video output validated on real Xbox 360 | PENDING | physical-console observation required |
| 22 | Controller input validated on real Xbox 360 | PENDING | physical-console observation required |
| 23 | Audio output validated on real Xbox 360 | PENDING | physical-console observation required |
| 24 | Full SMB1 playthrough validated on Xbox 360 | PENDING | physical-console playthrough required |
| 25 | Reproducible release package validated | PASS | LibXenon image pinned by digest; two clean real-core builds were byte-identical (`xenon.elf` SHA-256 `7c4622b586e796b08c491118b3fe28a84b924e6d4612bf1eb0a16c563c859ca9`); two normalized release packages were byte-identical; release SHA-256 `df3c548bbd8d3b07f1ab964c8baf7987eec1edfea8d4762c4e0ec40d4d53be44`; ROM scan empty |

Verified checklist state: **21 PASS / 25 total**.

The four remaining gates require evidence from an actual Xbox 360. See `docs/HARDWARE_VALIDATION.md`.
