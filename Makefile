ICK ?= ick
GREASE ?= grease
PROFILE ?= Debug
OUT ?= build/$(PROFILE)
OPT_Debug = -O0 -g
OPT_Release = -O2 -DNDEBUG
CFLAGS = -fno-link-libatomic -std=c11 -Wall -Wextra -Werror $(OPT_$(PROFILE))

.PHONY: host-test reference
host-test: $(OUT)/engine-test $(OUT)/session-test $(OUT)/render-policy-test $(OUT)/monomial-test
	$(OUT)/engine-test
	$(OUT)/session-test
	$(OUT)/render-policy-test
	$(OUT)/monomial-test
reference: $(OUT)/engine-test
	$(OUT)/engine-test --reference
$(OUT)/engine-test: native/mathieu.c native/mathieu.h tests/engine.c
	mkdir -p $(OUT)
	$(ICK) $(CFLAGS) native/mathieu.c tests/engine.c -o $@
$(OUT)/session-test: native/mathieu.c native/mathieu.h native/session.c native/session.h native/checkpoint.c native/render.c native/render.h tests/session.c
	mkdir -p $(OUT)
	$(ICK) $(CFLAGS) native/mathieu.c native/session.c native/checkpoint.c native/render.c tests/session.c -lm -o $@
$(OUT)/render-policy-test: native/mathieu.c native/session.c native/render.c native/render.h tests/render-policy.c
	mkdir -p $(OUT)
	$(ICK) $(CFLAGS) native/mathieu.c native/session.c native/render.c tests/render-policy.c -lm -o $@
$(OUT)/monomial-test: native/mathieu.c native/mathieu.h native/monomial.c native/monomial.h tests/monomial.c
	mkdir -p $(OUT)
	$(ICK) $(CFLAGS) native/mathieu.c native/monomial.c tests/monomial.c -o $@
