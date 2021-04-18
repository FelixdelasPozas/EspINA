/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_ROI_TOOLS_H
#define ESPINA_ROI_TOOLS_H

// ESPINA
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <Support/ROIProvider.h>
#include <Support/Context.h>
#include <App/ToolGroups/ToolGroup.h>

// Qt
#include <QAction>

class QUndoStack;

namespace ESPINA
{
  class ROISettings;
  class FreehandROITool;
  class OrthogonalROITool;
  class DeleteROITool;

  /** \class RestrictToolGroup
   * \brief Implements a group of tools that manage a 3D region of the stack as a region of interest.
   *
   */
  class RestrictToolGroup
  : public ToolGroup
  , public ROIProvider
  {
      Q_OBJECT
    public:
      /** \brief RestrictToolGroup class constructor.
       * \param[in] settings application ROI settings.
       * \param[in] context application context.
       * \param[in] parent QWidget parent of this one.
       *
       */
      explicit RestrictToolGroup(ROISettings*      settings,
                                 Support::Context &context,
                                 QWidget          *parent = nullptr);

      /** \brief RestrictToolGroup class virtual destructor.
       *
       */
      virtual ~RestrictToolGroup();

      /** \brief Set roi to be managed by this tool
       * \param[in] roi region of interest
       *
       */
      void setCurrentROI(ROISPtr roi);

      virtual ROISPtr currentROI() override;

      virtual void consumeROI() override;

      /** \brief Returns the currently defined ROI.
       *
       */
      ROISPtr accumulator()
      { return m_accumulator; }

      /** \brief Sets the color of the ROI representation.
       *
       */
      void setColor(const QColor &color);

      /** \brief Returns true if there is a valid roi.
       *
       */
      bool hasValidROI() const;

      /** \brief Set wheter or not the accumulated ROI is visible
       *  \param[in] visible ROI visibility state
       *
       */
      void setVisible(bool visible);

      /** \brief Returns wheter or not the accumulated ROI is visible
       *
       */
      bool isVisible() const
      { return m_visible; }

    signals:
      void ROIChanged(ROISPtr);

    private slots:
      /** \brief Updates ROI accumulator when a new ROI is defined and signals the modification.
       * \param[in] roi ROI defined as a mask.
       *
       */
      void onManualROIDefined(BinaryMaskSPtr<unsigned char> roi);

      /** \brief Pushes the change to the undo stack and signals the ROI modification.
       * \param[in] roi Orthogonal ROI.
       *
       */
      void onOrthogonalROIDefined(ROISPtr roi);

      /** \brief Signals the ROI bounds modification.
       * \param[in] roi Orthogonal ROI.
       *
       */
      void onOrthogonalROIModified(ROISPtr roi);

      /** \brief Pushes a new command to the undo stack depending on the previous.
       * \param[in] command Undocommand object.
       *
       */
      void undoStackPush(QUndoCommand *command);

    private:
      /** \brief Add orthogonal ROI to accumulator if any is already defined
       *  \param[in] roi next ROI to be managed by the Orthogonal ROI tool
       *
       */
      void commitPendingOrthogonalROI(ROISPtr roi);

      /** \brief Adds an additional orthogonal ROI to the accumulator.
       * \param[in] bounds Bounds of the ROI to add.
       *
       */
      void addOrthogonalROI(const VolumeBounds &bounds);

      /** \brief Adds an additional manually created ROI to the accumulator.
       * \param[in] mask ROI defined as a mask.
       *
       */
      void addManualROI(const BinaryMaskSPtr<unsigned char> mask);

    private:
      class DefineOrthogonalROICommand;
      class DefineManualROICommand;
      class ConsumeROICommand;

    private:
      using FreehandROIToolSPtr    = std::shared_ptr<FreehandROITool>;
      using OrthogonalROIToolSPtr  = std::shared_ptr<OrthogonalROITool>;
      using CleanROIToolSPtr       = std::shared_ptr<DeleteROITool>;
      using TemporalPrototypesSPtr = GUI::Representations::Managers::TemporalPrototypesSPtr;

      Support::Context      &m_context;       /** application context.                                 */
      FreehandROIToolSPtr    m_freehandROI;   /** manual ROI tool.                                     */
      OrthogonalROIToolSPtr  m_orthogonalROI; /** orthogonal ROI tool.                                 */
      CleanROIToolSPtr       m_deleteROI;     /** delete current ROI tool.                             */
      bool                   m_visible;       /** true if the ROI representation is set to be visible. */
      QColor                 m_color;         /** color of the ROI representation.                     */
      ROISPtr                m_accumulator;   /** Accumulates ROI.                                     */
      TemporalPrototypesSPtr m_roiPrototypes; /** ROI representation prototypes.                       */
  };

  using RestrictToolGroupSPtr = std::shared_ptr<RestrictToolGroup>;

  /** \class DefineOrthogonalROICommand
   * \brief Undo command for defining and commiting an orthogonal ROI
   *
   */
  class RestrictToolGroup::DefineOrthogonalROICommand
  : public QUndoCommand
  {
    public:
      /** \brief DefineOrthogonalROICommand class constructor.
       * \param[in] roi ROI object pointer, containing region of interest definition.
       * \param[in] tool Orthogonal ROI tool object pointer.
       *
       */
      explicit DefineOrthogonalROICommand(ROISPtr roi, RestrictToolGroup *tool);

      virtual void redo() override;

      virtual void undo() override;

    private:
      ROISPtr            m_ROI;     /** Region of interest.          */
      RestrictToolGroup *m_tool;    /** Orthogonal ROI tool.         */
      ROISPtr            m_prevROI; /** Previous Region of interest. */
  };

} // namespace ESPINA

#endif// ESPINA_ROI_TOOLS_H
