# Various LMC things I am messing about with

This contains an assembler and runtime for LMC code, as well as a pre-processor (see below)

# Pre-processor (`dereferencer`)
This program will pre-process LMC files with the following changes made:

- `DAT` instructions can take labels as arguments, which means the value in that mailbox is the address of the label
- `DDAT` instructions can be used in code and will delay the `DAT` they declare till the end of the file; this means variables can be declared at the top of functions
- `P_BR <label>` is a macro that expands as follows:
  ```
           LDA <label>
           ADD BR_NUM
           STO __lat<m>
  __lat<m> DAT
  ```
  i.e. expands to a `BR` instruction with its argument being the mailbox address found at position `<label>`
- `OP &<label>` will run `OP` with the argument being the mailbox of `<label>`


