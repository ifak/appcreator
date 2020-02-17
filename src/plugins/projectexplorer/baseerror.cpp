#include "baseerror.hpp"

namespace ProjectExplorer {

BaseError::BaseError(const int errorType)
  : _errorType(errorType)
{}

BaseError::BaseError(const BaseError& origin)
  : _errorType(origin._errorType),
    _description(origin._description),
    _url(origin._url),
    _line(origin._line),
    _column(origin._column)
{}

BaseError::~BaseError()
{}

int BaseError::errorType () const
{
  return this->_errorType;
}

void BaseError::setErrorType(const int errorType)
{
  this->_errorType=errorType;

  return;
}

const QString& BaseError::description() const
{
  return this->_description;
}

void BaseError::setDescription(const QString& description)
{
  this->_description=description;

  return;
}

const QUrl& BaseError::url() const
{
  return this->_url;
}

void BaseError::setUrl(const QUrl& url)
{
  this->_url=url;

  return;
}

int BaseError::line() const
{
  return this->_line;
}

void BaseError::setLine(const int line)
{
  this->_line=line;

  return;
}

int BaseError::column() const
{
  return this->_column;
}

void BaseError::setColumn(const int column)
{
  this->_column=column;

  return;
}

QString BaseError::toString() const
{
  QString errorString;

  errorString.append(this->_description);

  return errorString;
}

} // namespace ProjectExplorer

