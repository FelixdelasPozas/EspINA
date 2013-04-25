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
    PickableItem();
    ~PickableItem(){}

    virtual bool isSelected() const {return m_isSelected;}
    virtual void setSelected(bool value) {m_isSelected = value;}

    virtual const FilterSPtr filter() const = 0;
    virtual FilterSPtr filter() = 0;
 
    virtual const FilterOutputId  outputId() const = 0;

    /// Convenience method
    OutputSPtr output();
    /// Convenience method
    const OutputSPtr output() const;

    /// Convenience method to access output's representations
    EspinaRepresentationList representations() const
    { return output()->representations(); }


    /// Return whether item's volume has been modified or not after its creation
    bool outputIsModified() { return m_outputIsModified; }

  protected slots:
    void onOutputModified() { m_outputIsModified = true; emit outputModified(); emit modified(this);}

  signals:
    void outputModified();

  protected:
    bool m_isSelected;
    bool m_outputIsModified; // sticky bit

  };

  typedef QSharedPointer<PickableItem> PickableItemSPtr;

  PickableItemPtr pickableItemPtr(ModelItemPtr item);
  PickableItemSPtr pickableItemPtr(ModelItemSPtr &item);

} // namespace EspINA

#endif // SELECTABLEITEM_H
