#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cap = new VideoCapture(0);
    winSelected = false;

    colorImage.create(240,320,CV_8UC3);
    grayImage.create(240,320,CV_8UC1);
    destColorImage.create(240,320,CV_8UC3);
    destColorImage.setTo(0);
    destGrayImage.create(240,320,CV_8UC1);
    destGrayImage.setTo(0);

    visorS = new ImgViewer(&grayImage, ui->imageFrameS);
    visorD = new ImgViewer(&destGrayImage, ui->imageFrameD);

    visorHistoS = new ImgViewer(260,150, (QImage *) NULL, ui->histoFrameS);
    visorHistoD = new ImgViewer(260,150, (QImage *) NULL, ui->histoFrameD);

    connect(&timer,SIGNAL(timeout()),this,SLOT(compute()));
    connect(ui->captureButton,SIGNAL(clicked(bool)),this,SLOT(start_stop_capture(bool)));
    connect(ui->colorButton,SIGNAL(clicked(bool)),this,SLOT(change_color_gray(bool)));
    connect(visorS,SIGNAL(mouseSelection(QPointF, int, int)),this,SLOT(selectWindow(QPointF, int, int)));
    connect(visorS,SIGNAL(mouseClic(QPointF)),this,SLOT(deselectWindow(QPointF)));

    connect(ui->pixelTButton,SIGNAL(clicked()),&pixelTDialog,SLOT(show()));
    connect(pixelTDialog.okButton,SIGNAL(clicked()),&pixelTDialog,SLOT(hide()));

    connect(ui->kernelButton,SIGNAL(clicked()),&lFilterDialog,SLOT(show()));
    connect(lFilterDialog.okButton,SIGNAL(clicked()),&lFilterDialog,SLOT(hide()));

    connect(ui->operOrderButton,SIGNAL(clicked()),&operOrderDialog,SLOT(show()));
    connect(operOrderDialog.okButton,SIGNAL(clicked()),&operOrderDialog,SLOT(hide()));

    connect(ui->loadButton,SIGNAL(clicked(bool)),this,SLOT(load_image()));
    connect(ui->saveButton,SIGNAL(clicked(bool)),this,SLOT(save_image()));

    timer.start(30);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete cap;
    delete visorS;
    delete visorD;
    colorImage.release();
    grayImage.release();
    destColorImage.release();
    destGrayImage.release();
}

void MainWindow::compute()
{

    //Captura de imagen
    if(ui->captureButton->isChecked() && cap->isOpened())
    {
        *cap >> colorImage;
        cv::resize(colorImage, colorImage, Size(320,240));
        cvtColor(colorImage, grayImage, COLOR_BGR2GRAY);
        cvtColor(colorImage, colorImage, COLOR_BGR2RGB);

    }


    //En este punto se debe incluir el c??digo asociado con el procesamiento de cada captura
    procesar();

    //Actualizaci??n de los visores
     if(!ui->colorButton->isChecked())
     {
         updateHistograms(grayImage, visorHistoS);
         updateHistograms(destGrayImage, visorHistoD);
     }

     if(winSelected)
     {
         visorS->drawSquare(QPointF(imageWindow.x+imageWindow.width/2, imageWindow.y+imageWindow.height/2), imageWindow.width,imageWindow.height, Qt::green );
     }
     visorS->update();
     visorD->update();
     visorHistoS->update();
     visorHistoD->update();

 }

 void MainWindow::updateHistograms(Mat image, ImgViewer * visor)
 {
     if(image.type() != CV_8UC1) return;

     Mat histogram;
     int channels[] = {0,0};
     int histoSize = 256;
     float grange[] = {0, 256};
     const float * ranges[] = {grange};
     double minH, maxH;

     calcHist( &image, 1, channels, Mat(), histogram, 1, &histoSize, ranges, true, false );
     minMaxLoc(histogram, &minH, &maxH);

     float maxY = visor->getHeight();

     for(int i = 0; i<256; i++)
     {
         float hVal = histogram.at<float>(i);
         float minY = maxY-hVal*maxY/maxH;

         visor->drawLine(QLineF(i+2, minY, i+2, maxY), Qt::red);
     }

}

void MainWindow::start_stop_capture(bool start)
{
    if(start)
        ui->captureButton->setText("Stop capture");
    else
        ui->captureButton->setText("Start capture");
}

void MainWindow::change_color_gray(bool color)
{
    if(color)
    {
        ui->colorButton->setText("Gray image");
        visorS->setImage(&colorImage);
        visorD->setImage(&destColorImage);
    }
    else
    {
        ui->colorButton->setText("Color image");
        visorS->setImage(&grayImage);
        visorD->setImage(&destGrayImage);
    }
}

void MainWindow::selectWindow(QPointF p, int w, int h)
{
    QPointF pEnd;
    if(w>0 && h>0)
    {
        imageWindow.x = p.x()-w/2;
        if(imageWindow.x<0)
            imageWindow.x = 0;
        imageWindow.y = p.y()-h/2;
        if(imageWindow.y<0)
            imageWindow.y = 0;
        pEnd.setX(p.x()+w/2);
        if(pEnd.x()>=320)
            pEnd.setX(319);
        pEnd.setY(p.y()+h/2);
        if(pEnd.y()>=240)
            pEnd.setY(239);
        imageWindow.width = pEnd.x()-imageWindow.x+1;
        imageWindow.height = pEnd.y()-imageWindow.y+1;

        winSelected = true;
    }
}

void MainWindow::deselectWindow(QPointF p)
{
    std::ignore = p;
    winSelected = false;
}

void MainWindow::load_image(){
    Mat auxImage;
    timer.stop();
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), ".", tr("Image Files (*.png *.jpg *bmp)"));
    auxImage =imread(fileName.toStdString());
    if (!auxImage.empty()){
        cv::resize(auxImage, auxImage, Size(320,240));
        cvtColor(auxImage, grayImage, COLOR_BGR2GRAY);
        cvtColor(auxImage, colorImage, COLOR_BGR2RGB);
    }
    timer.start(30);
}

void MainWindow::save_image(){
    Mat auxImage;
        timer.stop();
        QString fileName = QFileDialog::getSaveFileName(this,tr("Save Image"), ".", tr("Image Files (*.png *.jpg *bmp)"));
        if (ui->colorButton->isChecked()){
            cvtColor(destColorImage, auxImage, COLOR_RGB2BGR);
            imwrite(fileName.toStdString(),auxImage);
        }
        else{
            cvtColor(destGrayImage, auxImage, COLOR_GRAY2BGR);
            imwrite(fileName.toStdString(),auxImage);
        }

        timer.start(30);
}

void MainWindow::procesar(){
    Mat aux;
    Mat yuv[3];

    cvtColor(colorImage, aux, COLOR_RGB2YUV);
    split(aux,yuv);
    switch (ui->operationComboBox->currentIndex()) {
    //Transformaci??n de pixel
    case 0:
        transformarPixel(aux,yuv);
        break;
    //Umbralizar
    case 1:
        umbralizar(aux,yuv);
        break;
    //Ecualizar
    case 2:
        ecualizar(aux,yuv);
        break;
    //Filtro Gausiano
    case 3:
        filtroGausiano(aux,yuv);
        break;
    //Filtro medio borroso
    case 4:
        filtroMedio(aux,yuv);
        break;
    //Filtro lineal
    case 5:
        filtroLineal(aux,yuv);
        break;
    //Dilatar
    case 6:
        dilatar(aux,yuv);
        break;
    //Erosionar
    case 7:
        erosionar(aux,yuv);
        break;
    //Aplicar severo
    case 8:

        break;
    default:
        break;
    }

}



void MainWindow::transformarPixel(Mat aux, Mat yuv[3]){
    std::vector<int> s(4);
    std::vector<int> r(4);
    std::vector<uchar> lut(256);
    int cont = 0;
    for(int i = 0; i < 4; i++){
        for (int j =0; j < 2; j++){
            s[cont] = pixelTDialog.grayTransformW->item(i,1)->text().toInt();
            r[cont] = pixelTDialog.grayTransformW->item(i,0)->text().toInt();
        }
        cont++;
    }

    for(int x = 0; x < 3; x++){
        for(int y = r[x];y <= r[x+1];y++){
            lut[y] = ((y-r[x])*(s[x+1]-s[x]))/(r[x+1]-r[x])+s[x];
        }
    }

    if(ui->colorButton->isChecked()){

    }else{
        cv::LUT(grayImage, lut, destGrayImage);

    }


}

void MainWindow::ecualizar(Mat aux, Mat yuv[3]){
    if(ui->colorButton->isChecked()){
        cv::equalizeHist(yuv[0],yuv[0]);
        merge(yuv,3,aux);
        cvtColor(aux, destColorImage, COLOR_YUV2RGB);
    }else{
        cv::equalizeHist(grayImage,destGrayImage);
    }

}

void MainWindow::umbralizar(Mat aux, Mat yuv[3]){
    double umbral = ui->thresholdSpinBox->value();
    if(ui->colorButton->isChecked()){
        cv::threshold(yuv[0],yuv[0],umbral,255,THRESH_BINARY);
        merge(yuv,3,aux);
        cvtColor(aux, destColorImage, COLOR_YUV2RGB);
    }else{
        cv::threshold(grayImage,destGrayImage,umbral,255,THRESH_BINARY);
    }
}

void MainWindow::filtroGausiano(Mat aux, Mat yuv[3]){
    Size ventana;
    ventana.width = ui->gaussWidthBox->value();
    ventana.height = ui->gaussWidthBox->value();
    if(ui->colorButton->isChecked()){
        cv::GaussianBlur(yuv[0],yuv[0],ventana,ventana.width/5.);
        merge(yuv,3,aux);
        cvtColor(aux, destColorImage, COLOR_YUV2RGB);
    }else{
        cv::GaussianBlur(grayImage,destGrayImage,ventana,ventana.width/5.);

    }
}

void MainWindow::filtroMedio(Mat aux, Mat yuv[3]){
    int ventana;
    ventana = ui->gaussWidthBox->value();
    if(ui->colorButton->isChecked()){
        cv::medianBlur(yuv[0],yuv[0],ventana);
        merge(yuv,3,aux);
        cvtColor(aux, destColorImage, COLOR_YUV2RGB);
    }else{
        cv::medianBlur(grayImage,destGrayImage,ventana);
    }
}

void MainWindow::filtroLineal(Mat aux, Mat yuv[3]){
    Matx<float,3,3> kernel;
    for(int i = 0; i < 3;i++){
        for(int j = 0;j < 3; j++){
            kernel(i,j) = lFilterDialog.kernelWidget->item(i,j)->text().toFloat();
        }
    }
    if(ui->colorButton->isChecked()){
        cv::filter2D(yuv[0],yuv[0],-1,kernel,Point(-1,-1),lFilterDialog.addedVBox->value());
        merge(yuv,3,aux);
        cvtColor(aux, destColorImage, COLOR_YUV2RGB);
    }else{
        cv::filter2D(grayImage,destGrayImage,-1,kernel,Point(-1,-1),lFilterDialog.addedVBox->value());
    }
}

void MainWindow::dilatar(Mat aux, Mat yuv[3]){
    Mat imgAux;
    Mat kernel;
    double umbral = ui->thresholdSpinBox->value();
    if(ui->colorButton->isChecked()){
        cv::threshold(yuv[0],yuv[0],umbral,255,THRESH_BINARY);
        cv::dilate(yuv[0],yuv[0],kernel);
        merge(yuv,3,aux);
        cvtColor(aux, destColorImage, COLOR_YUV2RGB);
    }else{
        cv::threshold(grayImage,imgAux,umbral,255,THRESH_BINARY);
        cv::dilate(imgAux,destGrayImage,kernel);
    }
}

void MainWindow::erosionar(Mat aux, Mat yuv[3]){
    Mat imgAux;
    Mat kernel;
    double umbral = ui->thresholdSpinBox->value();
    if(ui->colorButton->isChecked()){
        cv::threshold(yuv[0],yuv[0],umbral,255,THRESH_BINARY);
        cv::erode(yuv[0],yuv[0],kernel);
        merge(yuv,3,aux);
        cvtColor(aux, destColorImage, COLOR_YUV2RGB);
    }else{
        cv::threshold(grayImage,imgAux,umbral,255,THRESH_BINARY);
        cv::erode(imgAux,destGrayImage,kernel);
    }
}

