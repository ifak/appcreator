#ifndef PROJECTEXPLORER_BASEPROJECTNODES_HPP
#define PROJECTEXPLORER_BASEPROJECTNODES_HPP

#include "projectexplorer_export.h"

#include "projectnodes.h"

namespace ProjectExplorer {

class BaseProject;
class BaseProjectDocument;

class PROJECTEXPLORER_EXPORT BaseProjectNode
    : public ProjectNode
{
  Q_OBJECT
public:
  explicit BaseProjectNode(BaseProject* project,
                           BaseProjectDocument *projectDocument,
                           const QString& projectMimeType);
  virtual ~BaseProjectNode();

public:
  virtual bool hasBuildTargets() const;
  virtual QList<ProjectAction> supportedActions(Node *node) const;
  virtual bool canAddSubProject(const QString &proFilePath) const;
  virtual bool addSubProjects(const QStringList &proFilePaths);
  virtual bool removeSubProjects(const QStringList &proFilePaths);
  virtual bool addFiles(const FileType fileType,
                        const QStringList &filePaths,
                        QStringList *notAdded = 0);
  virtual bool removeFiles(const FileType fileType,
                           const QStringList &filePaths,
                           QStringList *notRemoved = 0);
  virtual bool deleteFiles(const FileType fileType,
                           const QStringList &filePaths);
  virtual bool renameFile(const FileType fileType,
                          const QString &filePath,
                          const QString &newFilePath);
  virtual bool deploysFolder(const QString &folder) const;
  virtual QList<RunConfiguration *> runConfigurationsFor(Node *node);

  virtual QStringList allFiles() const;

public:
  void addFileKey(const QString& fileKey,
                  FolderNode* parentNode=0);
  void addArrayFilesKey(const QString& arrayFilesKey,
                        FolderNode* parentNode=0);

public:
  virtual BaseProject* project();
  virtual BaseProjectDocument* projectDocument();

public:
  bool update(QString* errorMessage);

protected:
  bool updateFileNodes(FolderNode* folderNode,
                       QString* errorMessage);

private:
  Q_DISABLE_COPY(BaseProjectNode)
  class Private;
  Private* _d;
};

class PROJECTEXPLORER_EXPORT BaseFileNode
    : public FileNode
{
  Q_OBJECT

public:
  BaseFileNode(const QString &filePath);
  virtual ~BaseFileNode();
};

} // namespace ProjectExplorer

#endif // PROJECTEXPLORER_BASEPROJECTNODES_HPP
