#include "dsloutlinemodel.hpp"

#include <mobata/memory_leak_start.hpp>

using namespace dslparser;

namespace dsleditor {

class DslOutlineModel::Private
{
  friend class DslOutlineModel;

  ModelTextLocations  _modelTextLocations;
  TokenTextLocations  _keywordTextLocations;
};

DslOutlineModel::DslOutlineModel(QObject* parent)
  : QStandardItemModel(parent),
    _d(new Private)
{}

DslOutlineModel::~DslOutlineModel()
{
  delete this->_d;
}

void DslOutlineModel::reset()
{
  this->_d->_modelTextLocations.clear();
  this->_d->_keywordTextLocations.clear();

  return;
}

TokenTextLocation const* DslOutlineModel::modelTextLocation(const QModelIndex &modelIntex) const
{
  ModelTextLocations::ConstIterator mt_it=this->_d->_modelTextLocations.find(modelIntex);
  if(mt_it == this->_d->_modelTextLocations.end())
    return 0;

  return &mt_it.value();
}

const ModelTextLocations& DslOutlineModel::modelTextSelections() const
{
  return this->_d->_modelTextLocations;
}

const TokenTextLocations& DslOutlineModel::keywordTextSelections() const
{
  return this->_d->_keywordTextLocations;
}

void DslOutlineModel::setModelTextLocations(const ModelTextLocations& modelTextLocations)
{
  this->_d->_modelTextLocations = modelTextLocations;

  return;
}

void DslOutlineModel::setKeywordTextLocations(const TokenTextLocations& keywordTextLocations)
{
  this->_d->_keywordTextLocations = keywordTextLocations;

  return;
}

} // namespace dsleditor
