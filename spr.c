#include <stdlib.h>

#define DSCR1   10
#define DSCR2   50
#define SPRN_DSCR      3
#define SPRN_PPR       896

__attribute__((used)) void break_here(void)
{
	//while(!cptr[1]);
}

void asm_spr(void)
{
	asm __volatile__(

		"li 0, %[dscr1];"        /* DSCR 10 */
		"mtspr %[sprn_dscr], 0;"
		"or     1,1,1;"         /* PPR (0x8000000000000) */

		"bl break_here;"

		"li 0, %[dscr2];"        /* DSCR 50 */
		"mtspr %[sprn_dscr], 0;"
		"or     31,31,31;"      /* PPR (0x4000000000000) */

		"bl break_here;"

		:
		: [sprn_dscr] "i" (SPRN_DSCR), [dscr1] "i" (DSCR1), [dscr2] "i" (DSCR2)
		: "memory", "r0" 
		);

	exit(0);
}

int main(int argc, char *argv[])
{
//	SKIP_IF(!((long)get_auxv_entry(AT_HWCAP2) & PPC_FEATURE2_DSCR));

	asm_spr();

	return 0;
}
