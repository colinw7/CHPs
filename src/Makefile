CC = g++
AR = ar
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = ../obj
LIB_DIR = ../lib
BIN_DIR = ../bin

all: $(BIN_DIR)/CHPs

SRC = \
CHPs.cpp \
CPs.cpp

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

LIBS =

CPPFLAGS = \
-I. \
-I$(INC_DIR) \
-I../../CStrUtil/include \
-I../../CGlob/include \
-I../../CUtil/include \
-I../../COS/include \

LFLAGS = \
-L$(LIB_DIR) \
-L../../CGlob/lib \
-L../../CStrUtil/lib \
-lCGlob -lCStrUtil

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(BIN_DIR)/CHPs

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

.SUFFIXES: .cpp

$(BIN_DIR)/CHPs: $(OBJ_DIR)/CHPs.o $(OBJ_DIR)/CPs.o
	$(CC) $(LDEBUG) -o $(BIN_DIR)/CHPs $(OBJ_DIR)/CHPs.o $(OBJ_DIR)/CPs.o $(LFLAGS)
