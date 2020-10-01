#ifndef ADJACENCYGRAPH_H
#define ADJACENCYGRAPH_H

// Qt
#include <QStringList>
#include <QMap>

namespace GS
{
namespace DbPluginBase
{

class AdjacencyGraph
{
public:
    /// \brief Constructor
    AdjacencyGraph();
    /// \brief Destructor
    virtual ~AdjacencyGraph();
    /// \brief copy constructor
    AdjacencyGraph(const AdjacencyGraph &source);
    /// \brief define = operator
    AdjacencyGraph & operator=(const AdjacencyGraph &source);
    /// \brief clear graph
    void        Clear();
    /// \brief true is vertices list or matrix are empty
    bool        IsEmpty() const;
    /// \brief define the list of vertices
    void        SetVertices(const QStringList& vertices);
    /// \brief add 1 edge between start and end in the graph
    void        AddEdge(const QString &startVertex, const QString &endVertex, int weight);
    /// \brief true if the edge between start and end exists
    bool        EdgeExists(const QString &startVertex, const QString &endVertex) const;
    /// \brief true if the vertex exists
    bool        VertexExist(const QString &vertex) const;
    /// \brief weight of the edge between start and end (0 if not exists)
    int         EdgetWeight(const QString &startVertex, const QString &endVertex) const;
    /// \brief weight of path described in the list
    int         PathWeight(const QStringList &path) const;
    /// \brief list of successor of the vertex in the adjacency matrix
    QStringList SuccessorVertices(const QString &vertex) const;
    /// \brief list of predecessor of the vertex in the adjacency matrix
    QStringList PredecessorVertices(const QString &vertex) const;
    /// \brief return list of all vertices
    QStringList GetVertices() const;

private:
    /// \brief list of adjacency vertices of the vertex in the adjacency matrix
    QStringList AdjacentsVertices(const QString &vertex,
                                const QMap<QString, QMap<QString, int> > &matrix)
                                const;

    int                                 mVerticesNumber;            ///< holds number of vertices in the graph
    QStringList                         mVertices;                  ///< holds list of vertices in the graph
    QMap<QString, QMap<QString, int> >  mAdjacencyMatrix;           ///< holds adjacency matrix
    QMap<QString, QMap<QString, int> >  mAdjacencyInversedMatrix;   ///< holds inversed adjacency matrix
};

} //END namespace DbPluginBase
} //END namespace GS

#endif // ADJACENCYGRAPH_H
