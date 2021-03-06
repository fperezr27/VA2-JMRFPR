#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/matx.hpp>

#include <imgviewer.h>
#include <ui_pixelTForm.h>
#include <ui_lFilterForm.h>
#include <ui_operOrderForm.h>

#include <QtWidgets/QFileDialog>




using namespace cv;

namespace Ui {
    class MainWindow;
}

class PixelTDialog : public QDialog, public Ui::PixelTForm
{
    Q_OBJECT

public:
    PixelTDialog(QDialog *parent=0) : QDialog(parent){
        setupUi(this);
    }
};

class LFilterDialog : public QDialog, public Ui::LFilterForm
{
    Q_OBJECT

public:
    LFilterDialog(QDialog *parent=0) : QDialog(parent){
        setupUi(this);
    }
};



class OperOrderDialog : public QDialog, public Ui::OperOrderForm
{
    Q_OBJECT

public:
    OperOrderDialog(QDialog *parent=0) : QDialog(parent){
        setupUi(this);
    }
};




class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    PixelTDialog pixelTDialog;
    LFilterDialog lFilterDialog;
    OperOrderDialog operOrderDialog;
    QTimer timer;

    VideoCapture *cap;
    ImgViewer *visorS, *visorD, *visorHistoS, *visorHistoD;
    Mat colorImage, grayImage, destColorImage, destGrayImage;
    bool winSelected;
    Rect imageWindow;

    void updateHistograms(Mat image, ImgViewer * visor);

public slots:
    void compute();
    void start_stop_capture(bool start);
    void change_color_gray(bool color);
    void selectWindow(QPointF p, int w, int h);
    void deselectWindow(QPointF p);
    void load_image();
    void save_image();
    void procesar();
    void ecualizar(Mat aux, Mat yuv[3]);
    void umbralizar(Mat aux, Mat yuv[3]);
    void filtroGausiano(Mat aux, Mat yuv[3]);
    void filtroMedio(Mat aux, Mat yuv[3]);
    void filtroLineal(Mat aux, Mat yuv[3]);
    void dilatar(Mat aux, Mat yuv[3]);
    void erosionar(Mat aux, Mat yuv[3]);
    void transformarPixel(Mat aux, Mat yuv[3]);

};
#endif // MAINWINDOW_H
