vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO oneapi-src/oneTBB
	REF v2021.3.0
	SHA512 969bc8d1dcf50bd12f70633d0319e46308eb1667cdc6f0503b373a35dcb2fe6b2adf59c26bd3c8e2a99a8d2d8b9f64088db5a43e784218b163b3661d12908c0e
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
