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

#ifndef GUI_WIDGETS_HISTOGRAMVIEW_H_
#define GUI_WIDGETS_HISTOGRAMVIEW_H_

// ESPINA
#include <GUI/EspinaGUI_Export.h>
#include <Core/Utils/Histogram.h>

// Qt
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QTransform>

class QWidget;

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      /** \class HistogramView
       * \brief QGraphicsView of an histogram
       *
       */
      class EspinaGUI_EXPORT HistogramBarsView
      : public QGraphicsView
      {
          Q_OBJECT
        public:
          /** \brief HistogramBarsView class constructor.
           * \param[in] parent Raw pointer of the widget parent of this one.
           *
           */
          explicit HistogramBarsView(QWidget * parent = nullptr);

          /** \brief HistogramBarsView class virtual destructor.
           *
           */
          virtual ~HistogramBarsView()
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
          Core::Utils::Histogram m_histogram;         /** unsigned char histogram data.    */
          QTransform             m_normalTransform;   /** normal view transformation.      */
          QTransform             m_inverseYTransform; /** inverse Y coords transformation. */
      };

      /** \class HistogramBar
       * \brief Bar item of the HistogramBarsView.
       *
       */
      class EspinaGUI_EXPORT HistogramBar
      : public QGraphicsRectItem
      {
        public:
          /** \brief HistogramBar class constructor.
           * \param[in] parent Raw pointer of the HistogramBarsView owner of the bar.
           * \param[in] rect Rect of the bar.
           * \param[in] height Value in [0,1] % of the bar filled.
           * \param[in] number Bar number.
           * \param[in] count Histogram count of the bar number.
           *
           */
          explicit HistogramBar(HistogramBarsView *parent, const QRectF &rect, double height, unsigned char number, unsigned long long count);
  
          /** \brief HistogramBar class virtual destructor.
           *
           */
          virtual ~HistogramBar()
          {};
      };
    } // namespace Widgets
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_WIDGETS_HISTOGRAMVIEW_H_
