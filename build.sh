cd ThirdParty
./BuildLibraries.sh
cd ../

mkdir build || true
cd build
cmake ..
cd ../

cd HelloTriangle/Shaders
./compile.sh
cd ../../

cd HelloColorTriangle/Shaders
./compile.sh
cd ../../

cd HelloCube/Shaders
./compile.sh
cd ../../

cd HelloCubeCamera/Shaders
./compile.sh
cd ../../

cd HelloTexturedCube/Shaders
./compile.sh
cd ../../

cd build
make -j 3

cd ../ThirdParty
