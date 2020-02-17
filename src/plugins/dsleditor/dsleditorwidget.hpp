#pragma once

#include "dsleditor_global.hpp"

#include <texteditor/basetexteditor.h>
#include <texteditor/texteditorconstants.h>

namespace dsleditor {

class DSLEDITOR_EXPORT DslEditorWidget
    : public TextEditor::BaseTextEditorWidget
{
  Q_OBJECT

public:
  explicit DslEditorWidget(QWidget *parent = 0);
  virtual ~DslEditorWidget();

public slots:
  virtual void setFontSettings(const TextEditor::FontSettings &);

protected:
  virtual QVector<TextEditor::TextStyle> highlighterFormatCategories();

private:
  Q_DISABLE_COPY(DslEditorWidget)
  class Private;
  Private* _d;
};

} // namespace dsleditor
