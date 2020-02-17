#pragma once

#include "dsleditor_global.hpp"

#include <texteditor/autocompleter.h>

namespace dsleditor {

class DSLEDITOR_EXPORT DslAutoCompleter
        : public TextEditor::AutoCompleter
{
public:
  DslAutoCompleter();
  virtual ~DslAutoCompleter();

public:
  virtual bool    contextAllowsAutoParentheses(const QTextCursor &cursor,
                                               const QString &textToInsert = QString()) const;
  virtual bool    contextAllowsElectricCharacters(const QTextCursor &cursor) const;
  virtual bool    isInComment(const QTextCursor &cursor) const;
  virtual QString insertMatchingBrace(const QTextCursor &tc,
                                      const QString &text,
                                      QChar la,
                                      int *skippedChars) const;
  virtual QString insertParagraphSeparator(const QTextCursor &tc) const;
};

} // namespace dsleditor
