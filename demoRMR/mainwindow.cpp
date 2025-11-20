#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <math.h>
#include <QDebug>
#include <QIcon>
#include <QResizeEvent>
#include <QSize>
#include <QPainterPath>
#include "skeleton.h"
///TOTO JE DEMO PROGRAM...AK SI HO NASIEL NA PC V LABAKU NEPREPISUJ NIC,ALE SKOPIRUJ SI MA NIEKAM DO INEHO FOLDERA
/// AK HO MAS Z GITU A ROBIS NA LABAKOVOM PC, TAK SI HO VLOZ DO FOLDERA KTORY JE JASNE ODLISITELNY OD TVOJICH KOLEGOV
/// NASLEDNE V POLOZKE Projects SKONTROLUJ CI JE VYPNUTY shadow build...
/// POTOM MIESTO TYCHTO PAR RIADKOV NAPIS SVOJE MENO ALEBO NEJAKY INY LUKRATIVNY IDENTIFIKATOR
/// KED SA NAJBLIZSIE PUSTIS DO PRACE, SKONTROLUJ CI JE MIESTO TOHTO TEXTU TVOJ IDENTIFIKATOR
/// AZ POTOM ZACNI ROBIT... AK TO NESPRAVIS, POJDU BODY DOLE... A NIE JEDEN,ALEBO DVA ALE BUDES RAD
/// AK SA DOSTANES NA SKUSKU


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    //tu je napevno nastavena ip. treba zmenit na to co ste si zadali do text boxu alebo nejaku inu pevnu. co bude spravna
    ipaddress="127.0.0.1";//"127.0.0.1";//192.168.1.14 toto je na niektory realny robot.. na lokal budete davat "127.0.0.1"

    ui->setupUi(this);
    datacounter=0;
    ui->pushButton_8->setVisible(false);
    ui->pushButton_8->setText("RESET BRZDY"); // Nastaví text na tlačidle

#ifndef DISABLE_OPENCV
    actIndex=-1;
    useCamera1=false;

#endif


    datacounter=0;

    ui->NotAusLabel->setVisible(false);
    /*ui->NotAusLabel->setStyleSheet(
        "QLabel { "
        "background-color: red; "
        "color: white; "
        "font-size: 24px; "
        "font-weight: bold; "
        "border: 2px solid black; "
        "padding: 10px; }"
        );*/

    // --- NOVÝ KÓD PRE REŽIM SVETLO/TMA (Nastavenie počiatočného štýlu) ---
    isDarkMode = false; // Začneme v svetlom režime
    applyStyleSheet();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::applyStyleSheet()
{
    QString style;
    if (isDarkMode) {
        // Dark Mode
        style = "QMainWindow { background-color: #2e3436; }"
                "QPushButton { background-color: #555753; border: 1px solid #888a85; border-radius: 6px; padding: 6px; color: #eeeeec; }"
                "QPushButton:hover { background-color: #666a65; }"
                "QLineEdit { background-color: #1c1c1c; color: #eeeeec; border: 1px solid #888a85; border-radius: 3px; }"
                "QLabel { color: #eeeeec; }";

        //ui->pushButton->setText("Svetlý Režim");
        ui->pushButton->setIcon(QIcon(":/tvary/9937122.png"));

    } else {
        // Light Mode
        style = "QMainWindow { background-color: #f7f7f7; }"
                "QPushButton { background-color: #dddddd; border: 1px solid #aaaaaa; border-radius: 6px; padding: 6px; color: #333333; }"
                "QPushButton:hover { background-color: #cccccc; }"
                "QLineEdit { background-color: white; color: #333333; border: 1px solid #aaaaaa; border-radius: 3px; }"
                "QLabel { color: #333333; }";

       // ui->pushButton->setText("Tmavý Režim");
        ui->pushButton->setIcon(QIcon(":/tvary/1664849-200.png"));
    }

    this->setStyleSheet(style);

    // Reaplikácia špecifických štýlov, aby mali vyššiu prioritu
    // NÚDZOVÉ ZASTAVENIE LABEL
    ui->NotAusLabel->setStyleSheet(
        "QLabel { "
        "background-color: red; "
        "color: white; "
        "font-size: 24px; "
        "font-weight: bold; "
        "border: 2px solid black; "
        "border-radius: 8px; "
        "padding: 10px; }"
        );

    // RESET BRZDY TLAČIDLO (pushButton_8)
    ui->pushButton_8->setStyleSheet(
        "QPushButton { "
        "background-color: #CC0000; " // Tmavá červená farba
        "color: white; "
        "font-size: 24px; " // Väčší font
        "font-weight: bold; "
        "border: 2px solid #990000; "
        "border-radius: 8px; "
        "padding: 10px 20px; }"
        "QPushButton:hover { "
        "background-color: #FF3333; }" // Svetlejšia červená pri prechode myšou
        );
}

void MainWindow::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    ///prekreslujem obrazovku len vtedy, ked viem ze mam nove data. paintevent sa
    /// moze pochopitelne zavolat aj z inych dovodov, napriklad zmena velkosti okna
    //painter.setBrush(Qt::white);//biela farba pozadia(pouziva sa ako fill pre napriklad funkciu drawRect)
    QPen pero;
    /*pero.setStyle(Qt::SolidLine);//styl pera - plna ciara
    pero.setWidth(3);//hrubka pera -3pixely
    pero.setColor(Qt::green);//farba je zelena*/
    QRect rect;
    rect= ui->centralWidget->geometry();//ziskate porametre stvorca,do ktoreho chcete kreslit
    rect.translate(0,15);
    //painter.drawRect(rect);*/




#ifndef DISABLE_OPENCV
    if(useCamera1==true && actIndex>-1)/// ak zobrazujem data z kamery a aspon niektory frame vo vectore je naplneny
    {
        std::cout<<actIndex<<std::endl;
        QImage image = QImage((uchar*)frame[actIndex].data, frame[actIndex].cols, frame[actIndex].rows, frame[actIndex].step, QImage::Format_RGB888  );//kopirovanie cvmat do qimage
        painter.drawImage(rect,image.rgbSwapped());

        QSize currentSize = this->size();
        double width_I = currentSize.width();
        double height_I = currentSize.height();
        //qDebug()<<"Vyska: "<< height_I<< ", Sirka: "<< width_I;
        QPen lidarDotPen;
        QBrush brush;   // farba výplne
        painter.setBrush(brush);  // aktivovanie výplne
        double f = 934.962;
        double Z = 210;
        double Z_D = -145;
        double Y_D = 115;
        double X_obr;
        double Y_obr;
        double uhol_POM;
        double sinus;
        double cosinus;
        int kanalAlfa;
        //qDebug()<<"uhlov pocet: "<< uhol_update.size();
        //double minimum = 960;
        //double f_POM = f/(1282/960);
        //qDebug()<<"f_POM: " <<f_POM;
        double prevodnik = width_I/960;//ej kej ej podvodnik :D
        //qDebug()<<"Prevodnik: " <<prevodnik;
        double f_new = f * prevodnik;
        //qDebug()<<"f_new: " <<f_new;
        for (int var = 0; var < uhol_update.size(); var++) {
            kanalAlfa = static_cast<int>((250 / vzdialenost_update[var])*255);
            uhol_POM = uhol_update[var] * (M_PI/180.0);
            //qDebug()<<"cislo: "<<var<< "vzdialenost" <<vzdialenost_update[var] << " uhol" << uhol_update[var];
            sinus = std::sin(uhol_POM);
            cosinus = std::cos(uhol_POM);

            X_obr = width_I / 2 - (f_new * (vzdialenost_update[var] * sinus))/( vzdialenost_update[var] * cosinus + Z_D);
            Y_obr = height_I / 2 + (f_new * (-Z+Y_D))/( vzdialenost_update[var] * cosinus + Z_D);

            if ((vzdialenost_update[var] > 185) && (vzdialenost_update[var] <= 250)){
                lidarDotPen.setStyle(Qt::SolidLine);
                lidarDotPen.setWidth(2); // Hrubšia bodka
                lidarDotPen.setColor(QColor(255,0,0,255)); // Červená farba
                painter.setPen(lidarDotPen);
                painter.setBrush(QBrush(QColor(255,0,0,255)));
                painter.drawRect(X_obr - 5,Y_obr - 5,10,10);

            }else if (vzdialenost_update[var] > 250){
                lidarDotPen.setStyle(Qt::SolidLine);
                lidarDotPen.setWidth(5); // Hrubšia bodka
                lidarDotPen.setColor(QColor(0,0,255,kanalAlfa)); // modra farba
                painter.setPen(lidarDotPen);
                painter.setBrush(QBrush(QColor(0,0,255,kanalAlfa)));
                painter.drawEllipse(QPoint(X_obr,Y_obr), 3,3);
            }else{
                lidarDotPen.setStyle(Qt::SolidLine);
                lidarDotPen.setWidth(5); // Hrubšia bodka
                lidarDotPen.setColor(QColor(0,0,0,kanalAlfa)); // modra farba
                painter.setPen(lidarDotPen);
                painter.setBrush(QBrush(QColor(0,0,0,kanalAlfa)));
                painter.drawEllipse(QPoint(X_obr,Y_obr), 3,3);
            }
        }

        if(!manual){
            // ---------- action intervention (vector ray) ----------
            // nastavenia - uprav si tieto konštanty podľa potreby
            const double SPEED_TO_PIXELS = 0.5;   // prevod jednotiek dopredu -> pixely (tweak)
            const double ANGLE_SCALE = -3.14159/8;  // ak vpravo je -1..0..1 a v kode násobíš pi/8
            const double MIN_LEN_PIX = 8.0;        // min dĺžka raye v pixeloch
            const double MAX_LEN_PIX = rect.height() * 0.9; // max dĺžka (aby nevytŕčal cez obraz)

            // počet bodov, ktoré nakreslíme
            const int NUM_POINTS = 8;

            // stred obrazu
            int cx = rect.x() + rect.width() / 2;
            int cy = rect.y() + rect.height() / 1.1;

            double linear;
            if(dopredu > 0)
                linear = static_cast<double>(dopredu);
            else
                linear = 0;
            double ang_cmd = static_cast<double>(vpravo);

            double len_pix = std::abs(linear) * SPEED_TO_PIXELS;
            if(len_pix < MIN_LEN_PIX) len_pix = MIN_LEN_PIX;
            if(len_pix > MAX_LEN_PIX) len_pix = MAX_LEN_PIX;

            double angle_rad = ang_cmd * ANGLE_SCALE;

            // nakreslíme body
            for(int i = 1; i <= NUM_POINTS; ++i) {
                double t = static_cast<double>(i) / NUM_POINTS; // 0..1
                double point_len = len_pix * t;
                double point_ang = angle_rad * t * 0.5; // jemné zakrivenie ako pri Bezier (polovičný uhol)

                int px = cx + static_cast<int>(point_len * std::sin(point_ang));
                int py = cy - static_cast<int>(point_len * std::cos(point_ang));

                // veľkosť bodu sa zmenšuje s rastúcim t
                int point_size = static_cast<int>(20 * (1.0 - 0.7*t)); // od 5 px do 1.5 px

                if(i<NUM_POINTS){
                    painter.setBrush(QColor(0, 0, 50, 100));
                    painter.setPen(Qt::NoPen);
                    painter.drawEllipse(QPoint(px, py), point_size, point_size);
                }else{
                    painter.setBrush(QColor(255, 255, 255, 255));
                    painter.setPen(Qt::NoPen);
                    painter.drawEllipse(QPoint(px, py), point_size, point_size);
                }
            }
        }
        // ---------- koniec action intervention ----------

    }
    else
#endif
    {
    }
#ifndef DISABLE_SKELETON
    if(updateSkeletonPicture==1 )
    {
        painter.setPen(Qt::red);
        for(int i=0;i<75;i++)
        {
            int xp=rect.width()-rect.width() * skeleJoints.joints[i].x+rect.topLeft().x();
            int yp= (rect.height() *skeleJoints.joints[i].y)+rect.topLeft().y();
            if(rect.contains(xp,yp))
                painter.drawEllipse(QPoint(xp, yp),2,2);
        }
    }
#endif
}

/// toto je slot. niekde v kode existuje signal, ktory je prepojeny. pouziva sa napriklad (v tomto pripade) ak chcete dostat data z jedneho vlakna (robot) do ineho (ui)
/// prepojenie signal slot je vo funkcii  on_pushButton_9_clicked

void MainWindow::on_pushButton_9_clicked() //start button
{
    QString zadany_text = ui->lineEdit->text();
    if (zadany_text.isEmpty()){
        this->ipaddress = "127.0.0.1";
        qDebug() << "IP adresa nastavená na" << QString::fromStdString(ipaddress);
    }
    else{
        this->ipaddress = zadany_text.toStdString();
        qDebug() << "IP adresa nastavená na" << QString::fromStdString(ipaddress);
    }

#ifndef DISABLE_OPENCV //toto som pridal
    useCamera1 = true; // automaticky kamera po štarte
#endif                       //po tadeto som pridaval

    //ziskanie joystickov
    //tu sa nastartuju vlakna ktore citaju data z lidaru a robota

    connect(&_robot,SIGNAL(publishPosition(double,double,double)),this,SLOT(setUiValues(double,double,double)));
    connect(&_robot,SIGNAL(publishLidar(const LaserMeasurement &)),this,SLOT(paintThisLidar(const LaserMeasurement &)));
    connect(&_robot,SIGNAL(publishCV(int, int, int, int,int, int, int, int)),this,SLOT(updateRevSensorDisplay(int,int,int,int,int,int,int,int)));
    connect(&_robot,SIGNAL(publishSystemBreakStatus(bool)),this,SLOT(updateSystemBrake(bool)));

    connect(&_robot, SIGNAL(publishFrontLidarPoints(const std::vector<double> &, const std::vector<double> &)),
            this, SLOT(receiveFrontLidarPoints(const std::vector<double> &, const std::vector<double> &)));
#ifndef DISABLE_OPENCV
    connect(&_robot,SIGNAL(publishCamera(const cv::Mat &)),this,SLOT(paintThisCamera(const cv::Mat &)));
#endif
#ifndef DISABLE_SKELETON
    connect(&_robot,SIGNAL(publishSkeleton(const skeleton &)),this,SLOT(paintThisSkeleton(const skeleton &)));
#endif

    //_robot.initAndStartRobot(ipaddress);
    qDebug() << "Pripájam sa na IP:" << QString::fromStdString(ipaddress);
    _robot.initAndStartRobot(ipaddress);

#ifndef DISABLE_JOYSTICK
    instance = QJoysticks::getInstance();
    /// prepojenie joysticku s jeho callbackom... zas cez lambdu. neviem ci som to niekde spominal,ale lambdy su super. okrem toho mam este rad ternarne operatory a spolocneske hry ale to tiez nikoho nezaujima
    /// co vas vlastne zaujima? citanie komentov asi nie, inak by ste citali toto a ze tu je blbosti
    connect(
        instance, &QJoysticks::axisChanged,
        [this]( const int js, const int axis, const qreal value) {
            double forw=0, rot=0;
            if(/*js==0 &&*/ axis==1){forw=-value*300;}
            if(/*js==0 &&*/ axis==0){rot=-value*(3.14159/2.0);}
            this->_robot.setSpeedVal(forw,rot);
        }
        );
#endif
    //dopredu = 0;
    //vpravo = 0;

    ui->lineEdit->setVisible(false);
}

void MainWindow::on_lineEdit_returnPressed()
{
    // Vykoná rovnakú akciu ako kliknutie na tlačidlo Štart
    on_pushButton_9_clicked();
}


int pohyb = 0;


// NOVÝ SLOT pre príjem stavu systémovej brzdy
void MainWindow::updateSystemBrake(bool activated)
{
    if (systemova_brzda_zablokovana && activated) {
        qDebug() << "Systémová brzda detekovaná, ale ignorovaná";
        return;
    }

    systemova_brzda = activated; // Nastav lokálnu premennú v MainWindow
    qDebug() << "brzda je: " << systemova_brzda;

    if (systemova_brzda) {
        // Ak je aktivovaná, zastav robota a ukáž upozornenie v UI
        dopredu = 0;
        vpravo = 0;
        _robot.setSpeedVal(dopredu, vpravo*3.14159/8); // Zastavenie robota
        qDebug() << "Systémová brzda AKTIVOVANÁ (prekážka) a robot ZASTAVENÝ!";
        ui->pushButton_8->setVisible(true);
        // Pridaj kód na zobrazenie upozornenia (ak treba)
    } else {
        qDebug() << "Systémová brzda DEAKTIVOVANÁ (cesta voľná).";
    }

    // Pre kontrolu pohybu - aby sa nemohol pohnúť, ak je brzda aktívna
    // (vidím to už implementované v on_pushButton_2_pressed)
}

void MainWindow::on_pushButton_8_clicked(){
    ui->pushButton_8->setVisible(false);

    systemova_brzda = false;
    //systemova_brzda_zablokovana = true;

    _robot.setSpeedVal(dopredu, vpravo*3.14159/8);
    emit signalStartRobotOverride(10000);
    qDebug() << "Manuálny reset System Break AKTIVOVANÝ na 10 sekúnd.";

}


void MainWindow::on_pushButton_2_clicked() // forward držanie
{
    if (!notaus && !systemova_brzda && manual) {
        int dopredu_pom = dopredu;
        dopredu += 100;
        if(dopredu_pom < 0 && dopredu >= 0)
            vpravo *= -1;
        _robot.setSpeedVal(dopredu, vpravo*3.14159/8); // rozbehni dopredu
    }
}

/*void MainWindow::on_pushButton_2_released() // forward pusti
{
    if (!notaus && !systemova_brzda) {
        _robot.setSpeedVal(0, 0); // zastav po pustení
    }
}*/



void MainWindow::on_pushButton_3_clicked() //back
{
    if (notaus == false && !systemova_brzda && manual){
        int dopredu_pom = dopredu;
        dopredu -=100;
        if(dopredu_pom >= 0 && dopredu < 0)
            vpravo *= -1;
        _robot.setSpeedVal(dopredu,vpravo*3.14159/8);
    }

}

void MainWindow::on_pushButton_6_clicked() //right
{
    if (notaus == false && !systemova_brzda && manual){
        /*if(vpravo + 3.14159/4 != 0){
            vpravo = 3.14159/4;
            _robot.setSpeed(dopredu,vpravo);
        }
        else{
            vpravo = 0;
            //dopredu = dopredu;
            std::cout <<"vpravo = "<< vpravo<< endl;
            std::cout <<"dopredu = "<< dopredu<< endl;
            _robot.setSpeed(dopredu,vpravo);
        }*/
        if(dopredu >= 0){
            if(vpravo == 0){
                vpravo = 1;
            }else if (vpravo == -1){
                vpravo = 0;
            }
        }else{
            if(vpravo == 0){
                vpravo = -1;
            }else if (vpravo == 1){
                vpravo = 0;
            }
        }

        //std::cout << "!!!!!!!!!!!!!!!!!!!!!!vpravo: " << vpravo << endl;
        _robot.setSpeedVal(dopredu, vpravo*3.14159/8);
    }
}

void MainWindow::on_pushButton_5_clicked()//left
{
    if (notaus == false && !systemova_brzda && manual){
        /*vpravo -= 3.14159/8;
        if(vpravo < -3.14159/8)
            vpravo = -3.14159/8;

        _robot.setSpeedVal(dopredu, vpravo);*/
        if(dopredu >= 0){
            if(vpravo == 0){
                vpravo = -1;
            }else if (vpravo == 1){
                vpravo = 0;
            }
        }else{
            if(vpravo == 0){
                vpravo = 1;
            }else if (vpravo == -1){
                vpravo = 0;
            }
        }
        //std::cout << "!!!!!!!!!!!!!!!!!!!!!!vpravo: " << vpravo << endl;
        _robot.setSpeedVal(dopredu, vpravo*3.14159/8);

    }
}

void MainWindow::on_pushButton_4_clicked() //stop
{
    if (notaus == false && manual){
        this->setVisible(true);
        dopredu = 0;
        vpravo = 0;
        _robot.setSpeedVal(dopredu, vpravo);
    }else if (!notaus && !manual)
        this->setVisible(false);
}

void MainWindow::on_pushButton_15_clicked() //stop
{
    // núdzové zastavenie
    notaus = !notaus;
    if (notaus) {
        dopredu = 0;
        vpravo = 0;
        _robot.setSpeedVal(dopredu, vpravo*3.14159/4);
        qDebug() << "NÚDZOVÉ ZASTAVENIE AKTIVOVANÉ!";
        ui->NotAusLabel->setVisible(true);
    } else {
        qDebug() << "NÚDZOVÉ ZASTAVENIE VYPNUTÉ!";
        ui->NotAusLabel->setVisible(false);
    }

}

/*void MainWindow::keyPressEvent(QKeyEvent *event) //netreba
{
    if (!event->isAutoRepeat()) {

        // núdzové zastavenie
        if (event->key() == Qt::Key_X) {
            notaus = !notaus;

            if (notaus) {
                dopredu = 0;
                vpravo = 0;
                _robot.setSpeedVal(dopredu, vpravo*3.14159/8);
                qDebug() << "NÚDZOVÉ ZASTAVENIE AKTIVOVANÉ!";
                ui->NotAusLabel->setVisible(true);
            } else {
                qDebug() << "NÚDZOVÉ ZASTAVENIE VYPNUTÉ!";
                ui->NotAusLabel->setVisible(false);
            }
        }

        // obyčajné zastavenie
        else if (event->key() == Qt::Key_0) {
            vpravo = 0;
            dopredu = 0;
            _robot.setSpeedVal(dopredu, vpravo*3.14159/4);
            qDebug() << "ZASTAVENIE!";
        }

        // pohyb šípkami
        else if (event->key() == Qt::Key_W) {    // dopredu
            if (!notaus) _robot.setSpeed(100, 0);
        }
        else if (event->key() == Qt::Key_S) { // dozadu
            if (!notaus) _robot.setSpeed(100, 0);
        }
        else if (event->key() == Qt::Key_A) { // doľava
            if (!notaus) _robot.setSpeed(0, 3.14159/4);
        }
        else if (event->key() == Qt::Key_D) { // doprava
            if (!notaus) _robot.setSpeed(0, -3.14159/4);
        }

        else {
            QMainWindow::keyPressEvent(event);
        }
    }
}*/



void MainWindow::on_pushButton_clicked()
{
    // Tlačidlo sa teraz používa na prepínanie tmavého/svetlého režimu
    isDarkMode = !isDarkMode;
    applyStyleSheet();





    /*#ifndef DISABLE_OPENCV
    if(useCamera1==true)
    {
        useCamera1=false;

        ui->pushButton->setIcon(QIcon(":/tvary/camera.png"));
    }
    else
    {
        useCamera1=true;

        ui->pushButton->setIcon(QIcon(":/tvary/laser.png"));
    }
#endif*/
}

void MainWindow::on_pushButton_10_clicked()
{
    manual = !manual;
    ui->pushButton_10->setText("");

    if (manual)
    {
        ui->pushButton_10->setIcon(QIcon(":/tvary/automatika.png")); // Manuálny režim je zapnutý (ikona prepne na automatika, ktorú chceme vypnúť)
        //qDebug()<<"manuaaaaaaaaaaaaaaaaaaaaaaal";
        // Zastav robota v momente prepnutia do manuálneho režimu (pre istotu)
        dopredu = 0;
        vpravo = 0;
        _robot.setSpeedVal(dopredu, vpravo);
    }
    else
    {
        ui->pushButton_10->setIcon(QIcon(":/tvary/manualis.png")); // Automatický režim je zapnutý (ikona prepne na manualis, ktorú chceme vypnúť)
        //qDebug()<<"automatikaaaaaaaaaaaaaaaaaa";
        // Zastav robota v momente prepnutia do automatického režimu
        dopredu = 0;
        vpravo = 0;
        _robot.setSpeedVal(dopredu, vpravo);
    }

    // --- TENTO RIADOK JE KĽÚČOVÝ ---
    updateManualControlVisibility();
}
int MainWindow::paintThisLidar(const LaserMeasurement &laserData)
{
    memcpy( &copyOfLaserData,&laserData,sizeof(LaserMeasurement));
    updateLaserPicture=1;

//update();
//ui->lidarWidget->updateLidarData(laserData);
//return 0;
#ifndef DISABLE_OPENCV
    // Ak kamera ešte nebola pripravená, vykresľujeme LiDAR
    if(cameraReady)
        update();
#else
    update();
#endif

    return 0;
}

#ifndef DISABLE_OPENCV

///toto je calback na data z kamery, ktory ste podhodili robotu vo funkcii initAndStartRobot
/// vola sa ked dojdu nove data z kamery
int MainWindow::paintThisCamera(const cv::Mat &cameraData)
{
    cameraData.copyTo(frame[(actIndex+1)%3]);//kopirujem do nasej strukury
    actIndex=(actIndex+1)%3;//aktualizujem kde je nova fotka
    //double height = ;
    updateLaserPicture=1;

    cameraReady = true;//nove
    update();//nove

    return 0;
}
#endif
double uhol(double Ax, double Ay, double Bx, double By, double Cx, double Cy){
    double Ux = Bx - Ax;
    double Uy = By - Ay;
    double Vx = Cx - Ax;
    double Vy = Cy - Ay;

    //skalar
    double skalar = Ux*Vx + Uy*Vy;
    //cross 2D
    double cross = Ux*Vy - Uy*Vx;

    //dlzky vektorov
    double dlzka_U = std::sqrt(Ux*Ux + Uy*Uy);
    double dlzka_V = std::sqrt(Vx*Vx + Vy*Vy);

    if(dlzka_U == 0 || dlzka_V == 0)
        return 0.0;

    double cos_Alfa = skalar / (dlzka_U * dlzka_V);

    if(cos_Alfa > 1.0) cos_Alfa = 1.0;
    if(cos_Alfa < -1.0) cos_Alfa = -1.0;

    double alfa = std::acos(cos_Alfa);

    //je v pravo alebo vlavo
    if(cross < 0)
        alfa = -alfa;

    //konverzia
    return alfa * 180 / M_PI;
}


#ifndef DISABLE_SKELETON
int MainWindow::paintThisSkeleton(const skeleton &skeledata)
{
    //dopln notaus
    //if(notaus)
        //qDebug()<<"stoooooooooooooooooooooooooj";
        //_robot.setSpeedVal(0, 0);
    if(!notaus && !systemova_brzda && !manual){
        //qDebug()<<"setujem aj tak ty trapaaaak";
        memcpy(&skeleJoints,&skeledata,sizeof(skeleton));

        int wtf_L=0;
        int wtf_R=0;

        int dop_L = 0;
        int dop_R = 0;

        double target_speed=0;

        double Ax_LW = skeleJoints.joints[left_wrist].x;
        double Ay_LW = skeleJoints.joints[left_wrist].y;

        double Bx_LM = skeleJoints.joints[left_ring_cmc].x;
        double By_LM = skeleJoints.joints[left_ring_cmc].y;

        double Cx_LPom = skeleJoints.joints[left_wrist].x;
        double Cy_LPom = 0;

        double poloha_lavej = uhol(Ax_LW,Ay_LW,Bx_LM,By_LM,Cx_LPom,Cy_LPom);
        //qDebug()<<"poloha lavej" << poloha_lavej;

        double Ax_RW = skeleJoints.joints[right_wrist].x;
        double Ay_RW = skeleJoints.joints[right_wrist].y;

        double Bx_RM = skeleJoints.joints[right_ring_cmc].x;
        double By_RM = skeleJoints.joints[right_ring_cmc].y;

        double Cx_RPom = skeleJoints.joints[right_wrist].x;
        double Cy_RPom = 0;

        double poloha_pravej = uhol(Ax_RW,Ay_RW,Bx_RM,By_RM,Cx_RPom,Cy_RPom);


        double Index_Man_L = skeleJoints.joints[left_ringy_tip].x;
        double Index_Man_R = skeleJoints.joints[right_ringy_tip].x;;
        //if (Index_Man_L < Index_Man_R)
        //    manual = true;

        if(poloha_lavej<25 && poloha_lavej>-25 && !manual){
            double Ax_L = skeleJoints.joints[left_thumb_mcp].x;
            double Ay_L = skeleJoints.joints[left_thumb_mcp].y;

            double Bx_L = skeleJoints.joints[left_thumb_tip].x;
            double By_L = skeleJoints.joints[left_thumb_tip].y;

            double Cx_L = skeleJoints.joints[left_index_cmc].x;
            double Cy_L = skeleJoints.joints[left_index_cmc].y;

            double palec_L = uhol(Ax_L,Ay_L,Bx_L,By_L,Cx_L,Cy_L);

            //std::cout << "lavý palec: " << palec << endl;
            //qDebug()<<"palec lavyyy:" << palec_L;
            if (135.0 >= palec_L && palec_L > 50.0){
                wtf_L = -1;
            }
            else
                wtf_L = 0;


            //lava ruka
            double Iy_m_L = skeleJoints.joints[left_index_mcp].y;
            double Iy_t_L = skeleJoints.joints[left_index_tip].y;

            double My_m_L = skeleJoints.joints[left_middle_mcp].y;
            double My_t_L = skeleJoints.joints[left_middle_tip].y;

            double Ry_m_L = skeleJoints.joints[left_ring_mcp].y;
            double Ry_t_L = skeleJoints.joints[left_ringy_tip].y;

            double Py_m_L = skeleJoints.joints[left_pink_mcp].y;
            double Py_t_L = skeleJoints.joints[left_pink_tip].y;


            if(Iy_m_L-Iy_t_L > 0){
                if(My_m_L-My_t_L > 0){
                    if(Ry_m_L-Ry_t_L > 0){
                        if(Py_m_L-Py_t_L > 0){
                            dop_L = -4;
                        }
                        else dop_L = -3;
                    }
                    else dop_L = -2;
                }
                else dop_L = -1;
            }
            else dop_L = 0;

        }
    /**************************************************************************************/

        if(poloha_pravej<25 && poloha_pravej>-25 && !manual){
            double Ax_R = skeleJoints.joints[right_thumb_cmc].x;
            double Ay_R = skeleJoints.joints[right_thumb_cmc].y;

            double Bx_R = skeleJoints.joints[right_thumb_tip].x;
            double By_R = skeleJoints.joints[right_thumb_tip].y;

            double Cx_R = skeleJoints.joints[right_index_cmc].x;
            double Cy_R = skeleJoints.joints[right_index_cmc].y;

            double palec_R = uhol(Ax_R,Ay_R,Bx_R,By_R,Cx_R,Cy_R);

            if (-135.0 <= palec_R && palec_R < -30.0){
                wtf_R = 1;
            }
            else
                wtf_R = 0;

            //prava ruka
            double Iy_m_R = skeleJoints.joints[right_index_mcp].y;
            double Iy_t_R = skeleJoints.joints[right_index_tip].y;

            double My_m_R = skeleJoints.joints[right_middle_mcp].y;
            double My_t_R = skeleJoints.joints[right_middle_tip].y;

            double Ry_m_R = skeleJoints.joints[right_ring_mcp].y;
            double Ry_t_R = skeleJoints.joints[right_ringy_tip].y;

            double Py_m_R = skeleJoints.joints[right_pink_mcp].y;
            double Py_t_R = skeleJoints.joints[right_pink_tip].y;



            if(Iy_m_R-Iy_t_R > 0){
                if(My_m_R-My_t_R > 0){
                    if(Ry_m_R-Ry_t_R > 0){
                        if(Py_m_R-Py_t_R > 0){
                            dop_R = 4;
                        }
                        else dop_R = 3;
                    }
                    else dop_R = 2;
                }
                else dop_R = 1;
            }
            else dop_R = 0;
            //qDebug()<<"dop_r: " << dop_R;
        }

        vpravo = wtf_L+wtf_R;

        if(dop_L < 0 && dop_R > 0)
            target_speed = 0;
        else if(dop_L < 0 && dop_R == 0)
            target_speed = dop_L * 125;
        else if(dop_R > 0 && dop_L == 0)
            target_speed = dop_R * 125;
        else target_speed = 0;

        //qDebug()<<"dopredu: " << dopredu;

        if(target_speed > dopredu && target_speed !=0){
            dopredu += SPEED_STEP;
            if (dopredu > target_speed){
                dopredu = target_speed;
            }
        }else if (target_speed < dopredu && target_speed !=0){
            dopredu -= SPEED_STEP;
            if (dopredu < target_speed){
                dopredu = target_speed;
            }
        }else if(target_speed == 0){
            dopredu = 0;
        }

        if(!notaus || !systemova_brzda)
            _robot.setSpeedVal(dopredu, vpravo*3.14159/4);
        else{
            dopredu = 0;
            vpravo = 0;
            _robot.setSpeedVal(dopredu, vpravo);
        }

        updateSkeletonPicture=1;

    }
    else if(notaus){
        dopredu = 0;
        vpravo = 0;
        _robot.setSpeedVal(dopredu, vpravo);

    }
    else if(systemova_brzda){
        dopredu = 0;
        vpravo=0;
        _robot.setSpeedVal(dopredu, vpravo);

    }
    /*else{
        dopredu = 0;
        vpravo=0;
        _robot.setSpeedVal(dopredu, vpravo);
        return 0;
    }*/
    return 0;
}
#endif

void MainWindow::updateRevSensorDisplay(int front, int back, int left, int right, int front_r, int back_r, int back_l, int front_l){
    Front = front;
    Back = back;
    Left = left;
    Right = right;
    Front_r = front_r;
    Back_r = back_r;
    Back_l = back_l;
    Front_l = front_l;
    ui->revSenWidget->updateSensorData(Front, Right, Front_r, Back_r, Back, Back_l, Left, Front_l);
}

void MainWindow::setUiValues(double robotX,double robotY,double robotFi)
{
    ui->lidarWidget->updateLidarData(copyOfLaserData);
}

void MainWindow::resizeEvent(QResizeEvent *event){
    // *** ZMENA TU: Pôvodne bolo 4.0 / 3.0, teraz je 16.0 / 9.0 ***
    const double ASPECT_RATIO = 16.0 / 9.0;

    int newWidth = event->size().width();
    int newHeight = event->size().height();


    if((double)newWidth / newHeight > ASPECT_RATIO){
        newWidth = qRound(newHeight * ASPECT_RATIO);
    }else{
        newHeight = qRound(newWidth / ASPECT_RATIO);
    }

    if(newWidth != event->size().width() || newHeight != event->size().height()){
        this->resize(newWidth, newHeight);
    }

    QMainWindow::resizeEvent(event);
}

void MainWindow::receiveFrontLidarPoints(const std::vector<double> &uhol, const std::vector<double> &vzdialenost){
    uhol_update.clear();
    vzdialenost_update.clear();

    this->uhol_update = uhol;
    this->vzdialenost_update = vzdialenost;
}

void MainWindow::updateManualControlVisibility()
{

    bool visible = manual;

    ui->pushButton_2->setVisible(visible); // dopredu
    ui->pushButton_3->setVisible(visible); // dozadu
    ui->pushButton_4->setVisible(visible); // stop
    ui->pushButton_6->setVisible(visible); // vpravo
    ui->pushButton_5->setVisible(visible); // vľavo
}
