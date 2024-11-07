## [Ace Engine](https://github.com/volcoma/ace) - Cross-platform C++ Game Engine

## Platform
![windows](https://github.com/volcoma/ace/actions/workflows/windows.yml/badge.svg)
![linux](https://github.com/volcoma/ace/actions/workflows/linux.yml/badge.svg)
![macos](https://github.com/volcoma/ace/actions/workflows/macos.yml/badge.svg)

## Info
Using c++20

WYSIWYG Editor

## Status
WIP - not production ready in any way

![Screenshot 2024-10-12 153527](https://github.com/user-attachments/assets/dacb054b-13c8-49e8-a757-dd43bdd8401a)
![Screenshot 2024-10-12 151734](https://github.com/user-attachments/assets/7eed707b-35fb-41f8-8831-4a235bd9934f)

## Building
Don't forget to update submodules
```
git clone https://github.com/volcoma/ace.git
cd ace
git submodule update --init --recursive

mkdir build
cd build
cmake ..

```
## LIBRARIES
bgfx - https://github.com/bkaradzic/bgfx

cereal - https://github.com/USCiLab/cereal

rttr - https://github.com/rttrorg/rttr

spdlog - https://github.com/gabime/spdlog

imgui - https://github.com/ocornut/imgui

assimp - https://github.com/assimp/assimp

glm - https://github.com/g-truc/glm

openal-soft = https://github.com/kcat/openal-soft
