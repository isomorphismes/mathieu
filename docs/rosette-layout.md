# Six-diamond Golay rosette

Owner assignment: https://github.com/isomorphisms/flexible-pipes/issues/253

M₂₄'s layout button cycles circle → grid → rosette → circle. The saved `grid`
field retains its old values 0 (circle) and 1 (grid), with 2 adding the rosette.
MATHS02 checkpoint size and old grid/circle bytes remain unchanged. M₁₂ does
not admit the rosette. The interactive button changes layouts immediately;
timed film geometry uses the independent sampled transition below.

Every ATLAS coordinate remains `6 × row + column`. Column c consists of
`{c, c+6, c+12, c+18}`. Its diamond center lies on the radius-105 regular hexagon
around (180, 308), starting at the top and proceeding clockwise in scene
coordinates. Rows 0, 1, 2, 3 occupy its north, east, south, west vertices with
half-diagonal 24. Each initial diamond therefore contains one of each suit
and a single marker form. Under group actions the dancers, not the sites,
carry their suit and form.

The native engine regression verifies all 15 pairwise column unions against
its actual 759 Golay octads. This is a valid Golay sextet in ATLAS coordinates;
no Curtis MOG coordinate identification is asserted.

## Deterministic geometry, exact group state

`mathieu_layout_transition` contains `from`, `to`, `frame`, `duration`.
`session_position_with_layout` samples the site geometry at frame/duration,
using the smoothstep fraction t²(3−2t). The source and target endpoints are
returned exactly; duration zero snaps to the target. Frames beyond duration,
unknown layouts, and rosette/M₁₂ combinations fail without writing coordinates.
A null trajectory uses the session's selected layout.

`session_dancer_with_layout` applies the same sampled site geometry to the
existing exact session motion. `mathieu_render_layout_with_options` draws that
same session, with the existing controls-visible policy. Invalid trajectories
leave pixels untouched. No group engine, video encoder, crop, overlay erasure,
or synthetic frame interpolation is introduced.

For the movie, use `{MATHIEU_GRID, MATHIEU_ROSETTE, frame, 90}` for a three-second
layout transition at 30 fps, then hold `{MATHIEU_ROSETTE, MATHIEU_ROSETTE, 0, 0}`.
Render with controls off. Keep mathematical `session_advance` ticks and layout
sampling on explicit tracks; sampling geometry never advances, resets or
mutates the session, selection, word, permutation, logical tick or checkpoint.
Only genuine `session_move` events change the discrete group action.

The five sites `{0,1,6,7,12}` complete the exact octad `T₀ ∪ T₁` (mask 0x0c30c3).
The tests carry that octad through `abAB`, check every endpoint and the native
block equivariance, checkpoint round-trips, button selection, marker transport
and controls-off rendering. These are host tests, not a produced/accepted film,
APK or physical-device result. The shared WIGGLE adapter must adopt this API
and update its exact-source pins before the rosette movie can be accepted.
