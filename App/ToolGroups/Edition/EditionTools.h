/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_EDITION_TOOLS_H_
#define ESPINA_EDITION_TOOLS_H_

// ESPINA
#include "ManualEditionTool.h"
#include "SplitTool.h"
#include "MorphologicalEditionTool.h"
#include <Core/Factory/FilterFactory.h>
#include <Support/Widgets/ToolGroup.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/Selection.h>
#include <GUI/ModelFactory.h>

class QUndoStack;

namespace ESPINA
{
  class EditionTools
  : public ToolGroup
  {
    class ManualFilterFactory
    : public FilterFactory
    {
    	/* \brief Implements FilterFactory::createFilter().
    	 *
    	 */
      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

    	/* \brief Implements FilterFactory::providedFilters().
    	 *
    	 */
      virtual FilterTypeList providedFilters() const;

    private:
      mutable FetchBehaviourSPtr m_fetchBehaviour;
    };

    Q_OBJECT
  public:
    /* \brief EditionTools class constructor.
     * \param[in] model, model adapter smart pointer.
     * \param[in] dactory, factory smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] undoStack, QUndoStack object raw pointer.
     * \param[in] parent, QWidget raw pointer of the parent of this object.
     *
     */
    explicit EditionTools(ModelAdapterSPtr model,
                          ModelFactorySPtr factory,
                          ViewManagerSPtr  viewManager,
                          QUndoStack      *undoStack,
                          QWidget         *parent = nullptr);

    /* \bried EditionTools class virtual destructor.
     *
     */
    virtual ~EditionTools();

    /* \brief Implements ToolGroup::setEnabled().
     *
     */
    virtual void setEnabled(bool value);

    /* \brief Implements ToolGroup::enabled().
     *
     */
    virtual bool enabled() const;

    /* \brief Implements ToolGroup::tools().
     *
     */
    virtual ToolSList tools();

  public slots:
		/* \brief Updates the tools based on current selection.
		 *
		 */
    void selectionChanged();

    /* \brief Aborts current operation (if any).
     *
     */
    void abortOperation();

    /* \brief Adds/Modifies a segmentation with the stroke.
     *
     */
    void drawStroke(CategoryAdapterSPtr, BinaryMaskSPtr<unsigned char> mask);

  private slots:
  	/* \brief Deletes a segmentation from the model if all its voxels have been erased.
  	 *
  	 */
    void onEditionFinished(ViewItemAdapterPtr item, bool eraserModeEntered);

  private:
    ManualEditionToolSPtr        m_manualEdition;
    SplitToolSPtr                m_split;
    MorphologicalEditionToolSPtr m_morphological;
    ModelFactorySPtr             m_factory;
    QUndoStack                  *m_undoStack;
    ModelAdapterSPtr             m_model;
    FilterFactorySPtr            m_filterFactory;

    bool                         m_enabled;
  };

} // namespace ESPINA

#endif // ESPINA_EDITION_TOOLS_H_
