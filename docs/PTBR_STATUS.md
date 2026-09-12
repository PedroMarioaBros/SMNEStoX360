# PT-BR ROM status and architecture boundary

SMB360 recognizes the owner-supplied base ROM and the IPS-derived PT-BR image by exact SHA-1. The translated CHR is consumed by the current nathsou static-recompilation path, which is why translated menu glyphs are visible in host frames.

However, exact PT-BR gameplay is not yet accepted as complete.

## Evidence

The supplied IPS contains 50 records. Its writes span both PRG and CHR space:

- PRG writes begin at iNES file offset `0x003C` and include very large replacement ranges, including `0x0C54..0x662A` and `0x6650..0x7F0F`.
- CHR writes occur in `0x8511..0xA008`.
- Local byte comparison after applying the IPS found approximately 29.6 KiB of effective PRG changes and 441 effective CHR byte changes relative to the supplied base image.

The pinned `nathsou/smb` project is a static recompilation generated from `src/smb.asm`. Its runtime ROM handling is for CHR/graphics extraction; it does not execute the owner ROM's PRG bytes. Consequently, arbitrary PRG changes from an IPS are not automatically represented by loading the patched ROM.

This was observed in the real-core PT-BR host frame: translated menu text is present, while at least one status string remains inconsistent (`CORLD` rather than the intended translated/original word). Therefore SMB360 must not claim exact PT-BR compatibility yet.

## Options for exact PT-BR support

1. Port the translation/hack's PRG changes into the disassembly/static-recompilation source and regenerate/modify the C output. This preserves the current native static-recomp architecture but may require translating code and data changes across a large portion of PRG space.
2. Add a separate legally clean NES execution core for variants whose PRG differs from the static recompilation. This would execute the owner-supplied translated PRG exactly but changes the implementation architecture for that variant.
3. Rebase/recreate the Portuguese translation directly against the pinned `src/smb.asm` source, restricting changes to known text/data tables where possible. This is preferable if the translation can be reconstructed without the large binary PRG replacement ranges.

Until one of these paths is verified, the World/base gameplay path is the fidelity baseline and the PT-BR image remains an accepted experimental asset source, not a fully verified translated gameplay build.
