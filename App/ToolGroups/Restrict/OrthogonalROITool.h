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
#ifndef ESPINA_ORTHOGONAL_ROI_H
#define ESPINA_ORTHOGONAL_ROI_H

// ESPINA
#include <Support/Widgets/ProgressTool.h>
#include <Support/Context.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/View/Widgets/OrthogonalRegion/OrthogonalSliceSelector.h>
#include <GUI/View/Widgets/OrthogonalRegion/OrthogonalRepresentation.h>

// Qt
#include <QUndoCommand>

class QPushButton;
class QAction;
namespace ESPINA
{
  class ROISettings;
  class RestrictToolGroup;

  /** \class OrthogonalROITool
   * \brief Tool for the orthogonal ROI creator.
   *
   */
  class OrthogonalROITool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief OrthogonalROITool class constructor.
       * \param[in] context application context.
       * \param[in] toolGroup parent toolgroup.
       *
       */
      explicit OrthogonalROITool(ROISettings       *settings,
                                 Support::Context  &context,
                                 RestrictToolGroup *toolgroup);

      /** \brief OrthogonalROITool class virtual destructor.
       *
       */
      virtual ~OrthogonalROITool();

      /** \brief Sets ROI to be resized by this tool
       *
       *  If ROI is null then resize action is disabled
       */
      void setROI(ROISPtr roi);

      /** \brief Returns the current orthogonal ROI.
       *
       */
      ROISPtr currentROI() const
      { return m_roi; }

      /** \brief Sets the ROI widget visibility.
       * \param[in] visible true to set to visible and false otherwise.
       *
       */
      void setVisible(bool visible);

      /** \brief Sets the color of the ROI widget.
       * \param[in] color widget color.
       *
       */
      void setColor(const QColor &color);

      /** \brief Creates the orthogonal region.
       * \param[in] centroid center of the region.
       * \param[in] xSize size inthe x coordinate in Nm.
       * \param[in] ySize size inthe y coordinate in Nm.
       * \param[in] zSize size inthe z coordinate in Nm.
       *
       */
      static Bounds createRegion(const NmVector3 &centroid, const Nm xSize, const Nm ySize, const Nm zSize);

    signals:
      void roiDefined(ROISPtr);
      void roiModified(ROISPtr);

    private slots:
      /** \brief Activates/Deactivates the tool
       *  \param[in] value true to activate the tool
       *
       *  When the tool is active, it will display the two actions available.
       *  When the tool is deactivated, Orthogonal ROI widget interaction will be disabled.
       */
      void setActive(bool value);


      /** \brief Modifies the tool and activates/deactivates the event handler for this tool.
       * \param[in] value true to activate tool and eventhandler, false to deactivate event handler.
       */
      void setDefinitionMode(bool value);

      /** \brief Update GUI status to be in accordance with the event handler state
       * \param[in] active event handler status
       */
      void onEventHandlerChanged();

      /** \brief Sets the operation mode of the Orthogonal ROI
       * \param[in] resizable true value allows ROI modification using a widget,
       *            false value only diplays the ROI on the views
       */
      void setResizable(bool resizable);

      /** \brief Defines a new ROI based on the selection.
       * \param[in] selection selection containing the active channel and selected voxel.
       *
       */
      void defineROI(Selector::Selection selection);

      /** \brief Helper method to update the ROI bounds.
       *
       */
      void updateBounds(Bounds bounds);

      /** \brief Helper method to update the ROI visual representation.
       *
       */
      void updateRegionRepresentation();

    private:
      using TemporalPrototypesSPtr = GUI::Representations::Managers::TemporalPrototypesSPtr;
      using OrthogonalSelector     = GUI::View::Widgets::OrthogonalRegion::OrthogonalSliceSelector;
      using OrthogonalSelectorSPtr = std::shared_ptr<OrthogonalSelector>;
      using Representation         = GUI::View::Widgets::OrthogonalRegion::OrthogonalRepresentation;
      using RepresentationSPtr     = GUI::View::Widgets::OrthogonalRegion::OrthogonalRepresentationSPtr;

    private:
      /** \brief Helper method to create the buttons and connect the signals.
       *
       */
      void initControls();

      /** \brief Creates the rectangular region widget for the current roi
       *
       */
      void createOrthogonalWidget();

      /** \brief Removes the current widget.
       *
       */
      void destroyOrthogonalWidget();

      /** \brief Disables the interaction of the widget.
       *
       */
      void disableOrthogonalWidget();

      /** \brief Returns true if the widget is interactive.
       *
       */
      bool isResizable() const;

      /** \brief Returns true if any of the current ROI settings are invalid.
       *
       */
      bool invalidSettings() const;

      /** \brief Shows the slice selectors in the orthogonal views.
       *
       */
      void showSliceSelectors();

      /** \brief Hides the slice selectors from the orthogonal views.
       *
       */
      void hideSliceSelectors();

      /** \brief Helper method that creates and returns the temporal representations prototypes.
       *
       */
      TemporalPrototypesSPtr createTemporalRepresentationPrototype() const;

    private:
      QPushButton           *m_resizeROI;         /** resize ROI button.                                                  */
      QPushButton           *m_applyROI;          /** apply ROI buttton.                                                  */
      bool                   m_visible;           /** true if the temporal representation is visible and false otherwise. */
      ROISPtr                m_roi;               /** current ROI.                                                        */
      RepresentationSPtr     m_roiRepresentation; /** Orthogonal representation widget.                                   */
      TemporalPrototypesSPtr m_prototype;         /** temporal representation prototypes.                                 */
      EventHandlerSPtr       m_resizeHandler;     /** event handler for resize operation.                                 */
      PixelSelectorSPtr      m_defineHandler;     /** event handler for orthogonal ROI definition.                        */
      OrthogonalSelectorSPtr m_sliceSelector;     /** slice selector widget.                                              */
      ROISettings           *m_settings;          /** Orthogonal ROI settings object pointer.                             */
  };

  using OrthogonalROIToolSPtr = std::shared_ptr<OrthogonalROITool>;

} // namespace ESPINA

#endif // ESPINA_ORTHOGONAL_ROI_H
