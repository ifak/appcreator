#pragma once

#include "projectexplorer_export.h"

#include <coreplugin/minisplitter.h>
#include <coreplugin/imode.h>

#include <utils/fancymainwindow.h>

namespace ProjectExplorer {

class BaseMainModeWidgetPrivate;

class PROJECTEXPLORER_EXPORT BaseMainModeWidget
     : public Utils::FancyMainWindow
{
  Q_OBJECT

public:
  typedef QList<Core::Id> ProjectIdSet;

public:
  BaseMainModeWidget(Core::IMode* mode,
                     Core::Context modeContext,
                     const ProjectIdSet& projectIds);
  virtual ~BaseMainModeWidget();

public slots:
  void readSettings();
  void writeSettings() const;
  void onModeChanged(Core::IMode* mode);

protected:
  QDockWidget* createDockWidget(QWidget* widget, Qt::DockWidgetArea dockWidgetArea);

private:
  Q_DISABLE_COPY(BaseMainModeWidget)
  BaseMainModeWidgetPrivate* _d;
};

class PROJECTEXPLORER_EXPORT BaseModeWidget
    : public Core::MiniSplitter
{
  Q_OBJECT

public:
  BaseModeWidget(Core::IMode* mode,
                 BaseMainModeWidget* baseMainModeWidget);
};

} // namespace ProjectExplorer
