# SMB360

Clean-room platform shell for a native Xbox 360 homebrew port of Super Mario Bros gameplay logic.
No Nintendo ROM, level data, graphics, music or other proprietary assets are included.


### Xenon artifact verification

`scripts/build_xenon_with_nathsou.sh` now runs a strict toolchain preflight before compiling and verifies the resulting `smb360.elf32` as a 32-bit big-endian PowerPC ELF before accepting it. This prevents a missing/incorrect SDK or wrong-architecture output from being mistaken for a usable XeLL payload.

## PC validation

```sh
./scripts/build_pc.sh
```

## Xbox 360 cross-build

Install/use LibXenon, export `DEVKITXENON`, ensure its binaries are on `PATH`, then run:

```sh
./scripts/build_xenon.sh
```

The project also contains a Docker helper following Free60's documented prebuilt-image workflow.

## ROM handling

`tools/romcheck.py` only validates a ROM supplied locally by the user. Strict identity checking targets the verified headered SMB1 World dump (40,976 bytes; SHA-1 `33d23c2f2cfa4c9efec87f7bc1321ce3ce6c89bd`). ROM files are git-ignored.

When an external `nathsou/smb` checkout is available, `docs/NATHSOU_REAL_CORE_RUNBOOK.md` describes the headless real-core build path. The checkout is staged before compilation and receives an idempotent endian-portability patch; the user checkout itself is not modified. For Xbox 360, `scripts/build_xenon_with_nathsou.sh /path/to/smb` stages the same checkout and enables the production `SMB360_WITH_NATHSOU_CORE` gameplay path.

## Verified status

Previous percentage estimates were retired after a clean recovery audit. The project now reports evidence-backed acceptance gates rather than pretending infrastructure points equal total effort. See `docs/ACCEPTANCE_GATES.md`.

The primary gameplay-core candidate is now `nathsou/smb`, an Apache-2.0 static recompilation with its platform layer separable from the C99 core. `smb-vanilla-port` remains reference-only because no explicit repository license was found in the audited upstream. Nintendo ROM/assets are never bundled. See `docs/CORE_SELECTION.md`.


## ROM-independent hardware diagnostics

The Xenon runtime includes a generated 256x240 video/controller/audio diagnostic mode. If the real SMB core or an accepted owner ROM is unavailable, the Xbox build enters diagnostics instead of halting. This lets the first `xenon.elf` validate LibXenon video, controller input, audio, and 60 Hz pacing without bundling game assets.
