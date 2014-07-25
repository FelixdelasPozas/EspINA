/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef ESPINA_ITEM_ADAPTER_H
#define ESPINA_ITEM_ADAPTER_H

#include "GUI/EspinaGUI_Export.h"

#include <QModelIndex>

#include "Core/EspinaTypes.h"

namespace ESPINA
{

  const int RawPointerRole = Qt::UserRole+1;
  const int TypeRole       = Qt::UserRole+2;

  class ModelAdapter;

  class ItemAdapter;
  using ItemAdapterPtr   = ItemAdapter*;
  using ItemAdapterList  = QList<ItemAdapterPtr>;
  using ItemAdapterSPtr  = std::shared_ptr<ItemAdapter>;
  using ItemAdapterSList = QList<ItemAdapterSPtr>;

  /// Base class for every item in EspinaModel
  class EspinaGUI_EXPORT ItemAdapter
  : public QObject
  {
  public:
    enum class Type {
      SAMPLE,
      CHANNEL,
      SEGMENTATION,
      CLASSIFICATION,
      CATEGORY
    };

    static int typeId(Type type)
    {
      switch (type) {
        case Type::SAMPLE:
          return 0;
        case Type::CHANNEL:
          return 1;
        case Type::SEGMENTATION:
          return 2;
        case Type::CLASSIFICATION:
          return 4;
        case Type::CATEGORY:
          return 5;
      }
      return -1;
    }

    static Type type(int id)
    {
      switch (id) {
        case 0:
          return Type::SAMPLE;
        case 1:
          return Type::CHANNEL;
        case 2:
          return Type::SEGMENTATION;
        case 4:
          return Type::CLASSIFICATION;
        case 5:
          return Type::CATEGORY;
        default:
          throw -1;
      }
    }

    Q_OBJECT
  public:
    explicit ItemAdapter(PersistentSPtr analysisItem)
    : m_analysisItem(analysisItem) {}

    virtual ~ItemAdapter(){}

    virtual QVariant data(int role=Qt::DisplayRole) const = 0;

    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1) = 0;

    virtual ItemAdapter::Type type() const = 0;

  public slots:
    virtual void notifyModification()
    { emit modified(this); }

  signals:
    void modified(ItemAdapterPtr);

  protected:

    PersistentSPtr m_analysisItem;

    friend class ModelAdapter;
//     friend bool operator==(ItemAdapterSPtr lhs, PersistentSPtr  rhs);
//     friend bool operator==(PersistentSPtr  lhs, ItemAdapterSPtr rhs);
  };

//   bool operator==(ItemAdapterSPtr lhs, PersistentSPtr  rhs);
//   bool operator==(PersistentSPtr  lhs, ItemAdapterSPtr rhs);
// 
//   bool operator!=(ItemAdapterSPtr lhs, PersistentSPtr  rhs);
//   bool operator!=(PersistentSPtr  lhs, ItemAdapterSPtr rhs);
} // ESPINA



#endif // ESPINA_ITEM_ADAPTER_H
