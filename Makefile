.include <bsd.compiler.mk>

PROG = bossvol
MAN =

DESTDIR = $(HOME)/bin
BINOWN != id -nu
BINGRP != id -ng

SRCS = main.c gui.c mixerdev.c


USE_LTO ?= no

CSTD = c11
CFLAGS += -pedantic -Werror -Wall -Wextra
CFLAGS += $(GTK2_INCLUDE)
LDADD += $(GTK2_LIBS)

.if !defined(DEBUG_FLAGS)
CFLAGS += -O3 -DNDEBUG
LDFLAGS += -s
.endif

.if $(COMPILER_TYPE) == "clang"
CFLAGS += -fcolor-diagnostics
.endif

.if $(USE_LTO) == "yes"
.c.o:  # override sys.mk's defaults
	$(CC) $(CFLAGS) $(CCONLY) -c $(.IMPSRC) -o $(.TARGET)
	$(CTFCONVERT_CMD)
CFLAGS += -flto
CCONLY += -emit-llvm
LDFLAGS += -fuse-ld=gold
.endif


PKG_CONFIG ?= pkg-config
GTK2_PCNAME = gtk+-2.0
GTK2_LIBS != $(PKG_CONFIG) --libs $(GTK2_PCNAME)
GTK2_INCLUDE != $(PKG_CONFIG) --cflags $(GTK2_PCNAME)


.include <bsd.prog.mk>

