vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO oneapi-src/oneTBB
	REF v2021.5.0
	SHA512 0e7b71022e397a6d7abb0cea106847935ae79a1e12a6976f8d038668c6eca8775ed971202c5bd518f7e517092b67af805cc5feb04b5c3a40e9fbf972cc703a46
	HEAD_REF master
)

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}"
	OPTIONS
		-DTBB_TEST=OFF
		-DTBB_STRICT=OFF
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(
	PACKAGE_NAME TBB
	CONFIG_PATH lib/cmake/TBB
)

file(REMOVE_RECURSE
	"${CURRENT_PACKAGES_DIR}/debug/include"
	"${CURRENT_PACKAGES_DIR}/debug/share"
)

file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
