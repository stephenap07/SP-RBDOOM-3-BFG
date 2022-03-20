cd ..
del /s /q build
mkdir build
cd build
set VULKAN_SDK=C:\VulkanSDK\1.2.198.1
cmake -G "Visual Studio 17" -A x64 -DWINDOWS10=ON -DPhysX_DIR:PATH=C:/Libs/PhysX/out/PhysX/bin/cmake/physx ../neo
pause