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

#include <Core/Utils/Bounds.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <Support/ViewManager.h>

// Qt
#include <QList>
#include <QObject>

// VTK
#include <vtkCommand.h>
#include <vtkAbstractWidget.h>

namespace EspINA
{
  class ViewManager;
  class vtkRectangularSliceWidget;
  class RectangularRegion;

  //----------------------------------------------------------------------------
  class EspinaGUI_EXPORT vtkRectangularRegionCommand
  : public vtkEspinaCommand
  {
    public:
      vtkTypeMacro(vtkRectangularRegionCommand, vtkEspinaCommand);

      /* \brief VTK-style New() constructor, required for using vtkSmartPointer.
       *
       */
      static vtkRectangularRegionCommand *New()
      { return new vtkRectangularRegionCommand(); }

      /* \brief Implements vtkEspinaCommand::setWidget().
       *
       */
      void setWidget(EspinaWidgetPtr widget);

      /* \brief Implements vtkEspinaCommand::Execute.
       *
       */
      virtual void Execute(vtkObject *, unsigned long int, void*);

    private:
      /* \brief RulerCommand class private constructor.
       *
       */
      explicit vtkRectangularRegionCommand()
      : m_widget{nullptr}
      {}

      /* \brief RulerCommand class private destructor.
       *
       */
      virtual ~vtkRectangularRegionCommand()
      {};

      RectangularRegion *m_widget;
  };

  //----------------------------------------------------------------------------
  class EspinaGUI_EXPORT RectangularRegion
  : public QObject
  , public EspinaWidget
  {
    Q_OBJECT
  public:
    explicit RectangularRegion(Bounds bounds, ViewManagerSPtr vm);
    virtual ~RectangularRegion();

    virtual void registerView  (RenderView *view);
    virtual void unregisterView(RenderView *view);

    virtual void setEnabled(bool enable);

    virtual void setBounds(Bounds bounds);
    virtual Bounds bounds() const;

    void setResolution(NmVector3 resolution);
    NmVector3 resolution() const
    { return m_resolution; }

    // modify representation methods
    void setRepresentationColor(double *);
    void setRepresentationPattern(int);

  signals:
    void modified(Bounds);

  private:
    friend class vtkRectangularRegionCommand;

    void emitModifiedSignal()
    { emit modified(m_bounds); }

    ViewManagerSPtr m_viewManager;
    Bounds m_bounds;
    NmVector3 m_resolution;
    QMap<RenderView *, vtkRectangularSliceWidget *> m_widgets;
    double m_color[3];
    int m_pattern;
    vtkSmartPointer<vtkRectangularRegionCommand> m_command;
  };

}// namespace EspINA

#endif // ESPINA_RECTANGULAR_REGION_H
