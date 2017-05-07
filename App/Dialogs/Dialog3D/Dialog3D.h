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
#include <App/ToolGroups/ToolGroup.h>
#include <GUI/View/View3D.h>
#include <Support/Representations/RepresentationFactory.h>
#include <Support/Types.h>
#include "ui_Dialog3D.h"

// Qt
#include <QDialog>
#include <QTemporaryFile>
#include <QToolBar>

namespace ESPINA
{
  class Dialog3D;

  /** \class Dialog3DTool
   * \brief Implements the tool for the 3D view dialog.
   *
   */
  class Dialog3DTool
  : public Support::Widgets::ProgressTool
  {
    public:
      /** \brief Dialog3DTool class constructor.
       * \param[in] context application context.
       * \param[in] dialog pointer of the dialog managed by this tool.
       *
       */
      explicit Dialog3DTool(Support::Context &context, Dialog3D *dialog);

      /** \brief Dialog3DTool class virtual destructor.
       *
       */
      virtual ~Dialog3DTool()
      {};

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

    private:
      /** \brief Returns the tools of the dialog.
       *
       */
      Support::Widgets::ToolSList tools() const;

      Dialog3D *m_dialog; /** current dialog. */
  };

  /** \class Dialog3D
   * \brief Implements the 3D view dialog.
   *
   */
  class Dialog3D
  : public QDialog
  , public Ui::Dialog3D
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief Dialog3D class constructor.
       * \param[in] context application context
       *
       */
      explicit Dialog3D(Support::Context &context);

      /** \brief SegmentationInspector class virtual destructor.
       *
       */
      virtual ~Dialog3D()
      {}

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

      /** \brief Adds a representation switch to the toolbar.
       *
       */
      void addRepresentationSwitch(RepresentationSwitchSPtr repSwitch);

    signals:
      void dialogVisible(bool);

    private slots:
      /** \brief Restores the dialog's tools settings and hides/shows the dialog.
       * \param[in] checked true to show the dialog and false to hide it.
       *
       */
      void onToggled(bool checked);

    protected:
      virtual void showEvent(QShowEvent *event) override;
      virtual void closeEvent(QCloseEvent *event) override;

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

      /** \brief Sets the settings for the included tools.
       * \param[in] settings tools' settings.
       *
       */
      void setToolsSettings(std::shared_ptr<QSettings> settings);

      /** \brief Restores the settings of the tools.
       *
       */
      void restoreToolsSettings();

      /** \brief Saves the current settings of the tools to the QSettings object and deactivates the tools.
       *
       */
      void saveToolsSettings();

    private:
      friend class Dialog3DTool;

      QToolBar                  *m_toolbar;         /** dialog's toolbar.        */
      ToolGroup                  m_representations; /** visualization toolgroup. */
      QTemporaryFile             m_iniFile;         /** ini file storage.        */
      std::shared_ptr<QSettings> m_toolsSettings;   /** tools settings.          */
      View3D                     m_view3D;          /** 3D view.                 */
  };

} // namespace ESPINA

#endif // ESPINA_DIALOG_3D_H
