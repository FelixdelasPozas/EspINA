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
#ifndef ESPINA_BRUSH_VOI_H
#define ESPINA_BRUSH_VOI_H

#include <Support/Tool.h>
#include <Support/ViewManager.h>
#include <GUI/Model/ModelAdapter.h>
#include <ToolGroups/Editor/ManualEditionTool.h>
#include "VolumeOfInterestMask.h"

class QAction;
namespace EspINA
{
  // Volume Of Interest Toolbar
  class ManualVOITool
  : public ManualEditionTool
  {
    Q_OBJECT
  public:
    explicit ManualVOITool(ModelAdapterSPtr model,
                           ViewManagerSPtr  viewManager);
    virtual ~ManualVOITool();

  signals:
    void stroke(VOIMaskSPtr);

  protected slots:
    void changeSelector(QAction *selectorAction);
    void selectorInUse(bool value);
    void drawStroke(ViewItemAdapterPtr item, Selector::WorldRegion region, Nm radius, Plane plane);

    void changeVOI(QAction *action);
    void cancelVOI();

  private:

  };

  using ManualVOIToolSPtr = std::shared_ptr<ManualVOITool>;

} // namespace EspINA

#endif // ESPINA_BRUSH_VOI_H
