cd ..
del /s /q build
mkdir build
cd build
cmake -G "Visual Studio 17" -A x64 -DWINDOWS10=ON -DDOOM_CLASSIC=OFF ../neo
pause