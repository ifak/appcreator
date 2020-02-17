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

#include "dslindenter.hpp"
#include "dslcodeformatter.hpp"

#include <texteditor/tabsettings.h>

#include <QChar>
#include <QTextDocument>
#include <QTextBlock>

#include <mobata/memory_leak_start.hpp>

namespace dsleditor {

DslIndenter::DslIndenter()
{}

DslIndenter::~DslIndenter()
{}

bool DslIndenter::isElectricCharacter(const QChar &ch) const
{
  if (ch == QLatin1Char('{')
      || ch == QLatin1Char('}')
      || ch == QLatin1Char(']')
      || ch == QLatin1Char(':'))
    return true;
  return false;
}

void DslIndenter::indentBlock(QTextDocument *doc,
                              const QTextBlock &block,
                              const QChar &typedChar,
                              const TextEditor::TabSettings &tabSettings)
{
  Q_UNUSED(doc)

  DslCodeFormatter codeFormatter(tabSettings);

  codeFormatter.updateStateUntil(block);
  const int depth = codeFormatter.indentFor(block);
  if (depth == -1)
    return;

  if (isElectricCharacter(typedChar)) {
    // only reindent the current line when typing electric characters if the
    // indent is the same it would be if the line were empty
    const int newlineIndent = codeFormatter.indentForNewLineAfter(block.previous());
    if (tabSettings.indentationColumn(block.text()) != newlineIndent)
      return;
  }

  tabSettings.indentLine(block, depth);
}

void DslIndenter::invalidateCache(QTextDocument *doc)
{
  DslCodeFormatter codeFormatter;
  codeFormatter.invalidateCache(doc);
}

} // namespace dsleditor

