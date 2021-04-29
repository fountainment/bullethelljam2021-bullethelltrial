git submodule update --init
mkdir build
cd build
cmake ../cherrysoda-engine
cmake --build . --config Release
