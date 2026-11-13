CC      := clang
TARGET  := nitrogendb
BUILD   := build

BREW    := $(shell brew --prefix 2>/dev/null)

INCLUDES := -Isrc \
            -Isrc/Bytecodes \
            -Isrc/Execution_Engine \
            -Isrc/Parser \
            -Isrc/Storage_engine \
            $(if $(BREW),-I$(BREW)/include)

CFLAGS  := -std=gnu11 -O0 -g -pthread -MMD -MP $(INCLUDES)
LDFLAGS := $(if $(BREW),-L$(BREW)/lib)
LDLIBS  := -lraylib -framework IOKit -framework Cocoa -framework OpenGL

# All sources. Insert_pager.c (capital I) is a duplicate of insert_pager.c, so it is skipped.
SRCS := src/main.c \
        src/Parser/Create_parser.c \
        src/Parser/insert_parser.c \
        src/Storage_engine/b+trees.c \
        src/Storage_engine/buffer.c \
        src/Storage_engine/Header_page.c \
        src/Storage_engine/Insert_pager.c \
        src/Storage_engine/insert_parser_execute.c \
        src/Storage_engine/Page_Manager.c \
        src/Storage_engine/wal.c
OBJS := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all run clean reset test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# delete the database files so you start fresh
reset:
	rm -rf nitrogen.db normal_file toast_header toast_map toast_data wal

run: $(TARGET) reset
	./$(TARGET)

# build and run one test file:  make test T=tests/Storage_engine/buffer_pool_test.c
# (each test has its own main(), so src/main.c is left out)
TEST_OBJS := $(filter-out $(BUILD)/main.o, $(OBJS))
test: $(TEST_OBJS)
	@test -n "$(T)" || (echo "usage: make test T=tests/Storage_engine/buffer_pool_test.c"; exit 1)
	$(CC) $(CFLAGS) $(LDFLAGS) $(T) $(TEST_OBJS) $(LDLIBS) -o $(BUILD)/test_bin
	./$(BUILD)/test_bin

clean:
	rm -rf $(BUILD) $(TARGET)

-include $(DEPS)