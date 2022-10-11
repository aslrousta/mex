# MeX

MeX (stands for _Macro eXpander_) is a macro processing tool highly inspired by [TeX](https://en.wikipedia.org/wiki/TeX) originally written by [Donald E. Knuth](https://en.wikipedia.org/wiki/Donald_Knuth).

What IT IS:

- To put it simply, a text transformation tool, somewhat like [M4](https://en.wikipedia.org/wiki/M4_(computer_language)) and [T4](https://en.wikipedia.org/wiki/Text_Template_Transformation_Toolkit), but with a syntax very similar to TeX.
- A fast macro processing tool with a minimal memory footprint.
- A work in progress, far from complete.

What IT'S NOT:

- A replacement for TeX or any other digital typesetting system; nor it's meant to be.
- A production-ready solution.

## TODO

- [x] Collapse consequent whitespaces.
- [ ] Skip line comments (starting with `#`).
- [ ] Clear shipped tokens from the input buffer.
- [ ] Use hash-table to speed up token look-up.

## License

MeX is published under GPL-3.0; see `LICENSE` for more details.
