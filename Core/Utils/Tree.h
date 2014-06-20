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

// C++
#include <memory>

namespace EspINA
{
  //const QString DEFAULT_CATEGORY_COLOR = "#00FF00"; //Red

  struct Already_Defined_Node_Exception{};

  /// Tree-like structure representing categorical relationships
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

    Node  root() const {return m_root;}
    Node  node(const QString &classificationName);
    Node  parent(const Node node) const;

  private:
    QString m_name;
    Node    m_root;
  };

  template<typename T>
  QString print(std::shared_ptr<Tree<T>> tree, int indent = 0);

  template<typename T>
  Tree<T>::Tree(const QString& name)
  : m_name(name)
  , m_root(new T(nullptr, QString()))
  {
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Tree<T>::~Tree()
  {
     //qDebug() << "Destroy classification";
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  typename Tree<T>::Node Tree<T>::createNode(const QString& relativeName, Node parent)
  {
    T *parentNode = parent.get();

    if (!parentNode)
      parentNode = m_root.get();

    Node requestedNode;

    if (!relativeName.isEmpty())
    {
      QStringList path = relativeName.split("/", QString::SkipEmptyParts);
      for (int i = 0; i < path.size(); ++i)
      {
        requestedNode = parentNode->subCategory(path.at(i));
        if (i == path.size() - 1 && requestedNode != nullptr)
        {
          throw Already_Defined_Node_Exception();
        }

        if (!requestedNode)
        {
          requestedNode = parentNode->createSubCategory(path.at(i));
        }
        parentNode = requestedNode.get();
      }
    }
    else
    {
      requestedNode = parentNode->subCategory(QString("Unspecified"));

      if (!requestedNode)
        requestedNode = parentNode->createSubCategory(QString("Unspecified"));
    }

    return requestedNode;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void Tree<T>::removeNode(Node node)
  {
    Q_ASSERT(node);

    if (node != m_root)
    {
      T *parentElement = node->parent();
      parentElement->removeSubCategory(node);
    }
    else
      m_root.reset();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  typename Tree<T>::Node Tree<T>::node(const QString& qualifiedName)
  {
    QStringList path = qualifiedName.split("/", QString::SkipEmptyParts);
    Node node = m_root;

    int i = 0;
    while (node.get() != nullptr && i < path.length())
    {
      node = node->subCategory(path[i]);
      ++i;
    }

    if (node == m_root)
      node.reset();

    return node;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  typename Tree<T>::Node Tree<T>::parent(const Node subCategory) const
  {
    QStringList path = subCategory->classificationName().split("/", QString::SkipEmptyParts);

    Node parent = m_root;
    for(int i = 0; i < path.length() - 1; i++)
    {
      parent = parent->subCategory(path[i]);
      Q_ASSERT(parent.get() != nullptr);
    }

    return parent;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  QString print(std::shared_ptr<Tree<T>> tree, int indent)
  {
    QString out = QString("Tree: %1\n").arg(tree->name());

    foreach(typename Tree<T>::Node node, tree->root()->subCategories())
    {
      out += QString("%1").arg(print(node, indent));
    }

    return out;
  }

}// namespace EspINA

#endif // ESPINA_TREE_H
