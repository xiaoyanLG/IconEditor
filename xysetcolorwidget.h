#ifndef XYSETCOLORWIDGET_H
#define XYSETCOLORWIDGET_H

#include <QDialog>

namespace Ui {
class XYSetColorWidget;
}

class XYSetColorWidget : public QDialog
{
    Q_OBJECT

public:
    explicit XYSetColorWidget(QWidget *parent = nullptr);
    ~XYSetColorWidget();
    void setIndexs(const QList<QPoint> &indexs, bool ignoreAlpha);
    bool ignoreAlpha();
    QString getColor();

private slots:
    void on_cancelPushButton_clicked();
    void on_replacePushButton_clicked();

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::XYSetColorWidget *ui;
    bool pressed;
    QPoint lastPressedPos;
};

#endif // XYSETCOLORWIDGET_H
