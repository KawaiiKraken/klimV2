.PHONY: clean cleanup all fresh release 
CC=clang++.exe
objects := klim.o helperFunctions.o
BUILD_DIR := .\build
SRC_DIR := .\src
RELEASE_DIR := .\release
INC_DIRS := .\phnt,.\WinDivert,${SRC_DIR}
INC_FLAGS := -I,$(INC_DIRS)

all: klim.exe krekens_overlay.dll cleanup

klim.o: ${SRC_DIR}\main.cpp
	${CC} -o ${BUILD_DIR}\main.o -D "_UNICODE" -D "UNICODE" -Wall -c ${INC_FLAGS} ${SRC_DIR}\main.cpp 
	
helperFunctions.o: 
	${CC} -o ${BUILD_DIR}\helperFunctions.o -D "_UNICODE" -D "UNICODE" -Wall -c ${INC_FLAGS} ${SRC_DIR}\helperFunctions.cpp 

klim.exe: ${objects}
	${CC} -o ${BUILD_DIR}\klim.exe -l ".\Windivert\Windivert.lib" ${BUILD_DIR}\main.o ${BUILD_DIR}\helperFunctions.o 

krekens_overlay.dll: ${SRC_DIR}\krekens_overlay.cpp
	${CC} -o ${BUILD_DIR}\krekens_overlay.dll -D "_UNICODE" -D "UNICODE" -shared -Wall ${SRC_DIR}\krekens_overlay.cpp 

cleanup:
	-del /F /Q ${BUILD_DIR}\*.o
	-del /F /Q ${BUILD_DIR}\*.exp
	-del /F /Q ${BUILD_DIR}\*.lib
	
clean:
	-del /F /Q ${BUILD_DIR}\*

fresh: clean all cleanup

release: ${BUILD_DIR}\krekens_overlay.dll ${BUILD_DIR}\klim.exe
	-mkdir release
	-copy ${BUILD_DIR}\krekens_overlay.dll ${RELEASE_DIR}\ 
	-copy ${BUILD_DIR}\klim.exe ${RELEASE_DIR}\ 
	-copy .\WinDivert\WinDivert.dll ${RELEASE_DIR}\ 
	-copy .\WinDivert\WinDivert64.sys ${RELEASE_DIR}\ 
	
test:
	-cmd.exe /k "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
	mt.exe -nologo -manifest "..\klimV2.exe.manifest" -outputresource:"klimV2.exe;#1"

	
