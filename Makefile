all:
	@echo nothing

main: main/main.touch

main/main.touch: main/main.ino main/opeEEPROM.ino main/opeNET.ino main/opeRTC.ino main/opeSCH.ino ~/Arduino/libraries/M304_Library/src/M304.cpp
	arduino-cli compile -b arduino:avr:mega main
	touch main/main.touch

inst: main/main.touch 
	arduino-cli upload -p /dev/ttyUSB0 -b arduino:avr:mega main

clean: 
	rm main/main.touch 

