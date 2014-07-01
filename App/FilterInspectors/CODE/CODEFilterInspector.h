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


#ifndef CODESETTINGS_H
#define CODESETTINGS_H

#include <Core/Model/Filter.h>
#include <QWidget>
#include <boost/graph/graph_concepts.hpp>

// Forward declarations
class QSpinBox;

namespace EspINA
{
  class MorphologicalEditionFilter;
  class ViewManager;

  class CODESettings
  : public QWidget
  {
    Q_OBJECT
  public:
    explicit CODESettings(QString title,
                          MorphologicalEditionFilter *filter,
                          QUndoStack  *undoSactk,
                          ViewManager *viewManager);
    virtual ~CODESettings();

  protected slots:
    void modifyFilter();
    void updateWidget();

  private:
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    MorphologicalEditionFilter *m_filter;
    QSpinBox *m_spinbox;
  };

  class CODEFilterInspector
  : public Filter::FilterInspector
  {
  public:
    CODEFilterInspector(QString title, MorphologicalEditionFilter *filter);

    virtual QWidget *createWidget(QUndoStack *stack, ViewManager *viewManager);

  private:
    QString    m_title;
    MorphologicalEditionFilter *m_filter;
  };

} // namespace EspINA

#endif // CODESETTINGS_H
