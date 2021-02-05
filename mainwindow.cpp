#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QWheelEvent>
#include <QPainter>
#include <QFileInfo>
#include <QFileDialog>
#include <QApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setColorWidget = new XYSetColorWidget(this);
    ui->srcIconWidget->installEventFilter(this);
    ui->resultIconWidget->installEventFilter(this);
    ui->colorValueWidget->installEventFilter(this);
    ui->colorValueWidget->setMouseTracking(true);
    connect(ui->srcPathLineEdit, &QLineEdit::textChanged, this, &MainWindow::loadImage);
    ui->splitter->resize(18, 100);

    dealType = FixColorWithOutA;
    switchType = Single;
    keepAlpha = true;
    resultFileSuffix = "_convert";
    showColorValueWidget = nullptr;
    clear();
    resize(1120, 660);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::loadImage()
{
    clear();

    QString srcText = ui->srcPathLineEdit->text().trimmed();
    if (srcText.isEmpty() || !srcImage.load(srcText))
        return false;

    resultFile = ui->targetPathLineEdit->text().trimmed();
    if (resultFile.isEmpty())
    {
        QFileInfo info(srcText);
        resultFile = srcText.replace(info.baseName(), info.baseName() + resultFileSuffix);
        ui->targetPathLineEdit->setText(resultFile);
    }
    showColorValueWidget = ui->srcIconWidget;
    updateImage();
    return true;
}

void MainWindow::on_singleBtn_clicked()
{
    if (switchType == Single)
    {
        if (loadImage())
        {
            resultImage = srcImage;
            dealImage();
            resultImage.save(resultFile);
        }
    }
    else
    {
        auto files = QDir(ui->srcPathLineEdit->text()).entryInfoList();
        QDir targetDir(ui->targetPathLineEdit->text());
        for (int i = 0; i < files.size(); ++i)
        {
            QFileInfo info = files.at(i);
            QString srcText = info.absoluteFilePath();
            QImage img;
            if (!img.load(srcText))
                continue;

            srcImage = resultImage = img;
            if (!targetDir.exists())
                resultFile = srcText.replace(info.baseName(), info.baseName() + resultFileSuffix);
            else
                resultFile = targetDir.absoluteFilePath(info.fileName().replace(info.baseName(), info.baseName() + resultFileSuffix));

            dealImage();
            resultImage.save(resultFile);
        }
    }
    updateImage();
}

static void drawBackImage(QPainter &painter, QWidget *widget)
{
    QRect rect = widget->rect();
    QSize block(10, 10);
    QColor c1("#FFFFFF");
    QColor c2("#EEEEEE");
    bool white1 = false;
    for (int h = 0; h < rect.height() + 11; h += 10)
    {
        bool white = white1;
        for (int w = 0; w < rect.width() + 11; w += 10)
        {
            painter.fillRect(QRect(QPoint(w, h), block), white ? c1 : c2);
            white = !white;
        }
        white1 = !white1;
    }
}

static void drawImage(QPainter &painter, QWidget *widget, QImage &image, qreal scaled, bool focus)
{
    drawBackImage(painter, widget);
    if (!image.isNull())
    {
        auto imrect = image.rect();
        QImage im = image.scaled(imrect.size() * scaled);
        imrect = im.rect();
        imrect.moveCenter(widget->rect().center());
        painter.drawImage(imrect, im);
    }
    if (focus)
    {
        QPen pen = painter.pen();
        pen.setWidth(2);
        pen.setColor("red");
        pen.setStyle(Qt::DashDotLine);
        painter.setPen(pen);
        painter.drawRect(widget->rect().adjusted(1, 1, -2, -2));
    }
}

void MainWindow::drawColorValue(QPainter &painter, const QImage &image, int hoverColor)
{
    static QImage lastImage = QImage();
    static uint lastColorValueSize = 0;
    if (image.isNull()) {
        ui->scrollAreaWidgetContents->setFixedSize(ui->scrollArea->size());
        return;
    }

    drawColorValueBK(painter, image, hoverColor);
    if (lastImage != image || lastColorValueSize != colorValueSize)
    {
        lastImage = image;
        lastColorValueSize = colorValueSize;

        int height = image.height();
        int cl = image.bytesPerLine() / 4;
        QFont font = painter.font();
        font.setPointSize(colorValueSize);
        painter.setFont(font);
        allColors.clear();

        colorRectSize = painter.boundingRect(0, 0, 0, 0, Qt::AlignCenter, "FFFFFFFF").size() + QSize(4, 10);
        QSize wsize(colorRectSize.width() * cl + 10, colorRectSize.height() * height + 10);
        colorValueImage = QImage(wsize, QImage::Format_ARGB32);
        colorValueImage.fill(Qt::transparent);
        QPainter bkpainter(&colorValueImage);
        bkpainter.setFont(painter.font());
        for (int h = 10, ih = 0; ih < height; h += colorRectSize.height())
        {
            const int *row = (const int *)image.constScanLine(ih);
            for (int w = 10, i = 0; i < cl; i++, w += colorRectSize.width())
            {
                QRect rect(QPoint(w, h), colorRectSize);
                allColors[row[i]] += rect;
                bkpainter.drawText(rect, QString::asprintf("%08X", row[i]), QTextOption(Qt::AlignCenter));
            }

            ih++;
        }
    }

    painter.drawImage(0, 0, colorValueImageBK);
    painter.drawImage(0, 0, colorValueImage);
}

void MainWindow::drawColorValueBK(QPainter &painter, const QImage &image, int hoverColor)
{
    if (forceUpdateColorValueWidget)
    {
        forceUpdateColorValueWidget = false;

        int height = image.height();
        int cl = image.bytesPerLine() / 4;
        QFont font = painter.font();
        font.setPointSize(colorValueSize);
        painter.setFont(font);

        colorRectSize = painter.boundingRect(0, 0, 0, 0, Qt::AlignCenter, "FFFFFFFF").size() + QSize(4, 10);
        QSize wsize(colorRectSize.width() * cl + 10, colorRectSize.height() * height + 10);
        colorValueImageBK = QImage(wsize, QImage::Format_ARGB32);
        colorValueImageBK.fill(Qt::transparent);
        QPainter bkpainter(&colorValueImageBK);
        bkpainter.setRenderHints(QPainter::TextAntialiasing);
        bkpainter.setFont(painter.font());
        for (int h = 10, ih = 0; ih < height; h += colorRectSize.height())
        {
            const int *row = (const int *)image.constScanLine(ih);
            for (int w = 10, i = 0; i < cl; i++, w += colorRectSize.width())
            {
                QRect rect(QPoint(w, h), colorRectSize);
                if (isColorMeet(row[i], hoverColor))
                    bkpainter.fillRect(rect, QColor("#AAFFAA"));
                if (allSelectIndexs.contains(QPoint(i, ih)))
                    bkpainter.fillRect(rect, QColor("#AAAAFF"));
            }

            ih++;
        }

        ui->scrollAreaWidgetContents->setFixedSize(wsize + QSize(10, 10));
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Paint)
    {
        if (watched == ui->srcIconWidget)
        {
            QPainter painter(ui->srcIconWidget);
            drawImage(painter, ui->srcIconWidget, srcImage, srcImageScaled, ui->srcIconWidget == showColorValueWidget);
        }
        else if (watched == ui->resultIconWidget)
        {
            QPainter painter(ui->resultIconWidget);
            drawImage(painter, ui->resultIconWidget, resultImage, resultImageScaled, ui->resultIconWidget == showColorValueWidget);
        }
        else if (watched == ui->colorValueWidget)
        {
            QPainter painter(ui->colorValueWidget);
            drawColorValue(painter, getColorValueWidgetSrcImage(), hoverColorValue);
        }
    }
    else if (event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
        if (watched == ui->srcIconWidget)
        {
            srcImageScaled += wheel->delta() > 0 ? 0.1 : -0.1;
            ui->srcIconWidget->update();
        }
        else if (watched == ui->resultIconWidget)
        {
            resultImageScaled += wheel->delta() > 0 ? 0.1 : -0.1;
            ui->resultIconWidget->update();
        }
        else if (watched == ui->colorValueWidget)
        {
            if (!getColorValueWidgetSrcImage().isNull())
            {
                colorValueSize += wheel->delta() > 0 ? 1 : -1;
                if (colorValueSize <= 0)
                    colorValueSize = 1;
                updateColorValueWidget();
                return true;
            }
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        if (watched == ui->colorValueWidget)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                if (qApp->keyboardModifiers() == Qt::ControlModifier)
                {
                    QPoint colorIndex = mouseEvent->pos();
                    colorIndex.setX((colorIndex.x() - 10) / colorRectSize.width());
                    colorIndex.setY((colorIndex.y() - 10) / colorRectSize.height());
                    allSelectIndexs.append(colorIndex);
                }
                else
                {
                    allSelectIndexs.clear();
                }
            }
            updateColorValueWidget();
        }
    }
    else if (event->type() == QEvent::MouseButtonDblClick)
    {
        if (watched == ui->colorValueWidget)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            auto it = allColors.begin();
            while (it != allColors.end())
            {
                if (it.value().contains(mouseEvent->pos()))
                {
                    if (ui->resultIconWidget == showColorValueWidget)
                    {
                        if (!resultImage.isNull())
                            ui->targetColorLineEdit->setText(QString::asprintf("%08X", it.key()));
                    } else {
                        if (!srcImage.isNull())
                            ui->srcColorLineEdit->setText(QString::asprintf("%08X", it.key()));
                    }
                    break;
                }
                ++it;
            }
        }
    }
    else if (event->type() == QEvent::MouseMove)
    {
        if (watched == ui->colorValueWidget)
        {
            uint lastValue = hoverColorValue;
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            auto it = allColors.begin();
            while (it != allColors.end())
            {
                if (it.value().contains(mouseEvent->pos()))
                {
                    hoverColorValue = it.key();
                    break;
                }
                ++it;
            }
            if (lastValue != hoverColorValue)
                updateColorValueWidget();
        }
    }
    else if (event->type() == QEvent::ContextMenu)
    {
        QContextMenuEvent *mouseEvent = static_cast<QContextMenuEvent *>(event);
        if (watched == ui->colorValueWidget && !allSelectIndexs.isEmpty())
        {
            setColorWidget->move(mouseEvent->globalPos());
            if (setColorWidget->exec() == QDialog::Accepted)
            {
                if (resultImage.isNull())
                    resultImage = srcImage;

                uint tvalue = setColorWidget->getColor().toUInt(nullptr, 16);
                for (int i = 0; i < allSelectIndexs.size(); ++i)
                {
                    QPoint index = allSelectIndexs.at(i);
                    QRgb color = resultImage.pixel(index);
                    uint A = color >> 24 & 0x000000ff;

                    QColor switchColor = QColor::fromRgba(tvalue);
                    if (setColorWidget->ignoreAlpha())
                        switchColor = QColor::fromRgba(A << 24 | (tvalue & 0x00ffffff));
                    resultImage.setPixelColor(index, switchColor);
                }
                resultImage.save(resultFile);
                showColorValueWidget = ui->resultIconWidget;
                ui->resultIconWidget->update();
            }
            allSelectIndexs.clear();
            updateColorValueWidget();
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        if (watched == ui->srcIconWidget)
        {
            showColorValueWidget = ui->srcIconWidget;
        }
        else if (watched == ui->resultIconWidget)
        {
            showColorValueWidget = ui->resultIconWidget;
        }
        updateImage();
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::clear()
{
    resultImage = srcImage = QImage();
    resultImageScaled = srcImageScaled = 1.0;
    hoverColorValue = 0x12345678;
    colorValueSize = 10;
    updateColorValueWidget();
}

void MainWindow::dealImage()
{
    int height = srcImage.height();
    int cl = srcImage.bytesPerLine() / 4;
    for (int h = 0; h < height; ++h)
    {
        const uint *row = (const uint *)srcImage.constScanLine(h);
        for (int i = 0; i < cl; ++i)
        {
            uint A = row[i] >> 24 & 0x000000ff;
            uint value = ui->srcColorLineEdit->text().toUInt(nullptr, 16);
            uint tvalue = ui->targetColorLineEdit->text().toUInt(nullptr, 16);
            QColor switchColor = QColor::fromRgba(tvalue);
            if (keepAlpha)
                switchColor = QColor::fromRgba(A << 24 | (tvalue & 0x00ffffff));
            if (isColorMeet(row[i], value))
                resultImage.setPixelColor(QPoint(i, h), switchColor);
        }
    }
}

void MainWindow::updateImage()
{
    ui->resultIconWidget->update();
    ui->srcIconWidget->update();
    updateColorValueWidget();
}

void MainWindow::updateColorValueWidget()
{
    forceUpdateColorValueWidget = true;
    ui->colorValueWidget->update();
}

bool MainWindow::isColorMeet(int color, int target)
{
    switch (dealType)
    {
    case FixColorWithOutA: // 替换固定颜色，不包含Alpha
        if ((color | 0xff000000) == (0xff000000 | target))
            return true;
        break;
    case FixColor:        // 替换固定颜色，包含Alpha
        if (color == target)
            return true;
        break;
    case NotColor:        // 替换固定不为颜色的，包含Alpha
        if (color != target)
            return true;
        break;
    case NotColorOnlyA:   // 替换固定不为颜色的，仅判断Alpha
        if ((color & 0xff000000) != (0xff000000 & target))
            return true;
        break;
    }
    return false;
}

QImage MainWindow::getColorValueWidgetSrcImage()
{
    return ui->resultIconWidget == showColorValueWidget ? resultImage : srcImage;
}

void MainWindow::on_pushButton_clicked()
{
    QString file;
    if (switchType == Single)
        file = QFileDialog::getOpenFileName(this);
    else
        file = QFileDialog::getExistingDirectory(this);
    if (!file.isEmpty())
        ui->srcPathLineEdit->setText(file);
}

void MainWindow::on_pushButton_2_clicked()
{
    QString file;
    if (switchType == Single)
        file = QFileDialog::getOpenFileName(this);
    else
        file = QFileDialog::getExistingDirectory(this);
    if (!file.isEmpty())
        ui->targetPathLineEdit->setText(file);
}

void MainWindow::on_funcComboBox_activated(int index)
{
    dealType = index;
    hoverColorValue = 0x12345678;
    updateColorValueWidget();
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    keepAlpha = Qt::Checked == arg1;
}

void MainWindow::on_comboBox_activated(int index)
{
    switchType = index;
    srcImage = resultImage = QImage();
    showColorValueWidget = nullptr;
    ui->srcPathLineEdit->clear();
    ui->targetPathLineEdit->clear();
    updateImage();
}
