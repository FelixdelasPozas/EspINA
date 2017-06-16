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

#ifndef APP_TOOLGROUPS_EDIT_SKELETONEDITIONTOOL_H_
#define APP_TOOLGROUPS_EDIT_SKELETONEDITIONTOOL_H_

// ESPINA
#include <Core/Utils/Vector3.hxx>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/Representations/RepresentationState.h>
#include <GUI/Types.h>
#include <GUI/View/Widgets/Skeleton/SkeletonEventHandler.h>
#include <GUI/View/Widgets/Skeleton/SkeletonWidget2D.h>
#include <GUI/Widgets/ToolButton.h>
#include <Support/Context.h>
#include <Support/Widgets/EditTool.h>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace ESPINA
{
  /** \class SkeletonEditionTool
   * \brief Tool for skeleton segmentation edition
   *
   */
  class SkeletonEditionTool
  : public Support::Widgets::EditTool
  {
      Q_OBJECT
    public:
      /** \brief SkeletonEditionTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit SkeletonEditionTool(Support::Context &context);

      /** \brief SkeletonEditionTool class virtual destructor.
       *
       */
      virtual ~SkeletonEditionTool();

      virtual void abortOperation() override
      { deactivateEventHandler(); };

    private slots:
      /** \brief Performs tool initialization/de-initialization.
       * \param[in] value true to initialize and false otherwise.
       *
       */
      void initTool(bool value);

      /** \brief Updates the widget if the item being modified is removed from the model (i.e. by undo).
       * \param[in] segmentations List of segmentation adapter smart pointers removed from the model.
       *
       */
      void onSegmentationsRemoved(ViewItemAdapterSList segmentations);

      /** \brief Updates the minimum value of the tolerance widget.
       *
       */
      void onResolutionChanged();

      /** \brief Helper method to mark the tool un-initialized on model reset.
       *
       */
      void onModelReset();

      /** \brief Adds the cloned widget to the list of cloned and sets the parameters.
       * \param[in] clone cloned widget.
       *
       */
      void onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone);

      /** \brief Updates the created segmentation.
       * \param[in] polydata skeleton data.
       *
       */
      void onSkeletonModified(vtkSmartPointer<vtkPolyData> polydata);

      void onEraseButtonClicked(bool value);

      void onMoveButtonClicked(bool value);

    private:
      virtual bool acceptsNInputs(int n) const;

      virtual bool acceptsSelection(SegmentationAdapterList segmentations) override;

      /** \brief Initializes and connects the representation factory.
       *
       */
      void initRepresentationFactory();

      /** \brief Initializes and connects the parameters widgets.
       *
       */
      void initParametersWidgets();

      /** \brief Helper method to configure the event handler for the tool.
       *
       */
      void initEventHandler();

    private:
      /** \class NullRepresentationPipeline
       * \brief Implements an empty representation.
       *
       */
      class NullRepresentationPipeline
      : public RepresentationPipeline
      {
        public:
          /** \brief NullRepresentationPipeline class constructor.
           *
           */
          explicit NullRepresentationPipeline()
          : RepresentationPipeline("SegmentationSkeleton2D")
          { /* representation type must be the same as the default one. */ }

          /** \brief NullRepresentationPipeline class virtual destructor.
           *
           */
          virtual ~NullRepresentationPipeline()
          {};

          virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr    item,
                                                                 const RepresentationState &state)
          { return RepresentationPipeline::ActorList(); }

          virtual  void updateColors(RepresentationPipeline::ActorList &actors,
                                     ConstViewItemAdapterPtr            item,
                                     const RepresentationState         &state)
          {}

          virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
          { return false; }

          virtual RepresentationState representationState(ConstViewItemAdapterPtr    item,
                                                          const RepresentationState &settings)
          { return RepresentationState(); }
      };

    private:
      bool                                                   m_init;         /** true if the tool has been initialized.            */
      GUI::View::Widgets::Skeleton::SkeletonEventHandlerSPtr m_eventHandler; /** tool's event handler.                             */
      GUI::Widgets::ToolButton                              *m_eraseButton;  /** Paint/erase button.                               */
      GUI::Widgets::ToolButton                              *m_moveButton;   /** Move nodes button.                                */
      ViewItemAdapterPtr                                     m_item;         /** current element being created or channel in init. */
      GUI::Representations::Managers::TemporalPrototypesSPtr m_factory;      /** representation prototypes.                        */
      QList<GUI::View::Widgets::Skeleton::SkeletonWidget2DSPtr> m_widgets;   /** list of widgets currently on views.               */
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_EDIT_SKELETONEDITIONTOOL_H_
