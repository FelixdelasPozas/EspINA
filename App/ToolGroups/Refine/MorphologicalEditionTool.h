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

#ifndef ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_
#define ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_

// ESPINA
#include <Support/Widgets/Tool.h>
#include <Support/Factory/FilterDelegateFactory.h>
#include <GUI/Model/ModelAdapter.h>
#include <Filters/MorphologicalEditionFilter.h>
#include <Filters/FillHolesFilter.h>
#include <Filters/ImageLogicFilter.h>
#include "CODETool.h"

class QAction;
class QUndoStack;

namespace ESPINA
{
  class SpinBoxAction;

  class MorphologicalEditionTool
  : public Tool
  {
    Q_OBJECT

    class MorphologicalFilterFactory
    : public FilterFactory
    , public SpecificFilterDelegateFactory
    {
      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
      throw (Unknown_Filter_Exception);

      virtual QList<Filter::Type> availableFilterDelegates() const;

      virtual FilterDelegateSPtr createDelegate(FilterSPtr filter) throw (Unknown_Filter_Type_Exception);

    private:
      bool isCloseFilter       (const Filter::Type &type) const;
      bool isOpenFilter        (const Filter::Type &type) const;
      bool isDilateFilter      (const Filter::Type &type) const;
      bool isErodeFilter       (const Filter::Type &type) const;
      bool isFillHolesFilter   (const Filter::Type &type) const;
      bool isAdditionFilter    (const Filter::Type &type) const;
      bool isSubstractionFilter(const Filter::Type &type) const;

    private:
      mutable DataFactorySPtr m_dataFactory;
    };

  public:
    /** \brief MorphologicalEdtionTool class constructor.
     * \param[in] FilterDelegateFactory
     * \param[in] context ESPINA context
     *
     */
    MorphologicalEditionTool(FilterDelegateFactorySPtr filterDelegateFactory,
                             Support::Context    &context);

    /** \brief MorphologicalEditionTools class destructor.
     *
     */
    virtual ~MorphologicalEditionTool();


    virtual QList<QAction *> actions() const;

    virtual void abortOperation();

  private slots:
    /** \brief Merge selected segmentations.
     *
     */
    void mergeSegmentations();

    /** \brief Substract one segmentation from the other.
     *
     */
    void subtractSegmentations();

    /** \brief Close the selected segmentation with the radius set on the associated QSpinBox.
     *
     */
    void closeSegmentations();

    /** \brief Open the selected segmentation with the radius set on the associated QSpinBox.
     *
     */
    void openSegmentations();

    /** \brief Dilate the selected segmentation with the radius set on the associated QSpinBox.
     *
     */
    void dilateSegmentations();

    /** \brief Erode the selected segmentation with the radius set on the associated QSpinBox.
     *
     */
    void erodeSegmentations();

    /** \brief Fills all internals holes in the selected segmentation.
     *
     */
    void fillHoles();

    /** \brief Changes/Deletes the segmentation when the morphological after the filter has finished.
     *
     */
    void onMorphologicalFilterFinished();

    /** \brief Changes the segmentation when the morphological after the filter has finished.
     *
     */
    void onFillHolesFinished();

    /** \brief Modifies the GUI when the close operation is toggled.
     * \param[in] toggled, true if toggled.
     *
     */
    void onCloseToggled(bool toggled);

    /** \brief Modifies the GUI when the open operation is toggled.
     * \param[in] toggled, true if toggled.
     *
     */
    void onOpenToggled(bool toggled);

    /** \brief Modifies the GUI when the dilate operation is toggled.
     * \param[in] toggled, true if toggled.
     *
     */
    void onDilateToggled(bool toggled);

    /** \brief Modifies the GUI when the erode operation is toggled.
     * \param[in] toggled, true if toggled.
     *
     */
    void onErodeToggled(bool toggled);

    /** \brief Modifies the GUI based on the current selection.
     *
     */
    void updateAvailableActionsForSelection();

    /** \brief Changes the segmentation(s) when the image logic filter has finished.
     *
     */
    void onImageLogicFilterFinished();

  private:
    virtual void onToolEnabled(bool enabled) override;

    /** \brief Launches the CODE filter (Morphological filter).
     * \param[in] type type of the morphological operation.
     * \param[in] name name of the operation.
     * \param[in] radius radius of the morphological operation.
     *
     */
    template<typename T>
    void launchCODE(const Filter::Type& type, const QString& name, int radius)
    {
      m_context.viewState().setEventHandler(nullptr);

      auto selection = getSelection(m_context)->segmentations();

      if (selection.size() > 0)
      {
        for (auto segmentation :  selection)
        {
          InputSList inputs;

          inputs << segmentation->asInput();

          auto filter = m_context.factory()->createFilter<T>(inputs, type);

          filter->setRadius(radius);
          filter->setDescription(tr("%1 %2").arg(name)
                                            .arg(segmentation->data(Qt::DisplayRole).toString()));

          MorphologicalContext context;

          context.Task         = filter;
          context.Operation    = tr("%1 Segmentation").arg(name);
          context.Segmentation = segmentation;

          m_executingMorpholocialTasks[filter.get()] = context;

          connect(filter.get(), SIGNAL(finished()),
                  this,         SLOT(onMorphologicalFilterFinished()));

          Task::submit(filter);
        }
      }
    }

  private:
    struct MorphologicalContext
    {
      MorphologicalEditionFilterSPtr Task;
      SegmentationAdapterPtr         Segmentation;
      QString                        Operation;
    };
    struct FillHolesContext
    {
      FillHolesFilterSPtr    Task;
      SegmentationAdapterPtr Segmentation;
      QString                Operation;
    };
    struct ImageLogicContext
    {
      FilterSPtr                  Task;
      ImageLogicFilter::Operation Operation;
      SegmentationAdapterList     Segmentations;
    };

  private:
    Support::Context &m_context;

    std::shared_ptr<MorphologicalFilterFactory> m_filterFactory;

    CODETool m_close;
    CODETool m_open;
    CODETool m_dilate;
    CODETool m_erode;

    QAction* m_fill;

    QAction *m_addition;
    QAction *m_subtract;

    QMap<MorphologicalEditionFilterPtr, MorphologicalContext> m_executingMorpholocialTasks;
    QMap<FillHolesFilterPtr, FillHolesContext>                m_executingFillHolesTasks;
    QMap<ImageLogicFilterPtr, ImageLogicContext>              m_executingImageLogicTasks;
  };

  using MorphologicalEditionToolPtr  = MorphologicalEditionTool *;
  using MorphologicalEditionToolSPtr = std::shared_ptr<MorphologicalEditionTool>;

} // namespace ESPINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_
