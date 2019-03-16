/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_WIDGETS_HISTOGRAMTREEMAPVIEW_H_
#define GUI_WIDGETS_HISTOGRAMTREEMAPVIEW_H_

// ESPINA
#include <GUI/EspinaGUI_Export.h>
#include <Core/Utils/Histogram.h>

// Qt
#include <QGraphicsView>
#include <QGraphicsRectItem>

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      /** \class HistogramTreeMapView
       * \brief TreeMap view of an stack histogram.
       *
       */
      class EspinaGUI_EXPORT HistogramTreeMapView
      : public QGraphicsView
      {
          Q_OBJECT
        public:
          /** \brief HistogramTreeMapView class constructor.
           * \param[in] parent Raw pointer of the widget parent of this one.
           *
           */
          explicit HistogramTreeMapView(QWidget *parent = nullptr);

          /** \brief HistogramTreeMapView class virtual destructor.
           *
           */
          virtual ~HistogramTreeMapView()
          {};

          /** \brief Sets the histogram data.
           * \param[in] histogram Histogram object reference.
           *
           */
          void setHistogram(const Core::Utils::Histogram &histogram);

          /** \brief Brief returns the histogram of the view or an empty histogram if none set.
           *
           */
          const Core::Utils::Histogram &histogram() const;

        public slots:
          /** \brief Rebuilds the view based on the current histogram.
           *
           */
          void rebuild();

          /** \brief Clears the scene and shows a progress text.
           * \param[in] progress Value in [0,100].
           *
           */
          void setProgress(int progress);

        protected:
          void resizeEvent(QResizeEvent* event) override;

        private:
          /** \struct Element
           * \brief Element used for squarifying operations.
           *
           */
          struct Element
          {
            unsigned char      value; /** color value.   */
            unsigned long long count; /** element count. */

            /** \brief operator< for Element structs.
             * \param[in] other Other element struct.
             *
             */
            bool operator<(const struct Element &other) { return count < other.count; };
          };

          /** \brief Adds the elements to the area and returns the resulting area.
           * \param[in] elements List of elements to add.
           * \param[inout] area Area to add the elements.
           * \param[inout] remaining Remaining values to fit area.
           *
           */
          void insertElements(const std::vector<Element> &elements, QRect &area, unsigned long long &remaining);

          /** \brief Returns how many of the given list of elements are best for filling a row/column of given area with the most square ratios.
           * \param[in] elements List of candidates.
           * \param[in] area Available area.
           * \param[in] remaining Remaining values to fit area.
           *
           */
          int optimalRatios(const std::vector<Element> &elements, const QRect &area, const unsigned long long remaining);


          Core::Utils::Histogram m_histogram; /** unsigned char histogram data. */
      };

    } // namespace Widgets
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_WIDGETS_HISTOGRAMTREEMAPVIEW_H_
