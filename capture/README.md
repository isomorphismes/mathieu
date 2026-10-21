# Mathieu capture: session-to-video export

This is the **host-side export slice**, not a completed Mathieu interaction
or screen recorder. The repository's existing app/renderer is not yet built.
A future renderer must record the *real* session and produce the corresponding
numbered lossless PNG frames by replaying its own saved input events.

Follow [MATHIEU-CAPTURE-01](https://github.com/isomorphisms/flexible-pipes/issues/243).
Do not count this encoder or its synthetic test as end-to-end acceptance.

## Contract: mathieu.capture.v1

The session is a UTF-8 JSON document containing:

| Field | Meaning |
| --- | --- |
| format | Exactly mathieu.capture.v1 |
| source_revision | Exact 40-character lowercase Git commit SHA of the renderer |
| width, height | Output pixel dimensions, **even** for H.264/yuv420p; default target 1920×1080 |
| fps | 24, 30, or **60** frames per second |
| frame_sha256 | SHA-256 hex strings for each lossless PNG, beginning with frame zero |
| events | Nonempty, ordered original interaction events with 0-based frame index and kind |
| math.group | M11, M12, M22, M23, or M24 |
| math.generators | Dictionary of names to exact 0-based one-line permutation arrays |
| math.word | Generator names in chronological action order |
| math.final_mapping | Composite permutation, with generator words applied left to right |

The extra event fields preserve continuous input coordinates, source times
(e.g. monotonic nanoseconds), UI settings, viewport, label/shape/color choices,
camera positions, initial state, and identities of selected generators.
The future recorder/replay implementation must not discard those fields. The
encoder checks the recorded *frame indices* and mathematical word but does
not need to interpret the full input payload.

A future event recorder must persist the source events **as they occur**,
journal/checkpoint progress and expose Finish → Preview → Export Video. Replay
at fixed logical frame times: render every frame without throwing away frames
when the hardware is slow. Avoid encoding live EGL screen grabs at variable
fps or taking screenshots of an Android UI. Use an offscreen framebuffer at
the requested output dimensions, with explicit initial/final holds. Keep
original PNGs and the session log as editable source assets.

**Mathematical qualification is separate:** this exporter checks that the
named permutations are bijections and that the stated final permutation
equals their composite. It **cannot** establish that those permutations
generate the genuine Mathieu group. The renderer must use independently
verified exact Mathieu generator fixtures, not generic polynomial braids.

## Host export

Requires Python 3.9+, FFmpeg including libx264 and ffprobe.

Produce consecutive files 'frame_000000.png', 'frame_000001.png', and so on,
all with the dimensions in the session. The PNG SHA-256 hashes must match.
Then, from the repository root:

    python3 -m capture.export ./session.json ./frames ./mathieu.mp4

The exporter verifies frame numbering, hashes, image sizes, event ordering,
permutation composition, and output metadata with ffprobe. It checks the
decoded H.264 frame count, fps, dimensions, and duration before atomically
publishing the MP4. Missing/damaged frames and encoding failures leave no
new public MP4. Output uses H.264, yuv420p, CRF 17 and faststart for ordinary
sharing. Lossless source is the **original PNG sequence**, not the MP4.

Tests, including an actual tiny MP4 export and first/last frame comparison:

    python3 -m unittest discover -s capture/tests -v

The generated test pattern is deliberately *not* a Mathieu animation.

## Still required for usable capture

- Android/native (or eventual alternate UI) event journal, deterministic
  replay, exact generator data and calibrated mathematical-state checks.
- Offscreen renderer that recreates actual touch paths and strokes from the
  saved session, including real labels/colors/shapes and camera movement.
- Finish/preview/export interface and safe storage/share behavior on devices.
  C67/A1 export must be device-tested; offline/host encoding is the fallback.
- Acceptance fixtures: real interaction → log → mathematically verified replay
  → lossless frames → MP4; repeat replay yields identical *logical-state*
  hashes, video has correct endpoints, no missing frames, and is playable.
- Long/pause/resume/cancel/storage-full cases and exact-head AI-CI receipts.

Optional 4K output must be enabled only when the renderer and encoder can
supply and validate the full frame sequence. Do not claim a successful 4K
export from the ability to set 4K dimensions in a JSON file.
