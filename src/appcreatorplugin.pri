TARGET = $$QTC_PLUGIN_NAME

plugin_deps = $$QTC_PLUGIN_DEPENDS
plugin_recmds = $$QTC_PLUGIN_RECOMMENDS

include(../appcreator.pri)

plugin_paths = $$QTC_PLUGIN_PATHS

# for substitution in the .pluginspec
dependencyList = "<dependencyList>"

for(dep, plugin_deps) {
#  message(substitution of dependecies startet!)
  dep_file = $${dep}_dependencies.pri
  dep_file_found = 0

  for(path, plugin_paths) {
    abs_dep_file = $${path}/$$dep/$${dep_file}
#    message(abs_dep_file: $$abs_dep_file)

    exists($$abs_dep_file) {
#      message($$abs_dep_file found!)
      dep_file_found = 1
      include($$abs_dep_file)
      dependencyList += "        <dependency name=\"$$QTC_PLUGIN_NAME\" version=\"$$QTCREATOR_VERSION\"/>"
      break()
    }
  }
  contains($$dep_file_found, 0) {
    error($${abs_dep_file} not found)
  }
}

for(dep, plugin_recmds) {
  dep_file = $${dep}_dependencies.pri
  dep_file_found = 0

  for(path, plugin_paths) {
    abs_dep_file = $${path}/$$dep/$${dep_file}
#    message(abs_dep_file: $$abs_dep_file)

    exists($$abs_dep_file) {
#      message($${abs_dep_file} found!)
      dep_file_found = 1
      include($$abs_dep_file)
      dependencyList += "        <dependency name=\"$$QTC_PLUGIN_NAME\" version=\"$$QTCREATOR_VERSION\" type=\"optional\"/>"
      break
    }
  }
  contains($$dep_file_found, 0) {
    error($${abs_dep_file} not found)
  }
}

dependencyList += "    </dependencyList>"
dependencyList = $$join(dependencyList, $$escape_expand(\\n))

DESTDIR = $${IDE_APP_PATH}/lib/plugins

isEmpty(PROVIDER) {
    PROVIDER = ifak
} else {
    LIBS += -L$$$${IDE_APP_PATH}/lib/plugins
}
LIBS += -L$$DESTDIR

# copy the plugin spec
isEmpty(TARGET) {
    error("appcreatorplugin.pri: You must provide a TARGET")
}

isEqual(QT_MAJOR_VERSION, 5) {

defineReplace(stripOutDir) {
    return($$relative_path($$1, $$OUT_PWD))
}

} else { # qt5

defineReplace(stripOutDir) {
    1 ~= s|^$$re_escape($$OUT_PWD/)||$$i_flag
    return($$1)
}

} # qt5

PLUGINSPEC = $$_PRO_FILE_PWD_/$${TARGET}.pluginspec
PLUGINSPEC_IN = $${PLUGINSPEC}.in
exists($$PLUGINSPEC_IN) {
    OTHER_FILES += $$PLUGINSPEC_IN
    QMAKE_SUBSTITUTES += $$PLUGINSPEC_IN
    PLUGINSPEC = $$OUT_PWD/$${TARGET}.pluginspec
    copy2build.output = $$DESTDIR/${QMAKE_FUNC_FILE_IN_stripOutDir}
} else {
    # need to support that for external plugins
    OTHER_FILES += $$PLUGINSPEC
    copy2build.output = $$DESTDIR/${QMAKE_FUNC_FILE_IN_stripSrcDir}
}
copy2build.input = PLUGINSPEC
isEmpty(vcproj):copy2build.variable_out = PRE_TARGETDEPS
copy2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
copy2build.name = COPY ${QMAKE_FILE_IN}
copy2build.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += copy2build

greaterThan(QT_MAJOR_VERSION, 4) {
#   Create a Json file containing the plugin information required by
#   Qt 5's plugin system by running a XSLT sheet on the
#   pluginspec file before moc runs.
    XMLPATTERNS = $$targetPath($$[QT_INSTALL_BINS]/xmlpatterns)

    pluginspec2json.name = Create Qt 5 plugin json file
    pluginspec2json.input = PLUGINSPEC
    pluginspec2json.variable_out = GENERATED_FILES
    pluginspec2json.output = $${TARGET}.json
    pluginspec2json.commands = $$XMLPATTERNS -no-format -output $$pluginspec2json.output $$PWD/pluginjsonmetadata.xsl $$PLUGINSPEC
    pluginspec2json.CONFIG += no_link
    moc_header.depends += $$pluginspec2json.output
    QMAKE_EXTRA_COMPILERS += pluginspec2json
}

macx {
    !isEmpty(TIGER_COMPAT_MODE) {
        QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../PlugIns/$${PROVIDER}/
    } else {
        QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/PlugIns/$${PROVIDER}/
        QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../../,-rpath,@executable_path/../
    }
} else:linux-* {
    #do the rpath by hand since it's not possible to use ORIGIN in QMAKE_RPATHDIR
    QMAKE_RPATHDIR += \$\$ORIGIN
    QMAKE_RPATHDIR += \$\$ORIGIN/..
    QMAKE_RPATHDIR += \$\$ORIGIN/../..
    IDE_PLUGIN_RPATH = $$join(QMAKE_RPATHDIR, ":")
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$${IDE_PLUGIN_RPATH}\'
    QMAKE_RPATHDIR =
}
# put .pro file directory in INCLUDEPATH
CONFIG += include_source_dir

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

TEMPLATE = lib
CONFIG += plugin plugin_with_soname
linux*:QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF

MIMETYPES = $$_PRO_FILE_PWD_/$${TARGET}.mimetypes.xml
exists($$MIMETYPES):OTHER_FILES += $$MIMETYPES
TARGET = $$qtLibraryName($$TARGET)
