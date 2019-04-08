#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <windows.h>
#include <Tlhelp32.h>
#include <QPoint>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QHideEvent>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDrag>
#include "DrvCtrl.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
	// ���ĵ����¼�
	virtual void mousePressEvent(QMouseEvent* e);
	// �����ƶ��¼�
	virtual void mouseMoveEvent(QMouseEvent* e);
	// ���ĵ����ͷ��¼�
	virtual void mouseReleaseEvent(QMouseEvent* e);
	// ���عر��¼�
	virtual void closeEvent(QCloseEvent *event);
	// �¼�������
	bool eventFilter(QObject* obj, QEvent *evt);
private:
	// ��ק�����¼�
	virtual void dragEnterEvent(QDragEnterEvent *event);
	// ��ק�¼�
	virtual void dropEvent(QDropEvent *event);
public slots:
	// �����б�ѡ��
	void onAdd(void);
	// ���ӱ�����
	void onAdd2();
	// ˢ���б�
	void onRefresh(void);
	// ���̲˵��¼�
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
public:
	// ��ʼ�����̷���ǽ
	bool InitializeDriverMonitor();
	// ��ʼ��TableView
	void InitializeTableView();
	// ��ʼ��ö�ٽ���
	void InitializeProcessEnum();
	// ���ð�ťͼƬ��ʽ
	void SetButtonStyle(QPushButton *button, QString imgsrc, int CutSec);
private:
	//bool readFile(const QString &fileName);
    Ui::MainWindow *ui;
	// ��¼ ��ȡ���λ���봰��λ�õĲ�ֵ
	QPoint m_offset;
	// �ж�����Ƿ�����ޱ��ⴰ�����ı�������Χ��
	bool m_bisMoveable;
	// �Ҽ��˵�
	QMenu *rightMenu; 
	// ���̲˵�
	QMenu *trayIconMenu;
	// ����ͼ��
	QSystemTrayIcon *trayIcon;
	// ����ö����
	QStandardItemModel* model;
};

#endif // MAINWINDOW_H
