#include "basecleanbuildstep.hpp"

#include "target.h"
#include "projectexplorerconstants.h"
#include "basebuildconfiguration.hpp"

#include <QDir>

namespace ProjectExplorer {

class BaseCleanBuildStep::Private
{
  friend class BaseCleanBuildStep;

  Private()
  {}

  QDir  _buildDir;
};

BaseCleanBuildStep::BaseCleanBuildStep(ProjectExplorer::BuildStepList* parentStepList)
  : BaseBuildStep(parentStepList, Core::Id(Constants::BUILDSTEPS_CLEAN)),
    _d(new Private())
{}

BaseCleanBuildStep::BaseCleanBuildStep(ProjectExplorer::BuildStepList* parentStepList,
                                       const Core::Id& stepID)
  : BaseBuildStep(parentStepList, stepID),
    _d(new Private())
{}

BaseCleanBuildStep::~BaseCleanBuildStep()
{
  delete this->_d;
}

bool BaseCleanBuildStep::init()
{
  BaseBuildConfiguration* bc = this->baseBuildConfiguration();
  this->_d->_buildDir = bc->buildDirectory();
  if(!this->_d->_buildDir.exists())
  {
    QString errorMessage=tr("The build folder '%1' does not exist!")
                         .arg(this->_d->_buildDir.absolutePath());
    emit addOutput(errorMessage, BuildStep::ErrorMessageOutput);
    return false;
  }

  return true;
}

void BaseCleanBuildStep::run(QFutureInterface<bool> &fi)
{
  Q_ASSERT(this->_d->_buildDir.exists());

  foreach (const QFileInfo& dirFileInfo,
           this->_d->_buildDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
  {
    bool result=false;
    if(dirFileInfo.isDir())
    {
      QDir subDir(this->_d->_buildDir);
      result=subDir.cd(dirFileInfo.fileName());
      result=subDir.removeRecursively();
    }
    else
    {
      result=this->_d->_buildDir.remove(dirFileInfo.fileName());
    }

    if(!result)
    {
      QString errorMessage = tr("The build directory '%1' cannot be cleaned!")
                             .arg(this->_d->_buildDir.absolutePath());
      emit addOutput(errorMessage, BuildStep::ErrorMessageOutput);
      fi.reportResult(false);
      emit finished();
      return;
    }
  }

  fi.reportResult(true);
  emit finished();

  return;
}

//
// BaseCleanBuildStepFactory
//

BaseCleanBuildStepFactory::BaseCleanBuildStepFactory(const Core::Id& projectID,
                                                     const Core::Id& buildID,
                                                     QObject* parent)
  : BaseBuildStepFactory(projectID, buildID, parent)
{}

BaseCleanBuildStepFactory::~BaseCleanBuildStepFactory()
{}

ProjectExplorer::BuildStep* BaseCleanBuildStepFactory::create(ProjectExplorer::BuildStepList* parentStepList,
                                                              const Core::Id id)
{
  if (!this->canCreate(parentStepList, id))
    return 0;

  BaseCleanBuildStep* step = new BaseCleanBuildStep(parentStepList);

  return step;
}

ProjectExplorer::BuildStep* BaseCleanBuildStepFactory::restore(ProjectExplorer::BuildStepList* parentStepList,
                                                               const QVariantMap &map)
{
  if (!this->canRestore(parentStepList, map))
    return 0;

  BaseCleanBuildStep* cbc(new BaseCleanBuildStep(parentStepList));
  if (cbc->fromMap(map))
    return cbc;

  delete cbc;

  return 0;
}

} // namespace mbtproject

