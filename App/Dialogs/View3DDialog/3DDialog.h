/*

    Copyright (C) 2015  Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_DIALOG_3D_H
#define ESPINA_DIALOG_3D_H

// ESPINA
#include <GUI/View/View3D.h>
#include <Support/Context.h>
#include <Support/Widgets/Tool.h>
#include "ui_3DDialog.h"

// Qt
#include <QDialog>

namespace ESPINA
{
  namespace Support
  {
    namespace Widgets
    {
      class ProgressTool;
    }
  }

  class Dialog3D
  : public QDialog
  , public Ui::View3DDialog
  {
    Q_OBJECT
  public:
    /** \brief Dialog3D class constructor.
     * \param[in] context ESPINA context
     */
    Dialog3D(Support::Context &context);

    /** \brief SegmentationInspector class destructor.
     *
     */
    virtual ~Dialog3D();

    /** \brief Returns the internal pointer of the 3d view.
     *
     */
    RenderView *renderView();

    /** \brief Returns the QAction that shows/hides the dialog.
     *
     */
    QAction *toggleViewAction();

    /** \brief Returs the tool for this dialog.
     *
     */
    std::shared_ptr<Support::Widgets::ProgressTool> tool();

  signals:
    void dialogVisible(bool);

  private slots:
    void onToggled(bool checked);

  protected:
    virtual void closeEvent(QCloseEvent *e) override;

  private:
    /** \brief Helper method to initialize the view widget.
     *
     */
    void initView3D();

    /** \brief Helper method to restore the dialog geometry from the registry.
     *
     */
    void restoreGeometryState();

    /** \brief Helper method to save the dialog geometry to the registry.
     *
     */
    void saveGeometryState();

  private:
    static const QString GEOMETRY_SETTINGS_KEY;

    Support::Context &m_context;
    View3D            m_view3D;
  };

} // namespace ESPINA

#endif // ESPINA_DIALOG_3D_H