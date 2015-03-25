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
#define EPSINA_SEGMENTATION_INSPECTOR_H

// ESPINA
#include "ui_SegmentationInspector.h"
#include <QWidget>

#include <Docks/SegmentationExplorer/SegmentationExplorerLayout.h>
#include <Support/Factory/FilterDelegateFactory.h>
#include <Support/Widgets/TabularReport.h>
#include <Support/Widgets/RepresentationTools.h>
#include <Support/Representations/RepresentationFactory.h>
#include <GUI/View/View3D.h>
#include <GUI/Representations/ManualPipelineSources.h>

// Qt
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QToolBar>

class QUndoStack;

using namespace ESPINA::Support::Widgets;

namespace ESPINA
{
  class SegmentationInspector
  : public QWidget
  , public Ui::SegmentationInspector
  {
    Q_OBJECT
  public:
    /** \brief SegmentationInspector class constructor.
     * \param[in] segmentations list of segmentation adapters of the segmentations to be inspected.
     * \param[in] model model smart pointer containing the segmentations.
     * \param[in] factory factory smart pointer.
     * \param[in] viewManager view manager smart pointer.
     * \param[in] undoStack QUndoStack object raw pointer.
     * \param[in] parent parent widget pointer.
     * \param[in] flags flags of the dialog.
     */
    SegmentationInspector(SegmentationAdapterList     segmentations,
                          RepresentationFactorySList &representation,
                          ModelAdapterSPtr            model,
                          ModelFactorySPtr            factory,
                          FilterDelegateFactorySPtr   delegateFactory,
                          ViewManagerSPtr             viewManager,
                          QUndoStack*                 undoStack,
                          QWidget*                    parent = nullptr);

    /** \brief SegmentationInspector class destructor.
     *
     */
    virtual ~SegmentationInspector();

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

  signals:
    void inspectorClosed(SegmentationInspector *inspector);

  protected:
    virtual void showEvent(QShowEvent *event) override;

    virtual void closeEvent(QCloseEvent *e) override;

  private slots:
    /** \brief Updates which information is displayed according to current selection
     *
     */
    void updateSelection();

    /** \brief Updates the representations of the item in the view of the dialog.
     * \param[in] item item adapter raw pointer of the item to update.
     *
     */
    void updateScene(ItemAdapterPtr item);

  private:
    void updateWindowTitle();

    void initView3D(RepresentationFactorySList representations);

    void initReport();

    void configureLayout();

    void restoreGeometryState();

    void saveGeometryState();

    QVBoxLayout *createViewLayout();

    QHBoxLayout *createReportLayout();

  private:
    ModelAdapterSPtr          m_model;
    ModelFactorySPtr          m_factory;
    FilterDelegateFactorySPtr m_delegateFactory;
    ViewManagerSPtr           m_viewManager;
    QUndoStack*               m_undoStack;

    SegmentationAdapterList m_segmentations;
    ChannelAdapterList      m_channels;

    SegmentationAdapterPtr  m_selectedSegmentation;

    ManualPipelineSources m_channelSources;
    ManualPipelineSources m_segmentationSources;
    ColorEngineSPtr       m_colorEngine;

    QToolBar      m_toolbar;
    RepresentationTools tools;
    View3D        m_view;
    TabularReport m_tabularReport;
  };

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_INSPECTOR_H
