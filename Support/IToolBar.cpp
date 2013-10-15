/*
 * IToolBar.cpp
 *
 *  Created on: 16/06/2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "IToolBar.h"

namespace EspINA
{
  //----------------------------------------------------------------------------
  IToolBar::IToolBar(QWidget *parent)
  : QToolBar(parent)
  , m_undoIndex(INT_MAX)
  {
  }

  //----------------------------------------------------------------------------
  IToolBar::IToolBar(const QString &title, QWidget *parent)
  : QToolBar(title, parent)
  , m_undoIndex(INT_MAX)
  {
  }

  //----------------------------------------------------------------------------
  IToolBar::~IToolBar()
  {
  }
}


