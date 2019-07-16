from unittest import TestCase

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


class TestSais(TestCase):
    def test_sa_simple(self):
        self.assertEqual(sa_utf8("banana$"), [6, 5, 3, 1, 0, 4, 2])
        self.assertEqual(sa_str("banana$"), [6, 5, 3, 1, 0, 4, 2])
        self.assertEqual(sa_str("banana$"), sa_utf8("banana$"))

    def test_bwt_simple(self):
        self.assertEqual(bwt_str("banana$"), bwt_utf8("banana$"))

    def test_lcp_simple(self):
        sa = sa_utf8("banana$")
        sa2 = sa_str("banana$")
        self.assertEqual(lcp_str("banana$", sa), [-1, 0, 1, 3, 0, 0, 2])
        self.assertEqual(lcp_str("banana$", sa2), [-1, 0, 1, 3, 0, 0, 2])
        self.assertEqual(lcp_utf8("banana$", sa), [-1, 0, 1, 3, 0, 0, 2])
        self.assertEqual(lcp_utf8("banana$", sa2), [-1, 0, 1, 3, 0, 0, 2])

    def test_unicode(self):
        t = "此数据结构被运用于全文索引、数据压缩算法、以及生物信息学。"
        sa = sa_utf8(t)
        matches = sorted(find_all_matches(sa, t, "数"))

        self.assertTrue(matches == [1, 14])
        self.assertTrue(t[matches[0]] == "数")
        self.assertTrue(t[matches[1]] == "数")

        t2 = "bänänä"
        t3 = "banana"
        sa2 = sa_utf8(t2)
        sa3 = sa_str(t3)

        self.assertTrue(len(sa2) == len(sa3))

        matches2 = sorted(find_all_matches(sa2, t2, "ä"))
        matches3 = sorted(find_all_matches(sa3, t3, "a"))
        self.assertEqual(matches2, matches3)