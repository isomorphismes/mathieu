# Signed monomial operators

Purpose: retain signs on basis vectors while extending the existing Mathieu engine for SUN MATHIEU-UP-01 under https://github.com/isomorphisms/flexible-pipes/issues/244.

Domains: a checked permutation of a finite point set and one sign, +1 or −1, for each source basis vector. A basis image records destination and sign. A signed operator sends eᵢ to εᵢ eₚ₍ᵢ₎. Fixed points may carry a negative sign. Products follow the existing right-action convention: apply the first operator, then the second.

Types: `signed_on : Number → List (Number, Integer) → Maybe SignedOperator`; `followed_by : SignedOperator → SignedOperator → Maybe SignedOperator`; `inverse : SignedOperator → Maybe SignedOperator`. Admission checks every destination exactly once and every sign. Equality compares signed basis images, not just the permutation. Printing and exit are confined to main.

Laws: the second sign is read at the first destination; inverses move the original source sign to its destination. A negative signed transposition has order four and squares to −I. Negative fixed points survive inversion. Malformed degrees, repeated destinations and signs other than ±1 fail admission.

Backend: the previously qualified Idriç 94dfd99bd3e376507fedc8611053b7173b2519f0 host Chez entrypoint. The existing exact-permutations example supplies the syntax. Qualified Android C ABI export remains unavailable, as recorded in that example. The live application continues to use its existing ICK C boundary; host Idriç success does not establish Android execution.

This program exercises the algebra only. It does not transcribe any Griess generator or establish Proposition 7.29, book coordinates, or group membership. Source photographs remain outside git. Exact commands and results follow after execution.

Host result: PASS. The observed packaged compiler at `/workspace/scratch/b8ece546d46f/idric-runtime/idric-linux-x86_64-94dfd99bd3e376507fedc8611053b7173b2519f0/bin/idric` compiled `SignedMonomial.idric` with `--cg chez -o signed-monomial`; running `build/exec/signed-monomial` printed `PASS signed Idriç composition, inverse and negative fixed points`. No new compiler failure occurred. This is a bounded host example, not independent application acceptance.

Next language boundary: retain finite-domain bijection evidence and source-sign evidence through a qualified Android C ABI export. Acceptance would call those exported operations from the maintained A1 ARM32 and C67 ARM64 consumers and compare signed basis images against an independent oracle. No generated-C/RefC fallback was used.
