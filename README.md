# Various LMC things I am messing about with

This contains an assembler and runtime for LMC code, as well as a dereferencer that will let you convert LMC assembly which uses labels as `DAT` arguments into functioning code, as well as adding the `P_BR` macro which expands as follows:

```lmc
LDA xxx
ADD 600
STO here
here DAT
```