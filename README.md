# SILICON v0.3 prototype
Three genuine ymfm engines, 24 programs (12 engine/tone combinations with dry and space variants), brightness, saturation and feedback echo. OPL output gain doubled. Native SDK test in tests/v03.log. No custom skin, layering, chorus or reverb yet. ARM32 unbuilt and untested. Build: python3 /path/to/sdk/tools/build.py . --target arm32 --toolchain zig. Preserve ymfm BSD license. Programs are variations, not 24 independently designed FM patches.

## v0.4
Dual engine layering, 36 programs. Native SDK simulator 19/19. Hybrid presets are variations, not independently authored patches. ARM32 not built; custom skin, chorus and reverb pending.

## v0.5
Added ymfm YM2164 as fourth selectable engine and layer, 48 program variations. DX100 concept artwork is included as reference only, not an interactive skin. DX100 SysEx mapping and instrument-specific behavior are not implemented. Native host test only; ARM32 compiler unavailable.

## v0.6
Adds stereo chorus, simple feedback-delay reverb, PURE bypass and SDK-generated CHIPS/FX pages with Q-Link mappings. The approved DX100 artwork remains a reference, not a functional photorealistic skin. Native default host test 19/19; new FX and hybrid combinations require dedicated tests. 48 program variations, not 500 unique patches. No ARM32 binary, no DX100 SysEx importer. Build with the supplied SDK using the command above. The generated skin is an interim functional layout, not the final design.

## v0.7
112 program variations (48 previous plus 64 additional effect/layer combinations). Fixed invalid YM2612 key-off routing for channels 6/7 and kept the secondary engine tracking MIDI even at zero blend, avoiding missing held notes when blend is raised. This increases CPU load and needs device benchmarking. Native SDK host test 19/19. The DX100 concept remains noninteractive; generated CHIPS/FX skins are interim. No ARM32 compiler available here. Use `bash BUILD-ARM32.sh /path/to/sdk` in Debian/Termux with Zig installed. This is not a finished release.

## v0.8
Added bass and treble EQ, linked stereo compressor and variable bit crusher; all bypassed by PURE. Existing 112 programs retain neutral defaults for new controls. Generated COLOR page and Q-Link mappings (three pages total). Native GCC build and SDK host test 19/19; dedicated FX response tests not yet performed. Photorealistic artwork remains reference only. ARM32 build remains blocked by unavailable cross compiler; no device test. 

## v0.9
Added four-stage phaser and modulated feedback flanger with neutral preset defaults and PURE bypass. Native host test 19/19; individual effect response not verified. Skin remains interim. No ARM32 binary.

## v0.10
Custom panel and live parameter-bound chip A, selected chip B and layer mix labels on all three pages. Chip B selection does not imply audible contribution at zero mix. This is an interim custom panel, not the approved photorealistic DX100 skin. Existing Q-Link mappings retained. No DSP changes. Device rendering and ARM32 build unverified.

## 1.0 RC1 (source candidate, not an MPC release)
500 named program variations verified by native host dump; still four underlying tone templates per chip, not 500 individually authored FM patches. Fixed OPL note-off pitch preservation and removed intermediate layer clipping. Reference-derived textured panel retains existing functional control layout; full DX100 skin and live device rendering remain unfinished. Native host tests passed at 44.1, 48 and 96 kHz, plus default test (76 checks total). ARM32 GCC attempt failed because cross compiler is absent. SDK MidiEvent exposes only three-byte messages and discards SysEx; DX100 import requires SDK extension or offline conversion and is not implemented. No claim of DX100 hardware-exact emulation. Additional effects and hardware validation remain outstanding.

## RC7
Fixed integer Operator selector widget and regenerated all six pages and Q-Link maps against RC6 native parameter dump. DSP unchanged. Skin structural validation recorded in tests/rc7-skin-validation.log. Approved mockup fidelity, dynamic LCD and ARM32 hardware testing remain outstanding.
