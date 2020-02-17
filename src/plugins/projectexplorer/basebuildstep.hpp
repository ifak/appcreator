#ifndef PROJECTEXPLORER_BASEBUILDSTEP_HPP
#define PROJECTEXPLORER_BASEBUILDSTEP_HPP

#include "projectexplorer_export.h"

#include "buildstep.h"

namespace ProjectExplorer {

class BaseProject;
class BaseBuildConfiguration;

class PROJECTEXPLORER_EXPORT BaseBuildStep
    : public ProjectExplorer::BuildStep
{
  Q_OBJECT

  friend class BaseBuildStepFactory;

public:
  BaseBuildStep(ProjectExplorer::BuildStepList* parentStepList,
                const Core::Id& buildID);
  virtual ~BaseBuildStep();

public:
  virtual bool init();
  virtual void run(QFutureInterface<bool> &fi) =0;

public:
  virtual ProjectExplorer::BuildStepConfigWidget* createConfigWidget();
  virtual bool runInGuiThread() const;

protected:
  BaseBuildStep(ProjectExplorer::BuildStepList* parentStepList,
                ProjectExplorer::BuildStep* step);

protected:
  virtual BaseProject*             baseProject();
  virtual BaseBuildConfiguration*  baseBuildConfiguration();
};

class PROJECTEXPLORER_EXPORT BaseBuildStepFactory
    : public ProjectExplorer::IBuildStepFactory
{
  Q_OBJECT

public:
  explicit BaseBuildStepFactory(const Core::Id& projectID,
                                const Core::Id& buildID,
                                QObject *parent = 0);
  virtual ~BaseBuildStepFactory();

public:
  bool                        canCreate(ProjectExplorer::BuildStepList *parentStepList,
                                        const Core::Id id) const;
  bool                        canClone(ProjectExplorer::BuildStepList *parentStepList,
                                       ProjectExplorer::BuildStep *source) const;
  ProjectExplorer::BuildStep* clone(ProjectExplorer::BuildStepList* parentStepList,
                                    ProjectExplorer::BuildStep* source);

  bool                        canRestore(ProjectExplorer::BuildStepList* parentStepList,
                                         const QVariantMap& map) const;

  QList<Core::Id>             availableCreationIds(ProjectExplorer::BuildStepList* bsl) const;
  QString                     displayNameForId(const Core::Id id) const;

public:
  virtual ProjectExplorer::BuildStep* create(ProjectExplorer::BuildStepList* parentStepList,
                                             const Core::Id id) =0;
  virtual ProjectExplorer::BuildStep* restore(ProjectExplorer::BuildStepList *parentStepList,
                                              const QVariantMap &map) =0;

private:
  class Private;
  Private* _d;
};

} // namespace ProjectExplorer

#endif // PROJECTEXPLORER_BASEBUILDSTEP_HPP
