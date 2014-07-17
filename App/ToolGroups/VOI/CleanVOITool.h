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
#ifndef ESPINA_CLEAN_VOI_H
#define ESPINA_CLEAN_VOI_H

// EspINA
#include <Support/ViewManager.h>
#include <Support/Tool.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoStack>

class QAction;
namespace EspINA
{
  class VOIToolsGroup;

  /// Volume Of Interest Toolbar
  class CleanVOITool
  : public Tool
  {
    Q_OBJECT
  public:
    /* \brief CleanVOITool class constructor.
     * \param[in] model       Analysis model adapter.
     * \param[in] viewManager Application view manager.
     * \param[in] undoStack   Application qt undo stack.
     * \param[in] toolGroup   VOIToolsGroup pointer containing the VOI accumulator.
     */
    explicit CleanVOITool(ModelAdapterSPtr  model,
                          ViewManagerSPtr   viewManager,
                          QUndoStack       *undoStack,
                          VOIToolsGroup    *toolGroup);

    /* \brief CleanVOITool class virtual destructor.
     *
     */
    virtual ~CleanVOITool();

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
    void cancelROI();
    void ROIChanged();

  private:
    ModelAdapterSPtr  m_model;
    ViewManagerSPtr   m_viewManager;
    QUndoStack       *m_undoStack;
    VOIToolsGroup    *m_toolGroup;
    QAction          *m_cleanVOI;
    bool              m_enabled;
  };

  using CleanVOIToolSPtr = std::shared_ptr<CleanVOITool>;

} // namespace EspINA

#endif // ESPINA_CLEAN_VOI_H
