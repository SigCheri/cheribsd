PACKAGE=	clibs-dev

NO_WMISSING_VARIABLE_DECLARATIONS=
# Can't instrument these files since that breaks non-sanitized programs.
MK_ASAN:=	no
MK_UBSAN:=	no

.include <src.opts.mk>
MK_SSP=	no

.if !defined(BUILDING_TESTS)

OBJS+=	Scrt1.o crt1.o
OBJS+=	crtbegin.o crtbeginS.o crtbeginT.o
OBJS+=	crtend.o crtendS.o
.if !${MACHINE_ABI:Mpurecap}
OBJS+=	gcrt1.o
OBJS+=	crti.o crtn.o
.endif

SIG_OBJS =
.if ${MACHINE_CPU:Msigcheri} && ${MACHINE_CPU:Mriscv}
SIG_OBJS+=	Scrt1_sig.o crt1_sig.o
SIG_OBJS+=	crtbegin_sig.o crtbeginS_sig.o crtbeginT_sig.o
SIG_OBJS+=	crtend_sig.o crtendS_sig.o
.endif
OBJS += ${SIG_OBJS}

CRT1OBJS+=	crtbrand.o feature_note.o ignore_init_note.o

ACFLAGS+=	-DLOCORE

CFLAGS+=	-DSTRIP_FBSDID
CFLAGS+=	-fno-omit-frame-pointer
CFLAGS+=	-I${.CURDIR:H}/common \
		-I${SRCTOP}/lib/libc/include \
		-I${SRCTOP}/lib/libsys \

.if ${MACHINE_ABI:Mpurecap}
# Purecap code requires crt_init_globals to be called before using jump tables
CFLAGS+=	-fno-jump-tables
CFLAGS+=	-I${.CURDIR:H}/common-cheri
.endif

SIG_CFLAGS = ${CFLAGS}
.if ${MACHINE_CPU:Msigcheri} && ${MACHINE_CPU:Mriscv}
SIG_CFLAGS +=	-march=rv64imafdc_xcheri_xsigcheri -mabi=l64ps128d
.endif

# .info OBJS=${OBJS} MACHINE_CPU=${MACHINE_CPU} 
# .info ${CFLAGS}
# .info SIG_CFLAGS=${SIG_CFLAGS} 

CFLAGS_CRTS+=	-DSHARED ${PICFLAG}

FILES=		${OBJS} crt.json
FILESMODE=	${LIBMODE}
FILESOWN=	${LIBOWN}
FILESGRP=	${LIBGRP}
FILESDIR=	${LIBDIR}
# These FILES qualify as libraries for the purpose of LIBRARIES_ONLY.
.undef LIBRARIES_ONLY

CRT1SRC?=	crt1_c.c
CRT1OBJ?=
SIG_CRT1OBJ = $(patsubst %.o,%_sig.o,$(CRT1OBJ))

CLEANFILES+=	${OBJS} ${CRT1OBJS} crt1_c.o gcrt1_c.o Scrt1_c.o crt1_c_sig.o gcrt1_c_sig.o Scrt1_c_sig.o
CLEANFILES+=	crti_s.o ${CRT1SRC:C/.[S|c]$/.o/} crti_s_sig.o ${CRT1SRC:C/.[S|c]$/_sig.o/}

crt1.o:	${CRT1SRC:C/.[S|c]$/.o/} ${CRT1OBJS} ${CRT1OBJ}
	${LD} ${_LDFLAGS} -o ${.TARGET} -r ${.ALLSRC:M*.o}

gcrt1_c.o: ${CRT1SRC}
	${CC} ${CFLAGS} -DGCRT -c -o ${.TARGET} ${.CURDIR}/${CRT1SRC}

gcrt1.o: gcrt1_c.o ${CRT1OBJS} ${CRT1OBJ}
	${LD} ${_LDFLAGS} -o ${.TARGET} -r ${.ALLSRC:M*.o}

Scrt1_c.o: ${CRT1SRC}
	${CC} ${CFLAGS} -fPIC -DPIC -c -o ${.TARGET} ${.CURDIR}/${CRT1SRC}

Scrt1.o: Scrt1_c.o ${CRT1OBJS} ${CRT1OBJ}
	${LD} ${_LDFLAGS} -o ${.TARGET} -r ${.ALLSRC:M*.o}

# __FreeBSD_version is recorded in crt1.o et al via crtbrand.
crtbrand.o: ${SRCTOP}/sys/sys/param.h

crtbegin.o: crtbegin.c
crtbeginS.o: crtbegin.c
crtbeginT.o: crtbegin.c
crtend.o: crtend.c
crtendS.o: crtend.c

crtbegin.o crtend.o crtbeginT.o:
	${CC} ${CFLAGS} -I${.CURDIR} -c -o ${.TARGET} ${.ALLSRC:N*.h:[1]}

crtbeginS.o crtendS.o:
	${CC} ${CFLAGS} -I${.CURDIR} ${CFLAGS_CRTS} -c -o ${.TARGET} \
	    ${.ALLSRC:N*.h:[1]}

crti_s.o: crti.S
	${CC} ${CFLAGS} ${ACFLAGS} -c ${.ALLSRC:M*.S} -o ${.TARGET}

crti.o: crti_s.o crtbrand.o
	${LD} ${_LDFLAGS} -o ${.TARGET} -r ${.ALLSRC:M*.o}

.if ${MACHINE_CPU:Msigcheri} && ${MACHINE_CPU:Mriscv}

crt1_c_sig.o: ${CRT1SRC}
	${CC} ${SIG_CFLAGS} -c -o ${.TARGET} ${.CURDIR}/${CRT1SRC}

crt1_sig.o:	${CRT1SRC:C/.[S|c]$/_sig.o/} ${CRT1OBJS} ${SIG_CRT1OBJ}
	${LD} ${_LDFLAGS} -o ${.TARGET} -r ${.ALLSRC:M*_sig.o}

gcrt1_c_sig.o: ${CRT1SRC}
	${CC} ${SIG_CFLAGS} -DGCRT -c -o ${.TARGET} ${.CURDIR}/${CRT1SRC}

gcrt1_sig.o: gcrt1_c_sig.o ${CRT1OBJS} ${SIG_CRT1OBJ}
	${LD} ${_LDFLAGS} -o ${.TARGET} -r ${.ALLSRC:M*_sig.o}

Scrt1_c_sig.o: ${CRT1SRC}
	${CC} ${SIG_CFLAGS} -fPIC -DPIC -c -o ${.TARGET} ${.CURDIR}/${CRT1SRC}

Scrt1_sig.o: Scrt1_c_sig.o ${CRT1OBJS} ${SIG_CRT1OBJ}
	${LD} ${_LDFLAGS} -o ${.TARGET} -r ${.ALLSRC:M*_sig.o}

# __FreeBSD_version is recorded in crt1_sig.o et al via crtbrand.
crtbrand_sig.o: ${SRCTOP}/sys/sys/param.h

crtbegin_sig.o: crtbegin.c
crtbeginS_sig.o: crtbegin.c
crtbeginT_sig.o: crtbegin.c
crtend_sig.o: crtend.c
crtendS_sig.o: crtend.c

crtbegin_sig.o crtend_sig.o crtbeginT_sig.o:
	${CC} ${SIG_CFLAGS} -I${.CURDIR} -c -o ${.TARGET} ${.ALLSRC:N*.h:[1]}
	$(error "compile crtbegin)

crtbeginS_sig.o crtendS_sig.o:
	${CC} ${SIG_CFLAGS} -I${.CURDIR} ${CFLAGS_CRTS} -c -o ${.TARGET} \
	    ${.ALLSRC:N*.h:[1]}


crti_s_sig.o: crti.S
	${CC} ${SIG_CFLAGS} ${ACFLAGS} -c ${.ALLSRC:M*.S} -o ${.TARGET}

crti_sig.o: crti_s_sig.o crtbrand_sig.o
	${LD} ${_LDFLAGS} -o ${.TARGET} -r ${.ALLSRC:M*_sig.o}
.endif

.endif

.include "../Makefile.inc"
