.PHONY: run

default: main run

FLAGS=-lglfw3 -lglew32 -lopengl32 -lm -pthread

main:
	@gcc main.c libs/mman.c -o run.exe $(FLAGS)

main-strict:
	@gcc -Wall -Werror main.c libs/mman.c -o run.exe $(FLAGS)

debug:
	@gcc main.c libs/mman.c -g -O0 -o run.exe $(FLAGS)

run:
	@./run.exe

package: main-strict
	@echo "Creating portable package..."
	@rm -rf package.zip
	@mkdir -p package_temp/bin
	@mkdir -p package_temp/libs
	@cp run.exe package_temp/bin/
	@cp /mingw64/bin/glfw3.dll package_temp/libs/ 2>/dev/null || echo "glfw3.dll not found"
	@cp /mingw64/bin/glew32.dll package_temp/libs/ 2>/dev/null || echo "glew32.dll not found"
	@cp /mingw64/bin/libwinpthread-1.dll package_temp/libs/ || echo "libwinpthread-1.dll not found"
	@cp /mingw64/bin/libgcc_s_seh-1.dll package_temp/libs/ 2>/dev/null || echo "libgcc not found"
	@cp /mingw64/bin/libstdc++-6.dll package_temp/libs/ 2>/dev/null || echo "libstdc++ not found"
	@cp -r save/ package_temp/ 2>/dev/null || echo "save folder not found"
	@cp -r shaders/ package_temp/
	@cp README.txt package_temp/
	@echo "@echo off" > package_temp/run.bat
	@echo "set PATH=%~dp0libs;%PATH%" >> package_temp/run.bat
	@echo "cd /d %~dp0" >> package_temp/run.bat
	@echo "bin\run.exe" >> package_temp/run.bat
	@cd package_temp && zip -r ../package.zip .
	@rm -rf package_temp