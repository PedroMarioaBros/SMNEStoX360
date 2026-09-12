# SMB360 XEX target for ABadAvatar / BadUpdate

## Target

Produce `default.xex` that runs under the Xbox 360 system software and can be launched from Aurora or XeXMenu after ABadAvatar/BadUpdate has applied FreeMyXe or XeUnshackle patches.

This is a distinct runtime target from the existing XeLL/LibXenon `xenon.elf` build. The existing LibXenon target remains useful as a verified PowerPC/core build, but must not be relabeled or mechanically wrapped as the final dashboard build.

## Toolchain direction

Primary path: OpenXeChain (`ppc32-xbox360`) + xecorelib + SynthXEX.

Pinned XEX packager probe:
- OpenXeChain/SynthXEX
- commit `4bda05e21e3f6384ac447f8db3103a0a15b96887`

Pinned aggregate toolchain reference:
- OpenXeChain/buildscript
- commit `eed1fa65bf9577fd31625764b320a90182ea9ade`

The project CI contains `.github/workflows/openxechain-probe.yml` to prove that the XEX packager itself builds in our controlled CI before the full compiler/runtime is integrated.

## Runtime boundary

The portable game/core layer must continue to depend only on `smb360::PlatformApi`:

- monotonic time / sleeping
- controller input
- 32-bit framebuffer begin/end
- signed 16-bit stereo audio submission
- shutdown

The current `src/platform/xbox360` implementation is LibXenon-specific and therefore cannot be linked into a dashboard XEX unchanged.

A new Xbox-OS platform backend will be introduced without changing the nathsou core contract.

## XEX backend work items

1. Build and pin OpenXeChain in CI.
2. Produce a minimal title XEX (`default.xex`) with SynthXEX and verify XEX2 magic/headers.
3. Resolve Xbox kernel/XAM imports through xecorelib.
4. Implement controller backend using Xbox OS input APIs.
5. Implement framebuffer/GPU presentation using Xbox OS graphics APIs available to the open toolchain.
6. Implement 48 kHz signed 16-bit stereo output through Xbox OS audio APIs available to the open toolchain.
7. Implement file access relative to the title launch directory and locate `smb.nes` next to `default.xex`.
8. Compile the pinned nathsou core for `ppc32-xbox360` and retain the audited big-endian fixes.
9. Package only project code plus owner-supplied `smb.nes`; no Nintendo ROM/assets are committed to GitHub.
10. Validate the produced XEX metadata and hand the final folder to hardware testing through Aurora/XeXMenu.

## Acceptance gate

The XEX milestone is complete only when CI produces a real `default.xex` from source using the Xbox-OS ABI/toolchain and the file passes structural XEX checks. The LibXenon ELF milestone does not satisfy this gate.

Hardware completion requires launching that exact `default.xex` on the owner's ABadAvatar/BadUpdate Xbox 360 and validating video, controller, audio, ROM loading and frame pacing.
