#define TEST_MAIN
#include "test.h"


extern void test_instr ();
extern void test_mmu ();
extern void test_cpu ();


int main() {
    BEGIN_TESTS();

    SUITE(test_instr);
    SUITE(test_mmu);
    SUITE(test_cpu);

    END_TESTS();

    return 0;
}

