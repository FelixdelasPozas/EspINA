/*
 

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

#ifndef TUBULARWIDGET_H
#define TUBULARWIDGET_H

#include "EspinaGUI_Export.h"

// EspINA
#include <GUI/vtkWidgets/EspinaWidget.h>
#include <Core/Filters/TubularSegmentationFilter.h>
#include <Core/EspinaTypes.h>
#include "vtkTubularWidget.h"

// VTK
#include <vtkCommand.h>

// Qt
#include <QVector4D>

class vtkImageAlgorithm;

namespace EspINA
{

  class EspinaGUI_EXPORT TubularWidget
  : public QObject
  , public EspinaWidget
  , public vtkCommand
  {
    Q_OBJECT
    public:
      explicit TubularWidget();
      virtual ~TubularWidget();

      virtual vtkAbstractWidget* create3DWidget(VolumeView *view);

      virtual SliceWidget* createSliceWidget(SliceView *view);

      virtual void setEnabled(bool enable);
      virtual void Execute(vtkObject* caller, long unsigned int eventId, void* callData);

      TubularSegmentationFilter::NodeList nodes()
      {
        return m_nodes;
      }
      void setNodes(TubularSegmentationFilter::NodeList nodes);
      void setRoundExtremes(bool);

      virtual bool processEvent(vtkRenderWindowInteractor* iren, long unsigned int event);

    signals:
      void nodesUpdated(TubularSegmentationFilter::NodeList);

    private:
      vtkImageAlgorithm *m_box;
      QList<vtkAbstractWidget *> m_widgets;
      QList<vtkTubularWidget *> m_sliceWidgets;
      TubularSegmentationFilter::NodeList m_nodes;
  };
}
#endif // TUBULARWIDGET_H
