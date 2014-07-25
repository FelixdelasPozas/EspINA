/*
 * PlanarSplitWidget.h
 *
 *  Created on: Nov 5, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef PLANARSPLITWIDGET_H_
#define PLANARSPLITWIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/View/Widgets/EspinaWidget.h>
#include <Core/Utils/NmVector3.h>
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Bounds.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// Qt
#include <QMap>


class vtkAbstractWidget;
class vtkPoints;
class vtkPlane;
class vtkAbstractWidget;
class vtkImageStencilSource;
class vtkImplicitPlaneWidget2;
class vtkAlgorithmOutput;
class vtkImageStencilData;

namespace ESPINA
{
  class PlanarSplitSliceWidget;
  class vtkSplitCommand;
  class View2D;

  class EspinaGUI_EXPORT PlanarSplitWidget
  : public QObject
  , public EspinaWidget
  {
    Q_OBJECT
    public:
      /* \brief Enumeration of widget types.
       *
       */
      enum SplitWidgetType { AXIAL_WIDGET = 2, CORONAL_WIDGET = 1, SAGITTAL_WIDGET = 0, VOLUME_WIDGET = 3, NONE = 4 };

      /* \brief PlanarSplitWidget class virtual destructor.
       *
       */
      virtual ~PlanarSplitWidget();

      /* \brief VTK-style New() class constructor.
       *
       */
      static PlanarSplitWidget *New()
      {return new PlanarSplitWidget();};

      /* \brief Implements EspinaWidget::registerView().
       * \param[in] view, RenderView that will show the widget.
       *
       */
      void registerView(RenderView *view);

      /* \brief Implements EspinaWidget::unregisterView().
       * \param[in] view, RenderView that has the widget to be removed.
       *
       */
      void unregisterView(RenderView *view);

      /* \brief Implements EspinaWidget::manipulatesSegmentations().
       *
       */
      virtual bool manipulatesSegmentations() const { return true; };

      /* \brief Implmements EspinaWidget::setEnabled()
       * \param[in] enable
       */
      virtual void setEnabled(bool enable);

      /* \brief Set plane points.
       * \param[in] points
       */
      virtual void setPlanePoints(vtkSmartPointer<vtkPoints> points);

      /* \brief Get plane points.
       *
       */
      virtual vtkSmartPointer<vtkPoints> getPlanePoints() const;

      /* \brief Returns the vtkPlane defined in the tool.
       * \param[in] spacing
       */
      virtual vtkSmartPointer<vtkPlane> getImplicitPlane(const NmVector3 spacing) const;

      //TODO: virtual vtkSmartPointer<vtkImageStencilData> getStencilForVolume(SegmentationVolumeSPtr volume);

      /* \brief Sets the bounds of the segmentation to be splitted.
       * \param[in] bounds
       *
       */
      virtual void setSegmentationBounds(const Bounds bounds);

      /* \brief Returns true if the defined plane is valid.
       *
       */
      virtual bool planeIsValid() const;

      /* \brief Returns the ESPINA::Plane equivalent to the SplitWidgetType type specified as parameter.
       * \param[in] type, SplitWidgetType value.
       */
      static Plane toPlane(const SplitWidgetType type)
      {
        switch(type)
        {
          case AXIAL_WIDGET:
            return Plane::XY;
          case CORONAL_WIDGET:
            return Plane::XZ;
          case SAGITTAL_WIDGET:
            return Plane::YZ;
          default:
            Q_ASSERT(false);
            break;
        }
        return Plane::UNDEFINED;
      }

      /* \brief Returns the SplitWidgetType equivalent to the ESPINA::Plane type specified as parameter.
       * \param[in] plane, ESPINA::Plane value.
       */
      static SplitWidgetType toSplitType(const Plane plane)
      {
        switch(plane)
        {
          case Plane::XY:
            return SplitWidgetType::AXIAL_WIDGET;
            break;
          case Plane::XZ:
            return SplitWidgetType::CORONAL_WIDGET;
            break;
          case Plane::YZ:
            return SplitWidgetType::SAGITTAL_WIDGET;
            break;
          default:
            Q_ASSERT(false);
            break;
        }

        return SplitWidgetType::NONE;
      }

    public slots:
      void changeSlice(Plane plane, Nm slice);

    private:
      explicit PlanarSplitWidget();
      friend class vtkSplitCommand;

      QMap<RenderView *, vtkAbstractWidget *> m_widgets;
      SplitWidgetType                         m_mainWidget;
      vtkAlgorithmOutput                     *m_vtkVolumeInformation;
      vtkSmartPointer<vtkSplitCommand>        m_command;
  };

  using PlanarSplitWidgetPtr = PlanarSplitWidget *;
  using PlanarSplitWidgetSPtr = std::shared_ptr<PlanarSplitWidget>;

  class vtkSplitCommand
  : public vtkEspinaCommand
  {
    public:
      vtkTypeMacro(vtkSplitCommand, vtkEspinaCommand);

      /* \brief VTK-style New() constructor, required for using vtkSmartPointer.
       *
       */
      static vtkSplitCommand *New()
      { return new vtkSplitCommand(); }

      /* \brief Implements vtkEspinaCommand::Execute().
       *
       */
      virtual void Execute (vtkObject *caller, unsigned long eventId, void *callData);

      /* \brief Implements vtkEspinaCommand::setWidget()
       *
       */
      virtual void setWidget(EspinaWidgetPtr widget)
      { m_widget = dynamic_cast<PlanarSplitWidgetPtr>(widget); }

    private:
      /* \brief vtkSplitCommand private class constructor.
       *
       */
      vtkSplitCommand()
      : m_widget{nullptr}
      {};

      /* \brief vtkSplitCommand class private destructor.
       *
       */
      virtual ~vtkSplitCommand()
      {};

      PlanarSplitWidgetPtr m_widget;

  };


}// namespace ESPINA

#endif /* PLANARSPLITWIDGET_H_ */
