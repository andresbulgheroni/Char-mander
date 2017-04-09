################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../socket/cliente.c \
../socket/serializacion_mapa_entrenador.c \
../socket/serializacion_pokedex_cliente_servidor.c \
../socket/server.c \
../socket/servidor-select.c \
../socket/sockets-utils.c 

OBJS += \
./socket/cliente.o \
./socket/serializacion_mapa_entrenador.o \
./socket/serializacion_pokedex_cliente_servidor.o \
./socket/server.o \
./socket/servidor-select.o \
./socket/sockets-utils.o 

C_DEPS += \
./socket/cliente.d \
./socket/serializacion_mapa_entrenador.d \
./socket/serializacion_pokedex_cliente_servidor.d \
./socket/server.d \
./socket/servidor-select.d \
./socket/sockets-utils.d 


# Each subdirectory must supply rules for building sources it contributes
socket/%.o: ../socket/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


