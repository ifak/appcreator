#include "baseprojectnodes.hpp"

#include "baseproject.hpp"
#include "baseprojectdocument.hpp"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/messagemanager.h>

#include "projectnodes.h"

#include <utils/qtcassert.h>

#include <QDebug>

namespace ProjectExplorer {

///helper functions
QString getFileDir(const QString& filePath)
{
  QFileInfo fileInfo(filePath);
  if(!fileInfo.exists())
    return QString();

  return fileInfo.canonicalPath();
}

QString folderKey(FolderNode* folderNode)
{
  QString folderKey=QStringLiteral("");

  if(folderNode->nodeType()==ProjectNodeType
     || folderNode->nodeType()==VirtualFolderNodeType)
    return folderKey;

  folderKey=folderNode->displayName().replace(QStringLiteral(" "),
                                              QStringLiteral(""));

  FolderNode* parentFolderNode=folderNode->parentFolderNode();
  if(parentFolderNode->nodeType()==ProjectNodeType
     || parentFolderNode->nodeType()==VirtualFolderNodeType)
    return folderKey;

  while(parentFolderNode)
  {
    QString parentFolderKey=parentFolderNode->displayName().replace(QStringLiteral(" "),
                                                                    QStringLiteral(""));
    folderKey.insert(0, parentFolderKey + QStringLiteral("/"));

    parentFolderNode=parentFolderNode->parentFolderNode();

    if(parentFolderNode->nodeType()==ProjectNodeType
       || parentFolderNode->nodeType()==VirtualFolderNodeType)
      return folderKey;
  }

  return folderKey;
}

class BaseProjectNode::Private
{
  friend class BaseProjectNode;

  typedef QMap<FolderNode*,QStringList>  FileKeys;
  typedef QMap<FolderNode*,QStringList>  ArrayFilesKeys;

  BaseProject*          _project;
  BaseProjectDocument*  _projectDocument;
  QString               _projectMimeType;
  FileKeys              _fileKeys;
  ArrayFilesKeys        _arrayFilesKeys;

  Private(BaseProject* project,
          BaseProjectDocument* projectDocument,
          const QString& projectMimeType)
    : _project(project),
      _projectDocument(projectDocument),
      _projectMimeType(projectMimeType)
  {}
};

BaseProjectNode::BaseProjectNode(BaseProject *project,
                                 BaseProjectDocument *projectDocument,
                                 const QString& projectMimeType)
  : ProjectNode(projectDocument ?
                  QFileInfo(projectDocument
                            ->fileName()).absoluteFilePath()
                : QLatin1String("")),
    _d(new Private(project, projectDocument, projectMimeType))
{
  Q_ASSERT(project);
  Q_ASSERT(projectDocument);

  QFileInfo projectFileInfo(projectDocument->fileName());
  Q_ASSERT(projectFileInfo.exists());
  this->setDisplayName(projectFileInfo.completeBaseName());
}

BaseProjectNode::~BaseProjectNode()
{
  delete this->_d;
}

bool BaseProjectNode::hasBuildTargets() const
{
  return false;
}

QList<BaseProjectNode::ProjectAction>
BaseProjectNode::supportedActions(Node *node) const
{
  Q_UNUSED(node);

  QList<BaseProjectNode::ProjectAction> suppActions;

  if(node->nodeType() == FileNodeType)
  {
    suppActions.append(BaseProjectNode::RemoveFile);
    suppActions.append(BaseProjectNode::Rename);
  }

  return suppActions;
}

bool BaseProjectNode::canAddSubProject(const QString &proFilePath) const
{
  Q_UNUSED(proFilePath)
  return false;
}

bool BaseProjectNode::addSubProjects(const QStringList &proFilePaths)
{
  Q_UNUSED(proFilePaths)
  return false;
}

bool BaseProjectNode::removeSubProjects(const QStringList &proFilePaths)
{
  Q_UNUSED(proFilePaths)
  return false;
}

bool BaseProjectNode::addFiles(const FileType fileType,
                               const QStringList &filePaths,
                               QStringList *notAdded)
{
  Q_UNUSED(fileType);
  Q_UNUSED(filePaths);
  Q_UNUSED(notAdded);

  return false;
}

bool BaseProjectNode::removeFiles(const FileType fileType,
                                  const QStringList &filePaths,
                                  QStringList *notRemoved)
{
  Q_UNUSED(fileType);
  Q_UNUSED(notRemoved);

  bool result = true;
  for(const QString& filePath : filePaths)
  {
    result = this->_d->_projectDocument->removeProjectFile(filePath);
    if(!result)
      return result;
  }

  QString errorString;
  result = this->update(&errorString);
  if(!result)
    qWarning() << errorString;

  return result;
}

bool BaseProjectNode::deleteFiles(const FileType fileType,
                                  const QStringList &filePaths)
{
  Q_UNUSED(fileType);
  Q_UNUSED(filePaths);

  return true;
}

bool BaseProjectNode::renameFile(const FileType fileType,
                                 const QString & filePath,
                                 const QString & newFilePath)
{
  Q_UNUSED(fileType);

  bool result = this->_d->_projectDocument->renameProjectFile(filePath, newFilePath);

  QString errorString;
  result = this->update(&errorString);
  if(!result)
    qWarning() << errorString;

  return result;
}

bool BaseProjectNode::deploysFolder(const QString &folder) const
{
  Q_UNUSED(folder);

  return false;
}

QList<RunConfiguration *>
BaseProjectNode::runConfigurationsFor(Node *node)
{
  Q_UNUSED(node)
  return QList<RunConfiguration *>();
}

QStringList BaseProjectNode::allFiles() const
{
  QStringList allFiles=this->_d->_projectDocument->allProjectFiles();
  allFiles.append(this->_d->_projectDocument->fileName());

  return allFiles;
}

BaseProject* BaseProjectNode::project()
{
  return this->_d->_project;
}

BaseProjectDocument* BaseProjectNode::projectDocument()
{
  return this->_d->_projectDocument;
}

void BaseProjectNode::addFileKey(const QString& fileKey,
                                 FolderNode* parentNode)
{
  if(fileKey.trimmed().isEmpty())
    return;

  FolderNode* parentFolderNode=parentNode;
  if(!parentFolderNode)
    parentFolderNode=this;

  this->_d->_fileKeys[parentFolderNode].append(fileKey);

  QString wholeFileKey=folderKey(parentFolderNode);
  if(!wholeFileKey.isEmpty())
    wholeFileKey+=QStringLiteral("/");
  wholeFileKey+=fileKey;

  this->_d->_projectDocument->addFileKey(wholeFileKey);

  return;
}

void BaseProjectNode::addArrayFilesKey(const QString& arrayFilesKey,
                                       FolderNode* parentNode)
{
  if(arrayFilesKey.trimmed().isEmpty())
    return;

  FolderNode* parentFolderNode=parentNode;
  if(!parentFolderNode)
    parentFolderNode=this;

  this->_d->_arrayFilesKeys[parentFolderNode].append(arrayFilesKey);

  QString wholeArrayFilesKey=folderKey(parentFolderNode);
  if(!wholeArrayFilesKey.isEmpty())
    wholeArrayFilesKey+=QStringLiteral("/");
  wholeArrayFilesKey+=arrayFilesKey;

  this->_d->_projectDocument->addArrayFilesKey(wholeArrayFilesKey);

  return;
}

bool BaseProjectNode::update(QString* errorMessage)
{
  return this->updateFileNodes(this, errorMessage);
}

bool BaseProjectNode::updateFileNodes(FolderNode* folderNode,
                                      QString* errorMessage)
{
  Q_ASSERT(folderNode);

  this->removeFileNodes(folderNode->fileNodes(), folderNode);

  QString folderKeyString = folderKey(folderNode);
  if(!folderKeyString.isEmpty())
    folderKeyString += QStringLiteral("/");

  //////////folder-files with file key//////////////
  foreach(const QString& fileKeyString,
          this->_d->_fileKeys.value(folderNode))
  {
    QString wholeFileKey=folderKeyString + fileKeyString;

    QString filePath=this->_d->_projectDocument->projectFile(wholeFileKey);
    if(filePath.isEmpty())
    {
      if(errorMessage)
        *errorMessage+=tr("there is no project file for file key '%1'!\n").arg(wholeFileKey);
      continue;
    }

    if(!QFileInfo(filePath).exists())
    {
      if(errorMessage)
        *errorMessage+=tr("project file '%1' does not exist!\n").arg(filePath);
      return false;
    }

    BaseFileNode* newFileNode=new BaseFileNode(filePath);
    this->addFileNodes(QList<FileNode*>()<<newFileNode,
                       folderNode);
  }

  //////////folder-array-files//////////////
  foreach(const QString& arrayFilesKeyString,
          this->_d->_arrayFilesKeys.value(folderNode))
  {
    QString wholeArrayFilesKey=folderKeyString + arrayFilesKeyString;

    foreach(const QString& filePath,
            this->_d->_projectDocument->projectArrayFiles(wholeArrayFilesKey))
    {
      if(filePath.isEmpty())
      {
        if(errorMessage)
          *errorMessage+=tr("there is no project file for file key '%1'!\n").arg(wholeArrayFilesKey);
        continue;
      }

      if(!QFileInfo(filePath).exists())
      {
        if(errorMessage)
          *errorMessage+=tr("project file '%1' does not exist!\n").arg(filePath);
        return false;
      }

      BaseFileNode* newFileNode=new BaseFileNode(filePath);
      this->addFileNodes(QList<FileNode*>()<<newFileNode,
                         folderNode);
    }
  }

  foreach(FolderNode* subFolderNode,
          folderNode->subFolderNodes())
  {
    bool successful=this->updateFileNodes(subFolderNode, errorMessage);
    if(!successful)
      return false;
  }

  return true;
}

BaseFileNode::BaseFileNode(const QString& filePath)
  : FileNode(filePath,
             UnknownFileType,
             false)
{}

BaseFileNode::~BaseFileNode()
{}

} // namespace ProjectExplorer

