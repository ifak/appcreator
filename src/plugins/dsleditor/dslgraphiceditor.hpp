#pragma once

#include "dsleditor_global.hpp"

#include <coreplugin/editormanager/ieditor.h>

namespace dsleditor {

class DSLEDITOR_EXPORT DslGraphicEditor
    : public Core::IEditor
{
  Q_OBJECT

public:
  DslGraphicEditor(const Core::Id& id);
  virtual ~DslGraphicEditor();

public:
  virtual bool      createNew(const QString &contents = QString());
  virtual Core::Id  id() const;
  virtual QString   displayName() const;
  virtual void      setDisplayName(const QString &title);
  virtual bool      isTemporary() const;
  virtual void      addConnectedTextEditor(Core::IEditor const* textEditor);
  virtual void      removeConnectedTextEditor(Core::IEditor const* textEditor);

private:
signals:
  void closeLater();

private slots:
  void closeLaterSlot();

private:
  Q_DISABLE_COPY(DslGraphicEditor)
  class Private;
  Private* _d;
};

} // namespace dsleditor
