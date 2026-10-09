/* Embed the ENTIRE verified iNES ROM from the repository.
 * Build from the repo root so this path cannot silently resolve elsewhere. */
    .section .rodata
    .balign 16
    .global smb360_canonical_rom
smb360_canonical_rom:
    .incbin "assets/canonical/SMB_v026.nes"
    .global smb360_canonical_rom_end
smb360_canonical_rom_end:
