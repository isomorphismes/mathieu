# Exact book operations: incomplete source boundary

SUN MATHIEU-UP-01 extends the existing application under [FP #244](https://github.com/isomorphisms/flexible-pipes/issues/244) and [Mathieu PR #3, “Implement exact Mathieu choreography and native Android application core”](https://github.com/isomorphismes/mathieu/pull/3). No competing application or permutation engine is introduced.

## Implemented authoring slice

`native/monomial.h` adds signed operators on the existing finite permutations. A negative bit belongs to the **source** basis vector: eᵢ → εᵢ eₚ₍ᵢ₎. For a product applying the first operator and then the second, its sign at i is εᵢ × δₚ₍ᵢ₎. The inverse sign at p(i) is εᵢ. Negative fixed points are retained. Operator order includes the product of signs around each cycle. Scalar-sign inspection distinguishes +I, −I and nonscalar diagonal operators; it does not compute the center of an unspecified generated group.

Relabeling uses an explicitly supplied old-to-new bijection c: a basis image eᵢ → εᵢ eₚ₍ᵢ₎ becomes e꜀₍ᵢ₎ → εᵢ e꜀₍ₚ₍ᵢ₎₎. The permutation and source-sign set are both transported. Invalid maps and signs outside the point domain are rejected. Invalid powers fail even at exponent zero.

`mathieu_golay_coordinates_match` checks all 4096 binary words from a supplied full-rank twelve-row book basis after conversion against the existing initialized ATLAS code. Duplicate basis spans, out-of-domain coordinates and a wrong map fail. This proves equivalence of the supplied code data; the source must still establish that those data actually describe Griess's standard sextet. An arbitrary Golay-code isomorphism does not identify his exact named operations.

Tests compare all 2304 pairs of signed three-point operators with ordinary dense matrix multiplication, verify inverses by matrix transposition, transport both matrix indices under relabeling, and exercise negative fixed points and scalar signs at degrees 12 and 24. The coordinate tests use an explicitly **synthetic** relabeling. They do not provide the missing book conversion.

## Source audit

Reference: R. L. Griess, *Twelve Sporadic Groups* (1998), §5.38, §6.20, §7.28.1–3 and Proposition 7.29. [Book](https://doi.org/10.1007/978-3-662-03516-0), [Golay chapter](https://doi.org/10.1007/978-3-662-03516-0_5), [ternary-code chapter](https://doi.org/10.1007/978-3-662-03516-0_7).

Three user photographs were visually inspected. `1000000636.jpg` contains UP0–UP4. `1000000637.jpg` contains printed pp. 50–51 with UP5–UP20; several arrow crossings/heads are too small to transcribe confidently, and the right-hand descriptions are cropped. `1000000635.jpg` contains §7.28.1–3 and the start of Proposition 7.29; it omits the definition of the left-dodecad labeling and the continuation of the proposition. No photographs or reconstructed copyrighted diagrams are committed.

The inspected photographs do **not** include the definition of the standard sextet labeling or its Golay basis, nor §6.20's left-dodecad coordinates. The current application's row-major ATLAS grid and its verified tetrad unions do not establish either book labeling. The publisher's available chapter preview/abstract does not supply these missing definitions or the full proposition.

Required source to unblock exact import:

- The standard sextet labeling and its defining Golay-code basis in §5.
- §6.20, including the left-dodecad labels.
- Sharper, complete pp. 50–51 for the dense arrows and cropped comments.
- The signed-cycle notation definition and full Proposition 7.29 with its continuation/proof, enough to resolve every overbar and fixed-point sign.

`data/griess/operations.tsv` records all 24 requested names and their pending evidence. It contains **no guessed image lists or default positive signs**. No book-named operation is exposed in the application until its exact map, signs where applicable, coordinates, subgroup claims and independent acceptance are available.

## Remaining execution

Pending: all 21 exact UP transcriptions; all three exact signed lifts; a sourced book-to-ATLAS bijection and code/design verification; group/subgroup and central-extension proofs; standard-sextet/left-dodecad layouts; operation selection/composition/inspection; signed animation, session/checkpoint/replay and shared planned-movie integration; independent AI-CI exact-head acceptance; FP admission and maintained execution receipts; separate stable-signed A1/C67 APKs; physical update/behavior acceptance and Cat Food delivery.

The existing MATHS02 format, application identity, stable signer, engine, session, renderer, layouts and movie interface are unchanged. Existing host tests remain required. This authoring slice is not completion, dispatch, independent acceptance, an APK or a movie. Keep the existing draft unmerged.
