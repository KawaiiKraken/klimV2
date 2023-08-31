.DEFAULT_GOAL := all
#win10sdk := C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\ 

CPATH := C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\ 

all: klimV2


krekens_overlay: krekens_overlay.cpp
	clang++.exe -o .\krekens_overlay.dll -l"kernel32" -l"user32" -D "_UNICODE" -D "UNICODE" -lgdi32 -shared -Wall .\krekens_overlay.cpp 

klimV2: klimV2.cpp
	clang++.exe -o .\klimV2.exe -l"kernel32" -l"user32" -D "_UNICODE" -D "UNICODE" -l ".\WinDivert\WinDivert.lib" -I ".\WinDivert\" -Wall .\klimV2.cpp -v

clean:
	rm 