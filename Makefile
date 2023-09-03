krekens_overlay: krekens_overlay.cpp
	clang++.exe -o .\krekens_overlay.dll -D "_UNICODE" -D "UNICODE" -shared -Wall .\krekens_overlay.cpp 

klimV2: klimV2.cpp
	clang++.exe -o .\klimV2.exe -D "_UNICODE" -D "UNICODE" -l ".\WinDivert\WinDivert.lib" -I ".\WinDivert\" -Wall .\klimV2.cpp

