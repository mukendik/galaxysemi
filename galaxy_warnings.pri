# find . -name "*.pro"\
# -not -exec grep -q galaxy_warnings {} \;
# -not -exec grep -q galaxy_common {} \;
# -not -exec grep -q subdirs {} \;
# -exec vi {} \;
#
# include($(DEVDIR)/galaxy_warnings.pri)

QMAKE_CXXFLAGS_WARN_ON += -Wextra -Werror #-Wno-unused-parameter
QMAKE_CFLAGS_WARN_ON += -Wextra -Werror

macx|darwin-g++:{
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unknown-pragmas
    QMAKE_CXXFLAGS_WARN_ON += -Wno-nullability-completeness
    QMAKE_CFLAGS_WARN_ON += -Wno-unknown-pragmas
    QMAKE_CFLAGS_WARN_ON += -Wno-nullability-completeness

    # there is some issues of this kind in the Qt framework itself, not under
    # our control
    QMAKE_CXXFLAGS_WARN_ON += -Wno-return-stack-address
}


CONFIG(release, debug|release) {
    ! contains(QMAKE_CXXFLAGS, "-O2") {
        ! contains(QMAKE_CXXFLAGS, "-O1") {
            QMAKE_CXXFLAGS += -O1
        }
    }
    ! contains(QMAKE_CFLAGS, "-O2") {
        ! contains(QMAKE_CFLAGS, "-O1") {
            QMAKE_CFLAGS += -O1
        }
    }
    QMAKE_CXXFLAGS_WARN_ON += -D_FORTIFY_SOURCE=2
    QMAKE_CFLAGS_WARN_ON += -D_FORTIFY_SOURCE=2
}
