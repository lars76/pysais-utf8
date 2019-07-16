# pysais-utf8

pysais-utf8 is a Python 3 C module for creating suffix arrays (SA). The input text can be either ASCII (unsigned char) or UTF-8.

Internally, pysais-utf8 uses the library [sais-lite](https://sites.google.com/site/yuta256/sais). There is also another SA construction library called [libdivsufsort](https://github.com/y-256/libdivsufsort), which is a bit faster. However, the performance differences are negligible for most applications. The main advantage of sais-lite is that the library is quite small (500 LOC, one file) and can easily be extended.

The UTF-8 support is not provided by modifying `sais.c`, but rather by turning the input text into its code point representation.

# Setup

pysais-utf8 was tested with Python 3.7.3 and gcc 9.1.0 (Linux).

To build and test:

1. `python3 setup.py build_ext --inplace`
2. `python3 -m unittest test_pysais`

To install:

1. `python3 setup.py install --user`

# API

## Functions

`sa_str(text)`: Returns the suffix array from a string where each char is between 0 and 255.

`sa_utf8(text)`: Returns the suffix array from a UTF-8 string.

`bwt_str(text)`: Returns the Burrows-Wheeler Transform from a string where each char is between 0 and 255.

`bwt_utf8(text)`: Returns the Burrows-Wheeler Transform from a UTF-8 string.

`lcp_str(text, suff)`: Returns the longest common prefix (LCP) array from a string where each char is between 0 and 255.

`lcp_utf8(text, suff)`: Returns the longest common prefix (LCP) from a UTF-8 string.

## Example

```python
from sais import *
import operator

def find_all_matches(suffix_arr, text, query):
    def binary_search(lo, hi, op):
        m = len(query)
        while lo < hi:
            mid = (lo + hi) // 2
            suffix = suffix_arr[mid]
            if op(text[suffix:suffix+m], query):
                lo = mid + 1
            else:
                hi = mid

        return lo

    n = len(suffix_arr)
    start = binary_search(0, n, operator.lt)
    end = binary_search(start, n, operator.eq)

    return suffix_arr[start:end]

sa = sa_str("banana$") # [6, 5, 3, 1, 0, 4, 2]
pos = find_all_matches(sa, "banana$", "a") # [1, 3, 5]

text = "此数据结构被运用于全文索引、数据压缩算法、以及生物信息学。"
sa = sa_utf8(text)
pos = find_all_matches(sa, text, "数") # [1,14]
```
