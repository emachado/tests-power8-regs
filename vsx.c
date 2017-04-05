#include "ptrace.h"
#define VEC_MAX 128

extern void loadvsx(void *p, int tmp);

unsigned long fp_load[VEC_MAX];
unsigned long fp_load_new[VEC_MAX];
unsigned long fp_store[VEC_MAX];
unsigned long fp_load_ckpt[VEC_MAX];

__attribute__((used)) void load_vsx(void)
{
	loadvsx(fp_load, 0);
}

__attribute__((used)) void load_vsx_new(void)
{
	loadvsx(fp_load_new, 0);
}

__attribute__((used)) void load_vsx_ckpt(void)
{
	loadvsx(fp_load_ckpt, 0);
}

__attribute__((used)) void break_here(void)
{
	//while(!cptr[1]);
}

void tm_spd_vsx(void)
{
	unsigned long abort, texasr;

	asm __volatile__(
		"bl load_vsx_ckpt;"

		"1: ;"
		TBEGIN
		"beq 2f;"

		"bl load_vsx_new;"
		TSUSPEND
		"bl load_vsx;"
		"bl break_here;"
		TRESUME

		TEND
		"li 0, 0;"
		"ori %[abrt], 0, 0;"
		"b 3f;"

		"2: ;"
		"li 0, 1;"
		"ori %[abrt], 0, 0;"
		"mfspr %[texasr], %[sprn_texasr];"

		"3: ;"
		: [abrt] "=r" (abort), [texasr] "=r" (texasr)
		: [fp_load] "r" (fp_load), [fp_load_ckpt] "r" (fp_load_ckpt), [sprn_texasr] "i"  (SPRN_TEXASR)
		: "memory", "r0", "r1", "r2", "r3", "r4", "r8", "r9", "r10", "r11"
		);

	if (abort)
	  {
	    printf("failed transaction (texasr %lx)\n", texasr);
	    exit(1);
	  }
	exit(0);
}

int main(int argc, char *argv[])
{
	int i;

	SKIP_IF(!((long)get_auxv_entry(AT_HWCAP2) & PPC_FEATURE2_HTM));

	for(i = 0; i < 128; i++) {
		fp_load[i] = 1 + i;
		fp_load_new[i] = 2 * (1 + i);
		fp_load_ckpt[i] = 3 * (1 + i);
	}

	tm_spd_vsx();

	return 0;
}
