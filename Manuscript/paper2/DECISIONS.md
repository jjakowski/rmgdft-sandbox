# Manuscript decisions

This record covers the decision made in this editing session. An existing project decision log was not available here; merge this entry into that log if one exists in the manuscript repository.

## 2026-09-24: Electromagnetic units and dimensional constants

Approved by the author: use Gaussian electromagnetic units in the theory section, retaining hbar, electron mass m_e, elementary charge magnitude e, and speed of light c explicitly. Define electron charge q_e = -e, with e > 0. Do not set these constants to numerical values during the derivations.

At the start of the implementation section, explicitly adopt Hartree atomic units: hbar = m_e = e = 1 and q_e = -1. Retain the Gaussian form of electromagnetic coupling, with c = 1/alpha approximately 137.036. Do not set c = 1 or silently absorb e/c into the vector potential. Any scaled code variable must be defined separately.

In the Gaussian convention, E_h = e^2/a_0 and t_0 = hbar/E_h. The familiar atomic-unit condition 4*pi*epsilon_0 = 1 belongs to the SI-based description of Hartree atomic units; epsilon_0 does not enter the Gaussian field equations used here.

Update the UNIT SYSTEM comments in notation.tex to match this decision. Macro definitions and other pending notation issues are outside this edit. The revised EM subsection already follows this unit convention.
