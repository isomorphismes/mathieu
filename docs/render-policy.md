# Rendering a mathematical scene without controls

`mathieu_render_with_options` accepts `mathieu_render_options.controls_visible`.
Zero disables the entire controls pass at source and fits the complete dance to
the supplied viewport. One composes the scene and controls in the interactive
360×720 layout. `mathieu_render` retains that interactive default.

The scene owns positions, dancers, Tarot markers, optional mathematical labels,
cycle arrows and selected/Witt-block rings. The controls pass owns the title,
buttons, history, cycle text, selection instructions and error messages.
`mathieu_render_scene` and `mathieu_render_controls` expose these passes for
application composition. Neither pass advances or mutates the session. Render
policy is presentation state, not part of a mathematical checkpoint.

Movie callers select controls off and drive the session with a separately
authored timed motion plan. No screenshot cropping, masking or UI erasure is
involved. The host test checks both degrees/layouts, scene motion, control-only
error changes, pass composition, session immutability and stride boundaries.
The M₂₄ rosette and sampled layout transitions use the same scene path; see
[`rosette-layout.md`](rosette-layout.md). `render-policy-test OUTPUT.ppm` also
writes the real controls-free 1280×720 rosette for visual inspection.
These are host renderer checks, not physical-device acceptance or a new APK.
