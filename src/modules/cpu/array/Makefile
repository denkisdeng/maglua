# Copyright (C) 2011 Jason Mercer.  All rights reserved.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#


OBJECTS=array_luafuncs.lo array.o memory.o array_ops.o fourier.o  \
	array_core_cpu.o array_core_matrix_cpu.o \
	array_core_matrix_cpu_lapack.o


LIBNAME=Array
CATEGORY=cpu

#DEPEND is parsed to make the vs2010 project
DEPEND=luabaseobject
BASEDIR=../../..
EXTRA_PKG=fftw3f lapacke lapack blas
CC_EXTRAOPS=-D_LAPACK -D_MATRIX
# CC_EXTRAOPS=-D_MATRIX

EXTRA_CFLAGS=-lgfortran -lblas  -llapack
EXTRA_CLEAN=array_luafuncs.cpp array_luafuncs.h
include $(BASEDIR)/makefile.common.cpu
