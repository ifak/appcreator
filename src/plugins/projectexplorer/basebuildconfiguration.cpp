#include "basebuildconfiguration.hpp"

#include "target.h"
#include "baseproject.hpp"

#include <utils/qtcassert.h>

namespace ProjectExplorer {

class BaseBuildConfiguration::Private
{
  friend class BaseBuildConfiguration;

  QString         _buildDirectory;
  bool            _enabled;
  QString         _disabledReason;

  Private()
    : _enabled(true),
      _disabledReason(QStringLiteral("test runtime is running"))
  {}
};

BaseBuildConfiguration::BaseBuildConfiguration(ProjectExplorer::Target* parent,
                                               const Core::Id& id,
                                               const QString& displayName)
  : BuildConfiguration(parent, id),
    _d(new Private)
{
  this->setDisplayName(displayName);
  BaseProject* baseProject = qobject_cast<BaseProject*>(parent->project());
  Q_ASSERT(baseProject);
  this->_d->_buildDirectory = baseProject->shadowBuildDirectory(parent->kit(),
                                                                this->displayName());
}

BaseBuildConfiguration::~BaseBuildConfiguration()
{
  delete this->_d;
}

void BaseBuildConfiguration::setEnabled(const bool enabled)
{
  if(this->_d->_enabled == enabled)
    return;

  this->_d->_enabled = enabled;

  BaseProject* baseProject = qobject_cast<BaseProject*>(this->target()->project());
  Q_ASSERT(baseProject);
  emit baseProject->buildConfigurationEnabledChanged();

  return;
}

QString BaseBuildConfiguration::buildDirectory() const
{
  return this->_d->_buildDirectory;
}

ProjectExplorer::NamedWidget* BaseBuildConfiguration::createConfigWidget()
{
  qDebug()<<"BaseBuildConfiguration::createConfigWidget() executed()";

  return 0;
}

BaseBuildConfiguration::BuildType BaseBuildConfiguration::buildType() const
{
  return Unknown;
}

bool BaseBuildConfiguration::isEnabled() const
{
  return this->_d->_enabled;
}

QString BaseBuildConfiguration::disabledReason() const
{
  return this->_d->_disabledReason;
}

/*!
  \class BaseBuildConfigurationFactory
*/
class BaseBuildConfigurationFactory::Private
{
  friend class BaseBuildConfigurationFactory;

  const Core::Id _projectId;
  const Core::Id _buildId;

  Private(const Core::Id& projectId,
          const Core::Id& buildId)
    : _projectId(projectId),
      _buildId(buildId)
  {}
};

BaseBuildConfigurationFactory::BaseBuildConfigurationFactory(const Core::Id& projectId,
                                                             const Core::Id& buildId,
                                                             QObject *parent)
  : IBuildConfigurationFactory(parent),
    _d(new Private(projectId, buildId))
{}

BaseBuildConfigurationFactory::~BaseBuildConfigurationFactory()
{
  delete this->_d;
}

bool BaseBuildConfigurationFactory::canHandle(const ProjectExplorer::Target *t) const
{
  QTC_ASSERT(t, return false);
  QTC_ASSERT(t->project(), return false);

  if (!t->project()->supportsKit(t->kit()))
    return false;

  return t->project()->id() == this->_d->_projectId;
}

// used to show the list of possible additons to a target, returns a list of types
QList<Core::Id>
BaseBuildConfigurationFactory::availableCreationIds(const ProjectExplorer::Target* target) const
{
  QList<Core::Id> creationIds;

  if(!target
     || !target->project())
    return creationIds;

  if (target->project()->id() == this->_d->_projectId)
    return creationIds << this->_d->_buildId;

  return creationIds;
}

// used to translate the types to names to display to the user
QString BaseBuildConfigurationFactory::displayNameForId(const Core::Id id) const
{
  return id.toString();
}

bool BaseBuildConfigurationFactory::canCreate(const ProjectExplorer::Target* target,
                                              const Core::Id id) const
{
  if (!this->canHandle(target))
    return false;

  return id == this->_d->_buildId;
}

// used to recreate the runConfigurations when restoring settings
bool BaseBuildConfigurationFactory::canRestore(const ProjectExplorer::Target* target,
                                               const QVariantMap &map) const
{
  if (!this->canHandle(target))
    return false;

  return ProjectExplorer::idFromMap(map) == this->_d->_buildId;
}

bool BaseBuildConfigurationFactory::canClone(const ProjectExplorer::Target* target,
                                             ProjectExplorer::BuildConfiguration* product) const
{
  Q_UNUSED(target);
  Q_UNUSED(product);

  return false;
}

BaseBuildConfiguration* BaseBuildConfigurationFactory::clone(ProjectExplorer::Target* target,
                                                             ProjectExplorer::BuildConfiguration* product)
{
  Q_UNUSED(target);
  Q_UNUSED(product);

  return 0;
}

} // namespace ProjectExplorer


