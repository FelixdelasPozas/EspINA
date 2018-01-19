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

#ifndef ESPINA_VIEWITEM_H
#define ESPINA_VIEWITEM_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Analysis/NeuroItem.h"
#include "Input.h"

namespace ESPINA
{
  /** \class ViewItem
   * \brief Implements an item with visual representations.
   *
   */
  class EspinaCore_EXPORT ViewItem
  : public QObject
  , public NeuroItem
  {
    Q_OBJECT
    public:
      /** \brief ViewItem class constructor.
       * \param[in] input input object smart pointer.
       *
       */
      explicit ViewItem(InputSPtr input);

      /** \brief ViewItem class destructor.
       *
       */
      virtual ~ViewItem();

      /** \brief Returns the ViewItem as an input object to use as input of other objects.
       *
       */
      InputSPtr asInput() const
      {
        return m_input;
      }

      /** \brief Returns the filter associated to this.
       *
       */
      FilterSPtr filter()
      {
        return m_input->filter();
      }

      /** \brief Returns the filter associated to this.
       *
       */
      const FilterSPtr filter() const
      {
        return m_input->filter();
      }

      /** \brief Returns the output associated to this.
       *
       */
      OutputSPtr output(); //rename to input?

      /** \brief Returns the output associated to this.
       *
       */
      const OutputSPtr output() const;

      /** \brief Returns the output id.
       *
       */
      Output::Id outputId() const
      {
        return m_input->output()->id();
      }

      /** \brief Changes the output.
       * \param[in] input input object smart pointer.
       *
       */
      void changeOutput(InputSPtr input);

      /** \brief Changes the output.
       * \param[in] filter filter object smart pointer.
       * \param[in] outputId output id the the filter.
       *
       */
      void changeOutput(FilterSPtr filter, Output::Id outputId);

      /** \brief Returns true if the output has been modified its creation
       *
       */
      bool isOutputModified() const
      {
        return m_isOutputModified;
      }

      /** \brief Returns the bounds of this object.
       *
       */
      Bounds bounds() const
      {
        return output()->bounds();
      }

    protected slots:
      /** \brief Emit the modification signal for this object and updates modification flag.
       *
       */
      void onOutputModified()
      {
        m_isOutputModified = true;

        emit outputModified();
      }

    signals:
      void outputModified();

    private:
      InputSPtr m_input; /** input of the item. */
      bool m_isOutputModified; /** sticky bit         */
  };

  using ViewItemSPtr = std::shared_ptr<ViewItem>;
  using ViewItemSList = QList<ViewItemSPtr>;
} // namespace ESPINA

#endif // ESPINA_VIEWITEM_H
