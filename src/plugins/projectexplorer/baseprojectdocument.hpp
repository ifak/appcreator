#ifndef PROJECTEXPLORER_BASEPROJECTDOCUMENT_HPP
#define PROJECTEXPLORER_BASEPROJECTDOCUMENT_HPP

#include "projectexplorer_export.h"

#include <coreplugin/idocument.h>
#include <QDir>

#include "baseerror.hpp"

namespace ProjectExplorer {

class BaseProject;

class PROJECTEXPLORER_EXPORT BaseProjectDocument
    : public Core::IDocument
{
  Q_OBJECT
  friend class BaseProjectNode;

public:
  explicit BaseProjectDocument(BaseProject* project,
                               const QString& fileName,
                               const QString& mimeType);
  virtual ~BaseProjectDocument();

public:
  bool save(QString *errorString);
  bool save(QString *errorString,
            const QString &fileName,
            bool autoSave = false);

public:
  virtual bool reload(QString *errorString,
                      ReloadFlag flag,
                      ChangeType type);

public:
  QString fileName() const;
  void rename(const QString &newName);

  QString defaultPath() const;
  QString suggestedFileName() const;
  QString mimeType() const;

  bool isModified() const;
  bool isSaveAsAllowed() const;

public:
  void addFileKey(const QString& fileKey);
  void addArrayFilesKey(const QString& arrayFilesKey);

public:
  QDir                fileDir() const;
  int                 allProjectFilesCount() const;
  QStringList         allProjectFiles() const;
  int                 projectArrayFilesCount(const QString& projectArrayFilesKey) const;
  const QStringList&  projectArrayFiles(const QString& projectArrayFilesKey) const;
  const QString&      projectFile(const QString& projectFileKey) const;
  bool                removeProjectFile(const QString& absoluteFileName);
  bool                renameProjectFile(const QString& absoluteFileName,
                                        const QString& newAbsoluteFileName);
  bool                addProjectFile(const QString& fileKey,
                                     const QString& relFileName,
                                     QString* errorMessage);
  bool                hasProjectFile(const QString& fileKey,
                                     const QString& absFileName);
  bool                addProjectArrayFile(const QString& arrayKey,
                                          const QString& relFileName,
                                          QString* errorMessage);
  bool                hasProjectArrayFile(const QString& arrayKey,
                                          const QString& absFileName);

public:
  bool                    isError() const;
  const QList<BaseError>& errors() const;
  QString                 errorString() const;

protected:
  virtual void reset();

private:
  Q_DISABLE_COPY(BaseProjectDocument)
  class Private;
  Private*  _d;
};

} // namespace PROJECTEXPLORER_EXPORT

#endif // PROJECTEXPLORER_BASEPROJECTDOCUMENT_HPP
