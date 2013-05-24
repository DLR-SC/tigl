/*===========================================================================*
* common.c
* Utility functions commonly used by MEX source files
*
* Copyright 2008 Levente Hunyadi
* All rights reserved.
*===========================================================================*/

/* This file is not intended to be compiled directly. Instead, include it in
 * other MEX source files using the #include preprocessor directive:
 *
 *    #include "common.c"
 */

/* MEX functions throw MatLab exceptions with the following scheme:
 *
 *    package_name:function_name:exception_type
 *
 * where
 *
 * - package_name  is the package that contains the function, e.g. math
 *
 * - function_name  is the source file name or, equivalently, the identifier
 *   with which the function is invoked from MatLab
 *
 * - exception_type  is one of
 *   - ArgumentCountMismatch  if too many or too few input or output arguments
 *     are given
 *   - ArgumentTypeMismatch  if the arguments are not of the expected type
 *   - DimensionMismatch  if arguments are not of the expected dimensions
 *   - InvalidValue  if any argument is of or contains an inappropriate value
 *   - OutOfMemory
 */

#include "mex.h"

#ifndef __PACKAGENAME__
// define a generic package name if macro is not defined in including source file
#define __PACKAGENAME__ "TIXI"
#endif

#define __DIMENSIONMISMATCH__  __PACKAGENAME__ ":DimensionMismatch"
#define __ARGCOUNTMISMATCH__ __PACKAGENAME__ ":ArgumentCountMismatch"
#define __ARGTYPEMISMATCH__ __PACKAGENAME__ ":ArgumentTypeMismatch"
#define __INVALIDOPERATION__ __PACKAGENAME__ ":InvalidOperation"
#define __INVALIDVALUE__ __PACKAGENAME__ ":InvalidValue"
#define __OUTOFMEMORY__ __PACKAGENAME__ ":OutOfMemory"

#if defined(NAN_EQUALS_ZERO)
#define isnonzero(d) ((d)!=0.0 || mxIsNaN(d))
#else
#define isnonzero(d) ((d)!=0.0)
#endif

#ifdef __cplusplus
#define __INLINE__ inline
#else
#define __INLINE__
#ifndef bool
#define bool int
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif

#define isinteger(n) ( (int)(n) == (n) )

/**
* Test whether array is two-dimensional.
*/
__INLINE__ bool is2d(const mxArray* arr) {
	mwSize d = mxGetNumberOfDimensions(arr);

	return d == 2;
}

__INLINE__ bool is3d(const mxArray* arr) {
	return mxGetNumberOfDimensions(arr) == 3;
}

/**
* Test whether array is a square matrix.
*/
__INLINE__ bool issquare(const mxArray* arr) {
	mwSize m = mxGetM(arr);
	mwSize n = mxGetN(arr);

	return m == n;
}

/**
* Test whether array is a vector shape.
*/
__INLINE__ bool isvector(const mxArray* arr) {
	mwSize m = mxGetM(arr);
	mwSize n = mxGetN(arr);

	return m == 1 || n == 1;
}

/**
* Test whether array represents a scalar.
*/
__INLINE__ bool isscalar(const mxArray* arr) {
	mwSize m = mxGetM(arr);
	mwSize n = mxGetN(arr);

	return m == 1 && n == 1;
}

/**
* Test whether array is empty.
*/
__INLINE__ bool isnonempty(const mxArray* arr) {
	mwSize m = mxGetM(arr);
	mwSize n = mxGetN(arr);

	return m*n > 0;  /* safe for sparse arrays */
}

/**
* Number of elements in array.
*/
__INLINE__ mwSize numel(const mxArray* arr) {
	if (mxIsSparse(arr)) {
		mwSize m = mxGetM(arr);
		mwSize n = mxGetN(arr);
		return m*n;
	} else {
		return mxGetNumberOfElements(arr);
	}
}

/**
* Number of nonzero elements in numeric array.
*/
mwSize nonzerocount(const mxArray* arr) {
	if (mxIsCell(arr)) {
		mwSize n = mxGetNumberOfElements(arr);
		mwSize c = 0;
		mwIndex ix;

		for (ix = 0; ix < n; ix++) {
			const mxArray* cell = mxGetCell(arr, ix);
			if (cell != NULL) {
				c += nonzerocount(cell);
			}
		}
		return c;
	} else if (mxIsStruct(arr)) {
		mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Structure arrays are not supported.");
		return 0;
	} else if (mxIsSparse(arr)) {
		mwIndex* jc = mxGetJc(arr);
		mwSize n = mxGetN(arr);  /* number of columns */
		return jc[n];  /* number of nonzero elements */
	} else {  /* filter zero elements */
		mwSize n = mxGetNumberOfElements(arr);
		mwSize c = 0;
		const double* pr = mxGetPr(arr);
		const double* pi = mxGetPi(arr);
		mwSize i;

		if (pr == NULL) {
			mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Operation supported only on numeric arrays.");
		}

		if (pi != NULL) {  /* real and imaginary part */
			for (i = 0; i < n; i++) {
				if (isnonzero(*(pr++)) || isnonzero(*(pi++))) {
					c++;
				}
			}
		} else {  /* real part only */
			for (i = 0; i < n; i++) {
				if (isnonzero(*(pr++))) {
					c++;
				}
			}
		}
		return c;
	}
}

/**
* Test if any element in an array is a complex number.
*/
bool isanycomplex(const mxArray* arr) {
	if (mxIsCell(arr)) {
		mwIndex n = mxGetNumberOfElements(arr);
		mwIndex ix;

		for (ix = 0; ix < n; ix++) {
			const mxArray* cell = mxGetCell(arr, ix);
			if (cell != NULL && isanycomplex(cell)) {
				return true;
			}
		}
		return false;
	} else if (mxIsStruct(arr)) {
		mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Structure arrays are not supported.");
		return false;
	} else {
		return mxIsComplex(arr);
	}
}

/**
* Test if all elements in an array can be interpreted as integers.
*/
bool isallinteger(const mxArray* arr) {
	if (mxIsCell(arr)) {
		mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Cell arrays are not supported.");
		return false;
	} else if (mxIsStruct(arr)) {
		mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Structure arrays are not supported.");
        return false;
	} else if (mxIsSparse(arr)) {
		mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Sparse integer arrays are not supported.");
		return false;
	} else {
		mwSize n = mxGetNumberOfElements(arr);
		const double* pr = mxGetPr(arr);
		const double* pi = mxGetPi(arr);
		mwSize i;

		if (pr == NULL) {
			mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Operation supported only on numeric arrays.");
			return false;
		}

		if (pi != NULL) {  /* real and imaginary part */
			for (i = 0; i < n; i++) {
				if (!isinteger(*(pr++)) || !isinteger(*(pi++))) {
					return false;
				}
			}
		} else {  /* real part only */
			for (i = 0; i < n; i++) {
				if (!isinteger(*(pr++))) {
					return false;
				}
			}
		}
		return true;
	}
}

/**
* Prints a matrix M to the MatLab standard output.
*/
void printmatrix(double* M, int rows, int cols) {
	mxArray* matrix = mxCreateDoubleMatrix(rows, cols, mxREAL);	
	double* matrixcell = mxGetPr(matrix);
	int i;
	/* memcpy(matrixcells, M, rows*cols*sizeof(double)); */
	for (i = 0; i < rows*cols; i++) {
		*(matrixcell++) = *(M++);
	}
	mexCallMATLAB(0, NULL, 1, &matrix, "disp");
}

#define nargineqchk(nsup, nreq) \
    if (nsup != nreq) { mexErrMsgIdAndTxt(__ARGCOUNTMISMATCH__, "Exactly %d input argument(s) required, %d given.", nreq, nsup); }

#define nargouteqchk(nsup, nreq) \
    if (nsup != nreq) { mexErrMsgIdAndTxt(__ARGCOUNTMISMATCH__, "Exactly %d output argument(s) required, %d given.", nreq, nsup); }

#define narginltechk(nsup, nreq) \
    if (nsup > nreq) { mexErrMsgIdAndTxt(__ARGCOUNTMISMATCH__, "At most %d input argument(s) required, %d given.", nreq, nsup); }

#define nargoutltechk(nsup, nreq) \
    if (nsup > nreq) { mexErrMsgIdAndTxt(__ARGCOUNTMISMATCH__, "Too many output arguments, %d required, %d given.", nreq, nsup); }

#define narginrangechk(nsup, nlower, nupper) \
    if (nsup > nupper || nsup < nlower) { mexErrMsgIdAndTxt(__ARGCOUNTMISMATCH__, "At least %d and at most %d input argument(s) required, %d given.", nlower, nupper, nsup); }

#define arg2dchk(prhs, ix) \
	if ( !is2d(prhs[ix]) ) { mexErrMsgIdAndTxt(__DIMENSIONMISMATCH__, "Input argument %d must be two-dimensional.", ix + 1); }

#define arg3dchk(prhs, ix) \
	if ( !is3d(prhs[ix]) ) { mexErrMsgIdAndTxt(__DIMENSIONMISMATCH__, "Input argument %d must be three-dimensional.", ix + 1); }

#define argnonemptychk(prhs, ix) \
	if ( !isnonempty(prhs[ix]) ) { mexErrMsgIdAndTxt(__DIMENSIONMISMATCH__, "Input argument %d must be nonempty.", ix + 1); }

#define argdoublechk(prhs, ix) \
	if ( !mxIsDouble(prhs[ix]) ) { mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Input argument %d must be of type double.", ix + 1); }

#define argrealchk(prhs, ix) \
	if ( isanycomplex(prhs[ix]) ) { mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Input argument %d must be real.", ix + 1); }

#define argintegerchk(prhs, ix) \
	if ( !isallinteger(prhs[ix]) ) { mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Input argument %d must be integer.", ix + 1); }

#define argsquarechk(prhs, ix) \
	if ( !issquare(prhs[ix]) ) { mexErrMsgIdAndTxt(__DIMENSIONMISMATCH__, "Input argument %d must be a square matrix.", ix + 1); }

#define argvectorchk(prhs, ix) \
	if ( !isvector(prhs[ix]) ) { mexErrMsgIdAndTxt(__DIMENSIONMISMATCH__, "Input argument %d must be a vector.", ix + 1); }

#define argscalarchk(prhs, ix) \
	if ( !isscalar(prhs[ix]) ) { mexErrMsgIdAndTxt(__DIMENSIONMISMATCH__, "Input argument %d must be a scalar.", ix + 1); }

#define argcellchk(prhs, ix) \
	if ( !mxIsCell(prhs[ix]) ) { mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Input argument %d must be a cell array.", ix + 1); }

#define argscalarfunchk(prhs, ix) \
    if ( !isscalar(prhs[ix]) || mxGetClassID(prhs[ix]) != mxFUNCTION_CLASS ) { mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Input argument %d must be a scalar function handle.", ix + 1); }

#define argscalarstructchk(prhs, ix) \
    if ( !isscalar(prhs[ix]) || mxGetClassID(prhs[ix]) != mxSTRUCT_CLASS ) { mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Input argument %d must be a scalar structure.", ix + 1); }

#define argsparsechk(prhs, ix) \
    if ( !mxGetJc(prhs[ix]) || !mxGetIr(prhs[ix]) ) { mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Input argument %d must be sparse.", ix + 1); }

#define argfullchk(prhs, ix) \
    if ( mxGetJc(prhs[ix]) || mxGetIr(prhs[ix]) ) { mexErrMsgIdAndTxt(__ARGTYPEMISMATCH__, "Input argument %d must be full.", ix + 1); }
