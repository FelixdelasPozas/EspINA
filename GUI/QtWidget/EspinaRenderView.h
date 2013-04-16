/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef ESPINARENDERVIEW_H
#define ESPINARENDERVIEW_H

#include <QWidget>
#include <QMenu>
#include "GUI/QtWidget/IEspinaView.h"
#include "SegmentationContextualMenu.h"

#include "Core/EspinaTypes.h"
#include "GUI/Pickers/ISelector.h"

class vtkRenderer;
class vtkProp3D;
class vtkRenderWindow;

namespace EspINA
{

  class EspinaWidget;

  class EspinaRenderView
  : public QWidget
  , public IEspinaView
  {
    Q_OBJECT
  public:
    explicit EspinaRenderView(QWidget* parent = 0);
    virtual ~EspinaRenderView();

    virtual void reset() = 0;

    virtual void addChannel   (ChannelPtr channel) = 0;
    virtual void removeChannel(ChannelPtr channel) = 0;
    virtual bool updateChannel(ChannelPtr channel) = 0;

    virtual void addSegmentation   (SegmentationPtr seg) = 0;
    virtual void removeSegmentation(SegmentationPtr seg) = 0;
    virtual bool updateSegmentation(SegmentationPtr seg) = 0;

    virtual void addWidget   (EspinaWidget *widget) = 0;
    virtual void removeWidget(EspinaWidget *widget) = 0;

    virtual void addActor   (vtkProp3D *actor) = 0;
    virtual void removeActor(vtkProp3D *actor) = 0;

    virtual void previewBounds(Nm bounds[6], bool cropToSceneBounds = true);

    virtual void setCursor(const QCursor& cursor) = 0;

    virtual void eventPosition(int &x, int &y) = 0;

    virtual ISelector::PickList pick(ISelector::PickableItems filter, ISelector::DisplayRegionList regions) = 0;

    virtual void worldCoordinates(const QPoint &displayPos, double worldPos[3]) = 0;

    virtual void setSelectionEnabled(bool enabe) = 0;

    virtual vtkRenderWindow *renderWindow() = 0;
    virtual vtkRenderer     *mainRenderer() = 0;

    virtual void updateView() = 0;
    virtual void resetCamera() = 0;

    const Nm *sceneBounds() const {return m_sceneBounds;}
    const Nm *sceneResolution() const {return m_sceneResolution;}

    virtual void centerViewOn(Nm center[3], bool) = 0;
    virtual void showCrosshairs(bool visible) = 0;

    virtual void setViewType(PlaneType plane);
    virtual PlaneType getViewType();

    virtual void setContextualMenu(QSharedPointer<SegmentationContextualMenu> contextMenu)
    { m_contextMenu = contextMenu; }

    // WARNING: Only used in Brush.cpp to update the view while erasing voxels. Very taxing.
    virtual void forceRender(SegmentationList updatedSegs = SegmentationList()) = 0;
  protected slots:
    virtual void updateSceneBounds();

  protected:
    void addChannelBounds   (ChannelPtr channel);
    void removeChannelBounds(ChannelPtr channel);

    double suggestedChannelOpacity();

    void resetSceneBounds();

  protected:
    ChannelList m_channels;

    Nm m_sceneBounds[6];
    Nm m_sceneResolution[3];// Min distance between 2 voxels in each axis
    PlaneType m_plane;

    QSharedPointer<SegmentationContextualMenu> m_contextMenu;
  };

} // namespace EspINA

#endif // ESPINARENDERVIEW_H
