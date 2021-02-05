#include "xysetcolorwidget.h"
#include "ui_xysetcolorwidget.h"
#include <QPainter>
#include <QMouseEvent>

XYSetColorWidget::XYSetColorWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XYSetColorWidget), pressed(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAutoFillBackground(true);
}

XYSetColorWidget::~XYSetColorWidget()
{
    delete ui;
}

bool XYSetColorWidget::ignoreAlpha()
{
    return ui->checkBox->isChecked();
}

QString XYSetColorWidget::getColor()
{
    return ui->lineEdit->text().trimmed();
}

void XYSetColorWidget::on_cancelPushButton_clicked()
{
    reject();
}

void XYSetColorWidget::on_replacePushButton_clicked()
{
    accept();
}

bool XYSetColorWidget::event(QEvent *event)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
        pressed = true;
        lastPressedPos = mouseEvent->pos();
        break;
    case QEvent::MouseButtonRelease:
        pressed = false;
        break;
    case QEvent::MouseMove:
        if (pressed)
            move(pos() + mouseEvent->pos() - lastPressedPos);
        break;
    default:
        break;
    }

    return QDialog::event(event);
}

void XYSetColorWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QColor("cccccc"));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}
