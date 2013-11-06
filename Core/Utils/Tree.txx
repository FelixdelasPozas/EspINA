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
   //qDebug() << "Destroy taxonomy";
}

//-----------------------------------------------------------------------------
template<typename T>
typename Tree<T>::Node Tree<T>::createNode(const QString& relativeName, Node parent)
{
  T *parentNode = parent.get();

  if (!parentNode)
    parentNode = m_root.get();

  Node requestedTree;

  if (!relativeName.isEmpty())
  {
    QStringList path = relativeName.split("/", QString::SkipEmptyParts);
    for (int i = 0; i < path.size(); ++i)
    {
      requestedTree = parentNode->subCategory(path.at(i));
      if (i == path.size() - 1 && requestedTree != nullptr) {
        throw Already_Defined_Node_Exception();
      }

      if (!requestedTree)
      {
        requestedTree = parentNode->createSubCategory(path.at(i));
      }
      parentNode = requestedTree.get();
    }
  }
  else
  {
    requestedTree = parentNode->subCategory(QString("Unspecified"));

    if (!requestedTree)
      requestedTree = parentNode->createSubCategory(QString("Unspecified"));
  }

  return requestedTree;
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
  while (node.get() != NULL && i < path.length())
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
    Q_ASSERT(parent.get() != NULL);
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