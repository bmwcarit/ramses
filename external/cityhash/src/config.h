
/* Define to 1 if the compiler supports __builtin_expect. */
#if defined __GNUC__ || defined __CLANG__
#define HAVE_BUILTIN_EXPECT 1
#else
#undef HAVE_BUILTIN_EXPECT
#endif

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#undef WORDS_BIGENDIAN
