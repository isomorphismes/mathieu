# Mathieu agent instructions

## Mathematical video ownership (mandatory)

For generated movies, Mathieu owns the *mathematical action*, generator
verification, user gesture and control history, deterministic replay clock,
labels/shapes/colors/camera, and source frames. Produce contiguous numbered
PNG or PPM images beginning with frame zero, preserving their checksums,
trajectory and scene/session provenance.

**Do not implement or copy a Mathieu-specific FFmpeg encoder, video writer,
raw-RGB-to-FFmpeg wrapper or MP4 package.** Use the established
`isomorphisms/kitchen/tasks/movie-from-stills/build.sh` interface with a
reviewed/pinned Kitchen checkout. Features involving codec, muxing, audio,
containers or frame-rate conversion belong in Kitchen, with Kitchen tests.
The Mathieu-specific runner may validate its own event log, exact
permutations and image hashes, invoke Kitchen unchanged, and verify output;
it must not independently construct an ordinary FFmpeg encoder command.

Source boundaries and examples:
- https://github.com/isomorphisms/kitchen/issues/16
- https://github.com/isomorphisms/kitchen/tree/main/tasks/movie-from-stills
- https://github.com/isomorphismes/spinor/tree/main/movie
- https://github.com/isomorphismes/pauli/blob/main/AGENTS.md
- https://github.com/isomorphismes/wegert/blob/main/AGENTS.md

A replayed movie can illustrate mathematically correct source state. It
**cannot** by itself prove that the running Android application displayed
the same motion. Capture actual runtime evidence when an app or device
claim is being made; preserve backend, process and device identity. Do not
invent app behavior through independently synthesized animation.

No Mathieu group can be established by merely naming one in a manifest:
use exact independent generator fixtures and checks of the generated
permutation group, composition and transitivity.

The repo currently archives original mathematical papers. Do not claim
a completed gesture recorder, replay renderer, APK or actual Mathieu video
merely because the Kitchen movie command and synthetic test pass.

Integration: Flexible Pipes → maintained Kitchen/Ike where required →
independent AI-CI acceptance. Follow
https://github.com/isomorphisms/flexible-pipes/issues/243.
