CFLAGS=-Wall -pedantic -std=gnu99
CC=gcc
GPLC=gplc
TARGET=muccs
LIBS=ccs_scan.o ccs_parse.o libccs.o

all: $(TARGET)

$(TARGET): ccs_scan.c ccs_parse.c libccs.c $(TARGET).c
	$(GPLC) -o $(TARGET) -C -g $(TARGET).c sos.pl ccs_scan.c ccs_parse.c libccs.c -L -lreadline

ccs_scan.c: ccs.l ccs_parse.c
	flex --nounput -o ccs_scan.c ccs.l

ccs_parse.c: ccs.y
	bison -p ccs_ -d -o ccs_parse.c ccs.y

clean:
	rm -f $(TARGET) *.o ccs_scan.* ccs_parse.*

.PHONY: all clean
