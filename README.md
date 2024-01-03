Belegarbeit 2023

Vulkan und OpenGL Benchmark


Um das Projekt zu compilen benötigt es eine gültige installation von conan (https://conan.io/) und cmake.
Getestet wurde es auf Linux und Windows

1. git clone https://github.com/knimix/Belegarbeit.git
2. cd Belegarbeit/
3. conan install conanfile.py --build=missing
4. cd build/
5. cmake .. -DCMAKE_TOOLCHAIN_FILE=Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
6. cmake --build .
