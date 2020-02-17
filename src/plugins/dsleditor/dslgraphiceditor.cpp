#include "dslgraphiceditor.hpp"

#include <coreplugin/editormanager/editormanager.h>

namespace dsleditor {

class DslGraphicEditor::Private
{
  friend class DslGraphicEditor;

  Private(const Core::Id& id)
    : _id(id)
  {}

  Core::Id                    _id;
  QString                     _displayName;
  QSet<Core::IEditor const*>  _connectedTextEditors;
};

DslGraphicEditor::DslGraphicEditor(const Core::Id& id)
  : _d(new Private(id))
{
  connect(this, &DslGraphicEditor::closeLater,
          this, &DslGraphicEditor::closeLaterSlot,
          Qt::QueuedConnection);
}

DslGraphicEditor::~DslGraphicEditor()
{
  delete this->_d;
}

bool DslGraphicEditor::createNew(const QString &contents)
{
  Q_UNUSED(contents);

  return true;
}

Core::Id DslGraphicEditor::id() const
{
  return this->_d->_id;
}

QString DslGraphicEditor::displayName() const
{
  return this->_d->_displayName;
}

void DslGraphicEditor::setDisplayName(const QString &title)
{
  this->_d->_displayName=title;

  return;
}

bool DslGraphicEditor::isTemporary() const
{
  return true;
}

void DslGraphicEditor::addConnectedTextEditor(Core::IEditor const* textEditor)
{
  if(!textEditor)
    return;

  this->_d->_connectedTextEditors.insert(textEditor);

  return;
}

void DslGraphicEditor::removeConnectedTextEditor(Core::IEditor const* textEditor)
{
  if(!textEditor)
    return;

  this->_d->_connectedTextEditors.remove(textEditor);

  if(this->_d->_connectedTextEditors.empty())
    emit this->closeLater();

  return;
}

void DslGraphicEditor::closeLaterSlot()
{
  Core::EditorManager* editorManager=Core::EditorManager::instance();
  Q_ASSERT(editorManager);

  editorManager->closeEditors(QList<Core::IEditor*>{this});
}

} // namespace dsleditor
