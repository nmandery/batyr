#ifndef __batyr_macros_h__
#define __batyr_macros_h__


// these two macros convert macro values to strings
#define STRINGIFY2(x)   #x
#define STRINGIFY(x)    STRINGIFY2(x)

// macro to silence warnings regarding unused variables
// loop gets optimized out by the compiler
#define UNUSED(expr) do { (void)(expr); } while (0);

#endif // __batyr_macros_h__
