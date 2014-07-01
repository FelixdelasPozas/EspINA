/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_NEURO_ITEM_ADAPTER_H
#define ESPINA_NEURO_ITEM_ADAPTER_H

#include "FilterAdapter.h"
#include <GUI/Representations/Representation.h>
#include "GUI/Model/ItemAdapter.h"

#include <Core/Analysis/NeuroItem.h>

namespace EspINA {

  class ModelAdapter;
  using ModelAdapterPtr = ModelAdapter *;

  class EspinaGUI_EXPORT NeuroItemAdapter
  : public ItemAdapter
  {
  public:
    virtual ~NeuroItemAdapter(){}

    void setModel(ModelAdapterPtr model)
    { m_model = model; }

    ModelAdapterPtr model() const
    { return m_model; }

  protected:
    explicit NeuroItemAdapter(NeuroItemSPtr item);

  private:
    ModelAdapterPtr m_model;
  };

  using NeuroItemAdapterPtr  = NeuroItemAdapter *;
  using NeuroItemAdapterList = QList<NeuroItemAdapterPtr>;
  using NeuroItemAdapterSPtr = std::shared_ptr<NeuroItemAdapter>;

  NeuroItemAdapterPtr neuroItemAdapter(ItemAdapterPtr item);

} // namespace EspINA

#endif // ESPINA_NEURO_ITEM_ADAPTER_H
