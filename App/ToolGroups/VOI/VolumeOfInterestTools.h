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
#ifndef ESPINA_VOI_TOOLS_H
#define ESPINA_VOI_TOOLS_H

// EspINA
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include <Support/ToolGroup.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QAction>

class QUndoStack;

namespace EspINA
{
  class ManualVOITool;
  class OrthogonalVOITool;
  class CleanVOITool;

  /// Seed Growing Segmentation Plugin
  class VOIToolsGroup
  : public ToolGroup
  {
  Q_OBJECT
  public:
    /* \brief VolumeOfInterestTools class constructor.
     * \param[in] model       analysis model.
     * \param[in] factory     application factory.
     * \param[in] viewManager application view manager.
     * \param[in] undoStack   qt undo stack.
     * \param[in] parent      parent widget.
     */
    VOIToolsGroup(ModelAdapterSPtr model,
                  ModelFactorySPtr factory,
                  ViewManagerSPtr  viewManager,
                  QUndoStack      *undoStack,
                  QWidget         *parent = nullptr);

    /* \brief VolumeOfInteresetTools class virtual destructor.
     *
     */
    virtual ~VOIToolsGroup();

    /* \brief Implements ToolGroup::setEnabled(bool).
     *
     */
    virtual void setEnabled(bool value);

    /* \brief Implements ToolGroup::enabled().
     *
     */
    virtual bool enabled() const;

    /* \brief Implements ToolGroup::tools().
     *
     */
    virtual ToolSList tools();

    /* \brief Sets the value of roi accumulator and passes it to ViewManager.
     *
     */
    void setCurrentROI(ROISPtr roi);

    /* \brief Gets the current accumulator value.
     *
     */
    ROISPtr currentROI();

  private slots:
    /* \brief Changes the roi and associated widget when the
     * ROI is updated elsewhere (i.e. seedgrowsegmentation)
     * and not using ROI tools.
     *
     */
    void updateROI();

  private:
    using ManualVOIToolSPtr = std::shared_ptr<ManualVOITool>;
    using OrthogonalVOIToolSPtr = std::shared_ptr<OrthogonalVOITool>;
    using CleanVOIToolSPtr = std::shared_ptr<CleanVOITool>;

    ManualVOIToolSPtr     m_manualVOITool;
    OrthogonalVOIToolSPtr m_ortogonalVOITool;
    CleanVOIToolSPtr      m_cleanVOITool;
    ViewManagerSPtr       m_viewManager;

    bool m_enabled;

    ROISPtr m_accumulator;
    EspinaWidgetSPtr m_accumulatorWidget;
  };

} // namespace EspINA

#endif// ESPINA_VOI_TOOLS_H
