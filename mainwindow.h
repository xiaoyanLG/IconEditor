#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QRegion>
#include "xysetcolorwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    enum {FixColorWithOutA, FixColor, NotColorOnlyA, NotColor};
    enum {Single, Batch};
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    bool loadImage();
    void on_singleBtn_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_funcComboBox_activated(int index);
    void on_checkBox_stateChanged(int arg1);
    void on_comboBox_activated(int index);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void clear();
    void dealImage();
    void updateImage();
    void updateColorValueWidget();
    bool isColorMeet(int color, int target);
    QImage getColorValueWidgetSrcImage();
    void drawColorValue(QPainter &painter, const QImage &image, int hoverColor);
    void drawColorValueBK(QPainter &painter, const QImage &image, int hoverColor);

private:
    Ui::MainWindow *ui;
    QWidget *showColorValueWidget;
    XYSetColorWidget *setColorWidget;
    QImage srcImage;
    QImage resultImage;
    QImage colorValueImage;
    QImage colorValueImageBK;
    bool forceUpdateColorValueWidget;
    QString resultFile;
    QString resultFileSuffix;
    qreal resultImageScaled;
    qreal srcImageScaled;
    int dealType;
    int switchType;
    bool keepAlpha;
    QMap<uint, QRegion> allColors;
    QMap<uint, QColor> valueColors;
    uint hoverColorValue;
    uint colorValueSize;
    QSize colorRectSize;
    QList<QPoint> allSelectIndexs;
};
#endif // MAINWINDOW_H
