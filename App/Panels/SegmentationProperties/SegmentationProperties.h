/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef ESPINA_SEGMENTATION_INFORMATION_H
#define ESPINA_SEGMENTATION_INFORMATION_H

#include <Support/Factory/FilterRefinerFactory.h>
#include <Support/Widgets/Panel.h>
#include "IssueProperty.h"


class QUndoStack;

namespace ESPINA
{
  class SegmentationProperties
  : public Panel
  {
      Q_OBJECT
    public:
      /** \brief SegmentationProperties class constructor.
       * \param[in] filterRefiners refine widgets factory.
       * \param[in] context application context.
       * \param[in] parent QWidget parent of this one.
       *
       */
      explicit SegmentationProperties(Support::FilterRefinerFactory &filterRefiners,
                                      Support::Context               &context,
                                      QWidget                        *parent = nullptr);

      /** \brief SegmentationProperties class virtual destructor.
       *
       */
      virtual ~SegmentationProperties();

      virtual void showEvent(QShowEvent* event);

      virtual void hideEvent(QHideEvent* event);

    public slots:
      virtual void reset();

    private slots:
      /** \brief Updates the widget based on the current selection.
       *
       */
      void onSelectionChanged(SegmentationAdapterList selection);

      /** \brief Destroys the current refine widget and creates a new one for the modified output.
       *
       */
      void onOutputModified();

      /** \brief Shows the tags dialog and updates the changes.
       *
       */
      void manageTags();

      /** \brief Updates the segmentation's notes.
       *
       */
      void onNotesModified();

    private:
      /** \brief Creates or replaces the current refine widget for the given segmentation's refine widget.
       * \param[in] segmentation segmentation to refine.
       *
       */
      void showInformation(SegmentationAdapterPtr segmentation);

      /** \brief Hides the segmentation's information.
       *
       */
      void hideInformation();

      /** \brief Updates the segmentation's name widget.
       *
       */
      void showSegmentationName();

      /** \brief Clears the segmentation's name widget.
       *
       */
      void clearSegmentationName();

      /** \brief Adds a refine widget for the segmentation.
       *
       */
      void addRefineWidget();

      /** \brief Removes the current refine widget.
       *
       */
      void removeRefineWidget();

      /** \brief Updates the tags widget with the segmentation's tags.
       *
       */
      void showTags();

      /** \brief Clears the tags widget.
       *
       */
      inline void clearTags();

      /** \brief Updates the notes widget with the segmentation's notes.
       *
       */
      void showNotes();

      /** \brief Clears the notes widget.
       *
       */
      void clearNotes();

      /** \brief Updates the issues widget with the segmentation's issues.
       *
       */
      void showIssues();

      /** \brief Clears the notes widget.
       *
       */
      void clearIssues();

    private:
      class UI;

    private:
      Support::FilterRefinerFactory &m_register; /** refine widget's factory. */

      FilterSPtr             m_filter;        /** segmentation's filter.  */
      SegmentationAdapterPtr m_segmentation;  /** segmentation to refine. */

      UI *m_gui; /** widget's gui, chessire cat. */
      QList<QWidget*> map;
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_INFORMATION_H
