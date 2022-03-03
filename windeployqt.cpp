#include "windeployqt.h"
#include "ui_windeployqt.h"

#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>


windeployqt::windeployqt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::windeployqt)
{
    ui->setupUi(this);

    //查询是否有存储过路径信息
    QFile PathIni("./StoredPath.ini");
    if (!PathIni.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "错误", "无法打开StoredPath.ini: " + PathIni.errorString());
        return;
    }
    //读取文件
    QTextStream in(&PathIni);
    QStringList PathList = in.readAll().split(";", Qt::SkipEmptyParts);
    PathIni.close();
    if(!PathList.isEmpty())
    {
        //通过路径长短区分出Qt根目录，通过字符包含区分出MSVC和MinGW版本
        int size = 999;
        int qt_idx = 0;
        for(int i = 0; i < PathList.size(); i++)
        {
            //qDebug() << PathList.at(i).toLocal8Bit().constData();
            if(PathList.at(i).size() < size)
            {
                size = PathList.at(i).size();
                qt_idx = i;
            }
            if(PathList.at(i).contains("msvc",Qt::CaseInsensitive))
                MSVCPath = PathList.at(i);
            if(PathList.at(i).contains("mingw",Qt::CaseInsensitive))
                MinGWPath = PathList.at(i);
        }
        QtPath = PathList.at(qt_idx);
    }
    if(!QtPath.isEmpty())
    {
        //判断路径是否存在
        QDir Qtdir(QtPath);
        //存储的路径不对
        if(!Qtdir.exists())
        {
            QtPath = "";
            MSVCPath = "";
            MinGWPath = "";
        }
        //读到了Qt根目录
        else
        {
            ui->QtPathEdit->setText(QtPath);
            if(!MSVCPath.isEmpty())
            {
                //判断路径是否存在
                QFile MSVCexe(MSVCPath);
                if(!MSVCexe.exists())
                {
                    MSVCPath = "";
                    ui->MSVCPathEdit->setText("未找到MSVC版本的windeployqt.exe");
                }
                else
                {
                    find = 1;
                    ui->MSVCPathEdit->setText(MSVCPath);
                }
            }
            if(!MinGWPath.isEmpty())
            {
                //判断路径是否存在
                QFile MinGWexe(MinGWPath);
                if(!MinGWexe.exists())
                {
                    MinGWPath = "";
                    ui->MinGWPathEdit->setText("未找到MinGW版本的windeployqt.exe");
                }
                else
                {
                    find = 1;
                    ui->MinGWPathEdit->setText(MSVCPath);
                }
            }
        }
    }


}

windeployqt::~windeployqt()
{
    delete ui;
}

void windeployqt::on_SetQtPath_clicked()
{
    QtPath = QFileDialog::getExistingDirectory(this, "选择路径");
    QtPath = QDir::toNativeSeparators(QtPath);
    //是否填入了路径
    if (QtPath.isEmpty())
    {
        QMessageBox::warning(this, "错误", "请选择路径！");
        return;
    }
    //判断路径是否存在
    QDir Qtdir(QtPath);
    if(!Qtdir.exists())
    {
        QMessageBox::warning(this, "Error","路径不存在！");
        return;
    }

    //判断是否Qt根目录
    QString LicensePath = QtPath + "/Licenses";
    QDir Licensedir(LicensePath);
    if(!Licensedir.exists())
    {
        QMessageBox::warning(this, "Error","请选择Qt根目录！");
        return;
    }

    ui->QtPathEdit->setText(QtPath);    //Qt根目录显示到UI界面

    //查找windeployqt.exe
    QProcess p;
    QString argument = "where.exe /R " + QtPath + " windeployqt.exe";
    qDebug() << argument;
    p.start("powershell.exe", QStringList(argument));
    if(!p.waitForStarted())
    {
        qDebug() << "出错了！程序无法开始运行";
        return;
    }
    if(!p.waitForFinished(10000))
    {
        qDebug() << "程序运行出错，中止！请检查命令";
        return;
    }
    else if(true == QString::fromLocal8Bit(p.readAllStandardError()).contains("无法将“" + argument + "”项识别为"))
    {
        qDebug() << "无法识别命令"+argument+"!";
        return;
    }
    QString Ret = QString::fromLocal8Bit(p.readAllStandardOutput());
    //qDebug() << Ret;
    QStringList RetList = Ret.split("\r\n", Qt::SkipEmptyParts);
    for(int i = 0; i < RetList.size(); i++)
    {
        //qDebug() << RetList.at(i).toLocal8Bit().constData();
        if(RetList.at(i).contains("msvc",Qt::CaseInsensitive))
            MSVCPath = RetList.at(i);
        if(RetList.at(i).contains("mingw",Qt::CaseInsensitive))
            MinGWPath = RetList.at(i);
    }
    //判断是否找到windeployqt.exe所在位置
    if(MSVCPath.isEmpty())
        ui->MSVCPathEdit->setText("未找到MSVC版本的windeployqt.exe");
    else
    {
        QFile MSVCexe(MSVCPath);
        if(!MSVCexe.exists())
            ui->MSVCPathEdit->setText("未找到MSVC版本的windeployqt.exe");
        else
        {
            find = 1;
            ui->MSVCPathEdit->setText(MSVCPath);
        }
    }
    if(MinGWPath.isEmpty())
        ui->MinGWPathEdit->setText("未找到MinGW版本的windeployqt.exe");
    else
    {
        QFile MinGWexe(MinGWPath);
        if(!MinGWexe.exists())
            ui->MinGWPathEdit->setText("未找到MinGW版本的windeployqt.exe");
        else
        {
            find = 1;
            ui->MinGWPathEdit->setText(MinGWPath);
        }
    }
    if(!find)
    {
        QMessageBox testMassage;
        testMassage.setText("未找到任何可用的windeployqt.exe，请检查Qt路径选择是否正确或安装是否完整！");
        testMassage.exec();
    }
    //写入路径信息存储文件
    else
    {
        QFile PathIni("./StoredPath.ini");
        if (!PathIni.open(QIODevice::WriteOnly | QFile::Text)) {
            QMessageBox::warning(this, "错误", "无法写入StoredPath.ini: " + PathIni.errorString());
            return;
        }
        //写入文件
        QTextStream out(&PathIni);
        out << QtPath+";";
        if(!MSVCPath.isEmpty())
            out << MSVCPath+";";
        if(!MinGWPath.isEmpty())
            out << MinGWPath+";";
        PathIni.close();
    }
}

void windeployqt::on_SetExePath_clicked()
{
    //选择待发布的exe路径
    ExePath = QFileDialog::getOpenFileName(this, "选择待发布的exe路径");
    ExePath = QDir::toNativeSeparators(ExePath);
    //用户未选择路径
    if (ExePath.isEmpty())
    {
        QMessageBox::warning(this, "错误", "请选择有效的exe路径！");
        return;
    }
    if(!ExePath.contains(".exe"))
    {
        ExePath = "";
        QMessageBox::warning(this, "错误", "请选择有效的exe路径！");
        return;
    }
    //检查exe是否存在
    QFile Exefile(ExePath);
    if (!Exefile.exists())
    {
        ExePath = "";
        QMessageBox::warning(this, "错误", "目标EXE文件未找到！");
        return;
    }
    ui->ExePathEdit->setText(ExePath);
}

void windeployqt::on_MSVC_Release_clicked()
{
    if(!MSVCPath.isEmpty())
    {
        //判断windeployqt.exe是否存在
        QFile MSVCexe(MSVCPath);
        if(!MSVCexe.exists())
            QMessageBox::warning(this, "错误", "未找到MSVC版本的windeployqt.exe！");
        //执行打包发布程序
        else
        {
            //是否已经选择打包的目标程序
            if(ExePath.isEmpty())
            {
                QMessageBox::warning(this, "错误", "未选择待发布的exe路径！");
                return;
            }
            //执行windeployqt.exe对目标exe进行打包发布
            QProcess p;
            QString argument = "&\"" + MSVCPath + "\"" + " \"" + ExePath + "\"";
            qDebug() << argument;
            p.start("powershell.exe", QStringList(argument));
            if(!p.waitForStarted())
            {
                qDebug() << "出错了！程序无法开始运行";
                return;
            }
            if(!p.waitForFinished(10000))
            {
                qDebug() << "程序运行出错，中止！请检查命令";
                return;
            }
            else if(true == QString::fromLocal8Bit(p.readAllStandardError()).contains("无法将“" + argument + "”项识别为"))
            {
                qDebug() << "无法识别命令"+argument+"!";
                return;
            }
            QString Ret = QString::fromLocal8Bit(p.readAllStandardOutput());
            qDebug() << Ret;
            if(!Ret.contains(ExePath))
                QMessageBox::warning(this, "错误", "发布失败！");
            else
                QMessageBox::warning(this, "OK", "发布成功！");

        }
    }
    else
       QMessageBox::warning(this, "错误", "未找到MSVC版本的windeployqt.exe！");
}

void windeployqt::on_MinGW_Release_clicked()
{
    if(!MinGWPath.isEmpty())
    {
        //判断windeployqt.exe是否存在
        QFile MinGWexe(MinGWPath);
        if(!MinGWexe.exists())
            QMessageBox::warning(this, "错误", "未找到MinGW版本的windeployqt.exe！");
        //执行打包发布程序
        else
        {
            //是否已经选择打包的目标程序
            if(ExePath.isEmpty())
            {
                QMessageBox::warning(this, "错误", "未选择待发布的exe路径！");
                return;
            }
            //执行windeployqt.exe对目标exe进行打包发布
            QProcess p;
            QString argument = "&\"" + MinGWPath + "\"" + " \"" + ExePath + "\"";
            //qDebug() << argument;
            p.start("powershell.exe", QStringList(argument));
            if(!p.waitForStarted())
            {
                qDebug() << "出错了！程序无法开始运行";
                return;
            }
            if(!p.waitForFinished(10000))
            {
                qDebug() << "程序运行出错，中止！请检查命令";
                return;
            }
            else if(true == QString::fromLocal8Bit(p.readAllStandardError()).contains("无法将“" + argument + "”项识别为"))
            {
                qDebug() << "无法识别命令"+argument+"!";
                return;
            }
            QString Ret = QString::fromLocal8Bit(p.readAllStandardOutput());
            qDebug() << Ret;
            if(!Ret.contains(ExePath))
                QMessageBox::warning(this, "错误", "发布失败！");
            else
                QMessageBox::warning(this, "OK", "发布成功！");

        }
    }
    else
       QMessageBox::warning(this, "错误", "未找到MinGW版本的windeployqt.exe！");
}

