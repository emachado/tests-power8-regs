CFLAGS+=-g3 -flto -Wall -DGIT_VERSION='"unknown"'
DEPS=harness.c ptrace.S utils.c
EXEC=gpr fpr vsx spr

all: $(EXEC)

gpr: gpr.c $(DEPS)
fpr: fpr.c $(DEPS)
vsx: vsx.c $(DEPS)
spr: spr.c

clean:
	rm $(EXEC)
