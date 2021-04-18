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


#ifndef ESPINA_ORTHOGONAL_WIDGET_2D_H
#define ESPINA_ORTHOGONAL_WIDGET_2D_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <Core/Utils/Bounds.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include "OrthogonalRepresentation.h"

// VTK
#include <vtkAbstractWidget.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>

class QColor;
namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace OrthogonalRegion
        {
          class vtkOrthogonalWidget2D;

          /** \class OrthogonalWidget2D
           * \brief Interactive widget for a 2D representation of a orthogonal region.
           *
           */
          class EspinaGUI_EXPORT OrthogonalWidget2D
          : public EspinaWidget2D
          {
            Q_OBJECT

          public:
            /** \brief OrthogonalWidget2D class constructor.
             * \param[in] reopresentation Orthogonal representation smart pointer.
             *
             */
            explicit OrthogonalWidget2D(OrthogonalRepresentationSPtr representation);

            /** \brief OrthogonalWidget2D class virtual destructor.
             *
             */
            virtual ~OrthogonalWidget2D()
            {};

            virtual void setPlane(Plane plane) override;

            virtual void setRepresentationDepth(Nm depth) override;

            virtual bool acceptCrosshairChange(const NmVector3& crosshair) const override;

            virtual bool acceptSceneResolutionChange(const NmVector3& resolution) const override;

            virtual bool acceptSceneBoundsChange(const Bounds& bounds) const override;

            virtual bool acceptInvalidationFrame(const Representations::FrameCSPtr frame) const;

            virtual void initializeImplementation(RenderView* view) override;

            virtual void uninitializeImplementation() override;

            virtual vtkAbstractWidget* vtkWidget() override;

          private:
            virtual void display(const Representations::FrameCSPtr& frame) override;

            virtual Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation() override;

          private slots:
            /** \brief Enables/disables the interaction process depending on the region mode.
             * \param[in] mode
             *
             */
            void onModeChanged(const OrthogonalRepresentation::Mode mode);

            /** \brief Udpates the representation resolution when it changes.
             * \para[in] resolution new scene resolution.
             *
             */
            void onResolutionChanged(const NmVector3 &resolution);

            /** \brief Updates the representation when the bounds of the region changes.
             * \param[in] bounds new region bounds.
             *
             */
            void onBoundsChanged(const Bounds &bounds);

            /** \brief Updates the representation color.
             * \param[in] color new color of the representation.
             *
             */
            void onColorChanged(const QColor &color);

            /** \brief Updates the line pattern of the representation.
             * \param[in] pattern line patter in hex mode, see widget representation for details.
             *
             */
            void onPatternChanged(const int pattern);

          private:
            class Command;

            OrthogonalRepresentationSPtr           m_representation; /** orthogonal region definition.                           */
            vtkSmartPointer<vtkOrthogonalWidget2D> m_widget;         /** vtk widget for representation.                          */
            vtkSmartPointer<Command>               m_command;        /** vtk command object to handle interaction with the view. */
            int                                    m_index;          /** plane index of the representation.                      */
            Nm                                     m_slice;          /** current representation slice.                           */
            Nm                                     m_resolution;     /** representation and scene resolution.                    */
          };
        }
      }
    }
  }
}// namespace ESPINA

#endif // ESPINA_ORTHOGONAL_WIDGET_2D_H
