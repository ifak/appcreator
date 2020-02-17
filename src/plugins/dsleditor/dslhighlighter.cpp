/*
 *  This file is part of the appcreator framework, based on Qt Creator source code.
 *
 *  Copyright (C) 2013 Jan Krause <jan.krause.no19@gmail.com>
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
*/

#include "dslhighlighter.hpp"

#include <texteditor/basetextdocumentlayout.h>
#include <texteditor/texteditorconstants.h>

#include <utils/qtcassert.h>

#include <QSet>
#include <QtAlgorithms>
#include <QDebug>

#include <dslparser/common/comcreatecommontokens.hpp>

#include <mobata/memory_leak_start.hpp>

namespace dsleditor {

class DslHighlighter::Private
{
  friend class DslHighlighter;

  typedef QHash<int, QTextCharFormat> TextCharFormatSet;

private:
  typedef TextEditor::Parenthesis Parenthesis;
  typedef TextEditor::Parentheses Parentheses;

  int               _braceDepth;
  int               _foldingIndent;
  bool              _inMultilineComment;
  Parentheses       _currentBlockParentheses;
  TextCharFormatSet _textCharFormats;

  Private()
    : _braceDepth(0),
      _foldingIndent(0),
      _inMultilineComment(false)
  {
    this->_currentBlockParentheses.reserve(20);
  }
};

DslHighlighter::DslHighlighter(QTextDocument *parent)
  : TextEditor::SyntaxHighlighter(parent),
    _d(new Private)
{}

DslHighlighter::~DslHighlighter()
{
  delete this->_d;
}

void DslHighlighter::setTextCharFormat(const int textStyle,
                                       const QTextCharFormat &format)
{
  this->_d->_textCharFormats[textStyle] = format;

  return;
}

QTextCharFormat DslHighlighter::textCharFormat(const int textStyle) const
{
//  return this->_d->_textCharFormats.value(textStyle);
  QTextCharFormat tf = this->_d->_textCharFormats.value(textStyle);
  if(textStyle != TextEditor::C_PARENTHESES)
    tf.setBackground(Qt::NoBrush);

  return tf;
}

void DslHighlighter::highlightBlock(const QString& text)
{
//  qDebug()<<"DslHighlighter::highlightBlock() executed, blockText: "<<text;

  this->onBlockStart();

  int startBlockPos=0;
  QString blockText = text;
  if(this->_d->_inMultilineComment)
  {
    int indexCommentOff = blockText.indexOf(QStringLiteral("*/"));
    if(indexCommentOff >= 0)
    {
      this->setFormat(0, indexCommentOff +2,
                      this->textCharFormat(TextEditor::C_COMMENT));
      blockText.remove(0, indexCommentOff +2);
      startBlockPos=indexCommentOff+2;
      this->onClosingParenthesis(QLatin1Char('-'),
                                 startBlockPos - 1,
                                 blockText.size() == 0 /*at the end*/);
      this->_d->_inMultilineComment=false;
    }
    else
    {
      this->setFormat(0, blockText.size(),
                      this->textCharFormat(TextEditor::C_COMMENT));
      this->onBlockEnd(this->_d->_inMultilineComment);
      return;
    }
  }

  dslparser::common::ComCreateCommonTokens tokensCommand(blockText);
  QString errorString;
  bool result = tokensCommand.execute(true, &errorString);
  if(!result)
  {
    qDebug()<<"error during token creation: "<<errorString;
    return;
  }

  int tokenIndex = 0;
  for(const dslparser::TokenTextLocation& tokenTextLoc : tokensCommand.commonTokenList())
  {
    if(!tokenTextLoc.isValid())
      continue;

    switch (tokenTextLoc.tokenType())
    {
      case dslparser::String:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_STRING));
        break;

      case dslparser::Int:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_NUMBER));
        break;

      case dslparser::Real:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_NUMBER));
        break;

      case dslparser::True:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_KEYWORD));
        break;

      case dslparser::False:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_KEYWORD));
        break;

     case dslparser::LineComment:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_COMMENT));
        break;

      case dslparser::MultiLineComment:
         this->setFormat(startBlockPos + tokenTextLoc.start(),
                         tokenTextLoc.length(),
                         this->textCharFormat(TextEditor::C_COMMENT));
        if(blockText.midRef(tokenTextLoc.end()-1,2) != QStringLiteral("*/"))
        {
          onOpeningParenthesis(QLatin1Char('+'),
                               startBlockPos + tokenTextLoc.start(),
                               tokenIndex == 0);
          this->_d->_inMultilineComment = true;
        }
         break;

      case dslparser::LeftParen:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_PARENTHESES));
        onOpeningParenthesis(QLatin1Char('('), tokenTextLoc.start(), tokenIndex == 0);
        break;

      case dslparser::RightParen:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_PARENTHESES));
        onClosingParenthesis(QLatin1Char(')'),
                             tokenTextLoc.start(),
                             tokenIndex == tokensCommand.commonTokenList().size()-1);
        break;

      case dslparser::LeftBrace:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_PARENTHESES));
        onOpeningParenthesis(QLatin1Char('{'),
                             tokenTextLoc.start(),
                             tokenIndex == 0);
        break;

      case dslparser::RightBrace:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_PARENTHESES));
        onClosingParenthesis(QLatin1Char('}'),
                             tokenTextLoc.start(),
                             tokenIndex == tokensCommand.commonTokenList().size()-1);
        break;

      case dslparser::LeftBracket:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_PARENTHESES));
        onOpeningParenthesis(QLatin1Char('['),
                             tokenTextLoc.start(),
                             tokenIndex == 0);
        break;

      case dslparser::RightBracket:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_PARENTHESES));
        onClosingParenthesis(QLatin1Char(']'),
                             tokenTextLoc.start(),
                             tokenIndex == tokensCommand.commonTokenList().size()-1);
        break;

      case dslparser::NewLine:
         this->setFormat(startBlockPos + tokenTextLoc.start(),
                         tokenTextLoc.length(),
                         this->textCharFormat(TextEditor::C_VISUAL_WHITESPACE));
         break;

      case dslparser::WhiteSpace:
         this->setFormat(startBlockPos + tokenTextLoc.start(),
                         tokenTextLoc.length(),
                         this->textCharFormat(TextEditor::C_VISUAL_WHITESPACE));
         break;

      default:
        this->setFormat(startBlockPos + tokenTextLoc.start(),
                        tokenTextLoc.length(),
                        this->textCharFormat(TextEditor::C_TEXT));
        break;
    } // end swtich

    ++tokenIndex;
  }

  this->onBlockEnd(this->_d->_inMultilineComment);

  return;
}

int DslHighlighter::onBlockStart()
{
  using namespace TextEditor;

  this->_d->_currentBlockParentheses.clear();
  this->_d->_braceDepth = 0;
  this->_d->_foldingIndent = 0;
  this->_d->_inMultilineComment = false;
  if (TextBlockUserData *userData = BaseTextDocumentLayout::testUserData(currentBlock()))
  {
    userData->setFoldingIndent(0);
    userData->setFoldingStartIncluded(false);
    userData->setFoldingEndIncluded(false);
  }

  int state = 0;
  int previousState = previousBlockState();
  if (previousState != -1)
  {
    state = previousState & 0xff;
    if(state)
      this->_d->_inMultilineComment = true;
    this->_d->_braceDepth = (previousState >> 8);
  }
  this->_d->_foldingIndent = this->_d->_braceDepth;

  return state;
}

void DslHighlighter::onBlockEnd(int state)
{
  using namespace TextEditor;

  setCurrentBlockState((this->_d->_braceDepth << 8) | state);
  BaseTextDocumentLayout::setParentheses(currentBlock(), this->_d->_currentBlockParentheses);
  BaseTextDocumentLayout::setFoldingIndent(currentBlock(), this->_d->_foldingIndent);

  return;
}

void DslHighlighter::onOpeningParenthesis(QChar parenthesis, int pos, bool atStart)
{
  using namespace TextEditor;

  if (parenthesis == QLatin1Char('{') || parenthesis == QLatin1Char('[') || parenthesis == QLatin1Char('+'))
  {
    ++this->_d->_braceDepth;
    // if a folding block opens at the beginning of a line, treat the entire line
    // as if it were inside the folding block
    if (atStart)
      TextEditor::BaseTextDocumentLayout::userData(currentBlock())->setFoldingStartIncluded(true);
  }
  this->_d->_currentBlockParentheses.push_back(Parenthesis(Parenthesis::Opened, parenthesis, pos));

  return;
}

void DslHighlighter::onClosingParenthesis(QChar parenthesis, int pos, bool atEnd)
{
  using namespace TextEditor;

  if (parenthesis == QLatin1Char('}') || parenthesis == QLatin1Char(']') || parenthesis == QLatin1Char('-'))
  {
    --this->_d->_braceDepth;
    if (atEnd)
      TextEditor::BaseTextDocumentLayout::userData(currentBlock())->setFoldingEndIncluded(true);
    else
      this->_d->_foldingIndent = qMin(this->_d->_braceDepth, this->_d->_foldingIndent); // folding indent is the minimum brace depth of a block
  }
  this->_d->_currentBlockParentheses.push_back(Parenthesis(Parenthesis::Closed, parenthesis, pos));

  return;
}

} // namespace dsleditor
