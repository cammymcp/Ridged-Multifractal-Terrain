# DirectX 11 | Ridged-Multifractal Procedural Terrain

This application demonstrates the use of dynamic hardware tessellation and procedural fractal algorithms for efficiently rendering complex terrain in real-time graphics applications. It is built on the DXFramework, but all application-level code and HLSL shaders were my own work. 

The tessellation factor is set dynamically based on patch distance from the camera, and the near and far values of this calculation can be set in real-time by the user. The tessellated geometry is then displaced by a ridged multifractal algorithm to generate realistic mountain terrain, the parameters of which may also be tuned on the fly. Normals are recalculated on the GPU to then light and texture the terrain, with a slope-based system switching dynamically between snow and rock materials. 

For efficiency, the application performs CPU-side frustum culling to only render visible geometry. The effects of which can be seen in the Test Screens folder: frame times dropping by over 100%. There is also a feature to dynamically adjust the near and far tessellation values to reach a target frame rate.

This repository contains everything you need to compile and run this program from Visual Studio. If you wish to do so, follow the steps below:

- Download repository as a .zip file and extract.
- Open Visual Studio solution file
- Right click on the 'DXFramework' project and Build.
- Right click on the 'E01_MultifractalTerrain' project and select as Startup Project
- Run in Debug for Win32. 

Controls:

-WASD - Move Camera
-Arrows - Rotate Camera
-Esc - Close application

If you can't get the program running let me know and I can make some changes. Alternatively, you can find a video of it running here: https://www.youtube.com/watch?v=Ejc-A_pi4VA

For those who are really interested, there is a PDF technical report included in the repository that was submitted alongside this application as part of my degree course. 

