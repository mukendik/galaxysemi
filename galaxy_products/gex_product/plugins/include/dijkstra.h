#ifndef DIJKSTRA_H
#define DIJKSTRA_H

// Qt
#include <QPair>
#include <QStringList>
#include <QMap>
// Galaxy
#include "adjacencygraph.h"

namespace GS
{
namespace DbPluginBase
{

class Dijkstra
{
public:
    /// \brief Constructor
    Dijkstra();
    /// \brief Destructor
    virtual ~Dijkstra();
    /// \brief set the graph to  be used in the algorithm
    bool        SetGraph(const AdjacencyGraph& graph);
    /// \brief shortest path from start vertex to end vertex, empty if path doesn't exists
    QStringList ShortestPath(const QString &startVertex,
                                const QString &endVertex,
                                int &weight);

private:
    Q_DISABLE_COPY(Dijkstra);
    /// \brief run the Dijkstra algorithm
    bool        Run(const QString &startVertex, const QString &endVertex);
    /// \brief update shortest to vertex adajcent vertices
    void        Relax(const QString &vertex);
    /// \brief weight of shortest path from source to vertex
    int         ShortestPathFromSource(const QString &vertex);
    /// \brief set shortest path from source to vertex
    void        SetShortestPathFromSource(const QString &vertex, int pathWeight);
    /// \brief compare function to sort remaining vertices starting from the closest
    static bool VerticesLessThan(const QPair<QString, int > & e1,
                                 const QPair<QString, int > & e2);

    AdjacencyGraph              mGraph;             ///< holds the adjacency graph
    QStringList                 mDeterminedVertices;///< holds list of vertices already checked in the algorithm
    QList<QPair<QString, int> > mRemainingVertices; ///< holds list of vertices to check in the algorithm and the best computed weight to them
    QMap<QString, int>          mShortestPath;      ///< holds the weight of the shortest path from the source to a vertex in the graph
    QMap<QString, QString>      mPredecessorPath;   ///< holds the predecessor map of the shortest path from the source to a vertex in the graph
};

} //END namespace DbPluginBase
} //END namespace GS

#endif // DIJKSTRA_H
