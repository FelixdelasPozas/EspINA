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
#include <GUI/Model/ModelAdapter.h>
#include <Support/ROIProvider.h>
#include <Support/ViewManager.h>
#include <ToolGroups/ToolGroup.h>

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
  class RestrictToolGroup
  : public ToolGroup
  , public ROIProvider
  {
  Q_OBJECT
  public:
    /** \brief VolumeOfInterestTools class constructor.
     * \param[in] model model adapter smart pointer.
     * \param[in] factory factory smart pointer.
     * \param[in] viewManager view manager smart pointer.
     * \param[in] undoStack QUndoStack object raw pointer.
     * \param[in] parent QWidget raw pointer of the parent of this object.
     */
    RestrictToolGroup(ROISettings*     settings,
                  ModelAdapterSPtr model,
                  ModelFactorySPtr factory,
                  ViewManagerSPtr  viewManager,
                  QUndoStack      *undoStack,
                  QWidget         *parent = nullptr);

    /** \brief VolumeOfInteresetTools class virtual destructor.
     *
     */
    virtual ~RestrictToolGroup();

    /** \brief Set roi to be managed by this tool
     * \param[in] roi region of interest
     *
     */
    void setCurrentROI(ROISPtr roi);

    virtual ROISPtr currentROI() override;

    virtual void consumeROI() override;

    ROISPtr accumulator()
    { return m_accumulator; }

    void setColor(const QColor &color);

    /** \brief Returns true if there is a valid roi.
     *
     */
    bool hasValidROI() const;

    /** \brief Set wheter or not the accumulated ROI is visible
     *  \param[in] visible ROI visibility state
     */
    void setVisible(bool visible);

    /** \brief Returns wheter or not the accumulated ROI is visible
     */
    bool isVisible() const
    { return m_visible; }

  signals:
    void roiChanged(ROISPtr);

  private slots:
    /** \brief Updates ROI accumulator when a new ROI is defined
     *
     */
    void onManualROIDefined(Selector::Selection strokes);

    void onOrthogonalROIDefined(ROISPtr roi);

  private:
    /** \brief Add orthogonal ROI to accumulator if any is already defined
     *
     *  \param[in] roi next ROI to be managed by the Orthogonal ROI tool
     */
    void commitPendingOrthogonalROI(ROISPtr roi);

    void addMask(const BinaryMaskSPtr<unsigned char> mask);

  private:
    class DefineOrthogonalROICommand;
    class DefineManualROICommand;

  private:
    using ManualROIToolSPtr     = std::shared_ptr<ManualROITool>;
    using OrthogonalROIToolSPtr = std::shared_ptr<OrthogonalROITool>;
    using CleanROIToolSPtr      = std::shared_ptr<CleanROITool>;

    ViewManagerSPtr       m_viewManager;
    QUndoStack           *m_undoStack;

//     ManualROIToolSPtr     m_manualROITool;
    OrthogonalROIToolSPtr m_ortogonalROITool;
    CleanROIToolSPtr      m_cleanROITool;

    bool   m_enabled;
    bool   m_visible;
    QColor m_color;

    ROISPtr          m_accumulator;
    EspinaWidgetSPtr m_accumulatorWidget;
  };

} // namespace ESPINA

#endif// ESPINA_ROI_TOOLS_H
