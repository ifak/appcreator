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

#include "dsleditorwidget.hpp"

#include "dslhighlighter.hpp"

#include <texteditor/basetextdocument.h>
#include <texteditor/fontsettings.h>

#include <utils/uncommentselection.h>

#include <QTextCodec>

#include <mobata/memory_leak_start.hpp>

namespace dsleditor {

class DslEditorWidget::Private
{
  friend class DslEditorWidget;
};

DslEditorWidget::DslEditorWidget(QWidget *parent)
  : TextEditor::BaseTextEditorWidget(parent),
    _d(new Private)
{
  this->setRevisionsVisible(true);
  this->setParenthesesMatchingEnabled(true);
  this->setMarksVisible(true);
  this->setLineSeparatorsAllowed(true);
  this->setCodeFoldingSupported(true);
}

DslEditorWidget::~DslEditorWidget()
{
  delete this->_d;
}

void DslEditorWidget::setFontSettings(const TextEditor::FontSettings &fs)
{
  TextEditor::BaseTextEditorWidget::setFontSettings(fs);
  DslHighlighter* highlighter = qobject_cast<DslHighlighter*>(baseTextDocument()->syntaxHighlighter());
  if (!highlighter)
    return;

  for (const TextEditor::TextStyle textStyle : this->highlighterFormatCategories())
    highlighter->setTextCharFormat(textStyle, fs.toTextCharFormat(textStyle));
  highlighter->rehighlight();

  return;
}

QVector<TextEditor::TextStyle> DslEditorWidget::highlighterFormatCategories()
{
  QVector<TextEditor::TextStyle> categories;
  categories << TextEditor::C_NUMBER
             << TextEditor::C_STRING
             << TextEditor::C_TYPE
             << TextEditor::C_KEYWORD
             << TextEditor::C_FIELD
             << TextEditor::C_COMMENT
             << TextEditor::C_PARENTHESES
             << TextEditor::C_VISUAL_WHITESPACE
             << TextEditor::C_TEXT;

  return categories;
}

} // namespace dsleditor
