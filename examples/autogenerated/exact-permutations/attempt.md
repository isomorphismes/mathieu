# Exact finite permutations in Idriç

Purpose: preserve the mathematical boundary for the native choreography engine in a small, executable Idriç program.

Domains: a point belongs to a declared finite action; a permutation is a bijection of that point set. An image list implements the mapping but does not define a new group. Products act left to right. Inputs are a degree and a list of images; admission returns a checked permutation or failure. Composition and inversion return checked permutations or failure. The executable checks ATLAS M12 generators, inverses and a noncommuting product.

Central types: `permutation_on : Number → List Number → Maybe Permutation`, `compose : Permutation → Permutation → Maybe Permutation`, `inverse : Permutation → Maybe Permutation`. Domain checking and arithmetic are pure; printing and process exit are effects only in `main`.

Invariants: exactly one image for each point, no duplicates or out-of-range values, equal domains for products. Equality is equality of admitted image lists. This first slice uses runtime admission; it does not pretend to carry a dependent bijection proof. A stronger type should bind the point domain to every admitted mapping and preserve that witness under products.

Required backend: current canonical Idriç host Chez runtime. The program must compile through the actual Idriç entrypoint, not an invented grammar. Android C ABI export and the ARM32/ARM64 runtime are separately unqualified here; host success does not select an Android Idriç build route. The app uses the independently tested ICK native engine while retaining this language slice.

Compiler command, result and first missing capability will be appended after execution. No generated-C or RefC route is allowed.

First attempt: Idriç 94dfd99bd3e376507fedc8611053b7173b2519f0, `idric --cg chez -o exact-permutations ExactPermutations.idric`. FAIL: inherited `Data.List.findIndex` returns a bounded index rather than a whole number. The failed source is retained as `ExactPermutations.failed-v1.idric`. The corrected attempt explicitly converts the implementation index at this boundary. This is an adapter mistake, not evidence of a compiler defect.

Final host result: PASS with that exact compiler and the Chez backend. Executing `build/exec/exact-permutations` prints `PASS exact Idriç permutation admission, inverse and composition`. A second inference failure in an overloaded bind/case expression was removed by naming the semantic operation `followed_by` and making the intermediate case explicit. No compiler defect is claimed from that reduction.

Next language work: an admitted finite-domain permutation should retain its domain and bijection evidence under composition, with a qualified C ABI export usable by the maintained ARM32/ARM64 Android route. Acceptance: export the same admitted ATLAS mappings, call the exported operations from both Android ABI consumers, and compare their actual results with GAP. This host Chez program does not meet that Android boundary.

The portable session-checkpoint addition retains that same native C boundary. Its domains are bounded generator words, a cursor into the word, an exact forward/undo permutation transition, selected point sets, and fixed-step animation state. Encoding is a pure checked state-to-bytes function; decoding is bytes-to-admitted-state and cannot partially mutate the consumer on failure. Atomic file replacement remains an Android effect. The existing missing qualified Idriç Android C ABI export still prevents moving this live application boundary into the host Chez slice; no new language or backend capability is claimed.
