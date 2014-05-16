/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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
#ifndef ESPINA_ORTHOGONAL_VOI_H
#define ESPINA_ORTHOGONAL_VOI_H

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

  /// Volume Of Interest Toolbar
  class OrthogonalVOITool
  : public Tool
  {
    Q_OBJECT
  public:
    explicit OrthogonalVOITool(ModelAdapterSPtr model,
                               ViewManagerSPtr  viewManager,
                               QUndoStack      *undoStack);
    virtual ~OrthogonalVOITool();

    virtual void setEnabled(bool value);

    virtual bool enabled() const;

    virtual QList<QAction *> actions() const;

  protected slots:
    void initTool(bool);
    void defineROI(Selector::Selection);
    void commitROI();

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;

    QAction         *m_applyVOI;
    bool             m_enabled;

    EventHandlerSPtr m_selector;
    EspinaWidgetSPtr m_widget;
    NmVector3        m_spacing;
    NmVector3        m_origin;
    RectangularRegionSliceSelector *m_sliceSelector;
    ROISettings                    *m_settings;
  };

  using OrthogonalVOIToolSPtr = std::shared_ptr<OrthogonalVOITool>;

} // namespace EspINA

#endif // ESPINA_ORTHOGONAL_VOI_H
