#include "baseprojecttoolchain.hpp"

#include "abi.h"

#include <utils/environment.h>
#include <utils/fileutils.h>
#include <utils/detailswidget.h>
#include <utils/pathchooser.h>

#include <QPlainTextEdit>
#include <QFormLayout>

namespace ProjectExplorer {

// --------------------------------------------------------------------------
// BaseProjectToolChain
// --------------------------------------------------------------------------

class BaseProjectToolChain::Private
{
  friend class BaseProjectToolChain;

  QString _type;
  QString _typeDisplayName;

  Private(const QString& type,
          const QString& typeDisplayName)
    : _type(type),
      _typeDisplayName(typeDisplayName)
  {}
};

BaseProjectToolChain::BaseProjectToolChain(const QString& id,
                                           const QString& type,
                                           const QString& displayName,
                                           const QString& typeDisplayName)
  : ToolChain(id, true),
    _d(new Private(type, typeDisplayName))
{
  this->setDisplayName(displayName);
}

BaseProjectToolChain::BaseProjectToolChain(const QString& id,
                                           const QString& type,
                                           const QString& displayName,
                                           const QString& typeDisplayName,
                                           bool autodetect)
  : ToolChain(id, autodetect),
    _d(new Private(type, typeDisplayName))
{
  this->setDisplayName(displayName);
}

BaseProjectToolChain::~BaseProjectToolChain()
{
  delete this->_d;
}

QString BaseProjectToolChain::type() const
{
  return this->_d->_type;
}

QString BaseProjectToolChain::typeDisplayName() const
{
  return this->_d->_typeDisplayName;
}

::ProjectExplorer::Abi BaseProjectToolChain::targetAbi() const
{
  return ::ProjectExplorer::Abi();
}

bool BaseProjectToolChain::isValid() const
{
  return true;
}

QByteArray BaseProjectToolChain::predefinedMacros(const QStringList &cxxflags) const
{
  Q_UNUSED(cxxflags);

  return QByteArray();
}

BaseProjectToolChain::CompilerFlags BaseProjectToolChain::compilerFlags(const QStringList &cxxflags) const
{
  Q_UNUSED(cxxflags);

  return NoFlags;
}

BaseProjectToolChain::WarningFlags BaseProjectToolChain::warningFlags(const QStringList &cxxflags) const
{
  Q_UNUSED(cxxflags);

  return WarningsDefault;
}

QList<ProjectExplorer::HeaderPath> BaseProjectToolChain::systemHeaderPaths(const QStringList &cxxFlags,
                                                                           const Utils::FileName &) const
{
  Q_UNUSED(cxxFlags);

  return QList<ProjectExplorer::HeaderPath>();
}

void BaseProjectToolChain::addToEnvironment(Utils::Environment &env) const
{
  Q_UNUSED(env);

  return;
}

void BaseProjectToolChain::setCompilerCommand(const Utils::FileName& path)
{
  Q_UNUSED(path);

  return;
}

Utils::FileName BaseProjectToolChain::compilerCommand() const
{
  return Utils::FileName();
}

QList<Utils::FileName> BaseProjectToolChain::suggestedMkspecList() const
{
  return QList<Utils::FileName>();
}

// TODO: Customize
ProjectExplorer::IOutputParser* BaseProjectToolChain::outputParser() const
{
  return 0;
}

QString BaseProjectToolChain::makeCommand(const Utils::Environment& env) const
{
  Q_UNUSED(env);

  return QString();
}

bool BaseProjectToolChain::canClone() const
{
  return false;
}

BaseProjectToolChain* BaseProjectToolChain::clone() const
{
  return 0;
}

QVariantMap BaseProjectToolChain::toMap() const
{
  return ToolChain::toMap();
}

bool BaseProjectToolChain::fromMap(const QVariantMap &data)
{
  return ToolChain::fromMap(data);
}

ProjectExplorer::ToolChainConfigWidget*
BaseProjectToolChain::configurationWidget()
{
  //  return new Internal::BaseProjectToolChainConfigWidget(this);
  return 0;
}

bool BaseProjectToolChain::operator ==(const ToolChain &other) const
{
  return ToolChain::operator ==(other);
}

// --------------------------------------------------------------------------
// BaseProjectToolChainFactory
// --------------------------------------------------------------------------
class BaseProjectToolChainFactory::Private
{
  friend class BaseProjectToolChainFactory;

  QString _id;
  QString _displayName;

  Private(const QString& id,
          const QString& displayName)
    : _id(id),
      _displayName(displayName)
  {}
};

BaseProjectToolChainFactory::BaseProjectToolChainFactory(const QString& id,
                                                         const QString& displayName)
  : _d(new Private(id, displayName))
{}

BaseProjectToolChainFactory::~BaseProjectToolChainFactory()
{
  delete this->_d;
}

QString BaseProjectToolChainFactory::displayName() const
{
  return this->_d->_displayName;
}

QString BaseProjectToolChainFactory::id() const
{
  return this->_d->_id;
}

bool BaseProjectToolChainFactory::canCreate()
{
  return true;
}

// Used by the ToolChainManager to restore user-generated tool chains
bool BaseProjectToolChainFactory::canRestore(const QVariantMap &data)
{
  const QString id = idFromMap(data);
  return id.startsWith(this->_d->_id + QLatin1Char(':'));
}

} // namespace ProjectExplorer
