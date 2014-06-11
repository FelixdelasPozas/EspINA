/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_
#define ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_

#include <Support/Tool.h>
#include <Support/ViewManager.h>

#include <GUI/Model/ModelAdapter.h>
#include <Filters/MorphologicalEditionFilter.h>
#include <Filters/FillHolesFilter.h>
#include <Filters/ImageLogicFilter.h>
#include "CODETool.h"


class QAction;
class QUndoStack;

namespace EspINA
{
  class SpinBoxAction;
  
  class MorphologicalEditionTool
  : public Tool
  {
    Q_OBJECT

    class MorphologicalFilterFactory
    : public FilterFactory
    {
      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const
      throw (Unknown_Filter_Exception);

    private:
      mutable FetchBehaviourSPtr m_fetchBehaviour;
    };

  public:
    /** \brief Class constructor.
     *
     */
    MorphologicalEditionTool(ModelAdapterSPtr model,
                             ModelFactorySPtr factory,
                             ViewManagerSPtr  viewManager,
                             QUndoStack      *undoStack);

    /** \brief Class destructor.
     *
     */
    virtual ~MorphologicalEditionTool();

    /** \brief Tool method to enable/disable this tool.
     *
     */
    virtual void setEnabled(bool value);

    /** \brief Returns if the class is actually enabled.
     *
     */
    virtual bool enabled() const;

    /** \brief Returns the group of actions provided by the tool.
     *
     */
    virtual QList<QAction *> actions() const;

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

    void onMorphologicalFilterFinished();

    void onFillHolesFinished();

    void onCloseToggled(bool toggled);

    void onOpenToggled(bool toggled);

    void onDilateToggled(bool toggled);

    void onErodeToggled(bool toggled);

    void updateAvailableActionsForSelection();

    void onImageLogicFilterFinished();

  private:
    template<typename T>
    void launchCODE(const Filter::Type& type, const QString& name, int r)
    {
      m_viewManager->unsetActiveEventHandler();

      auto selection = m_viewManager->selection()->segmentations();

      if (selection.size() > 0)
      {
        for (auto segmentation :  selection)
        {
          InputSList inputs;

          inputs << segmentation->asInput();

          auto adapter = m_factory->createFilter<T>(inputs, type);
          auto filter  = adapter->get();

          filter->setRadius(r);
          filter->setDescription(tr("%1 %2").arg(name)
                                            .arg(segmentation->data(Qt::DisplayRole).toString()));

          MorphologicalContext context;

          context.Task         = filter;
          context.Operation    = tr("%1 Segmentation").arg(name);
          context.Segmentation = segmentation;

          m_executingMorpholocialTasks[filter.get()] = context;

          connect(filter.get(), SIGNAL(finished()),
                  this,         SLOT(onMorphologicalFilterFinished()));

          adapter->submit();
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
      FilterAdapterSPtr           Task;
      ImageLogicFilter::Operation Operation;
      SegmentationAdapterList     Segmentations;
    };

  private:
    ModelAdapterSPtr m_model;
    ModelFactorySPtr m_factory;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;

    FilterFactorySPtr m_filterFactory;

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

    bool m_enabled;
  };

  using MorphologicalEditionToolPtr  = MorphologicalEditionTool *;
  using MorphologicalEditionToolSPtr = std::shared_ptr<MorphologicalEditionTool>;

} // namespace EspINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_
