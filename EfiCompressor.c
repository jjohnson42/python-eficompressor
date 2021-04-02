/** @file

Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available 
under the terms and conditions of the BSD License which accompanies this 
distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Python.h>
#include <Compress.h>
#include <Decompress.h>

/*
 UefiDecompress(data_buffer, size, original_size)
*/
STATIC
PyObject*
UefiDecompress(
  PyObject    *Self,
  PyObject    *Args
  )
{
  PyObject      *SrcData;
  UINT32        SrcDataSize;
  UINT32        DstDataSize;
  UINTN         Status;
  UINT8         *DstBuf;
  Py_buffer     PyB;


  Status = PyArg_ParseTuple(
            Args,
            "Oi",
            &SrcData,
            &SrcDataSize
            );
  if (Status == 0) {
    return NULL;
  }

  if (PyObject_CheckBuffer(SrcData) == 0) {
    PyErr_SetString(PyExc_Exception, "First argument is not a buffer\n");
    return NULL;
  }

  if (PyObject_GetBuffer(SrcData, &PyB, PyBUF_SIMPLE) < 0) {
    PyErr_SetString(PyExc_Exception, "Failed getting buffer for first argument\n");
    return NULL;
  }

  Status = Extract(PyB.buf, PyB.len, (VOID **)&DstBuf, &DstDataSize, 1);
  PyBuffer_Release(&PyB);
  if (Status != EFI_SUCCESS) {
    PyErr_SetString(PyExc_Exception, "Failed to decompress\n");
    goto ERROR;
  }
#if PY_MAJOR_VERSION >= 3
  return PyMemoryView_FromMemory((char*)DstBuf, (Py_ssize_t)DstDataSize, PyBUF_READ);
#else
  return PyBuffer_FromMemory(DstBuf, (Py_ssize_t)DstDataSize);
#endif
ERROR:
  if (DstBuf != NULL) {
    free(DstBuf);
  }
  return NULL;
}


STATIC
PyObject*
FrameworkDecompress(
  PyObject    *Self,
  PyObject    *Args
  )
{
  PyObject      *SrcData;
  UINT32        SrcDataSize;
  UINT32        DstDataSize;
  UINTN         Status;
  UINT8         *DstBuf;
  PyObject*     ret;
  Py_buffer     PyB;

  Status = PyArg_ParseTuple(
            Args,
            "Oi",
            &SrcData,
            &SrcDataSize
            );
  if (Status == 0) {
    return NULL;
  }

  if (PyObject_CheckBuffer(SrcData) == 0) {
    PyErr_SetString(PyExc_Exception, "First argument is not a buffer\n");
    return NULL;
  }

  // Because some Python objects which support "buffer" protocol have more than one
  // memory segment, we have to copy them into a contiguous memory.
  PyObject_GetBuffer(SrcData, &PyB, PyBUF_SIMPLE);

  Status = Extract(PyB.buf, PyB.len, (VOID **)&DstBuf, &DstDataSize, 2);
  PyBuffer_Release(&PyB);
  if (Status != EFI_SUCCESS) {
    PyErr_SetString(PyExc_Exception, "Failed to decompress\n");
    goto ERROR;
  }

  ret = PyBytes_FromStringAndSize((const char*)DstBuf, (Py_ssize_t)DstDataSize);
  free(DstBuf);
  return ret;

ERROR:

  if (DstBuf != NULL) {
    free(DstBuf);
  }
  return NULL;
}


STATIC
PyObject*
UefiCompress(
  PyObject    *Self,
  PyObject    *Args
  )
{
  PyObject      *SrcData;
  UINT32        SrcDataSize;
  UINT32        DstDataSize;
  UINTN         Status;
  UINT8         *DstBuf;
  Py_buffer     PyB;

  Status = PyArg_ParseTuple(
            Args,
            "Oi",
            &SrcData,
            &SrcDataSize
            );
  if (Status == 0) {
    return NULL;
  }

  if (PyObject_CheckBuffer(SrcData) == 0) {
    PyErr_SetString(PyExc_Exception, "First argument is not a buffer\n");
    return NULL;
  }

  if (PyObject_GetBuffer(SrcData, &PyB, PyBUF_SIMPLE) < 0) {
    PyErr_SetString(PyExc_Exception, "Failed getting buffer for first argument\n");
    return NULL;
  }

  DstDataSize = SrcDataSize;
  DstBuf = PyMem_Malloc(DstDataSize);

  Status = EfiCompress(PyB.buf, PyB.len, (VOID *)DstBuf, &DstDataSize);
  PyBuffer_Release(&PyB);
  if (Status != EFI_SUCCESS) {
    PyErr_SetString(PyExc_Exception, "Failed to compress\n");
    goto ERROR;
  }

#if PY_MAJOR_VERSION >= 3
  return PyMemoryView_FromMemory((char*)DstBuf, (Py_ssize_t)DstDataSize, PyBUF_READ);
#else
  return PyBuffer_FromMemory(DstBuf, (Py_ssize_t)DstDataSize);
#endif

ERROR:

  if (DstBuf != NULL) {
    free(DstBuf);
  }
  return NULL;
}


STATIC
PyObject*
FrameworkCompress(
  PyObject    *Self,
  PyObject    *Args
  )
{
  PyObject      *SrcData;
  UINT32        SrcDataSize;
  UINT32        DstDataSize;
  UINTN         Status;
  UINT8         *DstBuf;
  PyObject      *ret;
  Py_buffer     PyB;

  Status = PyArg_ParseTuple(
            Args,
            "Oi",
            &SrcData,
            &SrcDataSize
            );
  if (Status == 0) {
    return NULL;
  }

  if (PyObject_CheckBuffer(SrcData) == 0) {
    PyErr_SetString(PyExc_Exception, "First argument is not a buffer\n");
    return NULL;
  }

  if (PyObject_GetBuffer(SrcData, &PyB, PyBUF_SIMPLE) < 0) {
    PyErr_SetString(PyExc_Exception, "Failed getting buffer for first argument\n");
    return NULL;
  }

  DstDataSize = SrcDataSize;
  DstBuf = PyMem_Malloc(DstDataSize);
  if (DstBuf == NULL) {
    PyErr_SetString(PyExc_Exception, "Not enough memory\n");
    goto ERROR;
  }

  Status = TianoCompress(PyB.buf, PyB.len, (VOID *)DstBuf, &DstDataSize);
  PyBuffer_Release(&PyB);
  if (Status != EFI_SUCCESS) {
    PyErr_SetString(PyExc_Exception, "Failed to compress\n");
    goto ERROR;
  }

  ret = PyBytes_FromStringAndSize((const char*)DstBuf, (Py_ssize_t)DstDataSize);
  PyMem_Free(DstBuf);
  return ret;

ERROR:
  if (DstBuf != NULL) {
    free(DstBuf);
  }
  return NULL;
}

const char DecompressDocs[] = "Decompress(): Decompress data using UEFI standard algorithm\n";
const char CompressDocs[] = "Compress(): Compress data using UEFI standard algorithm\n";

STATIC PyMethodDef EfiCompressor_Funcs[] = {
  {"UefiDecompress", (PyCFunction)UefiDecompress, METH_VARARGS, DecompressDocs},
  {"UefiCompress", (PyCFunction)UefiCompress, METH_VARARGS, CompressDocs},
  {"FrameworkDecompress", (PyCFunction)FrameworkDecompress, METH_VARARGS, DecompressDocs},
  {"FrameworkCompress", (PyCFunction)FrameworkCompress, METH_VARARGS, CompressDocs},
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
#if PY_MAJOR_VERSION >= 3
PyInit_EfiCompressor(VOID) {
  static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "EfiCompressor",
        NULL,
        0,
        EfiCompressor_Funcs,
        NULL,
        NULL,
        NULL,
        NULL
  };
  return PyModule_Create(&moduledef);
#else
initEfiCompressor(VOID) {
  Py_InitModule3("EfiCompressor", EfiCompressor_Funcs, "EFI Compression Algorithm Extension Module");
#endif
}


