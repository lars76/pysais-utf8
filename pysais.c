#include <Python.h>
#include "sais.h"

static PyObject *sa_str(PyObject *self, PyObject *args);
static PyObject *sa_utf8(PyObject *self, PyObject *args);
static PyObject *bwt_str(PyObject *self, PyObject *args);
static PyObject *bwt_utf8(PyObject *self, PyObject *args);
static PyObject *lcp_str(PyObject *self, PyObject *args);
static PyObject *lcp_utf8(PyObject *self, PyObject *args);

static PyMethodDef module_methods[] = {
    {"sa_str", sa_str, METH_VARARGS, "Suffix array from a string where each char is between 0 and 255."},
    {"sa_utf8", sa_utf8, METH_VARARGS, "Suffix array from a UTF-8 string."},
    {"bwt_str", bwt_str, METH_VARARGS, "BWT from a string where each char is between 0 and 255."},
    {"bwt_utf8", bwt_utf8, METH_VARARGS, "BWT from a UTF-8 string."},
    {"lcp_str", lcp_str, METH_VARARGS, "LCP from a string where each char is between 0 and 255."},
    {"lcp_utf8", lcp_utf8, METH_VARARGS, "LCP from a UTF-8 string."},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC PyInit_sais(void) {
    
    PyObject *module;
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "sais",
        "This module provides a Python binding for sais-lite and adds UTF-8 support.",
        -1,
        module_methods,
        NULL,
        NULL,
        NULL,
        NULL
    };
    module = PyModule_Create(&moduledef);
    if (!module) return NULL;

    return module;
}

PyObject *create_list(const void *arr, const Py_ssize_t len) {
    const int *cast = (int*)arr;
    PyObject *item;
    PyObject *pylist = PyList_New(len);
    for (int i = 0; i < len; i++) {
      item = PyLong_FromLong(cast[i]);
      PyList_SetItem(pylist, i, item);
    }

    return pylist;
}

unsigned int strlen_utf8(const char *str) {
    unsigned int i = 0, len = 0;
    while (str[i]) {
        if ((str[i] & 0xc0) != 0x80)
            len++;
        i++;
    }
    return len;
}

unsigned int to_utf8_cp(int *A, const char *str) {
    int m = 0;
    int k = 0;
    int i = 0;
    while (str[i]) {
      if ((str[i] & 0b10000000) == 0) {
        // 1 byte code point (ASCII)
        A[k] = str[i];
      }
      else if ((str[i] & 0b11100000) == 0b11000000) {
        // 2 byte code point
        A[k] = ((str[i] & 0x1f) << 6) | ((str[i+1] & 0x3f) << 0);
        i++;
      }
      else if ((str[i] & 0b11110000) == 0b11100000) {
        // 3 byte code point
        A[k] = ((str[i] & 0x0f) << 12) | ((str[i+1] & 0x3f) << 6) | ((str[i+2] & 0x3f) << 0);
        i += 2;
      }
      else {
        // 4 byte code point
        A[k] = ((str[i] & 0x07) << 18) | ((str[i+1] & 0x3f) << 12) | ((str[i+2] & 0x3f) << 6) | ((str[i+3] & 0x3f) << 0);
        i += 3;
      }
      m = A[k] > m ? A[k] : m; 
      k++;
      i++;
    }

    // returns max value in array
    return m;
}

// no generics in C...
int compute_lcp_int(const int *T, const int *SA, int *LCP, const int n) {
    int h = 0, i, j, k;
    int *rank = (int *) malloc(n * sizeof(int));
     
    for (i = 0; i < n; i++) {
      rank[SA[i]] = i;
    }

    for (i = 0; i < n; i++) {
        k = rank[i];
        if (k == 0) {
            LCP[k] = -1;
        } else {
            j = SA[k-1];
            while ((i - h < n) && (j + h < n) && (T[i+h] == T[j+h])) { ++h; }
            LCP[k] = h;
        }
        if (h > 0) --h;
    }

    free(rank);
    return 0;
}

int compute_lcp_char(const char *T, const int *SA, int *LCP, const int n) {
    int h = 0, i, j, k;
    int *rank = (int *) malloc(n * sizeof(int));
     
    for (i = 0; i < n; i++) {
      rank[SA[i]] = i;
    }

    for (i = 0; i < n; i++) {
        k = rank[i];
        if (k == 0) {
            LCP[k] = -1;
        } else {
            j = SA[k-1];
            while ((i - h < n) && (j + h < n) && (T[i+h] == T[j+h])) { ++h; }
            LCP[k] = h;
        }
        if (h > 0) --h;
    }

    free(rank);
    return 0;
}

PyObject *lcp(PyObject *self, PyObject *args, const int utf8) {
    /* no need to free */
    const char *text;
    PyObject *list;

    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "sO", &text, &list)) {
        PyErr_BadArgument();
        return NULL;
    }

    Py_ssize_t len = PyList_Size(list);

    if (strlen_utf8(text) != (long unsigned int)len) {
        PyErr_SetString(PyExc_TypeError, "both parameters must be of the same length.");
        return NULL;
    }

    PyObject *item;
    int *sa = malloc(len * sizeof(int));
    for (int i = 0; i < len; i++) {
        item = PyList_GetItem(list, i);
        if (!PyLong_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "list items must be integers.");
            free(sa);
            return NULL;
        }
        sa[i] = PyLong_AsLong(item);
    }

    /* Allocate array */
    int *lcp = malloc(len * sizeof(int));
    if (lcp == NULL) {
        free(sa);
        return PyErr_NoMemory();
    }

    if (utf8) {
        int *A = malloc(len * sizeof(int));
        if (A == NULL) {
            free(sa);
            return PyErr_NoMemory();
        }

        to_utf8_cp(A, text);
        compute_lcp_int(A, sa, lcp, len);
        free(A);
    }
    else
        compute_lcp_char(text, sa, lcp, len);

    free(sa);

    PyObject *pylist = create_list(lcp, len);

    free(lcp);

    return pylist;
}

static PyObject *lcp_utf8(PyObject *self, PyObject *args) {
    return lcp(self, args, 1);
}

static PyObject *lcp_str(PyObject *self, PyObject *args) {
    return lcp(self, args, 0);
}

static PyObject *bwt(PyObject *self, PyObject *args, const int utf8) {
    /* no need to free */
    const char *text;

    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "s", &text)) {
        PyErr_BadArgument();
        return NULL;
    }

    const int len = strlen_utf8(text);

    /* Allocate arrays */
    int *sa = malloc(len * sizeof(int));
    if (sa == NULL)
        return PyErr_NoMemory();

    int *bwt = malloc(len * sizeof(int));
    if (bwt == NULL) {
        free(sa);
        return PyErr_NoMemory();
    }

    if (utf8) {
        int *A = malloc(len * sizeof(int));
        if (A == NULL) {
            free(sa);
            return PyErr_NoMemory();
        }

        const unsigned int m = to_utf8_cp(A, text);

        if (sais_int_bwt(A, bwt, sa, len, m+1) < 0) {
            free(bwt);
            free(sa);
            free(A);
            PyErr_SetString(PyExc_ValueError, "some error occurred.");
            return NULL;
        }
        free(A);
    }
    else {
        if (sais_bwt((unsigned char*)text, bwt, sa, len) < 0) {
            free(sa);
            free(bwt);
            PyErr_SetString(PyExc_ValueError, "some error occurred.");
            return NULL;
        }
    }

    free(sa);

    PyObject *pylist = create_list(bwt, len);

    free(bwt);

    return pylist;
}

static PyObject *bwt_utf8(PyObject *self, PyObject *args) {
    return bwt(self, args, 1);
}

static PyObject *bwt_str(PyObject *self, PyObject *args) {
    return bwt(self, args, 0);
}

static PyObject *sa(PyObject *self, PyObject *args, const int utf8) {
    /* no need to free */
    const char *text;

    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "s", &text)) {
        PyErr_BadArgument();
        return NULL;
    }

    const int len = strlen_utf8(text);

    /* Allocate arrays */
    int *sa = malloc(len * sizeof(int));
    if (sa == NULL)
        return PyErr_NoMemory();

    if (utf8) {
        int *A = malloc(len * sizeof(int));
        if (A == NULL) {
            free(sa);
            return PyErr_NoMemory();
        }

        const unsigned int m = to_utf8_cp(A, text);

        if (sais_int(A, sa, len, m+1) != 0) {
            free(sa);
            free(A);
            PyErr_SetString(PyExc_ValueError, "some error occurred.");
            return NULL;
        }
        free(A);
    }
    else {
        if (sais((unsigned char*)text, sa, len) != 0) {
            free(sa);
            PyErr_SetString(PyExc_ValueError, "some error occurred.");
            return NULL;
        }
    }

    PyObject *pylist = create_list(sa, len);

    free(sa);

    return pylist;
}

static PyObject *sa_utf8(PyObject *self, PyObject *args) {
    return sa(self, args, 1);
}

static PyObject *sa_str(PyObject *self, PyObject *args) {
    return sa(self, args, 0);
}