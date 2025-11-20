#include "robot.h"
#include <QDebug> //pre vstupy
            robot::robot(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<LaserMeasurement>("LaserMeasurement");
#ifndef DISABLE_OPENCV
    qRegisterMetaType<cv::Mat>("cv::Mat");
#endif
#ifndef DISABLE_SKELETON
    qRegisterMetaType<skeleton>("skeleton");
#endif
    qRegisterMetaType<std::vector<double>>("std::vector<double>");
    // *** NOVÉ: Inicializácia časovača pre override ***
    overrideTimer = new QTimer(this);
    connect(overrideTimer, &QTimer::timeout, this, &robot::overrideTimeout);
}

void robot::initAndStartRobot(std::string ipaddress)
{

    forwardspeed=0;
    rotationspeed=0;
    ///setovanie veci na komunikaciu s robotom/lidarom/kamerou.. su tam adresa porty a callback.. laser ma ze sa da dat callback aj ako lambda.
    /// lambdy su super, setria miesto a ak su rozumnej dlzky,tak aj prehladnost... ak ste o nich nic nepoculi poradte sa s vasim doktorom alebo lekarnikom...
    robotCom.setLaserParameters(ipaddress,52999,5299,/*[](LaserMeasurement dat)->int{std::cout<<"som z lambdy callback"<<std::endl;return 0;}*/std::bind(&robot::processThisLidar,this,std::placeholders::_1));
    robotCom.setRobotParameters(ipaddress,53000,5300,std::bind(&robot::processThisRobot,this,std::placeholders::_1));
#ifndef DISABLE_OPENCV
    robotCom.setCameraParameters("http://"+ipaddress+":8000/stream.mjpg",std::bind(&robot::processThisCamera,this,std::placeholders::_1));
#endif
#ifndef DISABLE_SKELETON
    robotCom.setSkeletonParameters("127.0.0.1",23432,23432,std::bind(&robot::processThisSkeleton,this,std::placeholders::_1));
#endif
    ///ked je vsetko nasetovane tak to tento prikaz spusti (ak nieco nieje setnute,tak to normalne nenastavi.cize ak napr nechcete kameru,vklude vsetky info o nej vymazte)
    robotCom.robotStart();


}

void robot::setSpeedVal(double forw, double rots)
{
    forwardspeed=forw;
    rotationspeed=rots;
    useDirectCommands=0;
}

void robot::setSpeed(double forw, double rots)
{
    if(forw==0 && rots!=0)
        robotCom.setRotationSpeed(rots);
    else if(forw!=0 && rots==0)
        robotCom.setTranslationSpeed(forw);
    else if((forw!=0 && rots!=0))
        robotCom.setArcSpeed(forw,forw/rots);
    else
        robotCom.setTranslationSpeed(0);
    useDirectCommands=1;
}
// *** NOVÝ SLOT: Spustí 10-sekundové blokovanie v robote ***
void robot::startSystemBreakOverride(int durationMs)
{
    systemova_brzda = false;
    emit publishSystemBreakStatus(false); // Pošli signál UI, že brzda je dole
    // 2. Aktivuj blokovanie a začni časovač
    manualOverrideActive = true;
    qDebug() << "Robot: System Break Override AKTIVOVANÝ na" << durationMs/1000 << "sekúnd. LiDAR bude ignorovaný.";
    overrideTimer->start(durationMs);
}

void robot::overrideTimeout()
{
    manualOverrideActive = false;
     overrideTimer->stop();
     qDebug() << "Robot: System Break Override DEAKTIVOVANÝ. Kontrola Lidaru obnovená.";
    // Po skončení override sa v ďalšom cykle processThisLidar, ak prekážka trvá, brzda opäť aktivuje.
}

///toto je calback na data z robota, ktory ste podhodili robotu vo funkcii initAndStartRobot
/// vola sa vzdy ked dojdu nove data z robota. nemusite nic riesit, proste sa to stane
int robot::processThisRobot(TKobukiData robotdata)
{


    ///tu mozete robit s datami z robota




    ///TU PISTE KOD... TOTO JE TO MIESTO KED NEVIETE KDE ZACAT,TAK JE TO NAOZAJ TU. AK AJ TAK NEVIETE, SPYTAJTE SA CVICIACEHO MA TU NATO STRING KTORY DA DO HLADANIA XXX

    ///kazdy piaty krat, aby to ui moc nepreblikavalo..
    if(datacounter%5==0)
    {

        ///ak nastavite hodnoty priamo do prvkov okna,ako je to na tychto zakomentovanych riadkoch tak sa moze stat ze vam program padne
        // ui->lineEdit_2->setText(QString::number(robotdata.EncoderRight));
        //ui->lineEdit_3->setText(QString::number(robotdata.EncoderLeft));
        //ui->lineEdit_4->setText(QString::number(robotdata.GyroAngle));
        /// lepsi pristup je nastavit len nejaku premennu, a poslat signal oknu na prekreslenie
        /// okno pocuva vo svojom slote a vasu premennu nastavi tak ako chcete. prikaz emit to presne takto spravi
        /// viac o signal slotoch tu: https://doc.qt.io/qt-5/signalsandslots.html
        ///posielame sem nezmysli.. pohrajte sa nech sem idu zmysluplne veci
        emit publishPosition(x,y,fi);
        ///toto neodporucam na nejake komplikovane struktury.signal slot robi kopiu dat. radsej vtedy posielajte
        /// prazdny signal a slot bude vykreslovat strukturu (vtedy ju musite mat samozrejme ako member premmennu v mainwindow.ak u niekoho najdem globalnu premennu,tak bude cistit bludisko zubnou kefkou.. kefku dodam)
        /// vtedy ale odporucam pouzit mutex, aby sa vam nestalo ze budete pocas vypisovania prepisovat niekde inde

    }
    ///---tu sa posielaju rychlosti do robota... vklude zakomentujte ak si chcete spravit svoje
    if(useDirectCommands==0)
    {
        if(forwardspeed==0 && rotationspeed!=0)
            robotCom.setRotationSpeed(rotationspeed);
        else if(forwardspeed!=0 && rotationspeed==0)
            robotCom.setTranslationSpeed(forwardspeed);
        else if((forwardspeed!=0 && rotationspeed!=0))
            robotCom.setArcSpeed(forwardspeed,forwardspeed/rotationspeed);
        else
            robotCom.setTranslationSpeed(0);
    }
    datacounter++;

    return 0;

}
int CV_F;
int CV_B;
int CV_R;
int CV_L;
int CV_FR;
int CV_BR;
int CV_BL;
int CV_FL;
///toto je calback na data z lidaru, ktory ste podhodili robotu vo funkcii initAndStartRobot
/// vola sa ked dojdu nove data z lidaru

int robot::asistentAsistenta(double vzdialenost_asist){
    int CV;
    if(vzdialenost_asist < 450 && vzdialenost_asist > 200){
        CV = 1;
        if(vzdialenost_asist < 400){
            CV = 2;
            if(vzdialenost_asist < 350){
                CV = 3;
                if(vzdialenost_asist < 300)
                    CV = 4;
            }
        }
    }else{
        CV = 0;}
    return  CV;
}

void robot::reverzAsistent(double uhol, double vzdialenost){
    if((uhol <= -157.5 && uhol >= -180) || (uhol >= 157.5 && uhol <= 180)){
        int CV_F_pom = asistentAsistenta(vzdialenost);
        if(CV_F < CV_F_pom)
            CV_F = CV_F_pom;
    }else if(uhol > -157.5 && uhol <= -112.5){
        int CV_FL_pom = asistentAsistenta(vzdialenost);
        if(CV_FL < CV_FL_pom)
        CV_FL = CV_FL_pom;
    }else if(uhol <= 157.5 && uhol >= 112.5){
        int CV_FR_pom = asistentAsistenta(vzdialenost);
        if(CV_FR < CV_FR_pom)
            CV_FR = CV_FR_pom;
    }else if(uhol >= -112.5 && uhol <= -67.5){
        int CV_L_pom = asistentAsistenta(vzdialenost);
        if(CV_L < CV_L_pom)
            CV_L = CV_L_pom;
    }else if(uhol < 112.5 && uhol >= 67.5){
        int CV_R_pom = asistentAsistenta(vzdialenost);
        if(CV_R < CV_R_pom)
            CV_R = CV_R_pom;
    }else if(uhol > -67.5 && uhol <= -22.5){
        int CV_BL_pom = asistentAsistenta(vzdialenost);
        if(CV_BL < CV_BL_pom)
            CV_BL = CV_BL_pom;
    }else if(uhol <= 67.5 && uhol >= 22.5){
        int CV_BR_pom = asistentAsistenta(vzdialenost);
        if(CV_BR < CV_BR_pom)
            CV_BR = CV_BR_pom;
    }else{
        int CV_B_pom = asistentAsistenta(vzdialenost);
        if(CV_B < CV_B_pom)
            CV_B = CV_B_pom;
    }
}


int robot::processThisLidar(LaserMeasurement laserData)
{


    memcpy( &copyOfLaserData,&laserData,sizeof(LaserMeasurement));
    //tu mozete robit s datami z lidaru.. napriklad najst prekazky, zapisat do mapy. naplanovat ako sa prekazke vyhnut.
    // ale nic vypoctovo narocne - to iste vlakno ktore cita data z lidaru
    // updateLaserPicture=1;
    //break if 5cm

    /*for (int i = 0; i< copyOfLaserData.numberOfScans; i++){
        qDebug() <<i << ". uhol: " << copyOfLaserData.Data[i].scanAngle <<" vzdialenost: "<< copyOfLaserData.Data[i].scanDistance;
    }*/

    /*for (int i = 0; i < copyOfLaserData.numberOfScans; i++) {
        // 1. Získanie pôvodného ľavotočivého uhla (napr. -180.0° až 180.0°)
        double povodnyUhol_lavotocivy = copyOfLaserData.Data[i].scanAngle;

        // 2. Preklopenie znamienka:
        // Týmto sa mení smer rastu uhla (z protismeru hodinových ručičiek na smer)
        double pravotocivyUhol_negativny = -povodnyUhol_lavotocivy;

        // 3. Normalizácia (voliteľné, ak chcete rozsah [0, 360])
        // Ak chcete, aby bol výsledný pravotočivý uhol v rozsahu [0°, 360°):
        double uhol_lid = pravotocivyUhol_negativny;

        // Ak je uhol záporný (napr. -90°), pridaním 360° ho dostaneme do kladnej časti (270°).
        if (uhol_lid < 0.0) {
            uhol_lid += 180.0;
        }

        qDebug() << i
                 << ". uhol (pravotočivý): " << uhol_lid
                 << " vzdialenost: " << copyOfLaserData.Data[i].scanDistance;
    }*/
    if (manualOverrideActive){
        emit publishSystemBreakStatus(false);
        return 0;
    }

    bool new_break_status = false;
    std::cout << "existujem"<<endl;
    CV_F = 0;
    CV_B = 0;
    CV_R = 0;
    CV_L = 0;
    CV_FR = 0;
    CV_BR = 0;
    CV_BL = 0;
    CV_FL = 0;

    prekazkyVpredu_uhol.clear();
    prekazkyVpredu_vzdialenost.clear();
    int lidar_ping = 0;
    for (int i = 0; i < copyOfLaserData.numberOfScans; i++) {

        double povodnyUhol_lavotocivy = copyOfLaserData.Data[i].scanAngle;

        double pravotocivyUhol_negativny = -povodnyUhol_lavotocivy;

        double uhol_lidar = pravotocivyUhol_negativny;

        double vzdialenost_lid = copyOfLaserData.Data[i].scanDistance;


        // Ak je uhol záporný (napr. -90°), pridaním 360° ho dostaneme do kladnej časti (270°).
        if (uhol_lidar < 0.0) {
            uhol_lidar += 180.0;
        }
        double uhol_lid = uhol_lidar;
        //nove
        if ((uhol_lid <= -146.0 && uhol_lid >= -190.0)||(uhol_lid >= 146.0 && uhol_lid <= 180.0)){
            prekazkyVpredu_uhol.push_back(uhol_lid);
            prekazkyVpredu_vzdialenost.push_back(vzdialenost_lid);
        }
        //nove

        reverzAsistent(uhol_lid, vzdialenost_lid);

        if ((uhol_lid>30 || uhol_lid <-30) && vzdialenost_lid < 200 && vzdialenost_lid > 100) {
            lidar_ping ++;
            //new_break_status = true;

        } else if ((uhol_lid >= -30 && uhol_lid <=30)&& vzdialenost_lid < 220 && vzdialenost_lid > 100){
            //std::cout << "ZIJEEEEM"<<endl;
            lidar_ping ++;
            //new_break_status = true;
        }
    }
    if (lidar_ping >= 2)
        new_break_status = true;

    qDebug()<<"Lidar PING" << lidar_ping;

    //int pom = copyOfLaserData.numberOfScans/2;
    //std::cout << "cuvanie: "<<pom<<endl;
    //std::cout << "cuvanie: "<<copyOfLaserData.Data[138].scanDistance<<endl;

    qDebug() << "SB: "<<systemova_brzda;

    if (systemova_brzda != new_break_status){
        systemova_brzda = new_break_status;
        qDebug() << "brzda!!!";
        emit publishSystemBreakStatus(systemova_brzda);
    }

    //break if 5cm
    emit publishLidar(copyOfLaserData);
    // update();//tento prikaz prinuti prekreslit obrazovku.. zavola sa paintEvent funkcia

    emit publishCV(CV_F, CV_B, CV_L, CV_R, CV_FR, CV_BR, CV_BL, CV_FL);

    emit publishFrontLidarPoints(prekazkyVpredu_uhol, prekazkyVpredu_vzdialenost);

    return 0;

}

#ifndef DISABLE_OPENCV
///toto je calback na data z kamery, ktory ste podhodili robotu vo funkcii initAndStartRobot
/// vola sa ked dojdu nove data z kamery
int robot::processThisCamera(cv::Mat cameraData)
{

    cameraData.copyTo(frame[(actIndex+1)%3]);//kopirujem do nasej strukury
    actIndex=(actIndex+1)%3;//aktualizujem kde je nova fotka

    emit publishCamera(frame[actIndex]);
    return 0;
}
#endif

#ifndef DISABLE_SKELETON
/// vola sa ked dojdu nove data z trackera
int robot::processThisSkeleton(skeleton skeledata)
{

    memcpy(&skeleJoints,&skeledata,sizeof(skeleton));

    emit publishSkeleton(skeleJoints);
    return 0;
}
#endif
