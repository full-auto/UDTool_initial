#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkAddressEntry>
#include <QNetworkReply>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QFile>

// 使用第三方xlsx库
#include <xlsxdocument.h>
#include <xlsxformat.h>
#include <xlsxcellrange.h>
#include <xlsxchart.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool dowmload_url(QString url,QString save_path); // 根据url下载图片到save_path
    bool dowmload_Excel(QString save_path);     //从全局QStringList temp里获取的URL去下载图片
private slots:
    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    bool busy; //为真代表忙，为假代表空闲

    int num; // 计数值，计算下载的图片量

    // 每次点击了文件获取按钮，就加载缓冲区
    QStringList temp;
};
#endif // MAINWINDOW_H
