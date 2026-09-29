    .section .rodata
    .balign 16
    .global smb360_embedded_chr
    .type smb360_embedded_chr,@object
smb360_embedded_chr:
    .incbin "assets/SMB_v026.chr"
    .size smb360_embedded_chr, 8192
