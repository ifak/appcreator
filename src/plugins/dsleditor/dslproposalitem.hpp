#pragma once

#include "dsleditor_global.hpp"

#include <texteditor/codeassist/basicproposalitem.h>

namespace dsleditor {

class DSLEDITOR_EXPORT DslProposalItem
    : public TextEditor::BasicProposalItem
{
public:
  DslProposalItem();
  virtual ~DslProposalItem();

public:
  void  setAfterRemoveLength(int afterRemoveLength);

public:
  virtual void applyContextualContent(TextEditor::BaseTextEditor *editor,
                                      int basePosition) const;

protected:
  int _afterRemoveLength = 0;
};

} // namespace dsleditor
