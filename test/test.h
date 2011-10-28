#ifndef TEST_H
#define TEST_H


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


#define BEGIN(name) printf(">>> %s\n", name);


#define TEST(type, fmt, expected, actual) { \
    printf(__FILE__ ":%d: " #actual " = ", __LINE__); \
    type TEST_expected = (expected); \
    type TEST_result = (actual); \
    printf(fmt ": ", TEST_result); \
    if (TEST_result != TEST_expected) { \
        printf("***FAILED***: expected " fmt "\n", TEST_expected); \
    } else { \
        printf("ok\n"); \
        test_tests_succeeded++; \
    } \
    test_tests_total++; \
}


#define TEST_BOOL(expected, actual) { \
    printf(__FILE__ ":%d: " #actual " = ", __LINE__); \
    bool TEST_expected = (expected); \
    bool TEST_result = (actual); \
    printf("%s: ", TEST_result? "true" : "false"); \
    if (TEST_result != TEST_expected) { \
        printf("***FAILED***: expected %s\n", TEST_expected? "true" : "false"); \
    } else { \
        printf("ok\n"); \
        test_tests_succeeded++; \
    } \
    test_tests_total++; \
}


#define TEST_STR(expected, actual) { \
    printf(__FILE__ ":%d: " #actual " = ", __LINE__); \
    const char* TEST_expected = (expected); \
    const char* TEST_result = (actual); \
    printf("\"%s\": ", TEST_result); \
    if (strcmp (TEST_result, TEST_expected)) { \
        printf("***FAILED***: expected \"%s\"\n", TEST_expected); \
    } else { \
        printf("ok\n"); \
        test_tests_succeeded++; \
    } \
    test_tests_total++; \
}


#define TEST_I32X(expected, actual) TEST(int32_t, "0x%x", expected, actual)
#define TEST_I32(expected, actual) TEST(int32_t, "%d", expected, actual)
#define TEST_BITSSET(value, bits) TEST_I32X((bits), (value) & (bits))
#define TEST_BITSCLR(value, bits) TEST_I32X(0, (value) & (bits))


#endif

