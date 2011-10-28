#include "common.h"
#define TEST_MAIN
#include "test.h"
#include "args.h"


extern void test_instr ();
extern void test_mmu ();
extern void test_cpu ();
extern void test_alu ();


int main() {
    args.verbosity = 2;

    BEGIN_TESTS();

    SUITE(test_instr);
    SUITE(test_mmu);
    SUITE(test_cpu);
    SUITE(test_alu);

    END_TESTS();

    return 0;
}

