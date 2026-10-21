# Cartographic sources: Malle, Zvonkin and Monsieur Mathieu

Source trail: the user's [24 July 2019 post](https://x.com/isomorphisms/status/1153999879448027138), read with all three attachments on 21 October 2026. The post names Malle, Zvonkin and NeverEndingBooks. The follow-up papers below were found through the authors' bibliography and citations.

These are external full texts and original reading notes. No later PDF, book scan or reproduced figure is stored here. Public access alone does not establish redistribution permission; the 2023 arXiv version uses arXiv's non-exclusive distribution license.

## 1. The six-edge Malle example

Alexander Zvonkin, *How to draw a group?*, **Discrete Mathematics 180** (1998), 403–413, [DOI 10.1016/S0012-365X(97)00128-3](https://doi.org/10.1016/S0012-365X(97)00128-3).

- [Author-hosted version](https://www.labri.fr/perso/zvonkin/Research/drawgroup.pdf), 13 pages.
- [FPSAC 1995 proceedings version](https://fpsac-archive.github.io/FPSAC95/ARTICLES/61.ZVONKIN.pdf), 11 scanned pages, printed pp. 601–611. This earlier version supplied the legible text used here; page numbers and versions must remain distinct.
- Read Example 1.2 / Figure 2, then Example 2.5 and §3. The example is credited to **Günter Malle's April 1993 Luminy conference talk**, not to an identified standalone 1993 paper.

Twelve labels mark the ends of six edges. One operation crosses each edge; the other rotates the ends meeting at a vertex. In this paper's labeling:

| Operation | Cycles; omitted labels stay fixed |
| --- | --- |
| Edge crossing α | (1 2)(3 4)(5 6)(7 8)(9 10)(11 12) |
| Vertex rotation σ | (1 6 2)(4 11 8)(5 7 9) |
| Face traversal α σ⁻¹ | (1 6 9 10 7 11 12 4 3 8 5) |

The paper multiplies left to right. It identifies the generated degree-12 group as M₁₂, of order 95,040. This cartographic action differs from the group of symmetries of the drawn graph.

## 2. A catalogue across all five Mathieu groups

Nicolas Hanusse and Alexander Zvonkin, *Cartographic generation of Mathieu groups*, **FPSAC 1999**, Barcelona, pp. 241–253; manuscript dated 6 April 1999.

- [Author-hosted PDF](https://www.labri.fr/perso/zvonkin/Research/mathieu.pdf).
- [Conference archive PDF](https://fpsac-archive.github.io/FPSAC99/ARTICLES/25Hanusse.pdf), used for the reading notes.
- Read §2 for constellations and passports, §3 for generation/verification, and §4 for the five groups.

Its opening “mouse” map is a **different** M₁₂ generating example. Example 1, printed p. 243, gives:

| Operation | Cycles; omitted labels stay fixed |
| --- | --- |
| Vertex rotation g₁ | (1 2 8 6 9 10)(3 12)(4 5 7) |
| Edge crossing g₂ | (1 2)(3 4)(5 6)(7 8)(9 10)(11 12) |
| Face traversal g₃ = (g₁g₂)⁻¹ | (2 10 6 4 12 11 3 7)(5 8) |

The refined passport is [6B, 2A, 8B]. The paper also supplies planar M₁₂ maps and M₂₄ examples. Its M₂₄ catalogue is explicitly incomplete at **130 or 131** essentially different maps; do not report 130 as a proved exhaustive count.

## 3. NeverEndingBooks: access remains unresolved

Lieven Le Bruyn, [*Monsieur Mathieu*](https://www.neverendingbooks.org/monsieur-mathieu). This is the blog reference in the user's post. Retrieval on 21 October 2026 returned gateway/time-out errors. **The article itself was not read.** Its attribution and connection to Malle's dessin are independently corroborated by the following paper, not reconstructed from the unavailable blog.

## 4. Exact Belyi functions behind the pictures

Nikolai M. Adrianov and Alexander M. Vatuzov, *Belyi functions of the special weighted trees of (2,3)-type*, submitted 30 December 2023, [arXiv:2401.00183v1](https://arxiv.org/abs/2401.00183v1), [full HTML](https://arxiv.org/html/2401.00183v1).

Locate **orbit 12.5** by its identifier: M₁₂ with passport (3³1³ | 2⁶ | 11¹1¹). It credits Malle's 1993 talk, Zvonkin's popularization and Le Bruyn's name. The two dessins have explicit Belyi functions with ω = (1 ± √−11) ÷ 2. **Orbit 24.1** gives an M₂₄ pair with passport (3⁶1⁶ | 2¹² | 23¹1¹) over ℚ(√−23). The introductory “alien” is another M₂₄ dessin, with face partition 21¹3¹; keep it separate. HTML/PDF figure numbers differ, so use orbit identifiers.

## Application boundary

These sources suggest a view in which dancers occupy **edge ends**, with edge crossing and vertex rotation as distinct moves. That is a proposed application use, not an implemented feature.

The tables preserve their respective source coordinates. Neither the post's numbered drawing nor either paper's numbering is an established ATLAS labeling. Before using these as application generators, independently verify the exact group and a coordinate bijection, preserve cyclic order at vertices, and document composition and face traversal. A cycle-type match alone does not establish that bijection. Map geometry and a dancer's interpolated path are separate choices.

This change is source/documentation authoring under [FP #241](https://github.com/isomorphisms/flexible-pipes/issues/241). It provides no application, build, dispatch, independent AI-CI, merge or device acceptance.
