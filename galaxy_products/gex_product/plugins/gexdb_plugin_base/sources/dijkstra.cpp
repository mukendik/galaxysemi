#include "dijkstra.h"
#include <gqtl_log.h>
#include <limits>

namespace GS
{
namespace DbPluginBase
{

Dijkstra::Dijkstra()
{
}

Dijkstra::~Dijkstra()
{
}

bool Dijkstra::SetGraph(const AdjacencyGraph &graph)
{
    if (graph.IsEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR,
               QString("Empty graph, unable to apply Dijkstra!").
               toLatin1().data());
        return false;
    }
    mGraph = graph;
    return true;
}

QStringList Dijkstra::ShortestPath(const QString &startVertex,
                            const QString &endVertex,
                            int & weight)
{
    weight = 0;
    if (!mGraph.VertexExist(startVertex) ||
            !mGraph.VertexExist(endVertex))
        return QStringList();

    if (startVertex == endVertex)
        return QStringList();

    // Run Dijkstra algo
    Run(startVertex, endVertex);
    QStringList lShortestPath;
    // If a path to end vertex has been found
    if (ShortestPathFromSource(endVertex) != std::numeric_limits<int>::max())
    {
        // Build path starting by end vertex
        QString lPredecessor = endVertex;
        do
        {
            lShortestPath.prepend(lPredecessor);
            lPredecessor = mPredecessorPath.value(lPredecessor);
        }
        while (lPredecessor != startVertex);
        // add start vertex
        lShortestPath.prepend(startVertex);
        GSLOG(SYSLOG_SEV_DEBUG,
               QString("Link from %1 to %2 found!").
               arg(startVertex).
               arg(endVertex).
               toLatin1().data());
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG,
               QString("No link found from %1 to %2 found!").
               arg(startVertex).
               arg(endVertex).
               toLatin1().data());
    }

    QMapIterator<QString, int> itVertex(mShortestPath);
    while (itVertex.hasNext())
    {
        itVertex.next();
        weight += itVertex.value();
    }
    return lShortestPath;
}

bool Dijkstra::Run(const QString &startVertex, const QString &endVertex)
{
    // Init structs
    mShortestPath.clear();
    mPredecessorPath.clear();
    mDeterminedVertices.clear();
    mRemainingVertices.clear();
    // add start vertex in the shortest path
    mShortestPath.insert(startVertex, 0);
    mRemainingVertices.append(qMakePair(startVertex, 0));
    // while it remains vertices to check
    while (!mRemainingVertices.isEmpty())
    {
        // Sort vertices starting by the closest to the source
        qSort(mRemainingVertices.begin(),
              mRemainingVertices.end(),
              VerticesLessThan);
        QString lClosestVertex = mRemainingVertices.takeFirst().first;
        // Path found!
        if (lClosestVertex == endVertex)
            break;
        // vertex now considered as checked
        mDeterminedVertices.append(lClosestVertex);
        // update shortest to vertex adajcent vertices
        Relax(lClosestVertex);
    }
    return true;
}

bool Dijkstra::VerticesLessThan(const QPair<QString, int > & e1,
                             const QPair<QString, int > & e2)
{
    // Compare distance to the source vertex
    return (e1.second < e2.second);
}

void Dijkstra::Relax(const QString &vertex)
{
    QStringList lVertexSuccessors = mGraph.SuccessorVertices(vertex);
    // For each successor vertice of vertex
    for (int i = 0; i < lVertexSuccessors.count(); ++i)
    {
        QString lAdjVertex = lVertexSuccessors.at(i);
        // if not already determined
        if (!mDeterminedVertices.contains(lAdjVertex))
        {
            int lDistance = ShortestPathFromSource(vertex) +
                    mGraph.EdgetWeight(vertex, lAdjVertex);
            // if is a new shortest path, record it
            if (ShortestPathFromSource(lAdjVertex) > lDistance)
            {
                SetShortestPathFromSource(lAdjVertex, lDistance);
                mPredecessorPath.insert(lAdjVertex, vertex);
                mRemainingVertices.append(qMakePair(lAdjVertex, lDistance));
            }
        }
    }
}

int Dijkstra::ShortestPathFromSource(const QString &vertex)
{
    if (mShortestPath.contains(vertex))
        return mShortestPath.value(vertex);
    else
        return std::numeric_limits<int>::max();
}

void Dijkstra::SetShortestPathFromSource(const QString &vertex, int pathWeight)
{
    mShortestPath.insert(vertex, pathWeight);
}

} //END namespace DbPluginBase
} //END namespace GS

