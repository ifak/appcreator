#ifndef PROJECTEXPLORER_BASEBUILDCONFIGURATION_HPP
#define PROJECTEXPLORER_BASEBUILDCONFIGURATION_HPP

#include "projectexplorer_export.h"

#include "buildconfiguration.h"
#include "namedwidget.h"

namespace ProjectExplorer {

class PROJECTEXPLORER_EXPORT BaseBuildConfiguration
    : public BuildConfiguration
{
  Q_OBJECT

public:
  explicit BaseBuildConfiguration(ProjectExplorer::Target* parent,
                                  const Core::Id& id,
                                  const QString& displayName);
  virtual ~BaseBuildConfiguration();

public:
  virtual QString buildDirectory() const;
  virtual ProjectExplorer::NamedWidget* createConfigWidget();
  virtual BuildType buildType() const;
  virtual bool isEnabled() const;
  virtual QString disabledReason() const;

public:
  void  setEnabled(const bool enabled);

private:
  Q_DISABLE_COPY(BaseBuildConfiguration)
  class Private;
  Private* _d;
};

class PROJECTEXPLORER_EXPORT BaseBuildConfigurationFactory
    : public ProjectExplorer::IBuildConfigurationFactory
{
  Q_OBJECT

public:
  explicit BaseBuildConfigurationFactory(const Core::Id& projectId,
                                         const Core::Id& buildId,
                                         QObject *parent = 0);
  virtual ~BaseBuildConfigurationFactory();

public:
  virtual bool canCreate(const ProjectExplorer::Target* parent,
                         const Core::Id id) const;
  virtual BaseBuildConfiguration* create(ProjectExplorer::Target* parent,
                                         const Core::Id id,
                                         const QString &name = QString()) = 0;

  // used to recreate the runConfigurations when restoring settings
  virtual bool canRestore(const ProjectExplorer::Target* parent,
                          const QVariantMap &map) const;
  virtual BaseBuildConfiguration* restore(ProjectExplorer::Target* parent,
                                          const QVariantMap &map) = 0;

public:
  // used to show the list of possible additons to a target, returns a list of types
  virtual QList<Core::Id> availableCreationIds(const ProjectExplorer::Target *parent) const;

  // used to translate the types to names to display to the user
  virtual QString displayNameForId(const Core::Id id) const;

  virtual bool canClone(const ProjectExplorer::Target* parent,
                        ProjectExplorer::BuildConfiguration* product) const;
  virtual BaseBuildConfiguration* clone(ProjectExplorer::Target* parent,
                                        ProjectExplorer::BuildConfiguration* product);

private:
  bool canHandle(const ProjectExplorer::Target* t) const;

private:
  Q_DISABLE_COPY(BaseBuildConfigurationFactory)
  class Private;
  Private* _d;
};

} // namespace ProjectExplorer

#endif // PROJECTEXPLORER_BASEBUILDCONFIGURATION_HPP
