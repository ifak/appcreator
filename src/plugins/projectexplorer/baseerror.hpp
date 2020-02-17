#ifndef PROJECTEXPLORER_BASEERROR_HPP
#define PROJECTEXPLORER_BASEERROR_HPP

#include "projectexplorer_export.h"

#include <QUrl>
#include <QtQml/qqmlerror.h>

#include <dslparser/dslerror.hpp>

namespace ProjectExplorer {

class PROJECTEXPLORER_EXPORT BaseError
{
public:
  enum ErrorType
  {
    UnknownError,
    FileError,
    DuplicateError,
    SyntaxError,
    AmbiguityError,
    ContextError,
    SensitivityError,
    BaseUserType
  };

public:
  BaseError(const int errorType=UnknownError);
  BaseError(const BaseError& origin);
  virtual ~BaseError();

public:
  int   errorType() const;
  void  setErrorType(const int errorType);

  const QString&  description() const;
  void            setDescription(const QString& description);

  const QUrl& url() const;
  void        setUrl(const QUrl& url);

  int   line() const;
  void  setLine(const int line);

  int   column() const;
  void  setColumn(const int column);

public:
  virtual QString toString() const;

protected:
  int     _errorType;
  QString _description;
  QUrl    _url;
  int     _line;
  int     _column;
};

static inline BaseError qmlError2baseError(const QQmlError& qmlError)
{
  BaseError baseError;
  baseError.setDescription(qmlError.description());
  baseError.setUrl(qmlError.url());
  baseError.setLine(qmlError.line());
  baseError.setColumn(qmlError.column());

  return baseError;
}

static inline BaseError dslError2baseError(const dslparser::DslError& dslError,
                                           const QString& errorFileName)
{
  ProjectExplorer::BaseError baseError;

  if(dslError.errorType()==dslparser::DslError::SyntaxError)
    baseError.setErrorType(ProjectExplorer::BaseError::SyntaxError);
  else if(dslError.errorType()==dslparser::DslError::AmbiguityError)
    baseError.setErrorType(ProjectExplorer::BaseError::AmbiguityError);
  else if(dslError.errorType()==dslparser::DslError::ContextError)
    baseError.setErrorType(ProjectExplorer::BaseError::ContextError);
  else if(dslError.errorType()==dslparser::DslError::SensitivityError)
    baseError.setErrorType(ProjectExplorer::BaseError::SensitivityError);
  else if(dslError.errorType()==dslparser::DslError::FileError)
    baseError.setErrorType(ProjectExplorer::BaseError::FileError);
  else if(dslError.errorType()==dslparser::DslError::UnknownError)
    baseError.setErrorType(ProjectExplorer::BaseError::UnknownError);

  baseError.setDescription(dslError.message());
  baseError.setUrl(QUrl(errorFileName));
  baseError.setLine(dslError.line());
  baseError.setColumn(dslError.charPositionInLine());

  return baseError;
}

static inline
void addDslError(const dslparser::DslError& newDslError,
                 const QString& errorFileName,
                 QList<BaseError>* baseErrorList)
{
  if(!baseErrorList)
    return;

  ProjectExplorer::BaseError error;

  if(newDslError.errorType()==dslparser::DslError::SyntaxError)
    error.setErrorType(ProjectExplorer::BaseError::SyntaxError);
  else if(newDslError.errorType()==dslparser::DslError::AmbiguityError)
    error.setErrorType(ProjectExplorer::BaseError::AmbiguityError);
  else if(newDslError.errorType()==dslparser::DslError::ContextError)
    error.setErrorType(ProjectExplorer::BaseError::ContextError);
  else if(newDslError.errorType()==dslparser::DslError::SensitivityError)
    error.setErrorType(ProjectExplorer::BaseError::SensitivityError);
  else if(newDslError.errorType()==dslparser::DslError::FileError)
    error.setErrorType(ProjectExplorer::BaseError::FileError);
  else if(newDslError.errorType()==dslparser::DslError::UnknownError)
    error.setErrorType(ProjectExplorer::BaseError::UnknownError);

  error.setDescription(newDslError.message());
  error.setUrl(QUrl(errorFileName));
  error.setLine(newDslError.line());
  error.setColumn(newDslError.charPositionInLine());

  baseErrorList->append(error);

  return;
}

} // namespace ProjectExplorer

#endif // PROJECTEXPLORER_BASEERROR_HPP
