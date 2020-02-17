#include "basemodewidget.hpp"

#include <utils/qtcassert.h>

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/icore.h>

#include "project.h"
#include "projectexplorer.h"
#include "session.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QDockWidget>

namespace ProjectExplorer {

///BaseModeWidget/////////////////
BaseModeWidget::BaseModeWidget(Core::IMode* mode,
                               BaseMainModeWidget* baseMainModeWidget)
{
  Q_ASSERT(mode);
  Q_ASSERT(baseMainModeWidget);

  QBoxLayout *editorHolderLayout = new QVBoxLayout;
  editorHolderLayout->setMargin(0);
  editorHolderLayout->setSpacing(0);

  QWidget *editorAndFindWidget = new QWidget;
  editorAndFindWidget->setLayout(editorHolderLayout);
  editorHolderLayout->addWidget(new Core::EditorManagerPlaceHolder(mode));
  editorHolderLayout->addWidget(new Core::FindToolBarPlaceHolder(editorAndFindWidget));

  MiniSplitter *documentAndRightPane = new MiniSplitter;
  documentAndRightPane->addWidget(editorAndFindWidget);
  documentAndRightPane->addWidget(new Core::RightPanePlaceHolder(mode));
  documentAndRightPane->setStretchFactor(0, 1);
  documentAndRightPane->setStretchFactor(1, 0);

  QWidget *centralWidget = new QWidget;
  QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
  centralWidget->setLayout(centralLayout);
  centralLayout->setMargin(0);
  centralLayout->setSpacing(0);
  centralLayout->addWidget(documentAndRightPane);
  centralLayout->setStretch(0, 1);
  centralLayout->setStretch(1, 0);

  baseMainModeWidget->setCentralWidget(centralWidget);

  // Right-side window with editor, output etc.
  MiniSplitter *mainWindowSplitter = new MiniSplitter;
  mainWindowSplitter->addWidget(baseMainModeWidget);//
  QWidget *outputPane = new Core::OutputPanePlaceHolder(mode, mainWindowSplitter);
  outputPane->setObjectName(QLatin1String("BaseOutputPanePlaceHolder"));
  mainWindowSplitter->addWidget(outputPane);
  mainWindowSplitter->setStretchFactor(0, 10);
  mainWindowSplitter->setStretchFactor(1, 0);
  mainWindowSplitter->setOrientation(Qt::Vertical);

  // Navigation and right-side window.
  this->addWidget(new Core::NavigationWidgetPlaceHolder(mode));
  this->addWidget(mainWindowSplitter);
  this->setStretchFactor(0, 0);
  this->setStretchFactor(1, 1);
  this->setObjectName(QLatin1String("BaseModeWidget"));
}

///DockWidgetEventFilter/////////////////
class DockWidgetEventFilter : public QObject
{
public:
  DockWidgetEventFilter(BaseMainModeWidgetPrivate* bmmwp)
    : _bmmwp(bmmwp)
  {}

private:
  virtual bool eventFilter(QObject *obj, QEvent *event);
  BaseMainModeWidgetPrivate* _bmmwp;
};

///BaseMainModeWidgetPrivate/////////////////
class BaseMainModeWidgetPrivate
    : public QObject
{
  Q_OBJECT

  friend class BaseMainModeWidget;

  typedef BaseMainModeWidget::ProjectIdSet  ProjectIdSet;

  BaseMainModeWidget*       _baseMainModeWidget;
  Core::IMode*              _mode;

  QHash<QString, QVariant>  _dockWidgetActiveState;
  DockWidgetEventFilter     _resizeEventFilter;
//  QWidget*                  _baseModeToolBar;
//  QHBoxLayout*              _baseModeToolBarLayout;
//  QToolButton*              _viewButton;
  Core::ActionContainer*    _viewsMenu;
  Core::Context             _modeContext;
  ProjectIdSet              _suitableProjectIds;
  QList<Core::Command*>     _menuCommandsToAdd;


  BaseMainModeWidgetPrivate(BaseMainModeWidget* baseMainModeWidget,
                            Core::IMode* mode,
                            Core::Context modeContext,
                            const ProjectIdSet& suitableProjectIds)
    : _baseMainModeWidget(baseMainModeWidget),
      _mode(mode),
      _resizeEventFilter(this),
//      _baseModeToolBar(new QWidget),
//      _baseModeToolBarLayout(new QHBoxLayout(this->_baseModeToolBar)),
//      _viewButton(nullptr),
      _viewsMenu(nullptr),
      _modeContext(modeContext),
      _suitableProjectIds(suitableProjectIds)
  {
    Q_ASSERT(this->_baseMainModeWidget);
    Q_ASSERT(this->_mode);

//    this->_mbtModeToolBarLayout->setMargin(0);
//    this->_mbtModeToolBarLayout->setSpacing(0);
  }

public:
  void createViewsMenuItems();

public slots:
  void updateUiForProject(ProjectExplorer::Project* project);
  void updateDockWidgetSettings();
};

void BaseMainModeWidgetPrivate::updateUiForProject(ProjectExplorer::Project* project)
{
  if(!project)
    return;

  if(!this->_suitableProjectIds.contains(project->id()))
  {
    if(Core::ModeManager::currentMode()==this->_mode)
      Core::ModeManager::activateMode(Core::Constants::MODE_EDIT);
    this->_mode->setEnabled(false);

    return;
  }

  this->_mode->setEnabled(true);

  return;
}

void BaseMainModeWidgetPrivate::createViewsMenuItems()
{
  this->_viewsMenu = Core::ActionManager::actionContainer(Core::Id(Core::Constants::M_WINDOW_VIEWS));
  QTC_ASSERT(this->_viewsMenu, return);

  // Add menu items
  Core::Command *cmd = 0;
  cmd = Core::ActionManager::registerAction(this->_baseMainModeWidget->menuSeparator1(),
                                            Core::Id(this->_mode->id()).withSuffix(".Views.Separator1"),
                                            this->_modeContext);
  Q_ASSERT(cmd);
  cmd->setAttribute(Core::Command::CA_Hide);
  this->_viewsMenu->addAction(cmd, Core::Constants::G_DEFAULT_THREE);

  cmd = Core::ActionManager::registerAction(this->_baseMainModeWidget->toggleLockedAction(),
                                            Core::Id(this->_mode->id()).withSuffix(".Views.ToggleLocked"),
                                            this->_modeContext);
  Q_ASSERT(cmd);
  cmd->setAttribute(Core::Command::CA_Hide);
  this->_viewsMenu->addAction(cmd, Core::Constants::G_DEFAULT_THREE);

  cmd = Core::ActionManager::registerAction(this->_baseMainModeWidget->menuSeparator2(),
                                            Core::Id(this->_mode->id()).withSuffix(".Views.Separator2"),
                                            this->_modeContext);
  Q_ASSERT(cmd);
  cmd->setAttribute(Core::Command::CA_Hide);
  this->_viewsMenu->addAction(cmd, Core::Constants::G_DEFAULT_THREE);

  cmd = Core::ActionManager::registerAction(this->_baseMainModeWidget->resetLayoutAction(),
                                            Core::Id(this->_mode->id()).withSuffix(".Views.ResetSimple"),
                                            this->_modeContext);
  Q_ASSERT(cmd);
  cmd->setAttribute(Core::Command::CA_Hide);
  this->_viewsMenu->addAction(cmd, Core::Constants::G_DEFAULT_THREE);

  for(Core::Command *cmd : this->_menuCommandsToAdd)
    this->_viewsMenu->addAction(cmd);

  return;
}

void BaseMainModeWidgetPrivate::updateDockWidgetSettings()
{
//  qDebug()<<"BaseMainModeWidgetPrivate::updateDockWidgetSettings()";
  if(Core::ModeManager::currentMode()->id()!=this->_mode->id())
    return;

//  qDebug()<<"BaseMainModeWidgetPrivate::saveSettings()";
  this->_dockWidgetActiveState=this->_baseMainModeWidget->saveSettings();

  return;
}

///implementation of DockWidgetEventFilter::eventFilter///
bool DockWidgetEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  switch (event->type()) {
  case QEvent::Resize:
  case QEvent::ZOrderChange:
    this->_bmmwp->updateDockWidgetSettings();
    break;
  default:
    break;
  }
  return QObject::eventFilter(obj, event);
}

///BaseMainModeWidget///////////////////////
BaseMainModeWidget::BaseMainModeWidget(Core::IMode* mode,
                                       Core::Context modeContext,
                                       const ProjectIdSet& projectIds)
  : _d(new BaseMainModeWidgetPrivate(this, mode, modeContext, projectIds))
{
  using namespace ProjectExplorer;

  ProjectExplorerPlugin *pe = ProjectExplorerPlugin::instance();
  connect(pe->session(), &SessionManager::startupProjectChanged,
          this->_d, &BaseMainModeWidgetPrivate::updateUiForProject);

  this->setDocumentMode(true);
  this->setDockNestingEnabled(true);

  //  connect(this, &MbtMainModeWidget::resetLayout,
  //          this, &MbtMainModeWidget::resetMbtModeLayout);//TODO: still to be implemented!
  connect(this->toggleLockedAction(), &QAction::triggered,
          this->_d, &BaseMainModeWidgetPrivate::updateDockWidgetSettings);

//  this->_d->_viewButton = new QToolButton();
//  // FIXME: Use real thing after string freeze.
//  QString hackyName = QCoreApplication::translate("Core::Internal::MainWindow", "&Views");
//  hackyName.replace(QLatin1Char('&'), QString());
//  this->_d->_viewButton->setText(hackyName);

  //  Utils::StyledBar *tcgToolBar = new Utils::StyledBar;
  //  tcgToolBar->setProperty("topBorder", true);
  //  QHBoxLayout* tcgToolBarLayout = new QHBoxLayout(tcgToolBar);
  //  tcgToolBarLayout->setMargin(0);
  //  tcgToolBarLayout->setSpacing(0);
  //  //  debugToolBarLayout->addWidget(d->m_debugToolBar);
  //  tcgToolBarLayout->addWidget(new Utils::StyledSeparator);
  //  tcgToolBarLayout->addWidget(this->_d->_viewButton);

  //  connect(this->_d->_viewButton, &QToolButton::clicked,
  //          this, &MbtMainModeWidget::showViewsMenu);

  //  QDockWidget *dock = new QDockWidget(tr("Test Generation Specification"));
  //  dock->setObjectName(QLatin1String("TCG Toolbar"));
  //  dock->setWidget(tcgToolBar);
  //  dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
  //  dock->setAllowedAreas(Qt::BottomDockWidgetArea);
  //  dock->setTitleBarWidget(new QWidget(dock));
  //  dock->setProperty("managed_dockwidget", QLatin1String("true"));
  //  this->addDockWidget(Qt::BottomDockWidgetArea, dock);
  //  this->setToolBarDockWidget(dock);

  Q_ASSERT(this->_d->_baseMainModeWidget);

  this->_d->createViewsMenuItems();

  this->setTrackingEnabled(true);

  this->readSettings();
  this->restoreSettings(this->_d->_dockWidgetActiveState);
  Core::ICore::updateAdditionalContexts(Core::Context(), this->_d->_modeContext);

  connect(Core::ModeManager::instance(), &Core::ModeManager::currentModeChanged,
          this, &BaseMainModeWidget::onModeChanged);
  connect(Core::ICore::instance(), &Core::ICore::saveSettingsRequested,
          this, &BaseMainModeWidget::writeSettings);
}

BaseMainModeWidget::~BaseMainModeWidget()
{
  delete this->_d;
}

void BaseMainModeWidget::readSettings()
{
//  qDebug()<<"BaseMainModeWidget::readSettings() executed!";

  QSettings *settings = Core::ICore::settings();
  this->_d->_dockWidgetActiveState.clear();

  settings->beginGroup(this->_d->_mode->id().toString());
  for (const QString &key : settings->childKeys())
    this->_d->_dockWidgetActiveState.insert(key, settings->value(key));
  settings->endGroup();

  this->writeSettings();

  return;
}

void BaseMainModeWidget::writeSettings() const
{
//  qDebug()<<"BaseMainModeWidget::writeSettings() executed!";

  QSettings *settings = Core::ICore::settings();
  settings->beginGroup(this->_d->_mode->id().toString());
  QHashIterator<QString, QVariant> it(this->_d->_dockWidgetActiveState);
  while (it.hasNext()) {
    it.next();
    settings->setValue(it.key(), it.value());
  }
  settings->endGroup();

  return;
}

void BaseMainModeWidget::onModeChanged(Core::IMode* mode)
{
//  qDebug()<<"BaseMainModeWidget::onModeChanged() executed!";

  if(mode->id()!=this->_d->_mode->id())
  {
    for(QDockWidget* dockWidget : this->dockWidgets())
    {
      if (dockWidget->isFloating())
        dockWidget->hide();
    }
    return;
  }

  //  this->readSettings();

  //  Core::ICore::updateAdditionalContexts(this->_d->_mbtModeContext, Core::Context());
  //  this->restoreSettings(this->_d->_dockWidgetActiveState);
  //  Core::ICore::updateAdditionalContexts(Core::Context(), this->_d->_mbtModeContext);

  return;
}

/*!
    Keep track of dock widgets so they can be shown/hidden for different languages
*/
QDockWidget* BaseMainModeWidget::createDockWidget(QWidget *widget,
                                                  Qt::DockWidgetArea dockWidgetArea)
{
  QDockWidget *dockWidget = this->addDockForWidget(widget);
  dockWidget->setObjectName(widget->objectName());
  dockWidget->setWindowTitle(widget->objectName());
  this->addDockWidget(dockWidgetArea, dockWidget);

  QAction *toggleViewAction = dockWidget->toggleViewAction();
  Core::Command *cmd = Core::ActionManager::registerAction(toggleViewAction,
                                                           Core::Id("MbtMode.").withSuffix(widget->objectName()),
                                                           this->_d->_modeContext /*globalContext*/);
  cmd->setAttribute(Core::Command::CA_Hide);
  this->_d->_menuCommandsToAdd.append(cmd);
  this->_d->_viewsMenu->addAction(cmd);

  dockWidget->installEventFilter(&this->_d->_resizeEventFilter);

  connect(dockWidget->toggleViewAction(), &QAction::triggered,
          this->_d, &BaseMainModeWidgetPrivate::updateDockWidgetSettings);
  connect(dockWidget, &QDockWidget::topLevelChanged,
          this->_d, &BaseMainModeWidgetPrivate::updateDockWidgetSettings);
  connect(dockWidget, &QDockWidget::dockLocationChanged,
          this->_d, &BaseMainModeWidgetPrivate::updateDockWidgetSettings);

  return dockWidget;
}

} // namespace ProjectExplorer

#include "basemodewidget.moc"
