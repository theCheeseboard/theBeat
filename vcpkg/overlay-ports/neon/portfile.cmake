vcpkg_download_distfile(ARCHIVE
        URLS "https://notroj.github.io/neon/neon-${VERSION}.tar.gz"
        FILENAME "neon-${VERSION}.tar.gz"
        SHA512 3ac77f6964bda3d3bb6190d982e0573f4e1a3e611afa40be7d79829419a2a1bc787905f12057aa30a4bbe80e8b8efd39408fd886e2e36fc4f7cae12b47ed8f29
)

vcpkg_extract_source_archive_ex(
        OUT_SOURCE_PATH SOURCE_PATH
        ARCHIVE ${ARCHIVE}
)

vcpkg_configure_make(
        SOURCE_PATH ${SOURCE_PATH}
        AUTOCONFIG
        DISABLE_VERBOSE_FLAGS
)

vcpkg_install_make()
vcpkg_fixup_pkgconfig()
