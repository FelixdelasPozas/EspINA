/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef RULERTOOL_H_
#define RULERTOOL_H_

// EspINA
#include <GUI/Tools/ITool.h>
#include <GUI/ViewManager.h>

namespace EspINA
{
  class ViewManager;
  class EspinaRenderView;
  class RulerWidget;
  
  class RulerTool
  : public ITool
  {
    Q_OBJECT
    public:
      explicit RulerTool(ViewManager *);
      virtual ~RulerTool();

      // implements ITool
      virtual QCursor cursor() const { return Qt::CrossCursor; };
      virtual bool filterEvent(QEvent *e, EspinaRenderView *view=NULL);
      virtual void setInUse(bool value);
      virtual void setEnabled(bool value);
      virtual bool enabled() const;

    public slots:
      void selectionChanged(ViewManager::Selection, bool);
      void selectedElementChanged();

    private:
      bool m_enabled;
      bool m_inUse;
      RulerWidget *m_widget;
      ViewManager *m_viewManager;
      ViewManager::Selection m_selection;
  };

  typedef boost::shared_ptr<RulerTool> RulerToolSPtr;

} /* namespace EspINA */
#endif /* RULERTOOL_H_ */
