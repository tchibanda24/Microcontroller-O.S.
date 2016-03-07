################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../Sources/PDB.c" \
"../Sources/delay.c" \
"../Sources/flexTimer.c" \
"../Sources/lcdc.c" \
"../Sources/lcdcConsole.c" \
"../Sources/led.c" \
"../Sources/main.c" \
"../Sources/mcg.c" \
"../Sources/myIO.c" \
"../Sources/myInterrupt.c" \
"../Sources/myMem.c" \
"../Sources/mySVC.c" \
"../Sources/nvic.c" \
"../Sources/priv.c" \
"../Sources/process.c" \
"../Sources/profont.c" \
"../Sources/pushbutton.c" \
"../Sources/sdram.c" \
"../Sources/shell.c" \
"../Sources/switchCmd.c" \
"../Sources/uart.c" \
"../Sources/util.c" \

C_SRCS += \
../Sources/PDB.c \
../Sources/delay.c \
../Sources/flexTimer.c \
../Sources/lcdc.c \
../Sources/lcdcConsole.c \
../Sources/led.c \
../Sources/main.c \
../Sources/mcg.c \
../Sources/myIO.c \
../Sources/myInterrupt.c \
../Sources/myMem.c \
../Sources/mySVC.c \
../Sources/nvic.c \
../Sources/priv.c \
../Sources/process.c \
../Sources/profont.c \
../Sources/pushbutton.c \
../Sources/sdram.c \
../Sources/shell.c \
../Sources/switchCmd.c \
../Sources/uart.c \
../Sources/util.c \

OBJS += \
./Sources/PDB.o \
./Sources/delay.o \
./Sources/flexTimer.o \
./Sources/lcdc.o \
./Sources/lcdcConsole.o \
./Sources/led.o \
./Sources/main.o \
./Sources/mcg.o \
./Sources/myIO.o \
./Sources/myInterrupt.o \
./Sources/myMem.o \
./Sources/mySVC.o \
./Sources/nvic.o \
./Sources/priv.o \
./Sources/process.o \
./Sources/profont.o \
./Sources/pushbutton.o \
./Sources/sdram.o \
./Sources/shell.o \
./Sources/switchCmd.o \
./Sources/uart.o \
./Sources/util.o \

OBJS_QUOTED += \
"./Sources/PDB.o" \
"./Sources/delay.o" \
"./Sources/flexTimer.o" \
"./Sources/lcdc.o" \
"./Sources/lcdcConsole.o" \
"./Sources/led.o" \
"./Sources/main.o" \
"./Sources/mcg.o" \
"./Sources/myIO.o" \
"./Sources/myInterrupt.o" \
"./Sources/myMem.o" \
"./Sources/mySVC.o" \
"./Sources/nvic.o" \
"./Sources/priv.o" \
"./Sources/process.o" \
"./Sources/profont.o" \
"./Sources/pushbutton.o" \
"./Sources/sdram.o" \
"./Sources/shell.o" \
"./Sources/switchCmd.o" \
"./Sources/uart.o" \
"./Sources/util.o" \

C_DEPS += \
./Sources/PDB.d \
./Sources/delay.d \
./Sources/flexTimer.d \
./Sources/lcdc.d \
./Sources/lcdcConsole.d \
./Sources/led.d \
./Sources/main.d \
./Sources/mcg.d \
./Sources/myIO.d \
./Sources/myInterrupt.d \
./Sources/myMem.d \
./Sources/mySVC.d \
./Sources/nvic.d \
./Sources/priv.d \
./Sources/process.d \
./Sources/profont.d \
./Sources/pushbutton.d \
./Sources/sdram.d \
./Sources/shell.d \
./Sources/switchCmd.d \
./Sources/uart.d \
./Sources/util.d \

OBJS_OS_FORMAT += \
./Sources/PDB.o \
./Sources/delay.o \
./Sources/flexTimer.o \
./Sources/lcdc.o \
./Sources/lcdcConsole.o \
./Sources/led.o \
./Sources/main.o \
./Sources/mcg.o \
./Sources/myIO.o \
./Sources/myInterrupt.o \
./Sources/myMem.o \
./Sources/mySVC.o \
./Sources/nvic.o \
./Sources/priv.o \
./Sources/process.o \
./Sources/profont.o \
./Sources/pushbutton.o \
./Sources/sdram.o \
./Sources/shell.o \
./Sources/switchCmd.o \
./Sources/uart.o \
./Sources/util.o \

C_DEPS_QUOTED += \
"./Sources/PDB.d" \
"./Sources/delay.d" \
"./Sources/flexTimer.d" \
"./Sources/lcdc.d" \
"./Sources/lcdcConsole.d" \
"./Sources/led.d" \
"./Sources/main.d" \
"./Sources/mcg.d" \
"./Sources/myIO.d" \
"./Sources/myInterrupt.d" \
"./Sources/myMem.d" \
"./Sources/mySVC.d" \
"./Sources/nvic.d" \
"./Sources/priv.d" \
"./Sources/process.d" \
"./Sources/profont.d" \
"./Sources/pushbutton.d" \
"./Sources/sdram.d" \
"./Sources/shell.d" \
"./Sources/switchCmd.d" \
"./Sources/uart.d" \
"./Sources/util.d" \


# Each subdirectory must supply rules for building sources it contributes
Sources/PDB.o: ../Sources/PDB.c
	@echo 'Building file: $<'
	@echo 'Executing target #1 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/PDB.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/PDB.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/delay.o: ../Sources/delay.c
	@echo 'Building file: $<'
	@echo 'Executing target #2 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/delay.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/delay.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/flexTimer.o: ../Sources/flexTimer.c
	@echo 'Building file: $<'
	@echo 'Executing target #3 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/flexTimer.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/flexTimer.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/lcdc.o: ../Sources/lcdc.c
	@echo 'Building file: $<'
	@echo 'Executing target #4 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/lcdc.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/lcdc.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/lcdcConsole.o: ../Sources/lcdcConsole.c
	@echo 'Building file: $<'
	@echo 'Executing target #5 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/lcdcConsole.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/lcdcConsole.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/led.o: ../Sources/led.c
	@echo 'Building file: $<'
	@echo 'Executing target #6 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/led.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/led.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/main.o: ../Sources/main.c
	@echo 'Building file: $<'
	@echo 'Executing target #7 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/main.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/main.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/mcg.o: ../Sources/mcg.c
	@echo 'Building file: $<'
	@echo 'Executing target #8 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/mcg.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/mcg.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/myIO.o: ../Sources/myIO.c
	@echo 'Building file: $<'
	@echo 'Executing target #9 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/myIO.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/myIO.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/myInterrupt.o: ../Sources/myInterrupt.c
	@echo 'Building file: $<'
	@echo 'Executing target #10 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/myInterrupt.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/myInterrupt.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/myMem.o: ../Sources/myMem.c
	@echo 'Building file: $<'
	@echo 'Executing target #11 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/myMem.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/myMem.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/mySVC.o: ../Sources/mySVC.c
	@echo 'Building file: $<'
	@echo 'Executing target #12 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/mySVC.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/mySVC.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/nvic.o: ../Sources/nvic.c
	@echo 'Building file: $<'
	@echo 'Executing target #13 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/nvic.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/nvic.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/priv.o: ../Sources/priv.c
	@echo 'Building file: $<'
	@echo 'Executing target #14 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/priv.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/priv.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/process.o: ../Sources/process.c
	@echo 'Building file: $<'
	@echo 'Executing target #15 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/process.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/process.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/profont.o: ../Sources/profont.c
	@echo 'Building file: $<'
	@echo 'Executing target #16 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/profont.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/profont.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/pushbutton.o: ../Sources/pushbutton.c
	@echo 'Building file: $<'
	@echo 'Executing target #17 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/pushbutton.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/pushbutton.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/sdram.o: ../Sources/sdram.c
	@echo 'Building file: $<'
	@echo 'Executing target #18 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/sdram.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/sdram.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/shell.o: ../Sources/shell.c
	@echo 'Building file: $<'
	@echo 'Executing target #19 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/shell.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/shell.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/switchCmd.o: ../Sources/switchCmd.c
	@echo 'Building file: $<'
	@echo 'Executing target #20 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/switchCmd.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/switchCmd.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/uart.o: ../Sources/uart.c
	@echo 'Building file: $<'
	@echo 'Executing target #21 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/uart.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/uart.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/util.o: ../Sources/util.c
	@echo 'Building file: $<'
	@echo 'Executing target #22 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/util.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/util.o"
	@echo 'Finished building: $<'
	@echo ' '


