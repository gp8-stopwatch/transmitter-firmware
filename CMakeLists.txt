CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET (CMAKE_VERBOSE_MAKEFILE OFF)
SET (CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

PROJECT (transmitter)
ENABLE_LANGUAGE (ASM-ATT)

INCLUDE_DIRECTORIES("src/")

# Resonator used in this project + arduino macros.
# ADD_DEFINITIONS (-DF_CPU=4000000UL -DBAUD=9600)
ADD_DEFINITIONS (-DF_CPU=4000000UL)
# ADD_DEFINITIONS (-DF_CPU=16000000UL -DBAUD=9600 -DARDUINO=10808 -DARDUINO_AVR_MEGA2560 -DARDUINO_ARCH_AVR -DCMAKE_PROJECT -DPIN_SERIAL1_TX=70)
# add_definitions(-mmcu=avrxmega3)

#/home/iwasz/local/share/arduino-1.8.8/hardware/tools/avr/bin/avr-g++ -c -g -Os -w -std=gnu++11 -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -MMD -flto -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=10808 -DARDUINO_AVR_MEGA2560 -DARDUINO_ARCH_AVR -I/home/iwasz/local/share/arduino-1.8.8/hardware/arduino/avr/cores/arduino -I/home/iwasz/local/share/arduino-1.8.8/hardware/arduino/avr/variants/mega /home/iwasz/local/share/arduino-1.8.8/hardware/arduino/avr/cores/arduino/Tone.cpp -o /tmp/arduino_build_543412/core/Tone.cpp.o

# +-----------------+
# | My code         |
# +-----------------+
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf
        "src/main.cpp"
)

#FIND_PROGRAM (AVRDUDE avrdude)
#ADD_CUSTOM_TARGET("upload" DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${AVRDUDE} -pm2560 -cusbasp -U "flash:w:${CMAKE_PROJECT_NAME}.elf:e")
ADD_CUSTOM_TARGET (${CMAKE_PROJECT_NAME}.hex ALL DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${CMAKE_OBJCOPY} -Oihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex)
