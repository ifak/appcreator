#pragma once

#include "dsleditor_global.hpp"

#include <texteditor/syntaxhighlighter.h>

namespace dsleditor {

class DSLEDITOR_EXPORT DslHighlighter
    : public TextEditor::SyntaxHighlighter
{
  Q_OBJECT

public:
  DslHighlighter(QTextDocument *parent = 0);
  virtual ~DslHighlighter();

public:
  void            setTextCharFormat(const int textStyle,
                                    const QTextCharFormat& format);
  QTextCharFormat textCharFormat(const int textStyle) const;

protected:
  virtual void highlightBlock(const QString &text);

private:
  int   onBlockStart();
  void  onBlockEnd(int state);

  // The functions are notified whenever parentheses are encountered.
  // Custom behaviour can be added, for example storing info for indenting.
  void onOpeningParenthesis(QChar parenthesis, int pos, bool atStart);
  void onClosingParenthesis(QChar parenthesis, int pos, bool atEnd);

private:
  Q_DISABLE_COPY(DslHighlighter)
  class Private;
  Private* _d;
};

} // namespace dslteditor
