CC = gcc
CFLAGS = -Wall -Wextra -g

PREF_SRC = ./src/
PREF_OBJ = ./obj/

SRC = $(wildcard $(PREF_SRC)*.c)
OBJ = $(patsubst $(PREF_SRC)%.c, $(PREF_OBJ)%.o, $(SRC))
TARGET = ipk-l4-scan

$(TARGET) : $(OBJ) 
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) -lpcap

$(PREF_OBJ)%.o : $(PREF_SRC)%.c  | $(PREF_OBJ) 
	$(CC) $(CFLAGS) -c $< -o $@ 

$(PREF_OBJ):
	mkdir -p $(PREF_OBJ)

clean:
	rm $(TARGET) $(PREF_OBJ)*.o
