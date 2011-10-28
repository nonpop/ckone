#include <string.h>
#include "test.h"
#include "util.h"
#include "cpu.h"
#include "instr.h"


extern void cpu_calculate_second_operand (s_ckone* kone);
extern void cpu_fetch_instr (s_ckone* kone);


void test_cpu () {
    s_ckone k;
    int32_t mem[512];

    k.mem = mem;
    k.mem_size = sizeof(mem)/sizeof(int32_t);

    /*
    BEGIN ("fetch_instr") {
        clear (&k);
        k.mmu_limit = 2;
        int32_t instr = make_instr (LOAD, R3, DIRECT, R1, 42);
        mem[1] = instr;

        TEST_I32X (0, k.ir);
        cpu_fetch_instr (&k);
        TEST_I32X (0, k.ir);
        TEST_I32X (0, k.sr & SR_M);
        cpu_fetch_instr (&k);
        TEST_I32X (instr, k.ir);
        TEST_I32X (0, k.sr & SR_M);
        cpu_fetch_instr (&k);
        TEST_I32X (SR_M, k.sr & SR_M);
    }
    BEGIN ("calculate_second_operand immediate") {
        clear (&k);
        k.mmu_limit = 2;
        int32_t instr = make_instr (LOAD, R0, IMMEDIATE, R1, 42);
        mem[0] = instr;
        k.r[1] = 5;

        cpu_fetch_instr (&k);
        cpu_calculate_second_operand (&k);
        TEST_I32 (47, k.tr);
        TEST_I32X (0, k.sr & (SR_M | SR_U));
    }
    BEGIN ("calculate_second_operand direct") {
        clear (&k);
        k.mmu_limit = 4;
        int32_t instr = make_instr (LOAD, R0, DIRECT, R1, 2);
        mem[0] = instr;
        mem[3] = 42;
        k.r[1] = 1;

        cpu_fetch_instr (&k);
        cpu_calculate_second_operand (&k);
        TEST_I32 (42, k.tr);
        TEST_I32X (0, k.sr & (SR_M | SR_U));
    }
    BEGIN ("calculate_second_operand indirect") {
        clear (&k);
        k.mmu_limit = 4;
        int32_t instr = make_instr (LOAD, R0, INDIRECT, R1, 2);
        mem[0] = instr;
        mem[1] = 1337;
        mem[3] = 1;
        k.r[1] = 1;

        cpu_fetch_instr (&k);
        cpu_calculate_second_operand (&k);
        TEST_I32 (1337, k.tr);
        TEST_I32X (0, k.sr & (SR_M | SR_U));
    }
    */

    BEGIN ("load/store, addressing modes") {
        clear (&k);
        mem[0] = 33554435;      // load r0, =i
        mem[1] = 16777220;      // store r0, pi
        mem[2] = 36700164;      // load r1, @pi
        mem[3] = 42;            // i dc 42
        mem[4] = 0;             // pi dc 0

        cpu_step (&k);          // load r0, =i
        TEST_I32 (3, k.r[R0]);
        TEST_I32 (0, k.sr);

        cpu_step (&k);          // store r0, pi
        TEST_I32 (3, mem[4]);
        TEST_I32 (0, k.sr);

        cpu_step (&k);          // load r1, @pi
        TEST_I32 (42, k.r[R1]);
        TEST_I32 (0, k.sr);
    }

    BEGIN ("arithmetic, indexing") {
        clear (&k);

        mem[ 0] = 35651595;     // load r1, =t
        mem[ 1] = 33554474;     // load r0, =42
        mem[ 2] = 16842752;     // store r0, 0(r1)
        mem[ 3] = 33555769;     // load r0, =1337
        mem[ 4] = 16842753;     // store r0, 1(r1)
        mem[ 5] = 18874381;     // store r1, p1
        mem[ 6] = 287309825;    // add r1, =1
        mem[ 7] = 18874382;     // store r1, p2
        mem[ 8] = 304087041;    // sub r1, =1
        mem[ 9] = 38797325;     // load r2, @p1
        mem[10] = 290455566;    // add r2, @p2
        mem[11] = 0;            // t ds 2
        mem[12] = 0;            //
        mem[13] = 0;            // p1 dc 0
        mem[14] = 0;            // p2 dc 0

        cpu_step (&k);          // load r1, =t
        TEST_I32 (11, k.r[R1]);
        cpu_step (&k);          // load r0, =42
        TEST_I32 (42, k.r[R0]);
        cpu_step (&k);          // store r0, 0(r1)
        TEST_I32 (42, mem[11]);
        cpu_step (&k);          // load r0, =1337
        TEST_I32 (1337, k.r[R0]);
        cpu_step (&k);          // store r0, 1(r1)
        TEST_I32 (1337, mem[12]);
        cpu_step (&k);          // store r1, p1
        TEST_I32 (11, mem[13]);
        cpu_step (&k);          // add r1, =1
        TEST_I32 (12, k.r[R1]);
        cpu_step (&k);          // store r1, p2
        TEST_I32 (12, mem[14]);
        cpu_step (&k);          // sub r1, =1
        TEST_I32 (11, k.r[R1]);
        cpu_step (&k);          // load r2, @p1
        TEST_I32 (42, k.r[R2]);
        cpu_step (&k);          // add r2, @p2
        TEST_I32 (42+1337, k.r[R2]);
        TEST_I32 (0, k.sr);
    }

    BEGIN ("conditional jump, comp a") {
        clear (&k);

        mem[0] = 33554434;      // load r0, =2
        mem[1] = 520617989;     // comp r0, i
        mem[2] = 704643076;     // jnles a
        mem[3] = 35717119;      // load r1, =-1
        mem[4] = 35651585;      // a load r1, =1
        mem[5] = 3;             // i dc 3

        cpu_step (&k);          // load r0, =2
        cpu_step (&k);          // comp r0, i
        TEST_I32 (SR_L, k.sr);
        cpu_step (&k);          // jnles a
        TEST_I32 (3, k.pc);
        cpu_step (&k);          // load r1, =-1
        TEST_I32 (-1, k.r[R1]);
        TEST_I32 (SR_L, k.sr);
    }

    BEGIN ("conditional jump, comp b") {
        clear (&k);

        mem[0] = 33554435;      // load r0, =3
        mem[1] = 520617989;     // comp r0, i
        mem[2] = 704643076;     // jnles a
        mem[3] = 35717119;      // load r1, =-1
        mem[4] = 35651585;      // a load r1, =1
        mem[5] = 3;             // i dc 3

        cpu_step (&k);          // load r0, =3
        cpu_step (&k);          // comp r0, i
        TEST_I32 (SR_E, k.sr);
        cpu_step (&k);          // jnles a
        TEST_I32 (4, k.pc);
        cpu_step (&k);          // a load r1, =1
        TEST_I32 (1, k.r[R1]);
        TEST_I32 (SR_E, k.sr);
    }

    BEGIN ("conditional jump, reg a") {
        clear (&k);

        mem[0] = 33554433;      // load r0, =1
        mem[1] = 587202563;     // jpos r0, a
        mem[2] = 35717119;      // load r1, =-1
        mem[3] = 35651585;      // a load r1, =1

        cpu_step (&k);          // load r0, =1
        cpu_step (&k);          // jpos r0, a
        TEST_I32 (3, k.pc);
        cpu_step (&k);          // a load r1, =1
        TEST_I32 (1, k.r[R1]);
        TEST_I32 (0, k.sr);
    }

    BEGIN ("conditional jump, reg b") {
        clear (&k);

        mem[0] = 33554432;      // load r0, =0
        mem[1] = 587202563;     // jpos r0, a
        mem[2] = 35717119;      // load r1, =-1
        mem[3] = 35651585;      // a load r1, =1

        cpu_step (&k);          // load r0, =0
        cpu_step (&k);          // jpos r0, a
        TEST_I32 (2, k.pc);
        cpu_step (&k);          // load r1, =-1
        TEST_I32 (-1, k.r[R1]);
        TEST_I32 (0, k.sr);
    }

    BEGIN ("indirect unconditional jump") {
        clear (&k);

        mem[0] = 33554436;      // load r0, =dest
        mem[1] = 16777221;      // store r0, destp
        mem[2] = 537395205;     // jump @destp
        mem[3] = 33619926;      // load r0, =-42
        mem[4] = 33554474;      // dest load r0, =42
        mem[5] = 0;             // destp dc 0

        cpu_step (&k);          // load r0, =dest
        cpu_step (&k);          // store r0, destp
        cpu_step (&k);          // jump @destp
        TEST_I32 (4, k.pc);
        cpu_step (&k);          // dest load r0, =42
        TEST_I32 (42, k.r[R0]);
        TEST_I32 (0, k.sr);
    }

    BEGIN ("svc halt") {
        clear (&k);

        mem[0] = 1891631115;    // svc sp, =halt

        cpu_step (&k);
        TEST_BOOL (true, k.halted);
        TEST_I32 (2, k.r[SP]);
        TEST_I32 (2, k.r[FP]);
    }

    BEGIN ("call/exit") {
        clear (&k);

        mem[0] = 46137352;      // load sp, =stack
        mem[1] = 868220970;     // push sp, =42
        mem[2] = 868222265;     // push sp, =1337
        mem[3] = 834666501;     // call sp, adder
        mem[4] = 1891631115;    // svc sp, =halt
        mem[5] = 36700157;      // adder load r1, a(fp)     ; a equ -3
        mem[6] = 288358398;     // add r1, b(fp)            ; b equ -2
        mem[7] = 851443714;     // exit sp, =2
        mem[8] = 0;             // stack ds 100 ...

        cpu_step (&k);          // load sp, =stack
        cpu_step (&k);          // push sp, =42
        TEST_I32 (9, k.r[SP]);
        TEST_I32 (42, mem[9]);
        cpu_step (&k);          // push sp, =1337
        TEST_I32 (10, k.r[SP]);
        TEST_I32 (1337, mem[10]);
        cpu_step (&k);          // call sp, adder
        TEST_I32 (12, k.r[SP]);
        TEST_I32 (12, k.r[FP]);
        TEST_I32 (4, mem[11]);
        TEST_I32 (0, mem[12]);
        TEST_I32 (5, k.pc);
        cpu_step (&k);          // adder load r1, a(fp)
        TEST_I32 (42, k.r[R1]);
        cpu_step (&k);          // add r1, b(fp)
        TEST_I32 (42+1337, k.r[R1]);
        cpu_step (&k);          // exit sp, =2
        TEST_I32 (8, k.r[SP]);
        TEST_I32 (4, k.pc);
        cpu_step (&k);          // svc sp, =halt
        TEST_I32 (10, k.r[SP]);
        TEST_I32 (10, k.r[FP]);
        TEST_BOOL (true, k.halted);
    }

    BEGIN ("factorial; call/exit/svc") {
        clear (&k);

        mem[ 0] = 46137357;     // load sp, =stack
        mem[ 1] = 868220938;    // push sp, =10
        mem[ 2] = 834666500;    // call sp, fac
        mem[ 3] = 1891631115;   // svc sp, =halt
        mem[ 4] = 36700158;     // fac load r1, n(fp)   (n equ -2)
        mem[ 5] = 522190849;    // comp r1, =1
        mem[ 6] = 738197516;    // jngre end
        mem[ 7] = 304087041;    // sub r1, =1
        mem[ 8] = 868286464;    // push sp, r1
        mem[ 9] = 834666500;    // call sp, fac
        mem[10] = 38797310;     // load r2, n(fp)
        mem[11] = 320995328;    // mul r1, r2
        mem[12] = 851443713;    // end exit sp, =1
        mem[13] = 0;            // stack ds 100 ...

        while (!k.halted)
            cpu_step (&k);

        TEST_I32 (3628800, k.r[R1]);
        TEST_I32 (15, k.r[SP]);
        TEST_I32 (15, k.r[FP]);
        TEST_BITSCLR (k.sr, SR_O | SR_M);
    }
}

