# Mathieu capture: reuse the shared Kitchen movie pipeline

**Mathematical ownership:** Mathieu records the user's exact interaction and
replays its mathematical state into numbered lossless PNG/PPM frames.

**Movie ownership:** `isomorphisms/kitchen/tasks/movie-from-stills/build.sh`
converts the numbered stills into H.264/yuv420p MP4 using FFmpeg. Do not
introduce a second encoder, local FFmpeg command, or raw-RGB movie protocol
in Mathieu. See [Kitchen's canonical contract](https://github.com/isomorphisms/kitchen/blob/main/tasks/movie-from-stills/README.md)
and [Pauli's existing usage](https://github.com/isomorphismes/pauli/blob/main/AGENTS.md).

**Acceptance:** AI-CI must independently inspect the real saved mathematical
session, exact permutation state, output frames, audio policy (if any) and MP4.
The repository currently has no Mathieu interaction renderer. This PR
implements *only the input verification + Kitchen invocation foundation*.

## `mathieu.capture.v1` session

Store a UTF-8 JSON manifest plus lossless numbered stills, starting at frame 0.

| Field | Meaning |
| --- | --- |
| `format` | `mathieu.capture.v1` |
| `source_revision` | 40-character lowercase source commit SHA |
| `width`, `height`, `fps` | Frame size and 24/30/60fps playback clock |
| `frame_sha256` | One checksum per numbered PNG |
| `events` | Ordered input records with frame number and event kind |
| `math.group` | `M11`, `M12`, `M22`, `M23`, or `M24` |
| `math.generators` | Named exact 0-based permutation arrays |
| `math.word` | Generator actions in chronological order |
| `math.final_mapping` | Independently checked composite permutation |

The eventual recorder must also preserve continuous pointer input
and timestamps, checkpoints, initial state, selected labels/colors/shapes,
camera/viewport state, and enough renderer/compiler identities to reproduce
the exact interaction. The current validator checks a subset of those fields,
not the full interactive recording contract. A name such as `M24` in a
manifest does **not** prove that the listed generators belong to that group;
independent reference fixtures and order/transitivity checks remain necessary.

## Host usage (only after a genuine replay renderer exists)

1. Replay the saved interaction at fixed logical frame times, producing
   `frames/frame_000000.png`, `frames/frame_000001.png`, etc.
   Do not skip logical frames because rendering is slow. Retain the original
   frame sequence and session as editable/provenance assets.
2. Use an **explicit pinned checkout** of `isomorphisms/kitchen`.
   The CI fixture checks out commit
   `739faa3068983531a04db33b3ecc43573272368e`.
3. Invoke the Mathieu-specific consistency checks and the shared encoder:

       KITCHEN_ROOT=/path/to/verified/kitchen \
         python3 -m capture.export session.json frames mathieu.mp4

The wrapper checks event ordering, permutation bijections/composition, frame
numbering/sizes/checksums, then calls Kitchen's **unchanged** `build.sh`
with the still-image pattern, FPS, and a private temporary output. It uses
ffprobe to check decoded frame count, codec, FPS, dimensions and duration,
then atomically publishes the MP4. It never constructs its own FFmpeg command.

Kitchen's ordinary preset is H.264, yuv420p, CRF 18, faststart. Keep
1080p/60fps as the quality target where the frame renderer supports it;
4K or device-local encoding requires actual performance and correctness
qualification. No claim of A1/C67 video export is made.

## Tests

With FFmpeg/ffprobe and a pinned Kitchen checkout:

    KITCHEN_ROOT=/path/to/verified/kitchen \
      python3 -m unittest discover -s capture/tests -v

The fixture includes a **synthetic** four-frame PNG sequence and the
identity permutation, so passing it verifies the media integration and
failure behavior **but not** the real Mathieu interaction. Physical-device
runtime capture is a separate evidence source: an offline replayed movie
must never be labeled proof that the Android app displayed those frames.

## Remaining work

- Actual interactive mathematics, validated Mathieu generators, and a
  durable user-event log with interruption recovery.
- Deterministic replay into the native renderer / offscreen framebuffer,
  retaining actual interaction timing, labels, choices and camera movement.
- Finish → Preview → Export UI and separate optional raw-runtime capture.
- Real end-to-end video + mathematical AI-CI acceptance, with device proof
  where applicable. See
  [MATHIEU-CAPTURE-01](https://github.com/isomorphisms/flexible-pipes/issues/243).

Do **not** start the deferred cross-repository framebuffer-motion campaign
as an accidental prerequisite for this narrow reuse correction.
