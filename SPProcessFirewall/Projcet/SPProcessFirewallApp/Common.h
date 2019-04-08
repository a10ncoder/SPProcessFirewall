#pragma once

#include <QObject>
#include <QFile>
#include "SingletonHandler.h"

class CCommon : public QObject
{
	Q_OBJECT

public:
	CCommon(QObject *parent = 0);
	~CCommon();
public:
	void setSkinStyle(const QString &style);
};

typedef CSingletonHandler<CCommon> Common;