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

#ifndef GUI_DIALOGS_CUSTOMFILEDIALOG_H_
#define GUI_DIALOGS_CUSTOMFILEDIALOG_H_

// Qt
#include <QFileDialog>
#include <QMap>
#include <QVariant>

namespace ESPINA
{
  class OptionsPanel;

  /** \class CustomFileDialog
   * \brief Custom dialog to open files, adding some additional options.
   *
   */
  class CustomFileDialog
  : public QFileDialog
  {
    Q_OBJECT
    public:
      /** \brief CustomFileDialog class constructor.
       * \param[in] parent Raw pointer of the widget parent of this one.
       * \param[in] flags Window flags.
       */
      explicit CustomFileDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      /** \brief CustomFileDialog class constructor.
       * \param[in] parent Raw pointer of the widget parent of this one.
       * \param[in] caption Caption text.
       * \param[in] directory Starting directory.
       * \param[in] filter Files filter expression.
       *
       */
      explicit CustomFileDialog(QWidget *parent = nullptr, const QString &caption = QString(), const QString &directory = QString(), const QString &filter = QString());

      /** \brief CustomFileDialog class virtual destructor.
       *
       */
      virtual ~CustomFileDialog()
      {};

      /** \brief Returns the values of the options in the panel options.
       *
       */
      QMap<QString, QVariant> options() const;

    protected:
      virtual void resizeEvent(QResizeEvent *event) override;
      virtual void showEvent(QShowEvent * event) override;

    private slots:
      /** \brief Hides/shows the options widget.
       *
       */
      void onOptionsToggled();
    private:
      /** \brief Modifies the default file open dialog inserting the custom UI elements.
       *
       */
      void modifyUI();

      OptionsPanel *m_options; /** options widget instance.                                         */
      QSize         m_size;    /** original dialog size with button inserted and no options widget. */
  };

} // namespace ESPINA

#endif // GUI_DIALOGS_CUSTOMFILEDIALOG_H_
