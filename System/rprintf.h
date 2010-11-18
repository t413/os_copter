#ifndef rprintf_h_
#define rprintf_h_

#ifdef __cplusplus
extern "C" {
#endif

/// Sets up which function to use to output data using rprintf.
/// @param put The function pointer to output data.
void rprintf_devopen( unsigned long(*put)(char, unsigned long) );

/// Works like printf, except for floating-point printf.
/// @param format printf syntax.
void rprintf ( char const *format, ... );

#ifdef __cplusplus
}
#endif

#endif
