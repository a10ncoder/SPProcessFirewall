#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	// 无标题栏
	this->setWindowFlags(Qt::FramelessWindowHint);

	// 设置窗体标题
	this->setWindowTitle(QString::fromLocal8Bit("进程管理器"));

	// 设置窗口图标
	this->setWindowIcon(QIcon(":/resource/SPManager.ico"));

	// 支持状体拖拽
	this->setAcceptDrops(true);

	//托盘初始化
	QIcon icon = QIcon(":/resource/SPManager.ico");
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setIcon(icon);
	trayIcon->setToolTip(QString::fromLocal8Bit("进程管理器"));
	trayIcon->show(); //必须调用，否则托盘图标不显示

	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(QString::fromLocal8Bit("退出"), qApp, SLOT(quit()));
	trayIcon->setContextMenu(trayIconMenu);
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

	// 增加菜单事件
	rightMenu = new QMenu;
	rightMenu->addAction(QString::fromLocal8Bit("增加"), this, SLOT(onAdd()));     //设置菜单项,并连接槽函数
	rightMenu->addAction(QString::fromLocal8Bit("刷新"), this, SLOT(onRefresh()));     //设置菜单项,并连接槽函数

	//设置系统按钮消息
	SetButtonStyle(ui->btn_close, ":/resource/menu/close.png", 4);
	QObject::connect(ui->btn_close, SIGNAL(clicked()), this, SLOT(close()));
	QObject::connect(ui->btn_add, SIGNAL(clicked()), this, SLOT(onAdd2()));

	// 初始化列表
	InitializeTableView();

	// 初始化遍历当前运行进程
	InitializeProcessEnum();

	// 初始化驱动
	if (!InitializeDriverMonitor())
	{
		ui->statusBar->showMessage(QString::fromLocal8Bit("驱动初始化失败!"));
	}
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
		trayIcon->showMessage("title", "你单击了"); //后面两个默认参数
		break;
	case QSystemTrayIcon::DoubleClick:
		{
			QWidget *pWindow = this->window();
			pWindow->isMaximized() ? pWindow->showNormal() : pWindow->showNormal();
		}
		break;
	case QSystemTrayIcon::MiddleClick:
		trayIcon->showMessage("title", "你中键了");
		break;
	default:
		break;
	}

}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (trayIcon->isVisible())
	{
		hide(); //隐藏窗口
		event->ignore(); //忽略事件
	}
}

// 鼠标的单击事件
void MainWindow::mousePressEvent(QMouseEvent* e)
{
	if (e->buttons() == Qt::LeftButton)
	{
		m_offset = e->globalPos() - pos();
		QRect rect(0, 0, this->size().width(), 27); //就是之前frame的大小
		if (rect.contains(e->pos()))
		{
			m_bisMoveable = true;
		}
	}
}

// 鼠标的移动事件
void MainWindow::mouseMoveEvent(QMouseEvent* e)
{
	if (e->buttons() && Qt::LeftButton&&m_bisMoveable)
	{
		move(e->globalPos() - m_offset);
	}
}

// 鼠标的单击释放事件
void MainWindow::mouseReleaseEvent(QMouseEvent* e)
{
	Q_UNUSED(e)
	if (m_bisMoveable)
	{
		m_bisMoveable = false;
	}
}

#include <QMessageBox>
void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
	event->acceptProposedAction();
	//QMessageBox::about(NULL, QString::fromLocal8Bit("测试"), QString::fromLocal8Bit(" 测试"));
}

#include <QUrl>
#include <QMimeData>
#include <QList>
void MainWindow::dropEvent(QDropEvent * event)
{
	QString name = event->mimeData()->urls().first().toString();
	//QMessageBox::about(NULL, QString::fromLocal8Bit("测试"), QString::fromLocal8Bit(" 测试"));
}

// 初始化进程防火墙
bool MainWindow::InitializeDriverMonitor()
{
	if (CProcessMonitor::Instance()->Open("\\\\.\\SPProcessFirewall"))
	{
		return true;
	}
	else
	{
		//if (CProcessMonitor::Instance()->Install(qApp->applicationDirPath().toStdString() += "/SPProcessFirewall.sys",
		if (CProcessMonitor::Instance()->Install("C:\\Users\\alan\\Desktop\\SPProcessFirewall\\Build\\Debug\\SPProcessFirewall.sys",
			"SPProcessFirewall", 
			"SPProcessFirewall"))
		{
			CProcessMonitor::Instance()->Start();
			if (CProcessMonitor::Instance()->Open("\\\\.\\SPProcessFirewall"))
			{
				return true;
			}
		}
	}

	return false;
}

void MainWindow::InitializeTableView()
{
	model = new QStandardItemModel(this);
	model->setColumnCount(2);
	model->setHeaderData(0, Qt::Horizontal, "Pid");
	model->setHeaderData(1, Qt::Horizontal, "Name");
	ui->tableView->setModel(model);
	ui->tableView->setColumnWidth(0, 60);
	ui->tableView->setColumnWidth(1, 170);

	// 默认显示行头，如果你觉得不美观的话，我们可以将隐藏        
	ui->tableView->verticalHeader()->hide();
	// 设置选中时为整行选中        
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	// 设置表格的单元为只读属性，即不能编辑        
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	// 设置表格为单选模式
	ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	// 设置事件
	ui->tableView->installEventFilter(this);
}

void MainWindow::InitializeProcessEnum()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return;
	}
	PROCESSENTRY32 pi;
	pi.dwSize = sizeof(PROCESSENTRY32); //第一次使用必须初始化成员
	BOOL bRet = Process32First(hSnapshot, &pi);
	int i = 0;
	while (bRet)
	{
		model->setItem(i, 0, new QStandardItem(QString("%1").arg(pi.th32ProcessID)));
		//设置字符颜色
		model->item(i, 0)->setForeground(QBrush(QColor(255, 0, 0)));
		//设置字符位置
		model->item(i, 0)->setTextAlignment(Qt::AlignCenter);
		model->setItem(i, 1, new QStandardItem(QString("%1").arg(pi.szExeFile)));
		i++;
		bRet = Process32Next(hSnapshot, &pi);
	}

	ui->lab_title->setText(QString::fromLocal8Bit(" 进程管理器-进程数量:%1").arg(i));
}

bool MainWindow::eventFilter(QObject* obj, QEvent *evt)
{
	if (obj == ui->tableView  &&  evt->type() == QEvent::ContextMenu)
	{
		if (ui->tableView->currentIndex().isValid() == true)
		{
			rightMenu->exec(QCursor::pos());
		}
	}
	return QWidget::eventFilter(obj, evt);
}

void MainWindow::onAdd()
{
	QString strProcess(model->index(ui->tableView->currentIndex().row(), 1).data().toString());
	if (!CProcessMonitor::Instance()->IoInsertMonitor(strProcess.toStdString().c_str()))
	{
		QMessageBox::about(NULL, QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("驱动通信异常,添加失败!"));
	}
}

void MainWindow::onAdd2()
{
	QString strProcess(ui->text_process->toPlainText());
	if (!CProcessMonitor::Instance()->IoInsertMonitor(strProcess.toStdString().c_str()))
	{
		QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("驱动通信异常,添加失败!"));
	}
}

void MainWindow::onRefresh()
{
	// 清理原列表
	model->removeRows(0, model->rowCount());
	// 清理标题
	ui->lab_title->setText(QString::fromLocal8Bit(" 进程管理器-进程数量:0"));
	// 重新遍历进程
	InitializeProcessEnum();
}

void MainWindow::SetButtonStyle(QPushButton *button, QString imgsrc, int CutSec)
{
	int img_w = QPixmap(imgsrc).width();
	int img_h = QPixmap(imgsrc).height();
	int PicWidth = img_w / CutSec;
	button->setFixedSize(PicWidth, img_h);
	button->setStyleSheet(QString("QPushButton{border-width: 41px; border-image: url(%1)  0 0 0 %2 repeat  repeat;border-width: 0px; border-radius: 0px;}")
		.append("QPushButton::hover{border-image: url(%1) 0 0 0 %3  repeat  repeat;}")
		.append("QPushButton::pressed{border-image: url(%1) 0  0 0 %4 repeat  repeat;}")
		.append("QPushButton::checked{border-image: url(%1) 0  0 0 %4 repeat  repeat;}")
		.append("QPushButton::disabled{border-image: url(%1) 0  0 0 %5 repeat  repeat;}")
		.arg(imgsrc).arg(0).arg(PicWidth * 1).arg(PicWidth * 2).arg(PicWidth * 3));
}