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

#ifndef ESPINA_TOOL_H
#define ESPINA_TOOL_H

#include "Support/EspinaSupport_Export.h"

// C++
#include <memory>

// Qt
#include <QCursor>
#include <QWidgetAction>
#include <QObject>

class QHBoxLayout;
class QAction;
class QEvent;
class QIcon;
class QPushButton;
class QString;

namespace ESPINA
{
  class RenderView;

  class EspinaSupport_EXPORT Tool
  : public QObject
  {
  public:
    class NestedWidgets
    : public QWidgetAction
    {
    public:
      explicit NestedWidgets(QObject *parent);

      void addWidget(QWidget *widget);

    private:
      QHBoxLayout *m_layout;
    };

  public:
    explicit Tool();

    /** \brief Enables/Disables the tool.
     * \param[in] value true to enable false otherwise.
     *
     */
    void setEnabled(bool value);

    /** \brief Returns true if the tool is enabled, false otherwise.
     *
     */
    bool isEnabled() const;

    virtual QList<QAction *> actions() const = 0;

    static QAction *createAction( const QString &icon, const QString &tooltip, QObject *parent );

    static QAction *createAction(const QIcon &icon, const QString &tooltip, QObject *parent);

    static QPushButton *createButton(const QString &icon, const QString &tooltip);

    static QPushButton *createButton(const QIcon &icon, const QString &tooltip);

    virtual void abortOperation() = 0;

  private:
    virtual void onToolEnabled(bool enabled) = 0;

  signals:
    void changedActions();

  private:
    bool m_enabled;
  };

  using ToolSPtr  = std::shared_ptr<Tool>;
  using ToolSList = QList<ToolSPtr>;
} // namespace ESPINA

#endif // ESPINA_TOOL_H
