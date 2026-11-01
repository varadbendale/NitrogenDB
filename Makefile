CC = gcc
SRC = src/main.c src/Parser/Create_parser.c \
      src/Storage_engine/Header_page.c src/Storage_engine/Page_Manager.c
INC = -I src/Parser -I src/Storage_engine
LIBS = $(shell pkg-config --cflags --libs raylib)

nitrogendb: $(SRC)
	$(CC) $(SRC) $(INC) $(LIBS) -o nitrogendb

clean:
	rm -f nitrogendb
