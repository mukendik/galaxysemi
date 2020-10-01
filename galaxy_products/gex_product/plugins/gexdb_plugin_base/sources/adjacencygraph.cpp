#include "adjacencygraph.h"
#include <gqtl_log.h>

namespace GS
{
namespace DbPluginBase
{

AdjacencyGraph::AdjacencyGraph()
{
}

AdjacencyGraph::~AdjacencyGraph()
{
}

AdjacencyGraph::AdjacencyGraph(const AdjacencyGraph &source)
{
    *this = source;
}

AdjacencyGraph &AdjacencyGraph::operator =(const AdjacencyGraph &source)
{
    if (this != &source)
    {
        mVerticesNumber = source.mVerticesNumber;
        mVertices = source.mVertices;
        mAdjacencyMatrix = source.mAdjacencyMatrix;
        mAdjacencyInversedMatrix = source.mAdjacencyInversedMatrix;
    }

    return *this;
}

void AdjacencyGraph::Clear()
{
    mVerticesNumber = 0;
    mVertices.clear();
    mAdjacencyMatrix.clear();
    mAdjacencyInversedMatrix.clear();
}

bool AdjacencyGraph::IsEmpty() const
{
    return (mVertices.isEmpty() ||
            mAdjacencyMatrix.isEmpty() ||
            mAdjacencyInversedMatrix.isEmpty());
}

void AdjacencyGraph::SetVertices(const QStringList& vertices)
{
    // init main structs
    mVertices = vertices;
    mAdjacencyMatrix.clear();
    mAdjacencyInversedMatrix.clear();
    // init adjacency matrix
    for (int i = 0; i < vertices.count(); ++i)
    {
        QMap<QString, int> distances;
        for (int j = 0; j < vertices.count(); ++j)
            distances.insert(vertices.at(j), 0);
        mAdjacencyMatrix.insert(vertices.at(i), distances);
    }
    // init inversed adajcency matrix
    mAdjacencyInversedMatrix = mAdjacencyMatrix;
}

void AdjacencyGraph::AddEdge(const QString &startVertex,
                             const QString &endVertex,
                             int weight)
{
    if (!mAdjacencyMatrix.contains(startVertex))
    {
        GSLOG(SYSLOG_SEV_ERROR,
               QString("Unable to add edge %1 not in the adjacency graph!").
               arg(startVertex).toLatin1().data());
        return;
    }

    if (!mAdjacencyMatrix.value(startVertex).contains(endVertex))
    {
        GSLOG(SYSLOG_SEV_ERROR,
               QString("Unable to add edge %1 not in the adjacency graph!").
               arg(endVertex).toLatin1().data());
        return;
    }

    mAdjacencyMatrix[startVertex][endVertex] = weight;
    mAdjacencyInversedMatrix[endVertex][startVertex] = weight;
}

bool AdjacencyGraph::EdgeExists(const QString &startVertex,
                                const QString &endVertex) const
{
    if (mAdjacencyMatrix[startVertex][endVertex] == 0)
        return false;
    return true;
}

bool AdjacencyGraph::VertexExist(const QString &vertex) const
{
    return mVertices.contains(vertex);
}

int AdjacencyGraph::EdgetWeight(const QString &startVertex,
                                const QString &endVertex) const
{
    if (!mVertices.contains(startVertex) || !mVertices.contains(endVertex))
        return 0;

    return mAdjacencyMatrix[startVertex][endVertex];
}

int AdjacencyGraph::PathWeight(const QStringList &path) const
{
    if (path.count() < 1)
        return 0;

    int lTotalWeight = 0;
    // compute the path weight, edge by edge
    for (int i = 0; i < (path.count() - 1); ++i)
    {
        int lWeight = EdgetWeight(path.at(i), path.at(i + 1));
        if (lWeight == 0) // if doesn't exist
            return 0;
        lTotalWeight += lWeight;
    }

    return lTotalWeight;
}

QStringList AdjacencyGraph::SuccessorVertices(const QString &vertex) const
{
    return AdjacentsVertices(vertex, mAdjacencyMatrix);
}

QStringList AdjacencyGraph::PredecessorVertices(const QString &vertex) const
{
    return AdjacentsVertices(vertex, mAdjacencyInversedMatrix);
}

QStringList AdjacencyGraph::GetVertices() const
{
    return mVertices;
}

QStringList AdjacencyGraph::AdjacentsVertices(
        const QString &vertex,
        const QMap<QString, QMap<QString, int> > &matrix) const
{
    QStringList lAdjacentsVertices;
    QMap<QString, int> lCandidatesVertices = matrix.value(vertex);
    QMapIterator<QString, int> it(lCandidatesVertices);
    // Retrieve all adjacent vertices
    while (it.hasNext())
    {
        it.next();
        int lWeight = it.value();
        if (lWeight > 0)
            lAdjacentsVertices.append(it.key());
    }
    return lAdjacentsVertices;
}

} //END namespace DbPluginBase
} //END namespace GS

