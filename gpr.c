#include "ptrace.h"
#define VEC_MAX 10

extern void load_gpr(void *p);

unsigned long gp_load[VEC_MAX];
unsigned long gp_load_new[VEC_MAX];
unsigned long gp_load_ckpt[VEC_MAX];

__attribute__((used)) void load(void)
{
	load_gpr(gp_load);
}

__attribute__((used)) void load_new(void)
{
	load_gpr(gp_load_new);
}

__attribute__((used)) void load_ckpt(void)
{
	load_gpr(gp_load_ckpt);
}

__attribute__((used)) void break_here(void)
{
	//while(!cptr[1]);
}

void tm_spd(void)
{
	unsigned long abort, texasr;

	asm __volatile__(
		"bl load_ckpt;"

		"1: ;"
		TBEGIN
		"beq 2f;"

		"bl load_new;"
		TSUSPEND
		"bl load;"
		"bl break_here;"
		TRESUME

		TEND
		"li 0, 0;"
		"ori %[abrt], 0, 0;"
		"b 3f;"

		// transaction abort handler
		"2: ;"
		"li 0, 1;"
		"ori %[abrt], 0, 0;"
		"mfspr %[texasr], %[sprn_texasr];"

		"3: ;"
		: [abrt] "=r" (abort), [texasr] "=r" (texasr)
		: [gp_load] "r" (gp_load), [gp_load_ckpt] "r" (gp_load_ckpt), [sprn_texasr] "i"  (SPRN_TEXASR)
		: "memory", "r0", "r1", "r2", "r3", "r4", "r8", "r9", "r10", "r11", "r12", "r13",
		"r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23"
		);

	if (abort)
	  {
	    printf("failed transaction (texasr %lx)\n", texasr);
	    exit(1);
	  }
	printf("successfull transaction (texasr %lx)\n", texasr);
	exit(0);
}

int main(int argc, char *argv[])
{
	int i;

	SKIP_IF(!((long)get_auxv_entry(AT_HWCAP2) & PPC_FEATURE2_HTM));

	for(i = 0; i < VEC_MAX; i++) {
		gp_load[i] = 1 + i;
		gp_load_new[i] = 2 * (1 + i);
		gp_load_ckpt[i] = 3 * (1 + i);
	}

	tm_spd();

	return 0;
}
