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
#ifndef ESPINA_CLEAN_ROI_H
#define ESPINA_CLEAN_ROI_H

// ESPINA
#include <Support/ViewManager.h>
#include <Support/Widgets/Tool.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoStack>

class QAction;
namespace ESPINA
{
  class ROIToolsGroup;

  class CleanROITool
  : public Tool
  {
    Q_OBJECT
  public:
    /* \brief CleanROITool class constructor.
     * \param[in] model, model adapter smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] undoStack, QUndoStack raw pointer.
     * \param[in] toolGroup, ROIToolsGroup raw pointer containing the ROI accumulator.
     */
    explicit CleanROITool(ModelAdapterSPtr  model,
                          ViewManagerSPtr   viewManager,
                          QUndoStack       *undoStack,
                          ROIToolsGroup    *toolGroup);

    /* \brief CleanROITool class virtual destructor.
     *
     */
    virtual ~CleanROITool();

    /* \brief Implements Tool::setEnabled(bool).
     *
     */
    virtual void setEnabled(bool value);

    /* \brief Implements Tool::enabled().
     *
     */
    virtual bool enabled() const;

    /* \implements Tool::actions().
     *
     */
    virtual QList<QAction *> actions() const;

  protected slots:
    void onROIChanged();
    void cancelROI();

  private:
    ModelAdapterSPtr  m_model;
    ViewManagerSPtr   m_viewManager;
    QUndoStack       *m_undoStack;
    ROIToolsGroup    *m_toolGroup;
    QAction          *m_cleanROI;
    bool              m_enabled;
  };

  using CleanROIToolSPtr = std::shared_ptr<CleanROITool>;

} // namespace ESPINA

#endif // ESPINA_CLEAN_ROI_H
