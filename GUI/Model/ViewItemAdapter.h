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

#ifndef ESPINA_VIEW_ITEM_ADAPTER_H
#define ESPINA_VIEW_ITEM_ADAPTER_H

// ESPINA
#include "GUI/Model/NeuroItemAdapter.h"
#include <GUI/Representations/RepresentationPipeline.h>
#include <Core/Analysis/Data.h>
#include <Core/Analysis/Output.h>
#include <Core/Analysis/ViewItem.h>

namespace ESPINA {

  class ViewItemAdapter;
  using ViewItemAdapterPtr   = ViewItemAdapter *;
  using ViewItemAdapterList  = QList<ViewItemAdapterPtr>;
  using ViewItemAdapterSPtr  = std::shared_ptr<ViewItemAdapter>;
  using ViewItemAdapterSList = QList<ViewItemAdapterSPtr>;

  class EspinaGUI_EXPORT ViewItemAdapter
  : public NeuroItemAdapter
  {
    Q_OBJECT
  public:
    /** \brief ViewItemAdapter class virtual destructor.
     *
     */
    virtual ~ViewItemAdapter()
    {}

    /** \brief Returns true if the item is selected.
     *
     */
    bool isSelected() const
    { return m_isSelected; }

    /** \brief Sets the selection of the item.
     * \param[in] value true to select it false otherwise.
     *
     */
    void setSelected(bool value)
    { m_isSelected = value; }

    /** \brief Sets the visibility of the item.
     * \param[in] value true to set visible false otherwise.
     *
     */
    void setVisible(bool value)
    { m_isVisible = value; }

    /** \brief Returns true if the item is visible.
     *
     */
    bool isVisible() const
    { return m_isVisible; }

    /** \brief Returns the item as a input smart pointer.
     *
     */
    virtual InputSPtr asInput() const = 0;

    /** \brief Changes the output of the item.
     * \param[in] input input smart pointer as new output.
     *
     */
    void changeOutput(InputSPtr input);

    /** \brief Returns the filter smart pointer of the item.
     *
     */
    FilterSPtr filter()
    { return m_viewItem->filter(); }

    /** \brief Returns the filter smart pointer of the item.
     *
     */
    const FilterSPtr filter() const
    { return m_viewItem->filter(); }

    /** \brief Returns the output smart pointer of the item.
     *
     * Convenience method.
     *
     */
    OutputSPtr output()
    { return m_viewItem->output(); }

    /** \brief Returns the output smart pointer of the item.
     *
     * Convenience method
     *
     */
    const OutputSPtr output() const
    { return m_viewItem->output(); }

    /** \brief Returns the data of the specified type of the item.
     * \param[in] type data type.
     *
     */
    DataSPtr outputData(Data::Type type)
    { return m_viewItem->data(type); }

    /** \brief Returns the data of the specified type of the item.
     * \param[in] type data type.
     *
     */
    const DataSPtr outputData(Data::Type type) const
    { return m_viewItem->data(type); }

    void setTemporalRepresentation(RepresentationPipelineSPtr pipeline);

    void clearTemporalRepresentation();

    RepresentationPipelineSPtr temporalRepresentation() const;

    void invalidateRepresentations();

  signals:
    void outputModified();

    void outputChanged(ViewItemAdapterPtr item);

    void representationsInvalidated(ViewItemAdapterPtr item);

  protected:
    /** \brief ViewItemAdapter class constructor.
     * \param[in] filter filter adapter smart pointer.
     * \param[in] item view item smart pointer to adapt.
     *
     */
    explicit ViewItemAdapter(ViewItemSPtr item);

    /** \brief Changes the output of the item
     * \param[in] input input smart pointer as new output.
     *
     */
    virtual void changeOutputImplementation(InputSPtr input) = 0;

  private slots:
    void onOutputModified();

  protected:
    ViewItemSPtr      m_viewItem;

    bool m_isSelected;
    bool m_isVisible;
    RepresentationPipelineSPtr m_temporalRepresentation;
  };

  ViewItemAdapterPtr EspinaGUI_EXPORT viewItemAdapter(ItemAdapterPtr item);

} // namespace ESPINA

#endif // ESPINA_VIEW_ITEM_ADAPTER_H
