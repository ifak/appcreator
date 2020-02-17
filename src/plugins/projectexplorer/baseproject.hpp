#pragma once

#include "projectexplorer_export.h"

#include "project.h"
#include "iprojectmanager.h"

#include "baseprojectdocument.hpp"
#include "baseprojectnodes.hpp"
#include "baseerror.hpp"

class QFileSystemWatcher;

namespace ProjectExplorer {
class FolderNode;

class BaseProjectManager;

class PROJECTEXPLORER_EXPORT BaseProject
    : public Project
{
  Q_OBJECT

public:
  typedef QList<ProjectExplorer::BaseError> ErrorSet;

public:
  explicit BaseProject(BaseProjectManager *projectManager,
                       const QString &fileName,
                       const QString& mimeType,
                       const QString& id);
  virtual ~BaseProject();


signals:
  void projectFileRemoved(const QString& projectFileName);
  void projectFileRenamed(const QString& projectFileName,
                          const QString& newProjectFileName);

public:
  QString           displayName() const;
  Core::Id          id() const;
  IProjectManager*  projectManager() const;
  BaseProjectNode*  rootProjectNode() const;

public:
  virtual BaseProjectDocument*  document() const;
  virtual bool                  needsConfiguration() const;
  virtual bool                  supportsNoTargetPanel() const;
  virtual bool                  reload(QString* errorString);
  virtual QStringList           files(FilesMode fileMode) const;

public:
  bool            isError() const;
  const ErrorSet& errors() const;
  void            addError(const ProjectExplorer::BaseError& newError);


public slots:
  void setModified(const bool modified);
  void projectFileChanged(const QString& fileName);

public:
  const QString&  projectName() const;
  bool            isModified() const;

public:
  FolderNode* addFolderNode(const QString& folderName,
                            const QIcon& folderIcon,
                            const bool virtualFolder=false);
  FolderNode* addFolderNode(const QString& folderName,
                            const QIcon& folderIcon,
                            FolderNode* parentNode,
                            const bool virtualFolder=false);
  void        addFileKey(const QString& key,
                         FolderNode* parentNode=0);
  void        addArrayFilesKey(const QString& arrayKey,
                               FolderNode* parentNode=0);
  QString     shadowBuildDirectory(ProjectExplorer::Kit const* kit,
                                   const QString& buildConfigName);
protected:
  FolderNode*         subFolderNode(const QString& subFolderName,
                                    FolderNode* parentNode=0);

protected:
  virtual bool        fromMap(const QVariantMap &map);
  virtual QVariantMap toMap() const;
  virtual void        resetErrors();

private:
  Q_DISABLE_COPY(BaseProject)
  class Private;
  Private*  _d;
};

class PROJECTEXPLORER_EXPORT BaseProjectManager
    : public IProjectManager
{
  Q_OBJECT
public:
  explicit BaseProjectManager(const QString& mimeType);
  virtual ~BaseProjectManager();

public:
  QString mimeType() const;
  BaseProject* openProject(const QString &fileName,
                           QString *errorString);

  void removeProject(BaseProject* project);

protected:
  virtual BaseProject* createProject(const QString &fileName)     =0;

private:
  Q_DISABLE_COPY(BaseProjectManager)
  class Private;
  Private*  _d;
};

} // namespace ProjectExplorer
