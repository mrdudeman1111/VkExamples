git clone https://github.com/glfw/glfw
mkdir glfwBin || true
cd glfwBin
cmake -S ../glfw ./
make -j 3

cd ../

git clone https://github.com/g-truc/glm

git clone https://github.com/assimp/assimp
mkdir assimpBin || true
cd assimpBin
cmake -S ../assimp ./
make -j 10

cd ../

git clone --recursive https://github.com/HappySeaFox/sail.git
cd sail/extra
./build
cd ../../
mkdir sailBin || true
cd sailBin

if [["$OSTYPE" =~ linux ]]; then
  cmake -A x64 -DCMAKE_INSTALL_PREFIX="C:\SAIL" -DCMAKE_BUILD_TYPE=Release ../sail
  cmake --build . --config Release --target install
else
  sudo cmake --build . --config Release --target install
fi
