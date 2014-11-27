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


#ifndef ESPINA_RECTANGULAR_REGION_H
#define ESPINA_RECTANGULAR_REGION_H

#include <Core/Utils/Bounds.h>
#include <GUI/View/Widgets/EspinaWidget.h>

// Qt
#include <QList>
#include <QObject>

// VTK
#include <vtkAbstractWidget.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>

namespace ESPINA
{
  class ViewManager;
  class vtkOrthogonalRegionSliceWidget;
  class OrthogonalRegion;

  //----------------------------------------------------------------------------
  class EspinaGUI_EXPORT vtkOrthogonalRegionCommand
  : public vtkEspinaCommand
  {
    public:
      vtkTypeMacro(vtkOrthogonalRegionCommand, vtkEspinaCommand);

      virtual ~vtkOrthogonalRegionCommand()
      {};

      /** \brief Creates a new instance.
       *
       */
      static vtkOrthogonalRegionCommand *New()
      { return new vtkOrthogonalRegionCommand(); }

      void setWidget(EspinaWidgetPtr widget);

      virtual void Execute(vtkObject *, unsigned long int, void*);

    private:
      /** \brief Private constructor.
       *
       */
      explicit vtkOrthogonalRegionCommand()
      : m_widget{nullptr}
      {}


      OrthogonalRegion *m_widget;
  };

  //----------------------------------------------------------------------------
  class EspinaGUI_EXPORT OrthogonalRegion
  : public QObject
  , public EspinaWidget
  {
    Q_OBJECT
  public:
    /** \brief RectangularRegion class constructor.
     * \param[in] bounds bounds of the region.
     *
     */
    explicit OrthogonalRegion(Bounds bounds);

    /** \brief RectangularRegion class destructor.
     *
     */
    virtual ~OrthogonalRegion();

    /** \brief Implements EspinaWidget::registerView().
     *
     */
    virtual void registerView  (RenderView *view);

    /** \brief Implements EspinaWidget::unregisterView().
     *
     */
    virtual void unregisterView(RenderView *view);

    /** \brief Implements EspinaWidget::setEnabled().
     *
     */
    virtual void setEnabled(bool enable);

    /** \brief Sets the bounds of the region.
     * \param[in] bounds bounds of the region.
     *
     */
    virtual void setBounds(Bounds bounds);

    /** \brief Returns the bounds of the region.
     *
     */
    virtual Bounds bounds() const;

    /** \brief Sets the spacing/resolution of the region.
     *
     */
    void setResolution(NmVector3 resolution);

    /** \brief Returns the resolution of the region.
     *
     */
    NmVector3 resolution() const
    { return m_resolution; }

    /** \brief Sets the color of the representation.
     * \param[in] color pointer to a vector of three double corresponding to the r,g,b values.
     *
     */
    void setRepresentationColor(double *color);

    /** \brief Sets the representation pattern.
     * \param[in] pattern pattern in hexadecimal.
     */
    void setRepresentationPattern(int pattern);

  signals:
    void modified(Bounds);

  private slots:
    /** \brief Update the representation when the view changes the slice.
     *
     */
    void sliceChanged(Plane plane, Nm pos);

  private:
    friend class vtkOrthogonalRegionCommand;

    /** \brief Emits the modification signal when the representation is modified.
     *
     */
    void emitModifiedSignal()
    { emit modified(m_bounds); }

    Bounds m_bounds;
    NmVector3 m_resolution;
    QMap<RenderView *, vtkOrthogonalRegionSliceWidget *> m_widgets;
    double m_color[3];
    int m_pattern;
    vtkSmartPointer<vtkOrthogonalRegionCommand> m_command;
  };

}// namespace ESPINA

#endif // ESPINA_RECTANGULAR_REGION_H
