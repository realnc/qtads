#ifndef CONFIG_H
#define CONFIG_H

/* We use the 'override' keyword from C++-11.  We define it as an empty
 * macro on older compilers. */
#if __cplusplus < 201103L
    #define override
#endif

#endif
