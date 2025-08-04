.PHONY: run

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM = linux
    BUILD_FLAGS = $(shell pkg-config --libs glfw3 glew) -lGL -lm -pthread
    EXECUTABLE = slime-mold
    PACKAGE_EXT = tar.gz
else ifneq (,$(findstring MINGW,$(UNAME_S)))
    PLATFORM = windows
    BUILD_FLAGS = -lglfw3 -lglew32 -lopengl32 -lm -pthread
    EXECUTABLE = run.exe
    PACKAGE_EXT = zip
else ifneq (,$(findstring MSYS,$(UNAME_S)))
    PLATFORM = windows
    BUILD_FLAGS = -lglfw3 -lglew32 -lopengl32 -lm -pthread
    EXECUTABLE = run.exe
    PACKAGE_EXT = zip
else
    PLATFORM = unknown
    BUILD_FLAGS = -lglfw3 -lglew32 -lopengl32 -lm -pthread
    EXECUTABLE = run.exe
    PACKAGE_EXT = zip
endif

# Source files
ifeq ($(UNAME_S),Linux)
    SOURCES = src/main.c src/graphics.c src/settings.c src/utils.c
else
    SOURCES = src/main.c src/graphics.c src/settings.c src/utils.c libs/mman.c
endif
INCLUDES = -Iinclude

# Auto-detect platform and build
default: main run

main:
	@echo "Building for $(PLATFORM)..."
	@mkdir -p build
	@gcc $(INCLUDES) $(SOURCES) -o build/$(EXECUTABLE) $(BUILD_FLAGS)

main-strict:
	@echo "Building for $(PLATFORM) with strict warnings..."
	@mkdir -p build
	@gcc -Wall -Werror $(INCLUDES) $(SOURCES) -o build/$(EXECUTABLE) $(BUILD_FLAGS)

debug:
	@echo "Building debug version for $(PLATFORM)..."
	@mkdir -p build
	@gcc $(INCLUDES) $(SOURCES) -g -O0 -o build/$(EXECUTABLE) $(BUILD_FLAGS)

run:
	@./build/$(EXECUTABLE)

# Linux build targets
LINUX_FLAGS = $(shell pkg-config --libs glfw3 glew) -lGL -lm -pthread

linux:
	@mkdir -p build
	@gcc $(INCLUDES) $(SOURCES) -o build/slime-mold $(LINUX_FLAGS)

linux-strict:
	@mkdir -p build
	@gcc -Wall -Werror $(INCLUDES) $(SOURCES) -o build/slime-mold $(LINUX_FLAGS)

linux-debug:
	@mkdir -p build
	@gcc $(INCLUDES) $(SOURCES) -g -O0 -o build/slime-mold $(LINUX_FLAGS)

run-linux:
	@./build/slime-mold


# Smart package target that auto-detects platform
package: main-strict
ifndef VERSION
	$(error VERSION is not set. Use: make package VERSION=x.x.x)
endif
ifeq ($(PLATFORM),linux)
	@$(MAKE) package-linux VERSION=$(VERSION)
else
	@$(MAKE) package-windows VERSION=$(VERSION)
endif

package-minimal: main-strict
ifeq ($(PLATFORM),linux)
	@$(MAKE) package-linux-minimal
else
	@$(MAKE) package-windows-minimal
endif

# Windows packaging targets
package-windows: main-strict
ifndef VERSION
	$(error VERSION is not set. Use: make package-windows VERSION=x.x.x)
endif
	@echo "Creating Windows package v$(VERSION)..."
	@rm -rf slime-mold-v$(VERSION)-windows-x64.zip
	@mkdir -p package_temp/bin
	@mkdir -p package_temp/libs
	@cp build/run.exe package_temp/bin/
	@cp /clang64/bin/glfw3.dll package_temp/libs/ 2>/dev/null || echo "glfw3.dll not found"
	@cp /clang64/bin/glew32.dll package_temp/libs/ 2>/dev/null || echo "glew32.dll not found"
	@cp /clang64/bin/libwinpthread-1.dll package_temp/libs/ 2>/dev/null || echo "libwinpthread-1.dll not found"
	@cp /clang64/bin/libgcc_s_seh-1.dll package_temp/libs/ 2>/dev/null || cp /mingw64/bin/libgcc_s_seh-1.dll package_temp/libs/ 2>/dev/null || cp /ucrt64/bin/libgcc_s_seh-1.dll package_temp/libs/ 2>/dev/null || true
	@cp /clang64/bin/libstdc++-6.dll package_temp/libs/ 2>/dev/null || cp /mingw64/bin/libstdc++-6.dll package_temp/libs/ 2>/dev/null || cp /ucrt64/bin/libstdc++-6.dll package_temp/libs/ 2>/dev/null || true
	@cp -r shaders/ package_temp/
	@cp -r data/ package_temp/
	@cp README.md package_temp/
	@echo "@echo off" > package_temp/run.bat
	@echo "set PATH=%~dp0libs;%PATH%" >> package_temp/run.bat
	@echo "cd /d %~dp0" >> package_temp/run.bat
	@echo "bin\run.exe" >> package_temp/run.bat
	@cd package_temp && zip -r ../slime-mold-v$(VERSION)-windows-x64.zip .
	@rm -rf package_temp
	@echo "Package created: slime-mold-v$(VERSION)-windows-x64.zip"

package-windows-minimal: main-strict
	@echo "Creating minimal Windows package..."
	@rm -rf slime-mold-minimal-windows-x64.zip
	@mkdir -p package_temp/bin
	@cp build/run.exe package_temp/bin/
	@cp -r shaders/ package_temp/
	@cp -r data/ package_temp/
	@cp README.md package_temp/
	@echo "@echo off" > package_temp/run.bat
	@echo "cd /d %~dp0" >> package_temp/run.bat
	@echo "bin\run.exe" >> package_temp/run.bat
	@cd package_temp && zip -r ../slime-mold-minimal-windows-x64.zip .
	@rm -rf package_temp
	@echo "Package created: slime-mold-minimal-windows-x64.zip"

# Linux packaging targets
package-linux: linux-strict
ifndef VERSION
	$(error VERSION is not set. Use: make package-linux VERSION=x.x.x)
endif
	@echo "Creating Linux package v$(VERSION)..."
	@rm -rf slime-mold-v$(VERSION)-linux-x64.tar.gz
	@mkdir -p slime-mold-v$(VERSION)-linux-x64
	@cp build/slime-mold slime-mold-v$(VERSION)-linux-x64/
	@cp -r shaders/ slime-mold-v$(VERSION)-linux-x64/
	@cp -r data/ slime-mold-v$(VERSION)-linux-x64/
	@cp README.md slime-mold-v$(VERSION)-linux-x64/
	@tar czf slime-mold-v$(VERSION)-linux-x64.tar.gz slime-mold-v$(VERSION)-linux-x64
	@rm -rf slime-mold-v$(VERSION)-linux-x64
	@rm -rf package_temp
	@echo "Package created: slime-mold-v$(VERSION)-linux-x64.tar.gz"

package-linux-minimal: linux-strict
	@echo "Creating minimal Linux package..."
	@rm -rf slime-mold-minimal-linux-x64.tar.gz
	@mkdir -p slime-mold-minimal-linux-x64
	@cp build/slime-mold slime-mold-minimal-linux-x64/
	@cp -r shaders/ slime-mold-minimal-linux-x64/
	@cp -r data/ slime-mold-minimal-linux-x64/
	@cp README.md slime-mold-minimal-linux-x64/
	@tar czf slime-mold-minimal-linux-x64.tar.gz slime-mold-minimal-linux-x64
	@rm -rf slime-mold-minimal-linux-x64
	@echo "Package created: slime-mold-minimal-linux-x64.tar.gz"
