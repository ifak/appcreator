#include "basebuildstep.hpp"

#include "basebuildconfiguration.hpp"
#include "baseproject.hpp"

#include <coreplugin/messagemanager.h>

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/target.h>

#include <QDir>

namespace ProjectExplorer {

BaseBuildStep::BaseBuildStep(ProjectExplorer::BuildStepList* parentStepList,
                             const Core::Id& buildID)
  : ProjectExplorer::BuildStep(parentStepList, buildID)
{}

BaseBuildStep::BaseBuildStep(ProjectExplorer::BuildStepList* parentStepList,
                             ProjectExplorer::BuildStep* step)
  : ProjectExplorer::BuildStep(parentStepList, step)
{}

BaseBuildStep::~BaseBuildStep()
{}

BaseProject*  BaseBuildStep::baseProject()
{
  BaseProject* baseProject = qobject_cast<BaseProject*>(this->project());
  Q_ASSERT(baseProject);

  return baseProject;
}

BaseBuildConfiguration* BaseBuildStep::baseBuildConfiguration()
{
  BaseBuildConfiguration* baseBC = qobject_cast<BaseBuildConfiguration*>(this->buildConfiguration());
  Q_ASSERT(baseBC);

  return baseBC;
}

bool BaseBuildStep::init()
{
//  qDebug()<<"BaseBuildStep::init() executed!";

  BaseBuildConfiguration* bc = this->baseBuildConfiguration();
  QString outputPath(bc->buildDirectory());
//  qDebug()<<"outputPath: "<<outputPath;

  QDir buildDir(outputPath);
  if(!buildDir.exists())
    buildDir.mkpath(outputPath);

  if(!buildDir.exists())
  {
    QString errorMessage=tr("The output folder '%1' cannot be created. "
                            "The build process cannot be initialized!").arg(outputPath);
    emit addOutput(errorMessage, BuildStep::ErrorMessageOutput);
    return false;
  }

  return true;
}

ProjectExplorer::BuildStepConfigWidget* BaseBuildStep::createConfigWidget()
{
  return new ProjectExplorer::SimpleBuildStepConfigWidget(this);
}

bool BaseBuildStep::runInGuiThread() const
{
  return true;
}

//
// BaseBuildStepFactory
//

class BaseBuildStepFactory::Private
{
  friend class BaseBuildStepFactory;

  Private(const Core::Id& projectID,
          const Core::Id& buildID)
    : _projectID(projectID),
      _buildID(buildID)
  {}

  const Core::Id  _projectID;
  const Core::Id  _buildID;
};

BaseBuildStepFactory::BaseBuildStepFactory(const Core::Id& projectID,
                                           const Core::Id& buildID,
                                           QObject* parent)
  : ProjectExplorer::IBuildStepFactory(parent),
    _d(new Private(projectID, buildID))
{}

BaseBuildStepFactory::~BaseBuildStepFactory()
{
  delete this->_d;
}

bool BaseBuildStepFactory::canCreate(ProjectExplorer::BuildStepList* parentStepList,
                                     const Core::Id id) const
{
  if (parentStepList->target()->project()->id() == this->_d->_projectID)
    return id == this->_d->_buildID;

  return false;
}

bool BaseBuildStepFactory::canClone(ProjectExplorer::BuildStepList* parentStepList,
                                    ProjectExplorer::BuildStep* source) const
{
  Q_UNUSED(parentStepList);
  Q_UNUSED(source);

  return false;
}

ProjectExplorer::BuildStep* BaseBuildStepFactory::clone(ProjectExplorer::BuildStepList* parentStepList,
                                                        ProjectExplorer::BuildStep* source)
{
  Q_UNUSED(parentStepList);
  Q_UNUSED(source);

  return 0;
}

bool BaseBuildStepFactory::canRestore(ProjectExplorer::BuildStepList *parentStepList,
                                      const QVariantMap &map) const
{
  return canCreate(parentStepList,
                   ProjectExplorer::idFromMap(map));
}

QList<Core::Id>
BaseBuildStepFactory::availableCreationIds(ProjectExplorer::BuildStepList* bsl) const
{
  if (bsl->target()->project()->id() == this->_d->_projectID)
    return QList<Core::Id>() << Core::Id(this->_d->_buildID);

  return QList<Core::Id>();
}

QString BaseBuildStepFactory::displayNameForId(const Core::Id id) const
{
  if (id == this->_d->_buildID)
    return id.toString();

  return QString();
}

} // namespace ProjectExplorer

