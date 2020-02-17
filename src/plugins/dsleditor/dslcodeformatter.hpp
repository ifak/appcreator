#pragma once

#include "dsleditor_global.hpp"

#include <dslparser/dslparser_types.hpp>

#include <texteditor/tabsettings.h>
#include <texteditor/basetextdocumentlayout.h>

#include <QStack>
#include <QList>
#include <QVector>
#include <QObject>

QT_BEGIN_NAMESPACE
class QTextDocument;
class QTextBlock;
QT_END_NAMESPACE

namespace dsleditor {

class DSLEDITOR_EXPORT CodeFormatter
{
  Q_GADGET
public:
  CodeFormatter();
  virtual ~CodeFormatter();

  // updates all states up until block if necessary
  // it is safe to call indentFor on block afterwards
  void updateStateUntil(const QTextBlock &block);

  // calculates the state change introduced by changing a single line
  void updateLineStateChange(const QTextBlock &block);

  int indentFor(const QTextBlock &block);
  int indentForNewLineAfter(const QTextBlock &block);

  void setTabSize(int tabSize);

  void invalidateCache(QTextDocument *document);

protected:
  virtual void onEnter(int newState, int *indentDepth, int *savedIndentDepth) const = 0;
  virtual void adjustIndent(const QList<dslparser::TokenTextLocation>& tokens,
                            int startLexerState,
                            int* indentDepth) const = 0;

  struct State;
  class DSLEDITOR_EXPORT BlockData
  {
  public:
    BlockData();

    QStack<State> m_beginState;
    QStack<State> m_endState;
    int m_indentDepth;
    int m_blockRevision;
  };

  virtual void saveBlockData(QTextBlock *block, const BlockData &data) const = 0;
  virtual bool loadBlockData(const QTextBlock &block, BlockData *data) const = 0;

  virtual void saveLexerState(QTextBlock *block, int state) const = 0;
  virtual int loadLexerState(const QTextBlock &block) const = 0;

public: // must be public to make Q_GADGET introspection work
  enum StateType {
    invalid = 0,

    topmost_intro, // The first line in a "topmost" definition.

    top_spenat, // root state for spenat

    multiline_comment,

    import_start, // after 'import'
    import_maybe_dot_or_version_or_as, // after string or identifier
    import_dot, // after .
    import_maybe_as, // after version
    import_as,

    attribute_start, // after 'Attribute'
    attribute_name, // after the type
    attribute_maybe_initializer, // after the identifier

    signal_start, // after 'Signal'
    signal_maybe_arglist, // after identifier
    signal_arglist_open, // after '('

    function_start, // after 'function'
    function_arglist_open, // after '(' starting function argument list
    function_arglist_closed, // after ')' in argument list, expecting '{'

    binding_or_objectdefinition, // after an identifier

    binding_assignment, // after : in a binding
    objectdefinition_open, // after {

    expression,
    expression_continuation, // at the end of the line, when the next line definitely is a continuation
    expression_maybe_continuation, // at the end of the line, when the next line may be an expression
    expression_or_objectdefinition, // after a binding starting with an identifier ("x: foo")
    expression_or_label, // when expecting a statement and getting an identifier

    paren_open, // opening ( in expression
    objectliteral_open, // opening { in expression

    objectliteral_assignment, // after : in object literal

    jsblock_open,
  };
  Q_ENUMS(StateType)

protected:
  struct State {
    State()
      : savedIndentDepth(0)
      , type(0)
    {}

    State(quint8 ty, quint16 savedDepth)
      : savedIndentDepth(savedDepth)
      , type(ty)
    {}

    quint16 savedIndentDepth;
    quint8 type;

    bool operator==(const State &other) const {
      return type == other.type
          && savedIndentDepth == other.savedIndentDepth;
    }
  };

  State state(int belowTop = 0) const;
  const QVector<State> &newStatesThisLine() const;
  int tokenIndex() const;
  int tokenCount() const;
  const dslparser::TokenTextLocation& currentToken() const;
  const dslparser::TokenTextLocation& tokenAt(int idx) const;
  int column(int position) const;

  bool isBracelessState(int type) const;
  bool isExpressionEndState(int type) const;

  void dump() const;
  QString stateToString(int type) const;

private:
  void recalculateStateAfter(const QTextBlock &block);
  void saveCurrentState(const QTextBlock &block);
  void restoreCurrentState(const QTextBlock &block);

  QStringRef currentTokenText() const;

  int tokenizeBlock(const QTextBlock &block);

  void turnInto(int newState);

  bool tryInsideExpression(bool alsoExpression = false);
  bool tryStatement();

  void enter(int newState);
  void leave(bool statementDone = false);
  void correctIndentation(const QTextBlock &block);

private:
  static QStack<State> initialState();

  QStack<State> m_beginState;
  QStack<State> m_currentState;
  QStack<State> m_newStates;

  QList<dslparser::TokenTextLocation> m_tokens;
  QString m_currentLine;
  dslparser::TokenTextLocation m_currentToken;
  int m_tokenIndex;

  // should store indent level and padding instead
  int m_indentDepth;

  int m_tabSize;
};

class DSLEDITOR_EXPORT QtStyleCodeFormatter
    : public CodeFormatter
{
public:
  QtStyleCodeFormatter();

  void setIndentSize(int size);

protected:
  virtual void onEnter(int newState,
                       int *indentDepth,
                       int *savedIndentDepth) const;
  virtual void adjustIndent(const QList<dslparser::TokenTextLocation> &tokens,
                            int lexerState,
                            int* indentDepth) const;

private:
  int m_indentSize;
};

class DSLEDITOR_EXPORT DslCodeFormatter
    : public QtStyleCodeFormatter
{
public:
  DslCodeFormatter();
  explicit DslCodeFormatter(const TextEditor::TabSettings &tabSettings);

  virtual ~DslCodeFormatter();

protected:
  virtual void saveBlockData(QTextBlock *block, const BlockData &data) const;
  virtual bool loadBlockData(const QTextBlock &block, BlockData *data) const;

  virtual void saveLexerState(QTextBlock *block, int state) const;
  virtual int loadLexerState(const QTextBlock &block) const;

private:
  class DslEditorCodeFormatterData: public TextEditor::CodeFormatterData
  {
  public:
    CodeFormatter::BlockData m_data;
  };
};

} // namespace dsleditor
