# SMB360 — Xbox 360 hardware validation protocol

This protocol is the evidence path for acceptance gates 21–24. It deliberately keeps Nintendo ROM data outside the repository and release artifacts.

## Prepare the USB device

1. Format a USB device as FAT32.
2. Extract the verified SMB360 real-core release candidate.
3. Copy `xenon.elf` to the USB root.
4. Verify `xenon.elf` against `SHA256SUMS.txt` before testing.
5. For diagnostics-only testing, do not put a ROM on the USB device.
6. For gameplay testing, place an accepted owner-supplied 40,976-byte SMB1 ROM at the USB root as `smb.nes`.

XeLL Reloaded should load `xenon.elf` from the FAT32 USB root.

## Gate 21 — video output

Boot without `smb.nes` so SMB360 enters hardware diagnostics. A valid video test requires all of the following to be physically visible on the console output:

- a stable 256x240 generated color/grid test image;
- a white sweep marker moving across the top of the image;
- no persistent corruption, endian-swapped palette blocks, or frame collapse;
- controller indicator blocks remain visible in the lower part of the frame.

Software success alone is not enough for this gate: the image must be observed on the real Xbox 360 output.

## Gate 22 — controller input

While the diagnostic image is visible, press each of these controls individually:

- D-pad Up, Down, Left, Right;
- A;
- B;
- Start;
- Back.

Each corresponding indicator block must visibly change while its control is held. The gate passes only after the physical controller response is observed on the real console.

## Gate 23 — audio output

The diagnostic mode continuously generates a deterministic 440 Hz stereo square wave at 48 kHz. The gate passes when the tone is physically audible from the Xbox 360 audio output without continuous severe distortion, silence, or repeated dropout.

## Gameplay bring-up

After gates 21–23 are observed, put an accepted owner ROM on the USB root as `smb.nes` and reboot.

Expected sequence:

1. SMB360 reports an accepted ROM identity and initializes the real nathsou gameplay core.
2. The SMB title screen appears.
3. Start begins the game.
4. D-pad and A/B control Mario in World 1-1.
5. Video, input and audio continue operating under gameplay rather than only diagnostic mode.

If the ROM is missing, the size is wrong, or the exact identity is rejected, SMB360 intentionally falls back to diagnostic mode.

## Gate 24 — full playthrough

Gate 24 requires a complete physical-console playthrough of SMB1 through the final completion sequence. Crashes, permanent audio/video failure, controller loss, watchdog halt, or unrecoverable timing failure prevent this gate from passing.

## Evidence to retain

For each physical test session retain the exact `xenon.elf` SHA-256, build manifest, Xbox/XeLL version if known, display connection used, controller type, whether the 440 Hz tone was audible, and the highest gameplay point reached. A phone photo or short video of diagnostics/gameplay is useful supplementary evidence but does not need to be committed to the source repository.
