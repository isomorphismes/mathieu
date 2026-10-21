# Exact source coordinates

The four mathematical permutation expressions are from the online ATLAS:

- https://brauer.maths.qmul.ac.uk/Atlas/spor/M12/gap/M12G1-p12aB0.g1
- https://brauer.maths.qmul.ac.uk/Atlas/spor/M12/gap/M12G1-p12aB0.g2
- https://brauer.maths.qmul.ac.uk/Atlas/spor/M24/gap/M24G1-p24B0.g1
- https://brauer.maths.qmul.ac.uk/Atlas/spor/M24/gap/M24G1-p24B0.g2

These small expressions are exact mathematical reference data, not reproductions of chapters or illustrations. Source pages retrieved 2026-10-21:

- https://brauer.maths.qmul.ac.uk/Atlas/v3/permrep/M12G1-p12aB0
- https://brauer.maths.qmul.ac.uk/Atlas/v3/permrep/M24G1-p24B0

ATLAS gives seed hexad {1,2,3,4,5,9} and seed octad {1,2,3,4,5,11,17,24}. The native engine subtracts one from each label. Every bit mask, generator, dancer position and grid cell uses that same coordinate order. The grid is row-major 4×6; it is not claimed to be Curtis's MOG. Products act left to right, as GAP's right action does. Uppercase A/B denote inverse moves.

Octads are the orbit of the ATLAS seed under the actual generators. Their binary span constructs the extended Golay code directly in ATLAS coordinates, avoiding an unexplained conjugation from another code convention. Qualification checks rank, all 4096 weights, and generator preservation; no numerical or graphical approximation participates.
