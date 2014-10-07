ROOT=./
CC_FLAGS=-ansi -I . -I ../UserApplication/KCG/ -I ../../../CommunicationFramework/ -I "C:/Program Files (x86)/Esterel Technologies/Esterel SCADE 6.4.2/SCADE/" -I "C:/Program Files (x86)/Esterel Technologies/Esterel SCADE 6.4.2/SCADE/include" -Wall -Wno-unused-variable -Wno-unused-function -Wno-unused-value
SRC=ts02UA.c ../../../CommunicationFramework/CommunicationFramework.c ../UserApplication/KCG/kcg_consts.c ../UserApplication/KCG/kcg_types.c ../UserApplication/KCG/MyOperator.c ../UserApplication/KCG/ua.c

OBJ=$(SRC:.c=.o)

ts02UA.exe: $(OBJ)
	gcc -o $@ $(OBJ)

.c.o:
	gcc $(CC_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) ts02UA.exe

.PHONY: clean
