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


## Source recovery follow-up — 2026-10-21

A second inspection recovered **part** of the missing convention from the publisher's freely accessible two-page chapter previews. This narrows, but does not remove, the source blocker above.

- [Chapter 6 preview, printed pp. 54–55](https://page-one.springer.com/pdf/preview/10.1007/978-3-662-03516-0_6): §6.2 explicitly refers back to the fixed sextet and labeling from Chapter 5. The §6.4 diagram on p. 55 was checked visually against the PDF, not inferred from OCR: all six columns have row labels **0, 1, ω, ω̄**, top to bottom. The six columns are displayed in three adjacent pairs. This establishes the displayed row order; it does not supply the Chapter 5 code construction/basis, the full column conventions, or a book-to-ATLAS bijection.
- [Chapter 5 preview, printed pp. 36–37](https://page-one.springer.com/pdf/preview/10.1007/978-3-662-03516-0_5): contains §§5.1–5.7 and the octad triangle, not the later standard-sextet construction or UP diagrams.
- [Chapter 7 preview, printed pp. 76–77](https://page-one.springer.com/pdf/preview/10.1007/978-3-662-03516-0_7): §7.5 indexes a 3×4 array by a row in F₃ and a column in {1,2,3,4}. This is useful ternary-code context, but does not establish §6.20's numbered left-dodecad correspondence or the signs in the requested transformations.
- The [publisher full-book PDF route](https://link.springer.com/content/pdf/10.1007/978-3-662-03516-0.pdf) redirected to the subscription book page in this session. The [author's publication list](https://sites.lsa.umich.edu/rlg/publications/) lists Gr98.1 without a PDF/DVI. Google Books did not yield a readable requested page. Bounded Archive/HathiTrust/library-catalog searches did not establish an accessible complete copy; this is not proof that no copy exists. [WorldCat's record](https://search.worldcat.org/title/38910263) identifies the print book but supplied no readable mathematical pages.

All three original photographs were retrieved and visually rechecked. The signed-operation photograph includes part of the arrow-sign explanation above Proposition 7.29: scalar markings are associated with arrows ending at the indicated point. Its opening is cropped. Do not equate that destination-indexed explanation with this implementation's source-sign bits without the complete convention and a checked conversion. The photograph still does not supply §6.20 or the continuation. No overbar, arrow direction, signed fixed point or image list was admitted on this inspection.

### Exact remaining source request

1. Chapter 5's definition/construction of the standard sextet and standard Golay basis (or the complete equivalent membership rules in those exact coordinates). The p. 55 row-label diagram is now available; it need not be photographed again. The precise earlier page numbers remain unverified.
2. The complete §6.20, including its numbered left-dodecad diagram and any preceding definitions it invokes; continue onto the next page if the section crosses a page boundary. Its printed page number remains unverified.
3. Separate sharp, uncropped photographs/scans of printed **pp. 50 and 51**, including every arrowhead, diagram boundary and comment. Preserve the existing UP0–UP4 photograph; request a sharper full p. 49 only if individual endpoints remain unresolved after the conventions are supplied.
4. The entire page containing §7.28.1–3 / Proposition 7.29, including the beginning of the signed notation paragraph above it, plus the following page(s) through the end of Proposition 7.29 and its proof. Printed page numbers are not visible in the supplied photograph and must not be guessed.

Result: sextet row labels recovered; full book coordinates still BLOCKED_SOURCE; **0/24 operations admitted**. No candidate with fully sourced images/signs and coordinates was available to exercise the signed algebra or Golay equivalence checker. Those candidate checks are NOT_RUN, not PASS. The inventory and application remain unchanged; no new APK, film or physical acceptance is claimed.
