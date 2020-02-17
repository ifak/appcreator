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

#include "dslautocompleter.hpp"

#include <dslparser/common/comcreatecommontokens.hpp>

#include <QChar>
#include <QLatin1Char>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>

#include <mobata/memory_leak_start.hpp>

namespace dsleditor {

static int blockStartState(const QTextBlock &block)
{
  int state = block.previous().userState();

  if (state == -1)
    return 0;
  else
    return state & 0xff;
}

static dslparser::TokenTextLocation tokenUnderCursor(const QTextCursor& cursor)
{
  QString blockText = cursor.block().text();
  const int blockState = blockStartState(cursor.block());
  const int pos = cursor.positionInBlock();

  if(blockState == 1)//multi line comment
  {
    int endOfCommentPos = blockText.indexOf(QStringLiteral("*/"));
    if(endOfCommentPos == -1)//not found -> everything is multi line comment
      endOfCommentPos=blockText.size();

    if (pos <= endOfCommentPos)
      return dslparser::TokenTextLocation(0, endOfCommentPos, dslparser::MultiLineComment);
    else
    {
      if(endOfCommentPos<blockText.size())
        blockText = blockText.mid(endOfCommentPos+2); //text out of comment
    }
  }

  dslparser::common::ComCreateCommonTokens tokenCommand(blockText);
  QString errorMessage;
  bool result = tokenCommand.execute(&errorMessage);
  if(!result)
    return dslparser::TokenTextLocation();//invalid

  for(const dslparser::TokenTextLocation& token : tokenCommand.commonTokenList())
  {
    int tokenType = token.tokenType();
    if(tokenType == dslparser::LineComment
       || tokenType == dslparser::MultiLineComment
       || tokenType == dslparser::String)
    {
      if (pos > token.start()
          && pos <= token.end())
        return token;
    }
    else
    {
      if (pos >= token.start()
          && pos < token.end())
        return token;
    }
  }

  return dslparser::TokenTextLocation();//invalid
}

static bool shouldInsertMatchingText(QChar lookAhead)
{
  switch (lookAhead.unicode()) {
    case '{': case '}':
    case ']': case ')':
    case ';': case ',':
    case '"': case '\'':
      return true;

    default:
      if (lookAhead.isSpace())
        return true;

      return false;
  } // switch
}

static bool shouldInsertMatchingText(const QTextCursor &tc)
{
  QTextDocument *doc = tc.document();
  return shouldInsertMatchingText(doc->characterAt(tc.selectionEnd()));
}

static bool shouldInsertNewline(const QTextCursor &tc)
{
  QTextDocument *doc = tc.document();
  int pos = tc.selectionEnd();

  // count the number of empty lines.
  int newlines = 0;
  for (int e = doc->characterCount(); pos != e; ++pos) {
    const QChar ch = doc->characterAt(pos);

    if (! ch.isSpace())
      break;
    else if (ch == QChar::ParagraphSeparator)
      ++newlines;
  }

  if (newlines <= 1 && doc->characterAt(pos) != QLatin1Char('}'))
    return true;

  return false;
}

static bool isCompleteStringLiteral(const QStringRef &text)
{
  if (text.length() < 2)
    return false;

  const QChar quote = text.at(0);

  if (text.at(text.length() - 1) == quote)
    return text.at(text.length() - 2) != QLatin1Char('\\'); // ### not exactly.

  return false;
}

DslAutoCompleter::DslAutoCompleter()
{}

DslAutoCompleter::~DslAutoCompleter()
{}

bool DslAutoCompleter::contextAllowsAutoParentheses(const QTextCursor &cursor,
                                                       const QString &textToInsert) const
{
  QChar ch;

  if (! textToInsert.isEmpty())
    ch = textToInsert.at(0);

  switch (ch.unicode()) {
    case '\'':
    case '"':

    case '(':
    case '[':
    case '{':

    case ')':
    case ']':
    case '}':

    case ';':
      break;

    default:
      if (ch.isNull())
        break;

      return false;
  } // end of switch

  const dslparser::TokenTextLocation token = tokenUnderCursor(cursor);
  int tokenType = token.tokenType();
  switch (tokenType)
  {
    case dslparser::MultiLineComment:
      return false;

    case dslparser::String:
    {
      const QString blockText = cursor.block().text();
      const QStringRef tokenText = blockText.midRef(token.start(), token.length());
      QChar quote = tokenText.at(0);
      // if a string literal doesn't start with a quote, it must be multiline
      if (quote != QLatin1Char('"') && quote != QLatin1Char('\''))
      {
        const int startState = blockStartState(cursor.block());
        if(startState)
          quote = QLatin1Char('"');//TODO: look at this!
//        if ((startState & QmlJS::Scanner::MultiLineMask) == QmlJS::Scanner::MultiLineStringDQuote)
//          quote = QLatin1Char('"');
//        else if ((startState & QmlJS::Scanner::MultiLineMask) == QmlJS::Scanner::MultiLineStringSQuote)
//          quote = QLatin1Char('\'');
      }

      // never insert ' into string literals, it adds spurious ' when writing contractions
      if (ch == QLatin1Char('\''))
        return false;

      if (ch != quote || isCompleteStringLiteral(tokenText))
        break;

      return false;
    }

    default:
      break;
  } // end of switch

  return true;
}

bool DslAutoCompleter::contextAllowsElectricCharacters(const QTextCursor &cursor) const
{
  dslparser::TokenTextLocation token = tokenUnderCursor(cursor);
  int tokenType = token.tokenType();
  switch (tokenType)
  {
    case dslparser::MultiLineComment:
    case dslparser::String:
      return false;
    default:
      return true;
  }
}

bool DslAutoCompleter::isInComment(const QTextCursor &cursor) const
{
  dslparser::TokenTextLocation token = tokenUnderCursor(cursor);
  int tokenType = token.tokenType();

  return (tokenType == dslparser::MultiLineComment
          || tokenType == dslparser::LineComment);
}

QString DslAutoCompleter::insertMatchingBrace(const QTextCursor &cursor,
                                                 const QString &text,
                                                 QChar,
                                                 int *skippedChars) const
{
  if (text.length() != 1)
    return QString();

  if (! shouldInsertMatchingText(cursor))
    return QString();

  const QChar la = cursor.document()->characterAt(cursor.position());

  const QChar ch = text.at(0);
  switch (ch.unicode()) {
    case '\'':
      if (la != ch)
        return QString(ch);
      ++*skippedChars;
      break;

    case '"':
      if (la != ch)
        return QString(ch);
      ++*skippedChars;
      break;

    case '(':
      return QString(QLatin1Char(')'));

    case '[':
      return QString(QLatin1Char(']'));

    case '{':
      return QString(); // nothing to do.

    case ')':
    case ']':
    case '}':
    case ';':
      if (la == ch)
        ++*skippedChars;
      break;

    default:
      break;
  } // end of switch

  return QString();
}

QString DslAutoCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
{
  if (shouldInsertNewline(cursor)) {
    QTextCursor selCursor = cursor;
    selCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    if (! selCursor.selectedText().trimmed().isEmpty())
      return QString();

    return QLatin1String("}\n");
  }

  return QLatin1String("}");
}

} // namespace dsleditor

