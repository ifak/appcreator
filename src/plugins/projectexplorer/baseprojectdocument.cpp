#include "baseprojectdocument.hpp"

#include "baseproject.hpp"
#include "baseprojectnodes.hpp"

#include <utils/qtcassert.h>

#include <QSettings>
#include <QDebug>

namespace ProjectExplorer {

typedef QMap<QString,QString>     ProjectFiles;
typedef QMap<QString,QStringList> ProjectArrayFiles;

class BaseProjectDocument::Private
{
  friend class BaseProjectDocument;

  BaseProject*      _project;
  const QString     _mimeType;
  bool              _modified;
  ProjectFiles      _projectFiles;
  ProjectArrayFiles _projectArrayFiles;
  QSettings*        _baseProjectFile;
  QFileInfo         _baseProjectFileInfo;
  QList<BaseError>  _errors;

  const QStringList _emptyStringList;
  const QString     _emptyString;

  Private(BaseProject* project,
          const QString& fileName,
          const QString& mimeType)
    : _project(project),
      _mimeType(mimeType),
      _modified(false),
      _baseProjectFile(new QSettings(fileName, QSettings::IniFormat)),
      _baseProjectFileInfo(fileName)
  {}

  ~Private()
  {
    if(this->_baseProjectFile)
      delete this->_baseProjectFile;
  }
};

BaseProjectDocument::BaseProjectDocument(BaseProject *project,
                                         const QString& fileName,
                                         const QString& mimeType)
  : Core::IDocument(project),
    _d(new Private(project, fileName, mimeType))
{
  Q_ASSERT(this->_d->_project);
}

BaseProjectDocument::~BaseProjectDocument()
{
  delete this->_d;
}

bool BaseProjectDocument::save(QString* errorString)
{
  return this->save(errorString,
                    this->_d->_baseProjectFileInfo.absoluteFilePath(),
                    false);
}

bool BaseProjectDocument::save(QString *errorString,
                               const QString &fileName,
                               bool autoSave)
{
  Q_UNUSED(autoSave);
  Q_UNUSED(errorString);

  Q_ASSERT(!fileName.isEmpty());

  if(this->_d->_baseProjectFile)
    delete this->_d->_baseProjectFile;
  this->_d->_baseProjectFile = new QSettings(fileName, QSettings::IniFormat);
  this->_d->_baseProjectFile->clear();
  this->_d->_baseProjectFile->sync();
  this->_d->_baseProjectFileInfo=QFileInfo(fileName);

  foreach(const QString& projectFileKey,
          this->_d->_projectFiles.keys())
  {
    QStringList keyValues=this->_d->_projectFiles.values(projectFileKey);
    Q_ASSERT(keyValues.size()==1);

    QString relFilePath=QDir::cleanPath(this->fileDir().relativeFilePath(keyValues.at(0)));

    this->_d->_baseProjectFile->setValue(projectFileKey,relFilePath);
  }

  foreach(const QString& projectFileKey,
          this->_d->_projectArrayFiles.keys())
  {
    QList<QStringList>  mapKeyValues=this->_d->_projectArrayFiles.values(projectFileKey);
    Q_ASSERT(mapKeyValues.size()==1);
    QStringList keyValues=mapKeyValues.at(0);
    if(keyValues.isEmpty())
      continue;

    this->_d->_baseProjectFile->beginWriteArray(projectFileKey);
    for (int i = 0; i < keyValues.size(); ++i)
    {
      this->_d->_baseProjectFile->setArrayIndex(i);

      QString relFilePath=QDir::cleanPath(this->fileDir().relativeFilePath(keyValues.at(i)));

      this->_d->_baseProjectFile->setValue(QStringLiteral("file"),
                                           relFilePath);
    }
    this->_d->_baseProjectFile->endArray();
  }

  this->_d->_baseProjectFile->sync();

  this->_d->_project->reload(errorString);

  return true;
}

QString BaseProjectDocument::fileName() const
{
  return this->_d->_baseProjectFileInfo.absoluteFilePath();
}

QDir  BaseProjectDocument::fileDir() const
{
  return this->_d->_baseProjectFileInfo.absoluteDir();
}

void BaseProjectDocument::rename(const QString& newName)
{
  // Can't happen...
  Q_UNUSED(newName);
  Q_ASSERT(false);
}

QString BaseProjectDocument::defaultPath() const
{
  return QString();
}

QString BaseProjectDocument::suggestedFileName() const
{
  return QString();
}

QString BaseProjectDocument::mimeType() const
{
  return this->_d->_mimeType;
}

bool BaseProjectDocument::isModified() const
{
  return this->_d->_modified;
}

bool BaseProjectDocument::isSaveAsAllowed() const
{
  return false;
}

void BaseProjectDocument::addFileKey(const QString& fileKey)
{
  this->_d->_projectFiles.insert(fileKey, QString());

  return;
}

void BaseProjectDocument::addArrayFilesKey(const QString& arrayFilesKey)
{
  this->_d->_projectArrayFiles.insert(arrayFilesKey, QStringList());

  return;
}

bool BaseProjectDocument::reload(QString* errorString,
                                 ReloadFlag flag,
                                 ChangeType type)
{
  QTC_ASSERT(errorString, return false);

  if(flag!=FlagReload || type!=TypeContents)
    return false;

  if(!this->_d->_baseProjectFileInfo.exists())
  {
    *errorString+=tr("project file '%1' does not exist!").arg(this->fileName());
    return false;
  }

  this->reset();

  foreach(const QString& projectFileKey,
          this->_d->_projectFiles.keys())
  {
    QString fileName=this->_d->_baseProjectFile->value(projectFileKey).toString();
    if(fileName.isEmpty())
    {
      QString errorMessage = tr("there is no project file for file key '%1'!").arg(projectFileKey);
      if(errorString)
        *errorString+=errorMessage;

      BaseError error;
      error.setDescription(errorMessage);
      error.setUrl(QUrl(this->fileName()));
      this->_d->_errors.append (error);
      continue;
    }

    fileName=QDir::cleanPath(this->fileDir().absolutePath()
                             +QLatin1String("/")
                             +fileName);

    QFileInfo fileInfo(fileName);
    if(!fileInfo.exists())
    {
      QString errorMessage = tr("file '%1' does not exist!").arg(fileName);
      if(errorString)
        *errorString+=errorMessage;

      BaseError error;
      error.setDescription(errorMessage);
      error.setUrl(QUrl(fileName));
      this->_d->_errors.append (error);
    }
    else
      this->_d->_projectFiles.insert(projectFileKey, fileName);
  }

  foreach(const QString& projectFileKey,
          this->_d->_projectArrayFiles.keys())
  {
    int size = this->_d->_baseProjectFile->beginReadArray(projectFileKey);
    for (int i = 0; i < size; ++i)
    {
      this->_d->_baseProjectFile->setArrayIndex(i);

      QString fileName=this->_d->_baseProjectFile->value(QStringLiteral("file")).toString();
      if(fileName.isEmpty())
      {
        QString errorMessage = tr("there is no project file for array '%1' with index '%2'!")
                               .arg(projectFileKey).arg(i);
        if(errorString)
          *errorString+=errorMessage;

        BaseError error;
        error.setDescription(errorMessage);
        error.setUrl(QUrl(this->fileName()));
        this->_d->_errors.append (error);
        continue;
      }

      fileName=QDir::cleanPath(this->fileDir().absolutePath()
                               +QLatin1String("/")
                               +fileName);

      QFileInfo fileInfo(fileName);
      if(!fileInfo.exists())
      {
        QString errorMessage = tr("file '%1' does not exist!\n").arg(fileName);
        if(errorString)
          *errorString+=errorMessage;

        BaseError error;
        error.setDescription(errorMessage);
        error.setUrl(QUrl(fileName));
        this->_d->_errors.append (error);
      }
      else
        this->_d->_projectArrayFiles[projectFileKey].append(fileName);
    }
    this->_d->_baseProjectFile->endArray();
  }

  if(this->isError())
    return false;

  return true;
}

void BaseProjectDocument::reset()
{
  for(ProjectFiles::iterator files_it = this->_d->_projectFiles.begin();
      files_it != this->_d->_projectFiles.end();
      ++files_it)
    files_it.value()=QStringLiteral("");

  for(ProjectArrayFiles::iterator array_it = this->_d->_projectArrayFiles.begin();
      array_it != this->_d->_projectArrayFiles.end();
      ++array_it)
    array_it.value()=QStringList();

  this->_d->_errors.clear();

  return;
}

int BaseProjectDocument::allProjectFilesCount() const
{
  int count = 0;
  count += this->_d->_projectFiles.size();
  for(const QString& arrayKey : this->_d->_projectArrayFiles.keys())
    count += this->_d->_projectArrayFiles.value(arrayKey).size();

  return count;
}

QStringList BaseProjectDocument::allProjectFiles() const
{
  QStringList allProjectFiles = this->_d->_projectFiles.values();
  for(const QString& arrayKey : this->_d->_projectArrayFiles.keys())
    allProjectFiles.append(this->_d->_projectArrayFiles.value(arrayKey));

  return allProjectFiles;
}

int BaseProjectDocument::projectArrayFilesCount(const QString& projectArrayFilesKey) const
{
  if(!this->_d->_projectArrayFiles.contains(projectArrayFilesKey))
    return 0;

  return this->_d->_projectArrayFiles[projectArrayFilesKey].size();
}

const QStringList& BaseProjectDocument::projectArrayFiles(const QString& projectArrayFilesKey) const
{
  if(!this->_d->_projectArrayFiles.contains(projectArrayFilesKey))
    return this->_d->_emptyStringList;

  return this->_d->_projectArrayFiles[projectArrayFilesKey];
}

const QString& BaseProjectDocument::projectFile(const QString& projectFileKey) const
{
  if(!this->_d->_projectFiles.contains(projectFileKey))
    return this->_d->_emptyString;

  return this->_d->_projectFiles[projectFileKey];
}

bool BaseProjectDocument::removeProjectFile(const QString& absoluteFileName)
{
  QFileInfo fileInfo(absoluteFileName);
  if(!fileInfo.exists())
  {
    qWarning() << QObject::tr("file '%1' cannot be removed, it does not exist!")
                  .arg(absoluteFileName);
    return false;
  }

  for(const QString& projectFileKey :
          this->_d->_projectFiles.keys())
  {
    QStringList keyValues=this->_d->_projectFiles.values(projectFileKey);
    Q_ASSERT(keyValues.size()==1);

    if(keyValues.contains(absoluteFileName))
      this->_d->_projectFiles.remove(projectFileKey);
  }

  for(const QString& projectFileKey :
          this->_d->_projectArrayFiles.keys())
  {
    QList<QStringList>  mapKeyValues=this->_d->_projectArrayFiles.values(projectFileKey);
    Q_ASSERT(mapKeyValues.size()==1);
    QStringList keyValues=mapKeyValues.at(0);

    if(keyValues.contains(absoluteFileName))
    {
      keyValues.removeAll(absoluteFileName);
      if(keyValues.isEmpty())
        this->_d->_projectArrayFiles.remove(projectFileKey);
      else
        this->_d->_projectArrayFiles.insert(projectFileKey, keyValues);
    }
  }

  QString errorString;
  bool result = this->save(&errorString);
  if(!result)
    qWarning() << errorString;

  if(result)
    emit this->_d->_project->projectFileRemoved(absoluteFileName);

 return result;
}

bool BaseProjectDocument::renameProjectFile(const QString& absoluteFileName,
                                            const QString& newAbsoluteFileName)
{
  QFileInfo fileInfo(newAbsoluteFileName);
  if(!fileInfo.exists())
  {
    qWarning() << QObject::tr("renamed file '%1' does not exist!")
                  .arg(newAbsoluteFileName);
    return false;
  }

  for(const QString& projectFileKey :
          this->_d->_projectFiles.keys())
  {
    QStringList keyValues=this->_d->_projectFiles.values(projectFileKey);
    Q_ASSERT(keyValues.size()==1);

    if(keyValues.contains(absoluteFileName))
      this->_d->_projectFiles[projectFileKey] = newAbsoluteFileName;
  }

  for(const QString& projectFileKey :
          this->_d->_projectArrayFiles.keys())
  {
    QList<QStringList>  mapKeyValues=this->_d->_projectArrayFiles.values(projectFileKey);
    Q_ASSERT(mapKeyValues.size()==1);
    QStringList keyValues=mapKeyValues.at(0);

    if(keyValues.contains(absoluteFileName))
    {
      keyValues.replaceInStrings(absoluteFileName, newAbsoluteFileName);
      this->_d->_projectArrayFiles.insert(projectFileKey, keyValues);
    }
  }

  QString errorString;
  bool result = this->save(&errorString);
  if(!result)
    qWarning() << errorString;

  if(result)
    emit this->_d->_project->projectFileRenamed(absoluteFileName, newAbsoluteFileName);

 return result;
}

bool BaseProjectDocument::addProjectFile(const QString& fileKey,
                                         const QString& relFileName,
                                         QString* errorMessage)
{
  QString fileName=QDir::cleanPath(this->fileDir().absolutePath()
                                   +QLatin1String("/")
                                   +relFileName);

  QFileInfo fileInfo(fileName);
  if(!fileInfo.exists())
  {
    if(errorMessage)
      *errorMessage += tr("file '%1' does not exist!").arg(fileName);

    return false;
  }
  else
    this->_d->_projectFiles[fileKey]=fileName;

  this->save(errorMessage, this->_d->_baseProjectFileInfo.absoluteFilePath());

  return true;
}

bool BaseProjectDocument::hasProjectFile(const QString& fileKey,
                                         const QString& absFileName)
{
  if(!this->_d->_projectFiles.contains(fileKey))
    return false;

  return (this->_d->_projectFiles.value(fileKey) == absFileName);
}

bool BaseProjectDocument::addProjectArrayFile(const QString& arrayKey,
                                              const QString& relFileName,
                                              QString* errorMessage)
{
  QString fileName=QDir::cleanPath(this->fileDir().absolutePath()
                                   +QLatin1String("/")
                                   +relFileName);

  QFileInfo fileInfo(fileName);
  if(!fileInfo.exists())
  {
    if(errorMessage)
      *errorMessage += tr("file '%1' does not exist!").arg(fileName);

    return false;
  }
  else
    this->_d->_projectArrayFiles[arrayKey].append(fileName);

  this->save(errorMessage, this->_d->_baseProjectFileInfo.absoluteFilePath());

  return true;
}

bool BaseProjectDocument::hasProjectArrayFile(const QString& arrayKey,
                                              const QString& absFileName)
{
  if(!this->_d->_projectArrayFiles.contains(arrayKey))
    return false;

  for(const QString& existArrayFileName : this->_d->_projectArrayFiles.value(arrayKey))
  {
    if(existArrayFileName==absFileName)
      return true;
  }

  return false;
}

bool BaseProjectDocument::isError() const
{
  return this->_d->_errors.size();
}

const QList<BaseError>& BaseProjectDocument::errors() const
{
  return this->_d->_errors;
}

QString BaseProjectDocument::errorString() const
{
  QString errorString;
  foreach (const BaseError& error, this->_d->_errors)
    errorString.append(error.toString()+QLatin1String("\n"));

  if(errorString.size())
    errorString.remove(errorString.size()-1, 1);

  return errorString;
}

} // namespace ProjectExplorer

