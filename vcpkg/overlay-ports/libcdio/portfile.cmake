vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://git.savannah.gnu.org/git/libcdio.git
    REF 9c7a2779846da161279bc1501e83c849cf89a594
    PATCHES
        remove-off_t-override.patch
        remove-unistd-include.patch
        remove-printf-define.patch
        remove-pretty-function.patch
        remove-docs-tests-examples.patch
        require-stdbool.patch
        rename-in-macro.patch
        fix-globals-linkage.patch
        fix-varargs-macro.patch
        disable-mmc-tool.patch
)

vcpkg_configure_make(
    SOURCE_PATH ${SOURCE_PATH}
    AUTOCONFIG
    OPTIONS
        --without-cd-drive
        --without-cd-info
        --without-cdda-player
        --without-cd-read
        --without-iso-info
        --without-iso-read
        --without-mmc-tool
)

vcpkg_install_make()
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

vcpkg_install_copyright(FILE_LIST ${SOURCE_PATH}/COPYING)