#include "dslproposalitem.hpp"

#include <texteditor/basetexteditor.h>

namespace dsleditor {

DslProposalItem::DslProposalItem()
{}

DslProposalItem::~DslProposalItem()
{}

void DslProposalItem::setAfterRemoveLength(int afterRemoveLength)
{
  this->_afterRemoveLength = afterRemoveLength;

  return;
}

void DslProposalItem::applyContextualContent(TextEditor::BaseTextEditor *editor,
                                             int basePosition) const
{
  Q_ASSERT(editor);

  const int currentPosition = editor->position();
  editor->setCursorPosition(basePosition);
  editor->replace(currentPosition - basePosition + this->_afterRemoveLength,
                  text());

  return;
}

} // namespace dsleditor
