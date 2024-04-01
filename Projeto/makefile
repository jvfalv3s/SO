# *********************************************************
#                         AUTHORES
# 
# --> Gonçalo José Carrajola Gaio           Nº: 2022224905
# --> João Vitor Fraga Maia Alves           Nº: 2016122878
# *********************************************************

#inicio da makefile pro projeto de SO.

#MakeFile

#Compilador
CC = gcc

#Flags de compilação
CFLAGS = -Wall -Wextra

#Nome do executável
TARGET = SystemManager

#Arquivos fonte
SOURCES = SystemManager.c mobileUser.c backOfficeUser.c AutorizationReqManager.c LogFileManager.c MonitorEngine.c

#Arquivos objetos
OBJS = $(SRCS:.c=.o)

#Arquivos de cabeçalho
HEADERS = mobileUser.h LogFileManager.h MonitorEngine.h AutorizationReqManager.h backOfficeUser.h SystemManager.h

all : $(TARGET)

#Regra Padrao
$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

#Regra para compilar os arquivos objetos
%.o : %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

#Regra para limpar os arquivos objetos
clean:
	rm -f $(OBJS) $(TARGET)



main : SystemManager.c
	gcc -Wall -Wextra SystemManager.c -o SystemManager

mobileUser : mobileUser.c
	gcc -Wall -Wextra mobileUser.c mobileUser.h -o mobileUser

backOfficeUser : backOfficeUser.c
	gcc -Wall -Wextra backOfficeUser.c -o backOfficeUser

AutorizationReqManager : AutorizationReqManager.c
	gcc -Wall -Wextra AutorizationReqManager.c -o AutorizationReqManager

LogFileManager : LogFileManager.c
	gcc -Wall -Wextra LogFileManager.c -o LogFileManager

MonitorEngine : MonitorEngine.c
	gcc -Wall -Wextra MonitorEngine.c -o MonitorEngine

clean:
	@echo "Limpando arquivos"
	rm -f mobileUser
	rm -f bacOfficeUser
	rm -f AutorizationReqManager
	rm -f LogFileManager
	rm -f MonitorEngine
	@echo "Arquivos limpos"

