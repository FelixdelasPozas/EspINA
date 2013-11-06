/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jaime Fernandez <jfernandez@cesvima.upm.es>
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

#ifndef ESPINA_TREE_H
#define ESPINA_TREE_H

#include "Core/EspinaCore_Export.h"

// Qt dependencies
#include <QMap>
#include <QStringList>

#include <memory>

namespace EspINA
{
  //const QString DEFAULT_CATEGORY_COLOR = "#00FF00"; //Red

  struct Already_Defined_Node_Exception{};

  /// Tree-like structure representing taxonomical relationships
  template<typename T>
  class EspinaCore_EXPORT Tree
  {
  public:
      using Node     = std::shared_ptr<T>;
      using NodeList = QList<Node>;

  public:
    explicit Tree(const QString& name=QString());
    ~Tree();

    void setName(const QString& name)
    { m_name = name; }

    QString name() const
    { return m_name; }

    Node createNode(const QString &relativeName,
                 Node parent = Node());

    void removeNode(Node element);

    Node  root(){return m_root;}
    Node  node(const QString &classificationName);
    Node  parent(const Node node) const;

  private:
    QString m_name;
    Node    m_root;
  };

  template<typename T>
  QString print(std::shared_ptr<Tree<T>> tree, int indent = 0);

#include "Core/Utils/Tree.txx"
}// namespace EspINA

#endif // ESPINA_TREE_H
