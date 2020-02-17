#pragma once

#include "dsleditor_global.hpp"

#include <extensionsystem/iplugin.h>

namespace dsleditor{

namespace Internal{

class DslEditorPlugin
    : public ExtensionSystem::IPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "dsleditor.json")

public:
  DslEditorPlugin();
  ~DslEditorPlugin();

public:
  bool          initialize(const QStringList &arguments,
                           QString *errorString);
  void          extensionsInitialized();

private:
  Q_DISABLE_COPY(DslEditorPlugin)
};

} // namespace Internal
} // namespace dsleditor
