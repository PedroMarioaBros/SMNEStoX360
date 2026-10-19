# NESRecomp feasibility evaluation — SMB_v026

Status: **evaluation harness prepared; upstream generation result pending CI execution**.

## Decision being tested

Pause expansion of our own static translator while evaluating the upstream
[NESRecomp framework](https://github.com/mstan/nesrecomp) against the exact
canonical ROM in this repository. This is a tool feasibility study, not a
decision to replace the project's architecture.

Pinned upstream revision:
`a39d9cdbec0ac98b1430ddb53747b5ed8f8cac87`

Upstream documentation describes a static 6502-to-C recompiler, a default
cycle-oriented backend, and a NES hardware runner. Mapper 0/NROM is listed as
supported, and an SMB project is listed as playable. Those facts make it a
strong candidate, but do not prove it can recompile our modified ROM or target
Xbox 360.

## Canonical input

- File: `assets/canonical/SMB_v026.nes`
- Size: 40,976 bytes
- SHA-256: `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`
- Expected format: iNES, 32 KiB PRG, 8 KiB CHR, mapper 0 / NROM-256.

The harness verifies the full ROM hash before downloading/building the tool.
It never patches or rewrites the canonical ROM.

## What is added to this repository

- `tools/evaluate_nesrecomp.sh`: repeatable host-side evaluation. It fetches
  only the pinned upstream commit, builds the standalone recompiler, generates
  cycle-backend C in a temporary directory, and reports generated file count,
  byte count, and line count.
- `.github/workflows/nesrecomp-evaluation.yml`: runs the harness on demand and
  for relevant changes.
- This evidence/decision document and updated project continuity docs.

Generated C is intentionally **not committed or uploaded as an artifact** by
this first-stage harness. It is derived from the ROM and should not be
redistributed by default. The harness also does not vendor upstream source.

## License and provenance

The upstream NESRecomp repository's current `LICENSE` is PolyForm
Noncommercial 1.0.0. It explicitly allows personal noncommercial research,
experimentation, testing, and hobby use, but commercial use is not granted by
that license. The framework's dependencies have separate licenses. Review the
exact pinned revision and all dependency notices before copying or modifying
upstream source inside this repository.

The existing project rule remains in force: do not adopt, copy, or migrate the
third-party Super Mario Bros. port as the product base. This evaluation invokes
the generic recompiler against our own ROM; it does not use the third-party
game project's generated code, game hooks, patches, or renderer.

## First-stage acceptance criteria

1. The canonical SHA-256 check passes.
2. The pinned upstream commit is fetched exactly.
3. The standalone recompiler builds on the Linux CI host.
4. It parses the canonical ROM and reports mapper/PRG/vectors.
5. Cycle-backend C source files are generated and non-empty.
6. Generated-source metrics are recorded by CI without publishing the code.

Passing these criteria establishes only that generation is technically
possible. It does **not** prove complete function discovery, successful
compilation of the generated game with its runner, full native coverage,
removal of interpreter fallback, or Xbox 360/XEX compatibility.

## Follow-up gates — do not skip

1. Inspect the generation report for mapper, reset/NMI/IRQ vectors, discovered
   functions, and warnings. Compare the generated source inventory with the
   canonical ROM's PRG layout.
2. Determine whether dynamically dispatched code, indirect jumps, data/code
   overlap, and modified routines require project-specific `game.toml`
   annotations. Do not borrow the SMB port's code/config as the product base.
3. Build the generated output and runner on the host, then run deterministic
   scenarios against our existing canonical reference. Record any divergence.
4. Measure native-versus-fallback execution. NESRecomp documents a generated
   interpreter fallback for code outside compiled ROM views. The final product
   requirement forbids that fallback at runtime; do not claim success unless
   coverage is proven and fallback execution is absent or disabled safely.
5. Assess runner dependencies and replace the desktop platform layer only after
   the above works. SDL2, desktop entry points, filesystem assumptions, and
   platform APIs must be audited before a Xenon/PowerPC build is attempted.
6. Only then decide whether to adapt NESRecomp, combine permitted generic
   components, or resume our own translator.

## Reproduce

From the repository root on a Linux host with Git, CMake, Ninja, a C compiler,
and network access:

```sh
bash tools/evaluate_nesrecomp.sh
```

The container used for this session cannot resolve GitHub from its shell, so
the actual upstream build/generation must be verified by the GitHub Actions
run triggered by this change. Do not describe it as passed until that run has
completed successfully.
