include Makefile.$(shell uname)

ROOT=./
CC_FLAGS=-ansi -I . -I ../UserApplication/KCG/ -I ../../../CommunicationServer/ -I "C:/Program Files (x86)/Esterel Technologies/Esterel SCADE 6.4.2/SCADE/" -I "C:/Program Files (x86)/Esterel Technologies/Esterel SCADE 6.4.2/SCADE/include" -Wall $(OS_D)
SRC=Launch_ts01UA.c ../UserApplication/KCG/kcg_consts.c ../UserApplication/KCG/kcg_types.c ../UserApplication/KCG/MyOperator.c ../UserApplication/KCG/ua.c

OBJ=$(SRC:.c=.o)

ua$(OS_EXE_EXT): $(OBJ)
	$(CC) -o $@ $(OBJ) $(OS_LIBS)

libua.a: $(OBJ)
	ar rcs $@ $(OBJ)

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) ua$(OS_EXE_EXT)

.PHONY: clean
