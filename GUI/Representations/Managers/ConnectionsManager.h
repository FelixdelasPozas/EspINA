/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_REPRESENTATIONS_MANAGERS_CONNECTIONSMANAGER_H_
#define GUI_REPRESENTATIONS_MANAGERS_CONNECTIONSMANAGER_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Model/ModelAdapter.h>

// VTK
#include <vtkSmartPointer.h>

class vtkGlyph3DMapper;
class vtkFollower;
class vtkPoints;
class vtkPolyData;
class vtkGlyphSource2D;
class vtkSphereSource;
class vtkTransformPolyDataFilter;

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      namespace Managers
      {
        /** \class ConnectionsManager
         * \brief Manager for the connections representation.
         *
         */
        class EspinaGUI_EXPORT ConnectionsManager
        : public RepresentationManager
        {
            Q_OBJECT
          public:
            /** \brief ConnectionsManager class constructor.
             * \param[in] flag view type for the manager.
             * \param[in] model session model.
             *
             */
            explicit ConnectionsManager(ViewTypeFlags flags, ModelAdapterSPtr model);

            /** \brief ConnectionsManager class virtual destructor.
             *
             */
            virtual ~ConnectionsManager();

            virtual ViewItemAdapterList pick(const NmVector3 &point, vtkProp *actor) const override;

            /** \brief Changes the representation size.
             * \param[in] size new size value in [3-15].
             *
             */
            void setRepresentationSize(int size);

          private slots:
            /** \brief Updates the internal data adding the connection information.
             * \param[in] connection added connection.
             *
             */
            void onConnectionAdded(Connection connection);

            /** \brief Updates the internal data removing the connection information.
             * \param[in] connection removed connection.
             *
             */
            void onConnectionRemoved(Connection connection);

            /** \brief Removes all connection data.
             *
             */
            void resetConnections();

          private:
            virtual bool hasRepresentations() const override;

            virtual void updateFrameRepresentations(const FrameCSPtr frame) override;

            virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override;

            virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override;

            virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override;

            virtual bool acceptInvalidationFrame(const FrameCSPtr frame) const override;

            virtual void displayRepresentations(const FrameCSPtr frame) override;

            virtual void hideRepresentations(const FrameCSPtr frame) override;

            virtual void onHide(const FrameCSPtr frame) override;

            virtual void onShow(const FrameCSPtr frame) override;

            virtual RepresentationManagerSPtr cloneImplementation() override;

            virtual void shutdown() override final;

          private:
            /** \brief Updates the actor with the current connection data.
             *
             */
            void updateActor(const FrameCSPtr frame);

            /** \brief Helper method to connect model signals.
             *
             */
            void connectSignals();

            /** \brief Fills the connections data map, needed for switches created mid-session (like segmentation inspector).
             *
             */
            void getConnectionData();

            /** \brief Builds the actor pipeline using the information from the given frame.
             *
             */
            void buildVTKPipeline(const FrameCSPtr frame);

          private:
            ModelAdapterSPtr                            m_model;           /** model with the connection information.         */
            vtkSmartPointer<vtkPoints>                  m_points;          /** connection points.                             */
            vtkSmartPointer<vtkPolyData>                m_polyData;        /** polydata.                                      */
            vtkSmartPointer<vtkTransformPolyDataFilter> m_transformFilter; /** transformed polydata filter for XZ & YZ views. */
            vtkSmartPointer<vtkGlyph3DMapper>           m_glyph;           /** glyph filter.                                  */
            vtkSmartPointer<vtkGlyphSource2D>           m_glyph2D;         /** glyph source for 2D views, a circle.           */
            vtkSmartPointer<vtkSphereSource>            m_glyph3D;         /** glyph source for 3D views, a sphere.           */
            vtkSmartPointer<vtkFollower>                m_actor;           /** representation actor.                          */
            ConnectionList                              m_connections;     /** list of connections                            */
            int                                         m_scale;           /** representation's scale value.                  */
        };
      
      } // namespace Managers
    } // namespace Representations
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_REPRESENTATIONS_MANAGERS_CONNECTIONSMANAGER_H_
