mkdir -p bin/debug/linux
cp -u shaders bin/debug/linux -d -r
cp -u textures bin/debug/linux -d -r
cp -u fonts bin/debug/linux -d -r
gcc -o bin/debug/linux/main src/*.c -Ilibs -lfreetype  -Iinclude/ -Iinclude/freetype2 -lglfw -lGLEW -lGL -msse2 -lm -ldl
./bin/debug/linux/main