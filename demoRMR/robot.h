#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include <QWidget>
#include "librobot.h"
#ifndef DISABLE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/imgcodecs.hpp"
#include <QTimer>
#include <vector>


          Q_DECLARE_METATYPE(cv::Mat)
#endif
#ifndef DISABLE_SKELETON
          Q_DECLARE_METATYPE(skeleton)
#endif
          Q_DECLARE_METATYPE(LaserMeasurement)

          class robot : public QObject
{
    Q_OBJECT
public:
    explicit robot(QObject *parent = nullptr);

    void initAndStartRobot(std::string ipaddress);

    //tato funkcia len nastavuje hodnoty.. posielaju sa v callbacku(dobre, kvoli asynchronnosti a zabezpeceniu,ze sa poslu len raz pri viacero prepisoch vramci callu)
    void setSpeedVal(double forw,double rots);
    //tato funkcia fyzicky posiela hodnoty do robota
    void setSpeed(double forw,double rots);
    void resetSystemBrakeOverride();
public slots:
    // NOVÝ SLOT: Pre príjem stavu resetu systémovej brzdy z MainWindow (UI)
    //void resetSystemBreakStatus(bool resetStatus);
    // NOVÝ SLOT: Spustí 10-sekundové blokovanie
    void startSystemBreakOverride(int durationMs);
    // NOVÝ SLOT: Volá sa po 10 sekundách
    void overrideTimeout();
signals:
    void publishPosition(double x, double y, double z);
    void publishCV(int CV_F, int CV_B, int CV_L, int CV_R,int CV_FR, int CV_BR, int CV_BL, int CV_FL);
    void publishLidar(const LaserMeasurement &lidata);
    void publishSystemBreakStatus(bool activated);
    void publishFrontLidarPoints(const std::vector<double> &uhol, const std::vector<double> &vzdialenost);
#ifndef DISABLE_OPENCV
    void publishCamera(const cv::Mat &camframe);
#endif
#ifndef DISABLE_SKELETON
    void publishSkeleton(const skeleton &skeledata);
#endif
private:

    /// toto su vase premenne na vasu odometriu
    double x;
    double y;
    double fi;
    bool systemova_brzda;
    ///-----------------------------
    QTimer *overrideTimer;
    bool manualOverrideActive = false;
    std::vector<double> prekazkyVpredu_uhol;
    std::vector<double> prekazkyVpredu_vzdialenost;
    /// toto su rychlosti ktore sa nastavuju setSpeedVal a posielaju v processThisRobot
    double forwardspeed;//mm/s
    double rotationspeed;//omega/s

    ///toto su callbacky co sa sa volaju s novymi datami
    int asistentAsistenta(double vzdialenost_asist);
    void reverzAsistent(double uhol, double vzdialenost);

    int processThisLidar(LaserMeasurement laserData);
    int processThisRobot(TKobukiData robotdata);
#ifndef DISABLE_OPENCV
    int processThisCamera(cv::Mat cameraData);
#endif


    ///pomocne strukutry aby ste si trosku nerobili race conditions
    LaserMeasurement copyOfLaserData;
#ifndef DISABLE_OPENCV
    cv::Mat frame[3];
#endif
    ///classa ktora riesi komunikaciu s robotom
    libRobot robotCom;


    ///pomocne premenne... moc nerieste naco su
    int datacounter;
#ifndef DISABLE_OPENCV
    bool useCamera1;
    int actIndex;
#endif


#ifndef DISABLE_SKELETON
    int processThisSkeleton(skeleton skeledata);
    int updateSkeletonPicture;
    skeleton skeleJoints;
#endif
    int useDirectCommands;


};

#endif // ROBOT_H
