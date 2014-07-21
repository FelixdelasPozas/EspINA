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
#ifndef ESPINA_ORTHOGONAL_ROI_H
#define ESPINA_ORTHOGONAL_ROI_H

// EspINA
#include <Support/Tool.h>
#include <Support/ViewManager.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Selectors/Selector.h>

// Qt
#include <QUndoCommand>

class QAction;
namespace EspINA
{
  class RectangularRegion;
  class RectangularRegionSliceSelector;
  class ROISettings;
  class ROIToolsGroup;

  /// Volume Of Interest Toolbar
  class OrthogonalROITool
  : public Tool
  {
    Q_OBJECT
  public:
    /* \brief OrthogonalROITool class constructor.
     * \param[in] model       Analysis model adapter
     * \param[in] viewManager Application view manager.
     * \param[in] undoStack   Application qt undo stack.
     * \param[in] toolGroup   ROIToolsGroup pointer that contains ROI accumulator.
     */
    explicit OrthogonalROITool(ROISettings     *settings,
                               ModelAdapterSPtr model,
                               ViewManagerSPtr  viewManager,
                               QUndoStack      *undoStack,
                               ROIToolsGroup   *toolgroup);

    /* \brief OrthogonalROITool class virtual destructor.
     *
     */
    virtual ~OrthogonalROITool();

    /* \brief Implements Tool::setEnabled(bool).
     *
     */
    virtual void setEnabled(bool value);

    /* \brief Implements Tool::enabled().
     *
     */
    virtual bool enabled() const;

    /* \brief Implements Tool::actions().
     *
     */
    virtual QList<QAction *> actions() const;

  protected slots:
    void initTool(bool);
    void defineROI(Selector::Selection);
    void commitROI();

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;
    ROIToolsGroup   *m_toolGroup;

    QAction         *m_applyROI;
    bool             m_enabled;

    EventHandlerSPtr m_selector;
    EspinaWidgetSPtr m_widget;
    NmVector3        m_spacing;
    NmVector3        m_origin;
    RectangularRegionSliceSelector *m_sliceSelector;
    ROISettings                    *m_settings;
  };

  using OrthogonalROIToolSPtr = std::shared_ptr<OrthogonalROITool>;

} // namespace EspINA

#endif // ESPINA_ORTHOGONAL_ROI_H
