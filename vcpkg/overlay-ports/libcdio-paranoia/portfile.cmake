vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/libcdio/libcdio-paranoia.git
    REF 9086b36d2b434af3eb7977e17653b41d01da92b2
    PATCHES
        remove-docs-tests-examples.patch
        remove-attribute-unused.patch
        add-stderr_fileno-define.patch
        guard-unistd-include.patch
)

vcpkg_configure_make(
    SOURCE_PATH ${SOURCE_PATH}
    AUTOCONFIG
)

vcpkg_install_make()
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
