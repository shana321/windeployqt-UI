#ifndef WINDEPLOYQT_H
#define WINDEPLOYQT_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class windeployqt; }
QT_END_NAMESPACE

class windeployqt : public QWidget
{
    Q_OBJECT

public:
    windeployqt(QWidget *parent = nullptr);
    ~windeployqt();

private slots:
    void on_SetQtPath_clicked();

    void on_MSVC_Release_clicked();

    void on_SetExePath_clicked();

    void on_MinGW_Release_clicked();

private:
    Ui::windeployqt *ui;
    QString QtPath = "";        //记录Qt安装根目录
    QString MSVCPath = "";      //记录MSVC版windeployqt.exe的位置
    QString MinGWPath = "";     //记录MinGW版windeployqt.exe的位置
    QString ExePath = "";       //待打包发布的程序，同时也是发布位置
    int find = 0;               //标记是否记录到windeployqt.exe位置
};
#endif // WINDEPLOYQT_H
