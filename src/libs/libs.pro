TEMPLATE  = subdirs

CONFIG += c++11

SUBDIRS   = \
    aggregation \
    extensionsystem \
    languageutils \
    utils \
    ssh

for(l, SUBDIRS) {
    QTC_LIB_DEPENDS =
    include($$l/$${l}_dependencies.pri)
    lv = $${l}.depends
    $$lv = $$QTC_LIB_DEPENDS
}

SUBDIRS += \
    qtcomponents/styleitem

QBS_DIRS = \
    ../shared/qbs/src/lib \
    ../shared/qbs/src/plugins \
    ../shared/qbs/static.pro

exists(../shared/qbs/qbs.pro): SUBDIRS += $$QBS_DIRS
TR_EXCLUDE = $$QBS_DIRS
