#include "mainwindow.h"
#include "ui_mainwindow.h"

//#define de_bug

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->hide();
    ui->progressBar->setMinimum(0);

    this->setWindowTitle("URL图片下载");
    this->setWindowIcon(QIcon("./bitbug_favicon.ico"));

    ui->label_stat->setText("空闲");

    busy = false;
    num = 0;
}


MainWindow::~MainWindow()
{
    delete ui;
}

int i = 0;
// 根据url下载图片到save_path的调用接口
bool MainWindow::dowmload_url(QString url,QString save_path)
{
    // 判断url链接正确性
    if(url.isEmpty()) return false;

    // 检查程序忙碌状态
    if(busy == false)
        busy = true;
    else{
        QMessageBox::warning(this,tr("Ops!"),"下载程序正忙，请等待下载完成",QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok);
        return false;
    }

    // 构造请求
    QNetworkRequest requese;
    requese.setUrl(QUrl(url));

    // 发送请求
    QNetworkAccessManager manager;
    QNetworkReply *pReplay = manager.get(requese);
    ui->label_stat->setText("正在加载图片，请稍等");

    //开启一个局部的事件循环，等待响应结束，退出
    QEventLoop eventLoop;
    QObject::connect(pReplay,SIGNAL(finished()), &eventLoop, SLOT(quit()));

    // 使用定时器来保证等待上限 6秒--这么久也代表失败了
    QTimer tim_finish;
    QObject::connect(&tim_finish,SIGNAL(timeout()),&eventLoop,SLOT(quit()) );
    tim_finish.setSingleShot(true);
    tim_finish.start(6000);
    eventLoop.exec();
    tim_finish.stop();

    // 从url获取文件名，用于保存路径
    //http://xbull.oss-cn-shenzhen.aliyuncs.com/attendance/5fec6769c91fa9.48737443.png
    int len = url.indexOf("attendance/");
    QString file_name;
    // 代表没有找到这个字符串，也就是不是上面的特定URL，是其他URL，这种情况就随便起文件名
    if(len <= 0){
        file_name = QString::number(i++) +".png";
    }
    else file_name = url.mid(len+11);

    QString save_file = save_path +"/" +file_name;
    qDebug() << "file_name is " << file_name;

#ifdef de_bug
    ui->label_stat->setText("图片数据已经存入缓冲区！下面开始保存图片!");
    qDebug() << "save_path_file_name :" << save_file;
#endif


    // 判断是否错误
    if (pReplay->error() == QNetworkReply::NoError)
    {
#ifdef de_bug
        qDebug() << QString("request [%1] success").arg(file_name);
#endif

        // 判断图片是否下载错误
        QImage saveF;
        QByteArray saveF_data = pReplay->readAll();
        if(!saveF.loadFromData(saveF_data)){
            ui->label_stat->setText("图片下载错误！可能存在丢包等问题！");
            busy = false; // 解除忙,避免卡死
            return false;
        }

        if(file_name.endsWith("png")) saveF.save(save_file,"PNG",100);
        else if(file_name.endsWith("jpg"))  saveF.save(save_file,"JPG",100);


        // 显示图片,选中则不显示图片，提升速度
        if(!ui->radioButton->isChecked())
        {
            QPixmap pic;
            if(!pic.load(save_file)){
                QMessageBox::warning(this,tr("Ops!"),tr("文件下载完成，但是打开错误！请检查路径、检查文件名！"),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok);
            }
            pic = pic.scaled(ui->label_2->size());
            ui->label_2->setPixmap(pic);
        }

        busy = false; // 解除忙
        num++; // 记录实际下载次数
        return true;
    }
    else // 有错误
    {
        QVariant statusCodeV = pReplay->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QString err_msg = QString("获取错误！request [%1] handle errors\n\n").arg(file_name);
        err_msg += pReplay->errorString();
        err_msg += QString("\n\nrequest [%1] found error ....code: [%2] [%3]\n").arg(file_name).arg(statusCodeV.toInt()).arg((int)pReplay->error());
        QMessageBox::warning(this,tr("警告"),err_msg,QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok);
        ui->label_stat->setText("请检查网路链接或重新输入URL!");
        busy = false; // 解除忙
        return false;
    }
}

// 加载EXCEL表格的内容，丢链接去下载
bool MainWindow::dowmload_Excel(QString save_path)
{
    // 设置进度条及for循环结束状态
    int len = temp.count();
    ui->progressBar->show();
    ui->progressBar->setRange(0,len);
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);

    // 调用下载接口并设置进度条
    for (int i = 0;i < len ;i++)
    {
        if(dowmload_url(temp.at(i),save_path)){
            ui->progressBar->setValue(i);
        }
        else  // 失败则重试一次
        {
            dowmload_url(temp.at(i),save_path);
            ui->progressBar->setValue(i);
        }

    }

    // 显示按钮
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(true);
    return true;
}

// 点击下载按钮-如果url框框是空的，代表是从excel文件下载(下载完记得清空)，若这个也是空的，就弹框错误
void MainWindow::on_pushButton_clicked()
{
    QString url        = ui->lineEdit->text();
    QString save_path  = ui->label_save_path->text();
    QString excel_path = ui->label_execl_tabel->text();

    // 错误判断-先判断保存路径选了没有，之后再判断二维可能性--选择URL? 选择文件?
    if(save_path.isEmpty())
    {
        ui->label_stat->setText("请先选择有效路径！请重试");
        QMessageBox::warning(this,tr("警告"),tr("请先选择有效保存路径"),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok);
        return;
    }
    // 为真，代表两个之间有一个是空或都非空,内部判断
    if(!url.isEmpty() || !excel_path.isEmpty())
    {
        // 选择从URL链接下载
        if(excel_path.isEmpty() && !url.isEmpty())
        {
            if(!dowmload_url(url,save_path))
                {ui->label_stat->setText("URL链接下载失败,请重试");}
            else {ui->label_stat->setText("URL链接下载完成，空闲");}

            return;
        }
        // 选择从文件下载
        else if(!excel_path.isEmpty() && url.isEmpty())
        {
            dowmload_Excel(save_path);
            ui->progressBar->hide();
            ui->label_stat->setText(QString("URL文件链接下载完成，已完成下载[%1]").arg(num));
            num = 0;
            return;
        }
        else // 都非空
             goto url_flag;
    }
    // 两个都没选 或两个都选了
    else{
url_flag:
        QMessageBox::warning(this,tr("警告"),tr("只允许选择url链接下载或选择EXcel文件下载,请重试"),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok);
        ui->label_execl_tabel->clear();

        return;
    }
}

// 点击路径选择
void MainWindow::on_pushButton_3_clicked()
{
    //获取保存文件的路径
    QString path = QFileDialog::getExistingDirectory(this,"选择保存路径","./");
    if(path.isEmpty()) return ;
    ui->label_save_path->setText(path);
}

// 点击添加excel表,从该表处读取
void MainWindow::on_pushButton_2_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,"选择excel表格,目前只支持xlsx格式，请等待后续更新","./",tr("Exel file(*.xlsx)") );
    if(file_name.isEmpty()) return;
    ui->label_execl_tabel->setText(file_name);

    // 每次清空缓冲区,并先关闭按钮，等待加载完成，才开启按钮
    num = 0;
    temp.clear();
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->label_stat->setText("正在加载文件中..请稍等！");

    // 加载文件并获取行数
    QXlsx::Document xlsx(file_name);
    int len = xlsx.dimension().rowCount();

    // 默认是从第一个工作表 sheet1获取数据
    for(int i = 1; i<len + 1; i++)
        temp.append(xlsx.read(i,1).toString());

#ifdef de_bug
    qDebug() << __FUNCTION__ << __LINE__ <<"temp.size()" << temp.size() << " 文件有效行数len "<<len;
    qDebug() << QString("头部 [%1] 尾部 [%2]").arg(temp.first()).arg(temp.back());
#endif

    // 开启按钮并提醒用户
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(true);
    ui->label_stat->setText("文件加载完成!");
}
