CC=clang++.exe
objects = .\klim.o

klim.o: klimV2.cpp
	${CC} -o klim.o -D "_UNICODE" -D "UNICODE" -l ".\WinDivert\WinDivert.lib" -Wall -c .\klimV2.cpp 

klim: ${objects}
	${CC} -o klimV2.exe klim.o -l ".\Windivert\Windivert.lib" -v

overlay: krekens_overlay.cpp
	${CC} -o .\krekens_overlay.dll -D "_UNICODE" -D "UNICODE" -shared -Wall .\krekens_overlay.cpp 

clean:
	-del ${objects}

