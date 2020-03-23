#include "mainwindow.h"
//#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    widget = new MainWidget(ui->centralWidget);
    widget->setGeometry(QRect(0, 0, 600, 600));
    widget->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

#include "mainwindow.moc"
