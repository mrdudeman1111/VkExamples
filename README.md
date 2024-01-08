### Vulkan Examples

***

 This repository is a container of example programs that I built to show how I use the Vulkan API.
I was hoping to get a better understanding of vulkan by revisiting simple programs I used to learn the API.
In the end it helped me understand Image/Buffer/memory barriers better, These concepts confused me for a long time so it is good that I got to work with Pipeline barriers more.


### Building

***

 To build this project, run these commands in the base of the project.

> mkdir build && cd build
>
> conan install .. --build=missing
>
> cd Release
>
> cmake ../../ --preset conan-release
>
> make

