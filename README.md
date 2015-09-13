# sx

C++ utilities, includes components like:

- array_view, initially started from Łukasz Mendakiewicz's C++ proposal [N4512](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4512.html) but since then I've rewritten it almost completely based on
  + the comments and critiques on the [Google ISO C++ forum](https://groups.google.com/a/isocpp.org/forum/#!forum/std-proposals)
  + [boost::multi_array](http://www.boost.org/doc/libs/1_59_0/libs/multi_array/doc/index.html)
  + [N4222](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4222.pdf)
  + Eric Niebler's [A Slice of Python in C++](http://ericniebler.com/2014/12/07/a-slice-of-python-in-c/) article
  + and my taste, experience with other languages (MatLab, K, Q, Julia)
- multi_array, which is the container version of the array_view
- STL abbreviations and helper macros (abbrev.h)
- random_access_iterator_pair (lightweight, OutputIterator version of the zip feature from boost::range or Niebler's range-v3, works for simultaneously sorting two containers
- Implementation of Python's `range` (for C++ range-based loops)
- `sort`, `sortperm` (like in MatLab/Julia) for array_view
- `static_const`, Niebler's customization point solution, copied from his range-v3 lib

This is an unstable work in progress, developing while porting scikit-learn's Random Forest
