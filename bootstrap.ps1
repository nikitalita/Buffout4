cmake -B build -S . "-DCMAKE_TOOLCHAIN_FILE=$Env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" "-DVCPKG_OVERLAY_PORTS=ports" "-DVCPKG_OVERLAY_TRIPLETS=triplets" "-DVCPKG_TARGET_TRIPLET=x64-windows-static-custom"
Read-Host -Prompt "Press Enter to exit"
