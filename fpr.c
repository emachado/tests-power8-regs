#include "ptrace.h"
#define VEC_MAX 32

extern void load_fpr(void *p);

float fp_load[VEC_MAX];
float fp_load_new[VEC_MAX];
float fp_load_ckpt[VEC_MAX];

__attribute__((used)) void load(void)
{
	load_fpr(fp_load);
}

__attribute__((used)) void load_new(void)
{
	load_fpr(fp_load_new);
}

__attribute__((used)) void load_ckpt(void)
{
	load_fpr(fp_load_ckpt);
}

__attribute__((used)) void break_here(void)
{
	//breakpoint;
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

		"2: ;"
		"li 0, 1;"
		"ori %[abrt], 0, 0;"
		"mfspr %[texasr], %[sprn_texasr];"

		"3: ;"
		: [abrt] "=r" (abort), [texasr] "=r" (texasr)
		: [fp_load] "r" (fp_load), [fp_load_ckpt] "r" (fp_load_ckpt), [sprn_texasr] "i"  (SPRN_TEXASR)
		: "memory"
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

	for(i = 0; i < VEC_MAX; i++) {
		fp_load[i] = 0.1;
		fp_load_new[i] = 0.2;
		fp_load_ckpt[i] = 0.3;
	}

	tm_spd();

	return 0;
}
