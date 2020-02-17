#ifndef PROJECTEXPLORER_BASECLEANBUILDSTEP_HPP
#define PROJECTEXPLORER_BASECLEANBUILDSTEP_HPP

#include "basebuildstep.hpp"

namespace ProjectExplorer {

class PROJECTEXPLORER_EXPORT BaseCleanBuildStep
    : public BaseBuildStep
{
  Q_OBJECT

public:
  BaseCleanBuildStep(ProjectExplorer::BuildStepList* parentStepList);
  virtual ~BaseCleanBuildStep();

public:
  virtual bool init();
  virtual void run(QFutureInterface<bool> &fi);

protected:
  BaseCleanBuildStep(ProjectExplorer::BuildStepList* parentStepList,
                     const Core::Id& stepID);

private:
  class Private;
  Private* _d;
};

class PROJECTEXPLORER_EXPORT BaseCleanBuildStepFactory
    : public BaseBuildStepFactory
{
  Q_OBJECT

protected:
  explicit BaseCleanBuildStepFactory(const Core::Id& projectID,
                                     const Core::Id& buildID,
                                     QObject *parent = 0);

public:
  virtual ~BaseCleanBuildStepFactory();

public:
  virtual ProjectExplorer::BuildStep* create(ProjectExplorer::BuildStepList* parentStepList,
                                             const Core::Id id);
  virtual ProjectExplorer::BuildStep* restore(ProjectExplorer::BuildStepList *parentStepList,
                                              const QVariantMap &map);
};

} // namespace ProjectExplorer

#endif // PROJECTEXPLORER_BASECLEANBUILDSTEP_HPP
