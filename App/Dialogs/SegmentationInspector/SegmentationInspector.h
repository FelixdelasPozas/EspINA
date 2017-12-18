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

#ifndef ESPINA_SEGMENTATION_INSPECTOR_H
#define ESPINA_SEGMENTATION_INSPECTOR_H

// ESPINA
#include "ui_SegmentationInspector.h"
#include <QWidget>

#include <Panels/SegmentationExplorer/SegmentationExplorerLayout.h>
#include <Support/Widgets/TabularReport.h>
#include <Support/Representations/RepresentationFactory.h>
#include <GUI/View/View3D.h>
#include <GUI/Representations/ManualPipelineSources.h>

// Qt
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QToolBar>

class QUndoStack;

namespace ESPINA
{
  /** \class SegmentationInspector
   * \brief Dialog to inspect the properties of a segmentation.
   *
   */
  class SegmentationInspector
  : public QDialog
  , public Ui::SegmentationInspector
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief SegmentationInspector class constructor.
       * \param[in] segmentations list of segmentation adapters of the segmentations to be inspected.
       * \param[in] context ESPINA context
       */
      SegmentationInspector(SegmentationAdapterList         segmentations,
                            Support::Context               &context);

      /** \brief SegmentationInspector class destructor.
       *
       */
      virtual ~SegmentationInspector()
      {};

      /** \brief Adds a segmentation to the dialog and connects signals.
       * \param[in] segmentation segmentation adapter raw pointer.
       *
       */
      virtual void addSegmentation(SegmentationAdapterPtr segmentation);

      /** \brief Removes a segmentation from the dialog and disconnects signals.
       * \param[in] segmentation segmentation adapter raw pointer.
       *
       */
      virtual void removeSegmentation(SegmentationAdapterPtr segmentation);

      /** \brief Adds a channel to the dialog.
       * \param[in] channel channel adapter raw pointer.
       *
       */
      virtual void addChannel(ChannelAdapterPtr channel);

      /** \brief Removes a channel from the dialog.
       * \param[in] channel channel adapter raw pointer.
       *
       */
      virtual void removeChannel(ChannelAdapterPtr channel);

      /** \brief Implments drag enter events in the dialog.
       * \param[in] event drag enter event raw pointer.
       *
       */
      virtual void dragEnterEvent(QDragEnterEvent *event) override;

      /** \brief Implements drop events in the dialog.
       * \param[in] event drop event raw pointer.
       *
       */
      virtual void dropEvent(QDropEvent *event) override;

      /** \brief Implements drag move events in the dialog.
       * \param[in] event drag move event raw pointer.
       *
       */
      virtual void dragMoveEvent(QDragMoveEvent *event) override;

      /** \brief Returns the list of segmentations in the view.
       *
       */
      const SegmentationAdapterList segmentations() const
      { return m_segmentations; }

    signals:
      void inspectorClosed(SegmentationInspector *inspector);

      void segmentationsUpdated();

    protected:
      virtual void showEvent(QShowEvent *event) override;

      virtual void closeEvent(QCloseEvent *e) override;

    protected slots:
      /** \brief Removes the given view items from the view. Closes the dialog if there are no more segmentations on the view.
       * \param[in] segmentations List of view items to remove from the view.
       *
       */
      void onSegmentationsRemoved(ViewItemAdapterSList segmentations);

    private:
      /** \brief Helper method to connect the QObject signals to its respective slots.
       *
       */
      void connectSignals();

      /** \brief Updates the window title with the names of the inspected segmentations.
       *
       */
      void updateWindowTitle();

      /** \brief Initializes the 3D view with the given representation factories.
       * \param[in] representations Representations factories and buttons.
       *
       */
      void initView3D(RepresentationFactorySList representations);

      /** \brief Helper method to initialize the tabular report widget.
       *
       */
      void initReport();

      /** \brief Helper method to configure the layout and add the toolbar.
       *
       */
      void configureLayout();

      /** \brief Helper method to restore the dialog geometry and position.
       *
       */
      void restoreGeometryState();

      /** \brief Helper method to save the dialog geometry and position.
       *
       */
      void saveGeometryState();

      /** \brief Helper method to create and return the view layout.
       *
       */
      QHBoxLayout *createViewLayout();

      /** \brief Helper method to create and return the report layout.
       *
       */
      QHBoxLayout *createReportLayout();

    private:
      SegmentationAdapterList m_segmentations;        /** list of segmentations in the view.                     */
      ChannelAdapterList      m_channels;             /** list of channels in the view.                          */
      SegmentationAdapterPtr  m_selectedSegmentation; /** Pointer to selected segmentation.                      */
      ManualPipelineSources   m_channelSources;       /** list of channels as sources for pipelines.             */
      ManualPipelineSources   m_segmentationSources;  /** list of segmentations as sources for pipelines.        */
      RepresentationList      m_representations;      /** list of view's representations factories and switches. */
      QToolBar               *m_toolbar;              /** view's toolbar with representations switches.          */
      View3D                  m_view;                 /** 3D View.                                               */
      TabularReport           m_tabularReport;        /** Tabular report.                                        */
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_INSPECTOR_H
