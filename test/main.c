#define TEST_MAIN
#include "test.h"


extern void test_instr ();
extern void test_mmu ();


int main() {
    BEGIN_TESTS();

    SUITE(test_instr);
    SUITE(test_mmu);

    END_TESTS();

    return 0;
}

