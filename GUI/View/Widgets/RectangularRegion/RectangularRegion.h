/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>

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


#ifndef ESPINA_RECTANGULAR_REGION_H
#define ESPINA_RECTANGULAR_REGION_H

#include <GUI/View/Widgets/EspinaWidget.h>

// Qt
#include <QList>
#include <QObject>

// VTK
#include <vtkCommand.h>

namespace EspINA
{
  class ViewManager;
  class vtkRectangularSliceWidget;

  class EspinaGUI_EXPORT RectangularRegion
  : public QObject
  , public EspinaWidget
  , public vtkCommand
  {
    Q_OBJECT
  public:
    explicit RectangularRegion(double bounds[6], ViewManager *vm);
    virtual ~RectangularRegion();

    virtual vtkAbstractWidget *create3DWidget(VolumeView *view);

    virtual SliceWidget *createSliceWidget(SliceView *view);

    virtual bool processEvent(vtkRenderWindowInteractor* iren,
                              long unsigned int event);
    virtual void setEnabled(bool enable);

    virtual void setBounds(Nm bounds[6]);
    virtual void bounds(Nm bounds[6]);

    void setResolution(Nm resolution[3]);
    void resolution(Nm resolution[3]) const
    { memcpy(resolution, m_resolution, 3*sizeof(Nm));}

    // modify representation methods
    void setRepresentationColor(double *);
    void setRepresentationPattern(int);

    virtual void Execute(vtkObject* caller, long unsigned int eventId, void* callData);

  signals:
    void modified(double *);

  private:
    ViewManager *m_viewManager;
    double m_bounds[6];
    Nm m_resolution[3];
    QList<vtkRectangularSliceWidget *> m_widgets;
    double m_color[3];
    int m_pattern;
  };
}// namespace EspINA

#endif // ESPINA_RECTANGULAR_REGION_H
