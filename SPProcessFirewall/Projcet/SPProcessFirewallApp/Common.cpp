#include "Common.h"
#include  <QApplication>

CCommon::CCommon(QObject *parent)
	: QObject(parent)
{
}

CCommon::~CCommon()
{
}

void CCommon::setSkinStyle(const QString & style)
{
	QFile file(style);
	if (file.open(QFile::ReadOnly))
	{
		QString styleSheet = QLatin1String(file.readAll());
		qApp->setStyleSheet(styleSheet);
	}
	file.close();
}
