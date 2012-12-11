/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "QueryView.h"

QueryView::QueryView(QStringList &query, QWidget* parent, Qt::WindowFlags f)
: QDialog(parent, f)
, m_query(query)
{
  setupUi(this);

//   m_queryModel.setStringList(m_query);
//   listView->setModel(&m_queryModel);
  foreach(QString info, query)
  {
    listView->addItem(info);
    listView->item(listView->count()-1)->setCheckState(Qt::Checked);
  }

  connect(acceptQuery, SIGNAL(clicked(bool)),
	  this, SLOT(accept()));
}

QueryView::~QueryView()
{
}

void QueryView::accept()
{
  m_query.clear();
  for (int r=0; r < listView->count(); r++)
  {
    if (listView->item(r)->checkState() == Qt::Checked)
      m_query << listView->item(r)->data(Qt::DisplayRole).toString();
  }
  QDialog::accept();
}

