#pragma once

#include "dsleditor_global.hpp"

#include <texteditor/indenter.h>

namespace dsleditor {

class DSLEDITOR_EXPORT DslIndenter
    : public TextEditor::Indenter
{
public:
  DslIndenter();
  virtual ~DslIndenter();

public:
  virtual bool isElectricCharacter(const QChar &ch) const;
  virtual void indentBlock(QTextDocument *doc,
                           const QTextBlock &block,
                           const QChar &typedChar,
                           const TextEditor::TabSettings &tabSettings);
  virtual void invalidateCache(QTextDocument *doc);
};

} // namespace dsleditor
