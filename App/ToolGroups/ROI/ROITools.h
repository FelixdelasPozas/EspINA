/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#ifndef ESPINA_ROI_TOOLS_H
#define ESPINA_ROI_TOOLS_H

// ESPINA
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include <Support/Widgets/ToolGroup.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QAction>

class QUndoStack;

namespace ESPINA
{

  class ROISettings;
  class ManualROITool;
  class OrthogonalROITool;
  class CleanROITool;

  /// Seed Growing Segmentation Plugin
  class ROIToolsGroup
  : public ToolGroup
  {
  Q_OBJECT
  public:
    /** \brief VolumeOfInterestTools class constructor.
     * \param[in] model, model adapter smart pointer.
     * \param[in] factory, factory smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] undoStack, QUndoStack object raw pointer.
     * \param[in] parent, QWidget raw pointer of the parent of this object.
     */
    ROIToolsGroup(ROISettings*     settings,
                  ModelAdapterSPtr model,
                  ModelFactorySPtr factory,
                  ViewManagerSPtr  viewManager,
                  QUndoStack      *undoStack,
                  QWidget         *parent = nullptr);

    /** \brief VolumeOfInteresetTools class virtual destructor.
     *
     */
    virtual ~ROIToolsGroup();

    /** \brief Implements ToolGroup::setEnabled(bool).
     *
     */
    virtual void setEnabled(bool value);

    /** \brief Implements ToolGroup::enabled().
     *
     */
    virtual bool enabled() const;

    /** \brief Implements ToolGroup::tools().
     *
     */
    virtual ToolSList tools();

    /** \brief Sets the value of roi accumulator and passes it to ViewManager.
     * \param[in] roi, roi object smart pointer.
     *
     */
    void setCurrentROI(ROISPtr roi);

    /** \brief Gets the current accumulator value.
     *
     */
    ROISPtr currentROI();

    /** \brief Returns true if there is a valid roi.
     *
     */
    bool hasValidROI();

  signals:
    void roiChanged();

  private slots:
    /** \brief Changes the roi and associated widget when the
     *        ROI is updated elsewhere (i.e. seedgrowsegmentation)
     *        and not using ROI tools.
     *
     */
    void updateROI();

  private:
    using ManualROIToolSPtr = std::shared_ptr<ManualROITool>;
    using OrthogonalROIToolSPtr = std::shared_ptr<OrthogonalROITool>;
    using CleanROIToolSPtr = std::shared_ptr<CleanROITool>;

    ManualROIToolSPtr     m_manualROITool;
    OrthogonalROIToolSPtr m_ortogonalROITool;
    CleanROIToolSPtr      m_cleanROITool;
    ViewManagerSPtr       m_viewManager;

    bool m_enabled;

    ROISPtr m_accumulator;
    EspinaWidgetSPtr m_accumulatorWidget;
  };

} // namespace ESPINA

#endif// ESPINA_ROI_TOOLS_H
