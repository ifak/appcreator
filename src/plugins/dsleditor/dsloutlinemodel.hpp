#pragma once

#include "dsleditor_global.hpp"

#include <dslparser/dslparser_types.hpp>

#include <QStandardItemModel>

namespace dsleditor {

class DSLEDITOR_EXPORT DslOutlineModel
    : public QStandardItemModel
{
public:
  DslOutlineModel(QObject* parent = 0);
  virtual ~DslOutlineModel();

public:
  virtual void reset();

public:
  dslparser::TokenTextLocation const*   modelTextLocation(const QModelIndex& modelIntex) const;
  const dslparser::ModelTextLocations&  modelTextSelections() const;
  const dslparser::TokenTextLocations&  keywordTextSelections() const;

protected:
  void setModelTextLocations(const dslparser::ModelTextLocations& modelTextLocations);
  void setKeywordTextLocations(const dslparser::TokenTextLocations& keywordTextLocations);

private:
  Q_DISABLE_COPY(DslOutlineModel)
  class Private;
  Private* _d;
};

} // namespace dsleditor
