#ifndef TEST_H
#define TEST_H

#include <stdio.h>


#ifdef TEST_MAIN
    unsigned int test_tests_succeeded;
    unsigned int test_tests_total;
#else
    extern unsigned int test_tests_succeeded;
    extern unsigned int test_tests_total;
#endif


#define BEGIN_TESTS() 
#define END_TESTS() printf("%u of %u tests succeeded\n", test_tests_succeeded, test_tests_total);


#define SUITE(f) { \
    printf("Running " #f "...\n"); \
    (f()); \
    printf("\n"); \
}


#define TEST(type, fmt, expected, actual) { \
    printf(__FILE__ ":%d: " #actual " = ", __LINE__); \
    type result = (actual); \
    printf(fmt ": ", result); \
    if (result != (expected)) { \
        printf("***FAILED***: expected " fmt "\n", (expected)); \
    } else { \
        printf("ok\n"); \
        test_tests_succeeded++; \
    } \
    test_tests_total++; \
}


#endif

