# Reuse the qualified Android assembler boundary

Run41964562298 failed before consumer compilation: its inherited compiler
provider emitted `.base64` in the literal-glyph qualification assembly, which
the declared NDK27 x86_64 assembler rejects. Run41964377648 is the preceding
same-change source, not current acceptance.

Reuse ai-ci source `f6d825d15cd3c0c34ae1b43240a463ff5d343090`, already qualified by
Pauli's Android compiler repair. It selects the same named ICK frontend with
source `fbe86e23d55cfec2000c08e61deea2a407fd7175`, records
`gcc_cv_as_base64=no` in the assembler capability, and qualifies the actual
emitted assembly. Metadata-bound archives are created and restored through
that owner's matching interface. The NDK27 target profile remains unchanged.

Literal assignment/division and all positive/negative compiler checks remain.
There is no source rewriting, NDK first-party C compilation, stock compiler or
post-processing substitute. The existing host provider already selects this
ICK source; fresh Android and application evidence is required for the repaired
boundary.

At source 5a405cfbe3af93bd766d45c84b6e7d5fb801c6e3, run41967298674 passes the
host suite and all three Android compiler qualifications. Actual ARMv7 app C
compilation/linking then passes; packaging fails because the current shared
packager requires ANDROID_GREASE explicitly. The producer now forwards its
already verified canonical GREASE path under that existing interface. No
alternate runtime or disabled receipt verification is introduced. Fresh package
and Android evidence remain required.
