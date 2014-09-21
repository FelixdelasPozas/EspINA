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

// ESPINA
#include "FilterAdapter.h"
#include <GUI/Representations/Representation.h>
#include "GUI/Model/ItemAdapter.h"
#include <Core/Analysis/NeuroItem.h>

namespace ESPINA
{
  class ModelAdapter;
  using ModelAdapterPtr = ModelAdapter *;

  class EspinaGUI_EXPORT NeuroItemAdapter
  : public ItemAdapter
  {
  public:
  	/** \brief NeuroItemAdapter class virtual destructor.
  	 *
  	 */
    virtual ~NeuroItemAdapter()
    {}

    /** \brief Sets the model of the item.
     * \param[in] model, model adapter raw pointer.
     *
     */
    void setModel(ModelAdapterPtr model)
    { m_model = model; }

    /** \brief Returns the model of the item.
     *
     */
    ModelAdapterPtr model() const
    { return m_model; }

  protected:
    /** \brief NeuroItemAdapter class constructor.
     * \param[in] item, smart pointer of the NeuroItem to adapt.
     *
     */
    explicit NeuroItemAdapter(NeuroItemSPtr item);

  private:
    ModelAdapterPtr m_model;
  };

  using NeuroItemAdapterPtr  = NeuroItemAdapter *;
  using NeuroItemAdapterList = QList<NeuroItemAdapterPtr>;
  using NeuroItemAdapterSPtr = std::shared_ptr<NeuroItemAdapter>;

  /** \brief Returns the NeuroItemAdapter raw pointer from the ItemAdapter raw pointer.
   * \param[in] item, ItemAdapter raw pointer.
   *
   */
  NeuroItemAdapterPtr EspinaGUI_EXPORT neuroItemAdapter(ItemAdapterPtr item);

} // namespace ESPINA

#endif // ESPINA_NEURO_ITEM_ADAPTER_H
