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

#include "dslcodeformatter.hpp"

#include <dslparser/common/comcreatecommontokens.hpp>

#include <QDebug>
#include <QMetaEnum>
#include <QTextDocument>
#include <QTextBlock>

using namespace dsleditor;

CodeFormatter::BlockData::BlockData()
  : m_indentDepth(0)
  , m_blockRevision(-1)
{
}

CodeFormatter::CodeFormatter()
  : m_indentDepth(0)
  , m_tabSize(4)
{
}

CodeFormatter::~CodeFormatter()
{
}

void CodeFormatter::setTabSize(int tabSize)
{
  m_tabSize = tabSize;
}

void CodeFormatter::recalculateStateAfter(const QTextBlock &block)
{
  using namespace dslparser;

  restoreCurrentState(block.previous());

  /*const int lexerState = */this->tokenizeBlock(block);
  m_tokenIndex = 0;
  m_newStates.clear();

//  qDebug() << "Starting to look at " << block.text() << block.blockNumber() + 1;

  for (; m_tokenIndex < m_tokens.size(); )
  {
    this->m_currentToken = this->tokenAt(m_tokenIndex);
    const int kind = this->m_currentToken.tokenType();

//    qDebug()<<"current code formatter state: "<<this->stateToString(this->state().type);
//    qDebug()<<"current indent depth: "<<this->state().savedIndentDepth;

//    qDebug() << "Token" << m_currentLine.mid(m_currentToken.start(), m_currentToken.length())
//             << m_tokenIndex << "in line" << block.blockNumber() + 1;
// //    dump();

    if (kind == MultiLineComment
        && this->state().type != multiline_comment)
    {
      int indexCommentOff = block.text().indexOf(QStringLiteral("*/"));
      if(indexCommentOff == 0) // really multi line
        this->enter(multiline_comment);

      ++this->m_tokenIndex;
      continue;
    }

    switch (m_currentState.top().type)
    {
      case topmost_intro:
        switch (kind)
        {
//          case Identifier:  enter(binding_or_objectdefinition); break;
          case LeftBrace:     enter(objectdefinition_open); break;
        } break;

//      case binding_or_objectdefinition:
//        switch (kind)
//        {
//          case Colon:         enter(binding_assignment); break;
//          case LeftBrace:     enter(objectdefinition_open); break;
//        } break;

      case objectdefinition_open:
        switch (kind)
        {
          case Colon:         enter(binding_assignment); break;
          case LeftBrace:     enter(objectdefinition_open); break;
          case RightBrace:    leave(); /*leave();*/ break;
          case LeftParen:     enter(paren_open); break;
//          case Signal:        enter(signal_start); break;
//          case Identifier:    enter(binding_or_objectdefinition); break;
        } break;

      case binding_assignment:
        switch (kind)
        {
          case Semi:          leave(); /*leave();*/ break;
          case LeftBrace:     enter(objectdefinition_open); break;
//          case Identifier:    enter(expression_or_objectdefinition); break;
        } break;

      case signal_start:
        switch (kind)
        {
          case Colon:         enter(binding_assignment); break; // oops, was a binding
          default:            enter(signal_maybe_arglist); break;
        } break;

      case signal_maybe_arglist:
        switch (kind)
        {
          case LeftParen:   turnInto(signal_arglist_open); break;
          default:          leave(true); continue;
        } break;

      case signal_arglist_open:
        switch (kind)
        {
          case RightParen:  leave(true); break;
        } break;

      case expression_or_objectdefinition:
        switch (kind)
        {
          case Dot:
          case Identifier:    break; // need to become an objectdefinition_open in cases like "width: Qt.Foo {"
          case LeftBrace:     turnInto(objectdefinition_open); break;

            // propagate 'leave' from expression state
          case RightBracket:
          case RightParen:    leave(); continue;
          case RightBrace:    leave(); break;

          default:            enter(expression); continue; // really? identifier and more tokens might already be gone
        } break;

      case expression_or_label:
        switch (kind)
        {
          //          case Colon:         turnInto(labelled_statement); break;

          // propagate 'leave' from expression state
          case RightBracket:
          case RightParen:    leave(); continue;

          default:            enter(expression); continue;
        } break;

      case expression:
       /*if (tryInsideExpression())
          break;*/
        switch (kind)
        {
          case RightBrace:  leave(); break;
          case Semi:        leave(); break;
        } break;

      case expression_continuation:
        leave();
        continue;

      case expression_maybe_continuation:
        switch (kind)
        {
          case LeftBracket:
          case LeftParen:   leave(); continue;
          default:          leave(true); continue;
        } break;

      case paren_open:
//        if (tryInsideExpression())
//          break;
        switch (kind)
        {
          case LeftParen:   enter(paren_open); break;
          case RightParen:  leave(); break;
        } break;

      case objectliteral_open:
        if (tryInsideExpression())
          break;
        switch (kind)
        {
          case Colon:             enter(objectliteral_assignment); break;
          case RightBracket:
          case RightParen:        leave(); continue; // error recovery
          case RightBrace:        leave(true); break;
        } break;

        // pretty much like expression, but ends with , or }
      case objectliteral_assignment:
        if (tryInsideExpression())
          break;
        switch (kind)
        {
          case RightBracket:
          case RightParen:        leave(); continue; // error recovery
          case RightBrace:        leave(); continue; // so we also leave objectliteral_open
          case Comma:             leave(); break;
        } break;

      case multiline_comment:
      {
        switch (kind)
        {
          case MultiLineComment:  break;
          default:                this->leave(); continue;
        } break;

      }

      default:
        qWarning() << "Unhandled state" << m_currentState.top().type;
        break;
    } // end of state switch

    ++m_tokenIndex;
  }

  this->saveCurrentState(block);

  return;
}

int CodeFormatter::indentFor(const QTextBlock &block)
{
  //    qDebug() << "indenting for" << block.blockNumber() + 1;

  restoreCurrentState(block.previous());
  correctIndentation(block);
  return m_indentDepth;
}

int CodeFormatter::indentForNewLineAfter(const QTextBlock &block)
{
  restoreCurrentState(block);

  m_tokens.clear();
  m_currentLine.clear();
  const int startLexerState = loadLexerState(block.previous());
  adjustIndent(m_tokens, startLexerState, &m_indentDepth);

  return m_indentDepth;
}

void CodeFormatter::updateStateUntil(const QTextBlock &endBlock)
{
  QStack<State> previousState = initialState();
  QTextBlock it = endBlock.document()->firstBlock();

  // find the first block that needs recalculation
  for (; it.isValid() && it != endBlock; it = it.next()) {
    BlockData blockData;
    if (!loadBlockData(it, &blockData))
      break;
    if (blockData.m_blockRevision != it.revision())
      break;
    if (previousState != blockData.m_beginState)
      break;
    if (loadLexerState(it) == -1)
      break;

    previousState = blockData.m_endState;
  }

  if (it == endBlock)
    return;

  // update everthing until endBlock
  for (; it.isValid() && it != endBlock; it = it.next()) {
    recalculateStateAfter(it);
  }

  // invalidate everything below by marking the state in endBlock as invalid
  if (it.isValid()) {
    BlockData invalidBlockData;
    saveBlockData(&it, invalidBlockData);
  }
}

void CodeFormatter::updateLineStateChange(const QTextBlock &block)
{
  if (!block.isValid())
    return;

  BlockData blockData;
  if (loadBlockData(block, &blockData) && blockData.m_blockRevision == block.revision())
    return;

  recalculateStateAfter(block);

  // invalidate everything below by marking the next block's state as invalid
  QTextBlock next = block.next();
  if (!next.isValid())
    return;

  saveBlockData(&next, BlockData());
}

CodeFormatter::State CodeFormatter::state(int belowTop) const
{
  if (belowTop < m_currentState.size())
    return m_currentState.at(m_currentState.size() - 1 - belowTop);
  else
    return State();
}

const QVector<CodeFormatter::State> &CodeFormatter::newStatesThisLine() const
{
  return m_newStates;
}

int CodeFormatter::tokenIndex() const
{
  return m_tokenIndex;
}

int CodeFormatter::tokenCount() const
{
  return m_tokens.size();
}

const dslparser::TokenTextLocation& CodeFormatter::currentToken() const
{
  return m_currentToken;
}

void CodeFormatter::invalidateCache(QTextDocument *document)
{
  if (!document)
    return;

  BlockData invalidBlockData;
  QTextBlock it = document->firstBlock();
  for (; it.isValid(); it = it.next()) {
    saveBlockData(&it, invalidBlockData);
  }
}

void CodeFormatter::enter(int newState)
{
  int savedIndentDepth = m_indentDepth;
  onEnter(newState, &m_indentDepth, &savedIndentDepth);
  State s(newState, this->m_indentDepth/*savedIndentDepth*/);
  m_currentState.push(s);
  m_newStates.push(s);

//  qDebug() << "enter state" << stateToString(newState);

  return;
}

void CodeFormatter::leave(bool statementDone)
{
  Q_UNUSED(statementDone)
  Q_ASSERT(m_currentState.size() > 1);

  if (m_currentState.top().type == topmost_intro)
    return;

  if (m_newStates.size() > 0)
    m_newStates.pop();

  /*State poppedState = */m_currentState.pop();
  State newTopState = this->m_currentState.top();
  // restore indent depth
  m_indentDepth = newTopState.savedIndentDepth;

//  qDebug() << "previous state" << stateToString(poppedState.type)
//           << ", now in state" << stateToString(newTopState.type);

  return;
}

void CodeFormatter::correctIndentation(const QTextBlock &block)
{
  tokenizeBlock(block);
  Q_ASSERT(m_currentState.size() >= 1);

  const int startLexerState = loadLexerState(block.previous());
  adjustIndent(m_tokens, startLexerState, &m_indentDepth);
}

bool CodeFormatter::tryInsideExpression(bool alsoExpression)
{
  using namespace dslparser;

  int newState = -1;
  const int kind = this->m_currentToken.tokenType();
  switch (kind)
  {
    case LeftParen:   newState = paren_open; break;
    case LeftBrace:   newState = objectliteral_open; break;
//    case Function:    newState = function_start; break;
  }

  if (newState != -1) {
    if (alsoExpression)
      enter(expression);
    enter(newState);
    return true;
  }

  return false;
}

bool CodeFormatter::tryStatement()
{
  using namespace dslparser;

  const int kind = this->m_currentToken.tokenType();
  switch (kind)
  {
    case Semi:
    case LeftBrace:
      enter(jsblock_open);
      return true;
    case Identifier:
      enter(expression_or_label);
      return true;
    case Import:
    case Signal:
    case As:
//    case Function:
    case Int:
    case String:
    case LeftParen:
      enter(expression);
      // look at the token again
      m_tokenIndex -= 1;
      return true;
  }

  return false;
}

bool CodeFormatter::isBracelessState(int type) const
{
  return
      type == binding_assignment ||
      type == binding_or_objectdefinition;
}

bool CodeFormatter::isExpressionEndState(int type) const
{
  return
      type == topmost_intro ||
      type == objectdefinition_open ||
      type == jsblock_open ||
      type == paren_open ||
      type == objectliteral_open;
}

const dslparser::TokenTextLocation& CodeFormatter::tokenAt(int idx) const
{
  static const dslparser::TokenTextLocation empty;
  if (idx < 0 || idx >= m_tokens.size())
    return empty;
  else
    return m_tokens.at(idx);
}

int CodeFormatter::column(int index) const
{
  int col = 0;
  if (index > m_currentLine.length())
    index = m_currentLine.length();

  const QChar tab = QLatin1Char('\t');

  for (int i = 0; i < index; i++) {
    if (m_currentLine[i] == tab)
      col = ((col / m_tabSize) + 1) * m_tabSize;
    else
      col++;
  }
  return col;
}

QStringRef CodeFormatter::currentTokenText() const
{
  return m_currentLine.midRef(m_currentToken.start(), m_currentToken.length());
}

void CodeFormatter::turnInto(int newState)
{
  leave(false);
  enter(newState);
}

void CodeFormatter::saveCurrentState(const QTextBlock &block)
{
  if (!block.isValid())
    return;

  BlockData blockData;
  blockData.m_blockRevision = block.revision();
  blockData.m_beginState = m_beginState;
  blockData.m_endState = m_currentState;
  blockData.m_indentDepth = m_indentDepth;

  QTextBlock saveableBlock(block);
  saveBlockData(&saveableBlock, blockData);
}

void CodeFormatter::restoreCurrentState(const QTextBlock &block)
{
  if (block.isValid()) {
    BlockData blockData;
    if (loadBlockData(block, &blockData)) {
      m_indentDepth = blockData.m_indentDepth;
      m_currentState = blockData.m_endState;
      m_beginState = m_currentState;
      return;
    }
  }

  m_currentState = initialState();
  m_beginState = m_currentState;
  m_indentDepth = 0;
}

QStack<CodeFormatter::State> CodeFormatter::initialState()
{
  static QStack<CodeFormatter::State> initialState;
  if (initialState.isEmpty())
    initialState.push(State(topmost_intro, 0));
  return initialState;
}

int CodeFormatter::tokenizeBlock(const QTextBlock &block)
{
  int startState = loadLexerState(block.previous());
  if (block.blockNumber() == 0)
    startState = 0;
  Q_ASSERT(startState != -1);

  this->m_tokens.clear();

  QString blockText = block.text();
  this->m_currentLine = blockText;

  if(this->state().type == multiline_comment)
  {
    int indexCommentOff = blockText.indexOf(QStringLiteral("*/"));
    if(indexCommentOff >= 0)
    {
      this->m_tokens.append(dslparser::TokenTextLocation(0, indexCommentOff +2,
                                                         dslparser::MultiLineComment));
      blockText.remove(0, indexCommentOff +2);
    }
    else
    {
      this->m_tokens.append(dslparser::TokenTextLocation(0, blockText.size(),
                                                         dslparser::MultiLineComment));
      blockText.clear();
    }
  }

  if(blockText.size())
  {
    dslparser::common::ComCreateCommonTokens tokensCommand(blockText);
    QString errorString;
    bool result = tokensCommand.execute(&errorString);
    if(!result)
    {
      qDebug()<<"error during token creation for line '"<<this->m_currentLine<<"': "<<errorString;
      return -1;
    }
    this->m_tokens.append(tokensCommand.commonTokenList().toList());
  }

  const int lexerState = startState;
  QTextBlock saveableBlock(block);
  saveLexerState(&saveableBlock, lexerState);

  return lexerState;
}

void CodeFormatter::dump() const
{
  qDebug() << "Current token index" << m_tokenIndex;
  qDebug() << "Current state:";
  foreach (const State &s, m_currentState) {
    qDebug() << stateToString(s.type) << s.savedIndentDepth;
  }
  qDebug() << "Current indent depth:" << m_indentDepth;
}

QString CodeFormatter::stateToString(int type) const
{
  const QMetaEnum &metaEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("StateType"));
  return QString::fromUtf8(metaEnum.valueToKey(type));
}

QtStyleCodeFormatter::QtStyleCodeFormatter()
  : m_indentSize(4)
{}

void QtStyleCodeFormatter::setIndentSize(int size)
{
  m_indentSize = size;
}

void QtStyleCodeFormatter::onEnter(int newState, int *indentDepth, int *savedIndentDepth) const
{
  const State &parentState = state();
  const dslparser::TokenTextLocation &tk = currentToken();
  const int tokenPosition = column(tk.start());
  const bool firstToken = (tokenIndex() == 0);
  const bool lastToken = (tokenIndex() == tokenCount() - 1);

  switch (newState) {
    case objectdefinition_open: {
      // special case for things like "gradient: Gradient {"
      if (parentState.type == binding_assignment)
        *savedIndentDepth = state(1).savedIndentDepth;

      if (firstToken)
        *savedIndentDepth = tokenPosition;

      *indentDepth = *savedIndentDepth + m_indentSize;
      break;
    }

    case binding_or_objectdefinition:
      if (firstToken)
        *indentDepth = *savedIndentDepth = tokenPosition;
      break;

    case binding_assignment:
      break;
//    case objectliteral_assignment:
//      if (lastToken)
//        *indentDepth = *savedIndentDepth + 4;
//      else
//        *indentDepth = column(tokenAt(tokenIndex() + 1).start());
//      break;

    case expression_or_objectdefinition:
      *indentDepth = tokenPosition;
      break;

    case expression_or_label:
      if (*indentDepth == tokenPosition)
        *indentDepth += 2*m_indentSize;
      else
        *indentDepth = tokenPosition;
      break;

    case expression:
      if (*indentDepth == tokenPosition) {
        // expression_or_objectdefinition doesn't want the indent
        // expression_or_label already has it
        if (parentState.type != expression_or_objectdefinition
            && parentState.type != expression_or_label
            && parentState.type != binding_assignment) {
          *indentDepth += 2*m_indentSize;
        }
      }
      // expression_or_objectdefinition and expression_or_label have already consumed the first token
      else if (parentState.type != expression_or_objectdefinition
               && parentState.type != expression_or_label) {
        *indentDepth = tokenPosition;
      }
      break;

    case expression_maybe_continuation:
      // set indent depth to indent we'd get if the expression ended here
      for (int i = 1; state(i).type != topmost_intro; ++i) {
        const int type = state(i).type;
        if (isExpressionEndState(type) && !isBracelessState(type)) {
          *indentDepth = state(i - 1).savedIndentDepth;
          break;
        }
      }
      break;

    case function_start:
      // align to the beginning of the line
      *savedIndentDepth = *indentDepth = column(tokenAt(0).start());
      break;

    case signal_arglist_open:
    case function_arglist_open:
    case paren_open:
      if (lastToken && parentState.type != paren_open)
        *indentDepth += m_indentSize;
      else
        *indentDepth = tokenPosition + 1;
      break;

    case jsblock_open:
      // closing brace should be aligned to case
      //      if (parentState.type == case_cont)
      //      {
      //        *savedIndentDepth = parentState.savedIndentDepth;
      //        break;
      //      }

    case objectliteral_open:
      if (parentState.type == expression
          || parentState.type == objectliteral_assignment) {
        // undo the continuation indent of the expression
        if (state(1).type == expression_or_label)
          *indentDepth = state(1).savedIndentDepth;
        else
          *indentDepth = parentState.savedIndentDepth;
        *savedIndentDepth = *indentDepth;
      }
      *indentDepth += m_indentSize;
      break;

    case multiline_comment:
      *indentDepth = tokenPosition /*+ 2*/;
      break;

//    case multiline_comment_cont:
//      *indentDepth = tokenPosition;
//      break;
  }

  return;
}

void QtStyleCodeFormatter::adjustIndent(const QList<dslparser::TokenTextLocation>& tokens,
                                        int startLexerState,
                                        int *indentDepth) const
{
  Q_UNUSED(startLexerState)

  Q_ASSERT(indentDepth);

  State topState = this->state();
  State previousState = this->state(1);

  // keep user-adjusted indent in multiline comments
  if (topState.type == multiline_comment)
  {
    if (!tokens.isEmpty())
    {
      *indentDepth = column(tokens.at(0).start());
      return;
    }
  }

  // don't touch multi-line strings at all
  //  if ((startLexerState & Scanner::MultiLineMask) == Scanner::MultiLineStringDQuote
  //      || (startLexerState & Scanner::MultiLineMask) == Scanner::MultiLineStringSQuote) {
  //    *indentDepth = -1;
  //    return;
  //  }

  const int kind = this->tokenAt(0).tokenType();
  switch (kind)
  {
    case dslparser::LeftBrace:
    {
      if (topState.type == binding_assignment)
      {
        *indentDepth = topState.savedIndentDepth;
      }
      break;
    }
    case dslparser::RightBrace:
    {
      if (topState.type == objectdefinition_open
          || topState.type == expression)
      {
//        qDebug()<<"'}': previousStateDepth"<<previousState.savedIndentDepth
//               <<"*indentDepth:"<<(*indentDepth = previousState.savedIndentDepth);
        *indentDepth = previousState.savedIndentDepth;
      }
      break;
    }

    case dslparser::LeftParen:
      break;
  }

  return;
}

DslCodeFormatter::DslCodeFormatter()
{}

DslCodeFormatter::DslCodeFormatter(const TextEditor::TabSettings &tabSettings)
{
  setTabSize(tabSettings.m_tabSize);
  setIndentSize(tabSettings.m_indentSize);
}

DslCodeFormatter::~DslCodeFormatter()
{}

void DslCodeFormatter::saveBlockData(QTextBlock *block,
                                     const BlockData &data) const
{
  TextEditor::TextBlockUserData *userData = TextEditor::BaseTextDocumentLayout::userData(*block);
  DslEditorCodeFormatterData* codeFormatData = static_cast<DslEditorCodeFormatterData *>(userData->codeFormatterData());
  if (!codeFormatData)
  {
    codeFormatData = new DslEditorCodeFormatterData;
    userData->setCodeFormatterData(codeFormatData);
  }
  codeFormatData->m_data = data;

  return;
}

bool DslCodeFormatter::loadBlockData(const QTextBlock &block, BlockData *data) const
{
  TextEditor::TextBlockUserData *userData = TextEditor::BaseTextDocumentLayout::testUserData(block);
  if (!userData)
    return false;

  DslEditorCodeFormatterData *codeFormatData = static_cast<DslEditorCodeFormatterData *>(userData->codeFormatterData());
  if (!codeFormatData)
    return false;

  *data = codeFormatData->m_data;
  return true;
}

void DslCodeFormatter::saveLexerState(QTextBlock *block,
                                      int state) const
{
  TextEditor::BaseTextDocumentLayout::setLexerState(*block,
                                                    state);

  return;
}

int DslCodeFormatter::loadLexerState(const QTextBlock &block) const
{
  return TextEditor::BaseTextDocumentLayout::lexerState(block);
}
