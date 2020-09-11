#for context-a5
ifneq ($(filter armv7, $(ARCH)),)
ifeq ($(USE_ASM), Yes)
ASM_ARCH = arm
ASMFLAGS += -I$(SRC_PATH)codec/common/arm/
CFLAGS += -DHAVE_NEON
endif
endif

SHAREDLIBSUFFIX = so
SHAREDLIBSUFFIXFULLVER=$(SHAREDLIBSUFFIX).$(FULL_VERSION)
SHAREDLIBSUFFIXMAJORVER=$(SHAREDLIBSUFFIX).$(SHAREDLIB_MAJORVERSION)
SHLDFLAGS = $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIXMAJORVER)
CFLAGS += --cpu cortex-a5 --diag_suppress 1,9,61,66,68,69,111,117,144,152,167,170,174,177,186,188,191,223,226 \
		--diag_suppress 236,375,494,513,550,815,940,1134,1287,1293,1294,1295,1296,138 \
		--diag_suppress 1441,1608,1652,1764,1786,2523,2775,3017,3052,6314,6329 \
		--diag_suppress 1300 \
		--fpu VFPv2 --enum_is_int --enum_is_int -D_RTOS -D_DEBUG --loose_implicit_cast --debug --no_unaligned_access --li -g --thumb \
		-DRTOS \
		-I../build_ys102/watch8910/external/libc

LDFLAGS += 
STATIC_LDFLAGS += -lpthread -lm