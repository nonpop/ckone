#ifndef ALU_H
#define ALU_H


struct s_ckone;

extern void alu_add (s_ckone* kone);
extern void alu_sub (s_ckone* kone);
extern void alu_mul (s_ckone* kone);
extern void alu_div (s_ckone* kone);
extern void alu_mod (s_ckone* kone);
extern void alu_and (s_ckone* kone);
extern void alu_or (s_ckone* kone);
extern void alu_xor (s_ckone* kone);
extern void alu_not (s_ckone* kone);
extern void alu_shl (s_ckone* kone);
extern void alu_shr (s_ckone* kone);
extern void alu_shra (s_ckone* kone);


#endif

