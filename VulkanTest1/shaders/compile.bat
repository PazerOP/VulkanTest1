@echo off
"C:\Program Files\VulkanSDK\1.0.46.0\Bin\glslangvalidator.exe" -V -S frag -o simple_pixel.spv simple_pixel.glsl
"C:\Program Files\VulkanSDK\1.0.46.0\Bin\glslangvalidator.exe" -V -S vert -o simple_vertex.spv simple_vertex.glsl
pause