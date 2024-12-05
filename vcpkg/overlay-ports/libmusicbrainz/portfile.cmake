vcpkg_from_git(
        OUT_SOURCE_PATH SOURCE_PATH
        URL https://github.com/metabrainz/libmusicbrainz.git
        REF 2adc507e79acec04abb8756e9e07319b980ca9df
)

vcpkg_cmake_configure(
        SOURCE_PATH ${SOURCE_PATH}
        ${OPTIONS}
)

vcpkg_cmake_install()
vcpkg_fixup_pkgconfig()

