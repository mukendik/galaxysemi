#include <QList>
#include "coutput.h"

QList<COutput*> COutput::s_ListCOutput;

QString COutput::Flush()
{
	while (!m_buffer.isEmpty())
		PopFront();
	return "ok";
}

