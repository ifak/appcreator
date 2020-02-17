!isEmpty(APPCREATOR_PRI_INCLUDED):error("appcreator.pri already included")
APPCREATOR_PRI_INCLUDED = 1

include(../appconfig.pri)

isEmpty(APP_NAME) {
APP_NAME = "App Creator"
}

isEmpty(APP_VERSION) {
APP_VERSION = 0.0.1
}

QTCREATOR_VERSION = 2.8.1

isEmpty(IDE_APP_PATH) {
IDE_APP_PATH = $$shadowed($$PWD)/build/
}

isEmpty(IDE_APP_TARGET) {
IDE_APP_TARGET = "AppCreator"
}

isEmpty(IDE_LIBRARY_BASENAME) {
    IDE_LIBRARY_BASENAME = lib
}

isEmpty(QTC_PLUGIN_PATHS) {
  QTC_PLUGIN_PATHS *= {PWD}/src/plugins
}

isEqual(QT_MAJOR_VERSION, 5) {

defineReplace(cleanPath) {
    return($$clean_path($$1))
}

defineReplace(targetPath) {
    return($$shell_path($$1))
}

} else { # qt5

defineReplace(cleanPath) {
    win32:1 ~= s|\\\\|/|g
    contains(1, ^/.*):pfx = /
    else:pfx =
    segs = $$split(1, /)
    out =
    for(seg, segs) {
        equals(seg, ..):out = $$member(out, 0, -2)
        else:!equals(seg, .):out += $$seg
    }
    return($$join(out, /, $$pfx))
}

defineReplace(targetPath) {
    return($$replace(1, /, $$QMAKE_DIR_SEP))
}

} # qt5

defineReplace(qtLibraryName) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   CONFIG(debug, debug|release) {
      !debug_and_release|build_pass {
          mac:RET = $$member(LIBRARY_NAME, 0)_debug
              else:win32:RET = $$member(LIBRARY_NAME, 0)d
      }
   }
   isEmpty(RET):RET = $$LIBRARY_NAME
   return($$RET)
}

defineTest(minQtVersion) {
    maj = $$1
    min = $$2
    patch = $$3
    isEqual(QT_MAJOR_VERSION, $$maj) {
        isEqual(QT_MINOR_VERSION, $$min) {
            isEqual(QT_PATCH_VERSION, $$patch) {
                return(true)
            }
            greaterThan(QT_PATCH_VERSION, $$patch) {
                return(true)
            }
        }
        greaterThan(QT_MINOR_VERSION, $$min) {
            return(true)
        }
    }
    greaterThan(QT_MAJOR_VERSION, $$maj) {
        return(true)
    }
    return(false)
}

isEqual(QT_MAJOR_VERSION, 5) {

# For use in custom compilers which just copy files
defineReplace(stripSrcDir) {
    return($$relative_path($$absolute_path($$1, $$OUT_PWD), $$_PRO_FILE_PWD_))
}

} else { # qt5

# For use in custom compilers which just copy files
win32:i_flag = i
defineReplace(stripSrcDir) {
    win32 {
        !contains(1, ^.:.*):1 = $$OUT_PWD/$$1
    } else {
        !contains(1, ^/.*):1 = $$OUT_PWD/$$1
    }
    out = $$cleanPath($$1)
    out ~= s|^$$re_escape($$_PRO_FILE_PWD_/)||$$i_flag
    return($$out)
}

} # qt5

INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/shared \
    $$PWD/src/libs \
    $$PWD/src/plugins \
    $$PWD/src/tools

CONFIG += depend_includepath

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
!macx:DEFINES += QT_USE_FAST_OPERATOR_PLUS QT_USE_FAST_CONCATENATION

win32-msvc* {
    #Don't warn about sprintf, fopen etc being 'unsafe'
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += _SCL_SECURE_NO_WARNINGS
}

qt:greaterThan(QT_MAJOR_VERSION, 4) {
    contains(QT, core): QT += concurrent
    contains(QT, gui): QT += widgets
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x040900
}

QMAKE_LIBDIR += $$IDE_APP_PATH/lib
QMAKE_LIBDIR += $$IDE_APP_PATH/lib/qtcreator

IDE_SOURCE_TREE = $$PWD
isEmpty(IDE_BUILD_TREE) {
    IDE_BUILD_TREE=$$shadowed($$PWD)
}

macx {
    IDE_LIBRARY_PATH = $$IDE_APP_PATH/$${IDE_APP_TARGET}.app/Contents/PlugIns
    IDE_PLUGIN_PATH  = $$IDE_LIBRARY_PATH
    IDE_LIBEXEC_PATH = $$IDE_APP_PATH/$${IDE_APP_TARGET}.app/Contents/Resources
    IDE_DATA_PATH    = $$IDE_APP_PATH/$${IDE_APP_TARGET}.app/Contents/Resources
    IDE_DOC_PATH     = $$IDE_DATA_PATH/doc
    IDE_BIN_PATH     = $$IDE_APP_PATH/$${IDE_APP_TARGET}.app/Contents/MacOS
    copydata = 1
    isEmpty(TIGER_COMPAT_MODE):TIGER_COMPAT_MODE=$$(QTC_TIGER_COMPAT)
    !isEqual(QT_MAJOR_VERSION, 5) {
        # Qt5 doesn't support 10.5, and will set the minimum version correctly to 10.6 or 10.7.
        isEmpty(TIGER_COMPAT_MODE) {
            QMAKE_CXXFLAGS *= -mmacosx-version-min=10.5
            QMAKE_LFLAGS *= -mmacosx-version-min=10.5
        }
    }
} else {
    contains(TEMPLATE, vc.*):vcproj = 1
    IDE_LIBRARY_PATH = $$IDE_APP_PATH/$$IDE_LIBRARY_BASENAME/qtcreator
    IDE_PLUGIN_PATH  = $$IDE_LIBRARY_PATH/plugins
    IDE_LIBEXEC_PATH = $$IDE_APP_PATH # FIXME
    IDE_DATA_PATH    = $$IDE_APP_PATH/share/qtcreator
    IDE_DOC_PATH     = $$IDE_APP_PATH/share/doc/qtcreator
    IDE_BIN_PATH     = $$IDE_APP_PATH/bin
    !isEqual(IDE_SOURCE_TREE, $$IDE_APP_PATH):copydata = 1
}

INCLUDEPATH += \
    $$IDE_BUILD_TREE/src \ # for <app/app_version.h>
    $$IDE_SOURCE_TREE/src/libs \
    $$IDE_SOURCE_TREE/tools \
    $$IDE_SOURCE_TREE/src/plugins

CONFIG += depend_includepath

LIBS += -L$$IDE_LIBRARY_PATH

!isEmpty(vcproj) {
    DEFINES += IDE_LIBRARY_BASENAME=\"$$IDE_LIBRARY_BASENAME\"
} else {
    DEFINES += IDE_LIBRARY_BASENAME=\\\"$$IDE_LIBRARY_BASENAME\\\"
}

DEFINES += QT_CREATOR QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

#CONFIG(release):DEFINES += QT_NO_DEBUG_OUTPUT

!macx:DEFINES += QT_USE_FAST_OPERATOR_PLUS QT_USE_FAST_CONCATENATION

# recursively resolve plugin deps
done_plugins =
for(ever) {
#  message(recursively resolving plugin deps startet!)

  isEmpty(QTC_PLUGIN_DEPENDS): \
      break()

  done_plugins += $$QTC_PLUGIN_DEPENDS
  for(dep, QTC_PLUGIN_DEPENDS) {
    dep_file = $${dep}_dependencies.pri
#    message(dep_file: $$dep_file)
    dep_file_found = 0

    for(path, QTC_PLUGIN_PATHS) {
      abs_dep_file = $${path}/$$dep/$${dep_file}
#      message(abs_dep_file: $$abs_dep_file)

      exists($$abs_dep_file) {
#        message($${abs_dep_file} found!)
        dep_file_found = 1
        include($$abs_dep_file)
        LIBS += -l$$qtLibraryName($$QTC_PLUGIN_NAME)
        break()
      }
    }
    contains($${dep_file_found}, 0) {
      error($${abs_dep_file} not found!)
    }
  }

  QTC_PLUGIN_DEPENDS = $$unique(QTC_PLUGIN_DEPENDS)
  QTC_PLUGIN_DEPENDS -= $$unique(done_plugins)
}

# recursively resolve library deps
done_libs =
for(ever) {
    isEmpty(QTC_LIB_DEPENDS): \
        break()
    done_libs += $$QTC_LIB_DEPENDS
    for(dep, QTC_LIB_DEPENDS) {
        include($$PWD/src/libs/$$dep/$${dep}_dependencies.pri)
        LIBS += -l$$qtLibraryName($$QTC_LIB_NAME)
    }
    QTC_LIB_DEPENDS = $$unique(QTC_LIB_DEPENDS)
    QTC_LIB_DEPENDS -= $$unique(done_libs)
}
