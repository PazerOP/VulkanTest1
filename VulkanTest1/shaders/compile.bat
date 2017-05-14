@echo off
glslangvalidator -V -H -S frag -o simple_pixel.spv simple_pixel.glsl
glslangvalidator -V -H -S vert -o simple_vertex.spv simple_vertex.glsl
pause