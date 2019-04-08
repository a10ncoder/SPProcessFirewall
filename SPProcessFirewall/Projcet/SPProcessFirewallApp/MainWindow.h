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
	// 鼠标的单击事件
	virtual void mousePressEvent(QMouseEvent* e);
	// 鼠标的移动事件
	virtual void mouseMoveEvent(QMouseEvent* e);
	// 鼠标的单击释放事件
	virtual void mouseReleaseEvent(QMouseEvent* e);
	// 重载关闭事件
	virtual void closeEvent(QCloseEvent *event);
	// 事件过滤器
	bool eventFilter(QObject* obj, QEvent *evt);
private:
	// 拖拽进入事件
	virtual void dragEnterEvent(QDragEnterEvent *event);
	// 拖拽事件
	virtual void dropEvent(QDropEvent *event);
public slots:
	// 增加列表选中
	void onAdd(void);
	// 增加表单输入
	void onAdd2();
	// 刷新列表
	void onRefresh(void);
	// 托盘菜单事件
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
public:
	// 初始化进程防火墙
	bool InitializeDriverMonitor();
	// 初始化TableView
	void InitializeTableView();
	// 初始化枚举进程
	void InitializeProcessEnum();
	// 设置按钮图片样式
	void SetButtonStyle(QPushButton *button, QString imgsrc, int CutSec);
private:
	//bool readFile(const QString &fileName);
    Ui::MainWindow *ui;
	// 记录 获取鼠标位置与窗口位置的差值
	QPoint m_offset;
	// 判断鼠标是否放在无标题窗口栏的标题栏范围内
	bool m_bisMoveable;
	// 右键菜单
	QMenu *rightMenu; 
	// 托盘菜单
	QMenu *trayIconMenu;
	// 托盘图标
	QSystemTrayIcon *trayIcon;
	// 进程枚举项
	QStandardItemModel* model;
};

#endif // MAINWINDOW_H
