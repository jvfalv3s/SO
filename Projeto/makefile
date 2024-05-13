# *********************************************************
#                         AUTHORES
# 
# --> Gonçalo José Carrajola Gaio           Nº: 2022224905
# --> João Vitor Fraga Maia Alves           Nº: 2016122878
# *********************************************************

#inicio da makefile pro projeto de SO.

#MakeFile

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -Wextra -g -pthread

#CFLAGS += -I./AutorizationReqManager
#CFLAGS += -I./LogFileManager
#CFLAGS += -I./MonitorEngine
#CFLAGS += -I./AuthorizationEngines
#CFLAGS += -I./mobileUser

# Nome do executável
TARGET = SystemManager MobileUserExe BackOfficeUser

# Arquivos fonte
SRCS = SystemManager.c AutorizationReqManager/AutorizationReqManager.c LogFileManager/LogFileManager.c MonitorEngine/MonitorEngine.c

# Arquivos objetos
OBJS = $(SRCS:.c=.o)

# Arquivos de cabeçalho
HEADERS = mobileUser/mobileUser.h backOfficeUser/backOfficeUser.h LogFileManager/LogFileManager.h MonitorEngine/MonitorEngine.h AutorizationReqManager/AutorizationReqManager.h  SystemManager/SystemManager.h

# Regra padrão
all: $(TARGET)

MobileUserExe : mobileUser/mobileUser.c 
	$(CC) $(CFLAGS) $^ -o $@ 

BackOfficeUser : backOffice/backOfficeUser.c
	$(CC) $(CFLAGS) $^ -o $@ 

SystemManager : SystemManager.c AutorizationReqManager/AutorizationReqManager.c LogFileManager/LogFileManager.c MonitorEngine/MonitorEngine.c AuthorizationEngines/AuthorizationEngine.c
	$(CC) $(CFLAGS) $^ -o $@
	
# Regra para o alvo principal
#$(TARGET):% : 
#	$(CC) $(CFLAGS) $^ -o $@ 

# Regra para compilar os arquivos objetos
%.o : %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos objetos e o executável
clean:
	@echo "Limpando arquivos"
	rm -f $(OBJS) $(TARGET)
	@echo "Arquivos limpos"