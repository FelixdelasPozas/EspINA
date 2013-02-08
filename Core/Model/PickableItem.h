/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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


#ifndef SELECTABLEITEM_H
#define SELECTABLEITEM_H

#include "Core/Model/ModelItem.h"

#include "Core/EspinaTypes.h"
#include "Core/Model/Filter.h"

#include <QIcon>

class vtkAlgorithmOutput;

namespace EspINA
{

  class PickableItem
  : public ModelItem
  {
    Q_OBJECT
  public:
    typedef QPair<QString, QString> ConditionInfo;

  public:
    ~PickableItem(){}

    virtual bool isSelected() const {return m_isSelected;}
    virtual void setSelected(bool value) {m_isSelected = value;}

    virtual const FilterSPtr filter() const = 0;
    virtual FilterSPtr filter() = 0;
    virtual const Filter::OutputId  outputId() const = 0;

    EspinaVolume::Pointer volume();
    const EspinaVolume::Pointer volume() const;

    /// Add a new condition to the item:
    /// Conditions provide extra information about the state of the item
    /// i.e. Discarted by Counting Region
    void addCondition(QString state, QString icon, QString description)
    {
      m_conditions[state] = ConditionInfo(icon, description);
    }

    /// Return whether item's volume has been modified or not after its creation
    bool isVolumeModified() {return m_isVolumeModified; }

  protected slots:
    void onVolumeModified() { m_isVolumeModified = true; emit volumeModified(); }

  signals:
    void volumeModified();

  protected:
    QMap<QString, ConditionInfo> m_conditions;

    bool m_isSelected;
    bool m_isVolumeModified; // sticky bit

  };

  typedef QSharedPointer<PickableItem> PickableItemSPtr;

  PickableItemPtr pickableItemPtr(ModelItemPtr item);
  PickableItemSPtr pickableItemPtr(ModelItemSPtr &item);

} // namespace EspINA

#endif // SELECTABLEITEM_H
