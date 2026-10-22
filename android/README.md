# Native Android boundary

The framework NativeActivity consumes the same portable engine, session controls, geometry and CPU framebuffer renderer used by host acceptance. It requests no network or storage permission. The app-private state checkpoint is atomically replaced; exact input events and resulting permutations are journaled with logical ticks and source identity.

Checkpoints use the portable `MATHS02` format rather than a C structure dump. They preserve the active cycle, animation frame, pause/play state, exact word and redo suffix, selected points, presentation choices, automatic generator cursor and 64-bit logical tick. A CRC detects interrupted/corrupt bytes; restoration additionally checks the word and the genuine generator/undo transition before replacing any live state. The earlier unshipped `MATH-01` checkpoint discarded in-flight motion and pause state and is not accepted as an equivalent session. This is session persistence, not complete event replay or video export.

The row-major 4×6 grid is a display coordinate map, not a claim of a Miracle Octad Generator arrangement. A move sends every dancer to the exact destination in the current permutation. Cycles animate sequentially, with curved opposite directions for swaps.

Package identity: `org.isomorphisms.mathieu`; launcher: Mathieu. A1 requires armeabi-v7a; C67 requires arm64-v8a. Use the maintained android-NDK NativeActivity packager. No signer is declared yet. That boundary must fail closed rather than generating a new key on each runner or silently borrowing another application's identity.

The event journal is an implementation foundation, not accepted replay/video export. Reconcile it with the existing capture branch before claiming the FP #243 contract. Export/share UI, source replay, cancelled/interrupted export tests, and actual video acceptance remain open. Host renderer tests do not establish Android window/input behavior.
