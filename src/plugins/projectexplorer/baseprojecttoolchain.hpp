#pragma once

#include "projectexplorer_export.h"

#include "headerpath.h"
#include "toolchain.h"
#include "toolchainconfigwidget.h"
#include "abi.h"

namespace ProjectExplorer {

class PROJECTEXPLORER_EXPORT BaseProjectToolChain
    : public ToolChain
{
public:
  BaseProjectToolChain(const QString& id,
                       const QString& type,
                       const QString& displayName,
                       const QString& typeDisplayName);
  virtual ~BaseProjectToolChain();

public:
  virtual  bool                 canClone() const;
  virtual BaseProjectToolChain* clone() const;
  virtual QVariantMap           toMap() const;
  virtual bool                  fromMap(const QVariantMap &data);
  virtual bool                  operator ==(const ToolChain &) const;

public:///relevant virtual implementations
  QString                                 type() const;
  QString                                 typeDisplayName() const;
  bool                                    isValid() const;
  void                                    addToEnvironment(Utils::Environment &env) const;
  void                                    setCompilerCommand(const Utils::FileName &path);
  Utils::FileName                         compilerCommand() const;
  ProjectExplorer::ToolChainConfigWidget* configurationWidget();

public:///not relevant virtual implementations (empty or default returns)
  ProjectExplorer::Abi                targetAbi() const;
  QByteArray                          predefinedMacros(const QStringList &cxxflags) const;
  CompilerFlags                       compilerFlags(const QStringList &cxxflags) const;
  WarningFlags                        warningFlags(const QStringList &cxxflags) const;
  QList<ProjectExplorer::HeaderPath>  systemHeaderPaths(const QStringList &cxxFlags,
                                                        const Utils::FileName &) const;
  QList<Utils::FileName>              suggestedMkspecList() const;
  ProjectExplorer::IOutputParser*     outputParser() const;
  QString                             makeCommand(const Utils::Environment &env) const;

protected:
  BaseProjectToolChain(const QString &id,
                       const QString& type,
                       const QString& displayName,
                       const QString& typeDisplayName,
                       bool autodetect);

private:
  Q_DISABLE_COPY(BaseProjectToolChain)
  class Private;
  Private* _d;
};

class PROJECTEXPLORER_EXPORT BaseProjectToolChainFactory
    : public ToolChainFactory
{
  Q_OBJECT

public:
  BaseProjectToolChainFactory(const QString& id,
                              const QString& displayName);
  virtual ~BaseProjectToolChainFactory();

public:/// relevant virtual implementations
  virtual bool                  canCreate();
  virtual BaseProjectToolChain* create() =0;
  virtual bool                  canRestore(const QVariantMap &data);
  virtual BaseProjectToolChain* restore(const QVariantMap &data) =0;

public:
  QString displayName() const;
  QString id() const;

private:
  Q_DISABLE_COPY(BaseProjectToolChainFactory)
  class Private;
  Private* _d;
};

//// --------------------------------------------------------------------------
//// BaseProjectToolChainConfigWidget
//// --------------------------------------------------------------------------

//class BaseProjectToolChainConfigWidget
//    : public ToolChainConfigWidget
//{
//  Q_OBJECT

//public:
//  BaseProjectToolChainConfigWidget(BaseProjectToolChain* baseProjectToolChain);
//  virtual ~BaseProjectToolChainConfigWidget();

//protected:
//  void applyImpl();
//  void discardImpl() { setFromToolChain(); }
//  bool isDirtyImpl() const;
//  void makeReadOnlyImpl();

//  void setFromToolChain();

//private:
//  Q_DISABLE_COPY(BaseProjectToolChainConfigWidget)
//  class Private;
//  Private* _d;
//};

} /// end namespace ProjectExplorer
