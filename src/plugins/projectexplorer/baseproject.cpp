#include "baseproject.hpp"

#include "baseprojectnodes.hpp"

#include <utils/qtcassert.h>

#include <coreplugin/icontext.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/documentmanager.h>

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>
#include <projectexplorer/taskhub.h>
#include <projectexplorer/projectmacroexpander.h>

#include <QFileSystemWatcher>

#include <QDebug>

namespace ProjectExplorer {

///////////////////////////BaseProject/////////////////////////////////////////
class BaseProject::Private
{
  friend class BaseProject;

private:
  BaseProjectManager* _projectManager;
  BaseProjectNode*    _rootNode;
  QString             _projectName;
  Core::Id            _id;
  bool                _modified;
  const int           _subFolderPriority;
  ErrorSet            _errors;


  Private(BaseProjectManager* projectManager,
          const QString& id)
    : _projectManager(projectManager),
      _rootNode(0),
      _projectName(QLatin1String("")),
      _id(Core::Id::fromString(id)),
      _modified(false),
      _subFolderPriority(200)
  {}
};

BaseProject::BaseProject(BaseProjectManager *projectManager,
                         const QString &fileName,
                         const QString& mimeType,
                         const QString& id)
  : Project(), _d(new Private(projectManager, id))
{
  this->_d->_projectName = QFileInfo(fileName).completeBaseName();
  this->_d->_rootNode = new BaseProjectNode(this,
                                            new BaseProjectDocument(this,
                                                                    fileName,
                                                                    mimeType),
                                            mimeType);
}

BaseProject::~BaseProject()
{
  if(this->_d->_projectManager)
    this->_d->_projectManager->removeProject(this);

  delete this->_d->_rootNode;

  delete this->_d;
}

void BaseProject::projectFileChanged(const QString &fileName)
{
  Q_UNUSED(fileName);

  QString errorString;
  this->reload(&errorString);

  return;
}

QString BaseProject::displayName() const
{
  return this->_d->_projectName;
}

Core::Id BaseProject::id() const
{
  return this->_d->_id;
}

BaseProjectDocument* BaseProject::document() const
{
  return this->_d->_rootNode->projectDocument();
}

IProjectManager* BaseProject::projectManager() const
{
  return this->_d->_projectManager;
}

BaseProjectNode* BaseProject::rootProjectNode() const
{
  return this->_d->_rootNode;
}

QStringList BaseProject::files(Project::FilesMode fileMode) const
{
  Q_UNUSED(fileMode);

  return this->_d->_rootNode->allFiles();
}

bool BaseProject::needsConfiguration() const
{
  return false;
}

bool BaseProject::supportsNoTargetPanel() const
{
  return false;
}

void BaseProject::setModified(const bool modified)
{
  this->_d->_modified=modified;
}

const QString& BaseProject::projectName() const
{
  return this->_d->_projectName;
}

bool BaseProject::isModified() const
{
  return this->_d->_modified;
}

bool BaseProject::reload(QString* errorString)
{
  if(!this->_d->_rootNode->projectDocument()->reload(errorString,
                                                     BaseProjectDocument::FlagReload,
                                                     BaseProjectDocument::TypeContents))
  {
    if(errorString)
      Core::MessageManager::instance()->printToOutputPane(*errorString,
                                                          Core::MessageManager::WithFocus);

    return false;
  }

  if(!this->_d->_rootNode->update(errorString))
  {
    if(errorString)
      Core::MessageManager::instance()->printToOutputPane(*errorString,
                                                          Core::MessageManager::WithFocus);

    return false;
  }

  if(errorString && !errorString->isEmpty())
    Core::MessageManager::instance()->printToOutputPane(*errorString,
                                                        Core::MessageManager::WithFocus);

  return true;
}

QString BaseProject::shadowBuildDirectory(const ProjectExplorer::Kit *kit,
                                          const QString& buildConfigName)
{
  QString projectFilePath=this->document()->fileName();

  if (projectFilePath.isEmpty())
    return QString();
  QFileInfo info(projectFilePath);

  const QString projectName = QFileInfo(info.absolutePath()).fileName();
  ProjectExplorer::ProjectMacroExpander expander(projectFilePath,
                                                 projectName,
                                                 kit,
                                                 buildConfigName);
  QDir projectDir = QDir(projectDirectory(projectFilePath));
  QString buildPath = Utils::expandMacros(Core::DocumentManager::buildDirectory(),
                                          &expander);

  return QDir::cleanPath(projectDir.absoluteFilePath(buildPath));
}

bool BaseProject::isError() const
{
  return this->_d->_errors.size();
}

const QList<ProjectExplorer::BaseError>&
BaseProject::errors() const
{
  return this->_d->_errors;
}

void BaseProject::addError(const ProjectExplorer::BaseError& newError)
{
  this->_d->_errors.append(newError);

  return;
}

FolderNode*
BaseProject::addFolderNode(const QString& folderName,
                           const QIcon& folderIcon,
                           const bool virtualFolder)
{
  QString trimmedFolderName=folderName.trimmed();
  if(trimmedFolderName.isEmpty())
    return this->_d->_rootNode;

  return this->addFolderNode(folderName,
                             folderIcon,
                             this->_d->_rootNode,
                             virtualFolder);
}

FolderNode*
BaseProject::addFolderNode(const QString& folderName,
                           const QIcon& folderIcon,
                           FolderNode* parentNode,
                           const bool virtualFolder)
{
  FolderNode* parentFolderNode=parentNode;
  if(!parentFolderNode)
    parentFolderNode=this->_d->_rootNode;

  QString trimmedFolderName=folderName.trimmed();
  if(trimmedFolderName.isEmpty())
    return this->_d->_rootNode;

  if(FolderNode* existSubFolderNode=this->subFolderNode(folderName,
                                                        parentFolderNode))
    return existSubFolderNode;

  QString displayName=trimmedFolderName;
  FolderNode* newFolderNode=0;
  if(virtualFolder)
    newFolderNode=new VirtualFolderNode(parentNode->path(),
                                        this->_d->_subFolderPriority);
  else
    newFolderNode=new FolderNode(parentNode->path()
                                 +QLatin1String("/")
                                 +trimmedFolderName.replace(QStringLiteral(" "),
                                                            QStringLiteral("")));
  newFolderNode->setDisplayName(displayName);
  newFolderNode->setIcon(folderIcon);
  this->_d->_rootNode->addFolderNodes(QList<FolderNode*>()<<newFolderNode,
                                      parentNode);

  return newFolderNode;
}

FolderNode* BaseProject::subFolderNode(const QString& displayName,
                                       FolderNode* parentNode)
{
  FolderNode* parentFolderNode=parentNode;
  if(!parentFolderNode)
    parentFolderNode=this->_d->_rootNode;

  QString normalizedDisplayName=displayName.trimmed();
  normalizedDisplayName.replace(QStringLiteral(" "), QStringLiteral(""));

  foreach(FolderNode* subFolderNode, parentFolderNode->subFolderNodes())
  {
    QString normalizedSubFolderName=subFolderNode->displayName().trimmed();
    normalizedSubFolderName.replace(QStringLiteral(" "), QStringLiteral(""));
    if(normalizedSubFolderName==normalizedDisplayName)
      return subFolderNode;
  }

  return 0;
}

void BaseProject::addFileKey(const QString& key,
                             FolderNode* parentNode)
{
  return this->_d->_rootNode->addFileKey(key, parentNode);
}

void BaseProject::addArrayFilesKey(const QString& arrayKey,
                                   FolderNode* parentNode)
{
  return this->_d->_rootNode->addArrayFilesKey(arrayKey, parentNode);
}

bool BaseProject::fromMap(const QVariantMap& map)
{
  return Project::fromMap(map);
}

QVariantMap BaseProject::toMap() const
{
  return Project::toMap();
}

void BaseProject::resetErrors()
{
  this->_d->_errors.clear();

  return;
}

////////////BaseProjectManager/////////////////////////////
class BaseProjectManager::Private
{
  friend class BaseProjectManager;

  Private(const QString& mimeType)
    : _mimeType(mimeType)
  {}

public:
  ~Private()
  {}

private:
  QList<BaseProject*> _projects;
  QString             _mimeType;
};

BaseProjectManager::BaseProjectManager(const QString& mimeType)
  : _d(new Private(mimeType))
{}

BaseProjectManager::~BaseProjectManager()
{
  delete this->_d;
}

QString BaseProjectManager::mimeType() const
{
  return this->_d->_mimeType;
}

BaseProject* BaseProjectManager::openProject(const QString &fileName,
                                             QString *errorString)
{
  QFileInfo fileInfo(fileName);
  ProjectExplorerPlugin *projectExplorer = 0;
  projectExplorer=ProjectExplorerPlugin::instance();
  QTC_ASSERT(projectExplorer, return 0);

  foreach (Project *pi,
           projectExplorer->session()->projects())
  {
    if (fileName == pi->document()->fileName())
    {
      if(errorString)
        *errorString = tr("Failed opening project '%1': "
                          "Project already open")
                       .arg(QDir::toNativeSeparators(fileName));
      return 0;
    }
  }

  if (!fileInfo.isFile())
  {
    if(errorString)
      *errorString = tr("Failed opening project '%1': "
                        "Project file is not a file")
                     .arg(QDir::toNativeSeparators(fileName));
    return 0;
  }

  BaseProject* newProject=this->createProject(fileName);
  Q_ASSERT(newProject);

  Core::DocumentManager::addDocument(newProject->document(),
                                     false);

  if(!newProject->reload(errorString))
  {
    if(newProject->document()->isError())
    {
      foreach(const BaseError& error,
              newProject->document()->errors())
      {
        ProjectExplorerPlugin::instance()
            ->taskHub()->addTask(Task(Task::Error,
                                      error.description(),
                                      Utils::FileName::fromString(error.url().path()),
                                      error.line(),
                                      Core::Id(Constants::TASK_CATEGORY_COMPILE)));
      }
    }
    delete newProject;
    return 0;
  }

  this->_d->_projects.append(newProject);

  return newProject;
}

void BaseProjectManager::removeProject(BaseProject* project)
{
  QTC_ASSERT(project, return);

  Core::DocumentManager::removeDocument(project->document());

  this->_d->_projects.removeAll(project);

  return;
}

} // namespace ProjectExplorer
