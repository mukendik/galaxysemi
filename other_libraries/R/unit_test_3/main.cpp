#define USE_RINTERNALS
#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h> !
#include <vector>
#include <iostream>
extern "C" SEXP fool()
{
	int n=10 ;
	int nbytes = sizeof(SEXPREC_ALIGN) ;
	Rprintf( "a SEXPREC_ALIGN takes %d bytes\n", nbytes ) ;
	int nd = nbytes / sizeof(double) ;
	Rprintf( " .. i.e. the size of %d doubles\n", nd ) ;
	// R allocating a numeric vector of size 0
	SEXP x = PROTECT( Rf_allocVector( REALSXP, 0 ) ) ;
	// for the purpose of this proposal we will use a std::vector<double>
	// as the host, but it can be shared memory
	//
	// we allocate a std::vector<double> of size n + nd
	// - nd is the number of bytes that are used for the self description
	// of the object
	// - n is the actual number of double we want to let available to R
	std::vector<double> data( nd + n) ;
	// we grab a pointer to the beginning of the data and we copy
	// description bytes from x
	double* raw = &data[0] ;
	memcpy( raw, x, nbytes) ;
	// we reinterpret the raw memory as a SEXP, making R believe it is
	// an R object
	SEXP y = reinterpret_cast<SEXP>( raw ) ;
	// what R should consider the size of vector
	SETLENGTH(y, n) ;
	// filling some data : 1:10
	for( int i=0; i<10; i++)
	data[i+nd] = i+1 ;
	// evaluating an R expression on this object.
	// we make the call : sum(y) and evaluate it.
	SEXP call = PROTECT( Rf_lang2( Rf_install( "sum") , y) ) ;
	SEXP res = PROTECT( Rf_eval( call, R_GlobalEnv ) ) ;
	UNPROTECT(3) ;
	// return result to R
	return res ;
}
