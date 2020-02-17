include(../../appcreator.pri)

TEMPLATE  = subdirs

SUBDIRS   = \
    coreplugin \
    welcome \
    find \
    texteditor \
    projectexplorer \
    locator \
    macros \
    dsleditor

for(p, SUBDIRS) {
    QTC_PLUGIN_DEPENDS =
    include($$p/$${p}_dependencies.pri)
    pv = $${p}.depends
    $$pv = $$QTC_PLUGIN_DEPENDS
}

