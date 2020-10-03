cmake -B"build" -S"." -A"x64" -D"CMAKE_TOOLCHAIN_FILE=$Env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -D"VCPKG_OVERLAY_PORTS=ports;$Env:CommonLibF4Path/ports" -D"VCPKG_OVERLAY_TRIPLETS=triplets" -D"VCPKG_TARGET_TRIPLET=x64-windows-static-custom"
Read-Host -Prompt "Press Enter to exit"
