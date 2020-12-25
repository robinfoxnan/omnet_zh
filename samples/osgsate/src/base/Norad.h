//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __OS3_Norad_H__
#define __OS3_Norad_H__

#include <omnetpp.h>

#include <string>
#include <ctime>

#include "libnorad/cEci.h"
#include "libnorad/cJulian.h"
#include "libnorad/ccoord.h"
#include "libnorad/cOrbit.h"

class cTle;
//class cOrbit;

using namespace omnetpp;

// 这个类返回的XYZ是空间的天球坐标系，而经纬度是对应的地球上的某个经纬度，所以用法不同
//-----------------------------------------------------
// Class: Norad
//
// Provides the functionality for satellite positioning
// this class provides the functionality needed to get the positions for satellites according
// to current tables from web information by providing known data
//-----------------------------------------------------
class Norad //: public cSimpleModule
{
public:
    Norad();

    // sets the internal calendar by translating the current gregorian time
    // currentTime: time at which the simulation takes place
    void setJulian(std::tm* currentTime);

    // Updates the end time of current linear movement for calculation of current position
    // targetTime: End time of current linear movement
    void updateTime(simtime_t& targetTime);

    // This method gets the current simulation time, cares for the file download (happens only once)
    // of the TLE files from the web and reads the values for the satellites according to the
    // omnet.ini-file. The information is provided by the respective mobility class.
    // targetTime: End time of current linear movement
    virtual void initializeMobility(simtime_t& targetTime, std::string & line0, std::string & line1, std::string & line2);

    // returns the longitude
    double getLongitude();

    // returns the latitude
    double getLatitude();

    // returns the elevation to a reference point
    double getElevation(const double& refLatitude, const double& refLongitude, const double& refAltitude = -9999);

    // returns the azimuth
    double getAzimuth(const double& refLatitude, const double& refLongitude, const double& refAltitude = -9999);

    // returns the altitude
    double getAltitude();

    void finish();

    // returns the distance to the satellite from a reference point (distance in km)
    double getDistance(const double& refLatitude, const double& refLongitude, const double& refAltitude = -9999);

    // 位置信息
    void getLogLatAlt(double &Longitude, double &Latitude, double &Altitude);
    void getXYZ_V(double &xv, double &yv, double &zv);
    void getXYZ(double &x, double &y, double &z);

    // 轨道基本信息: 半长轴，偏心率，轨道倾角；升交点，平近角，初始角度；轨道周期
    // 倾角
    double getInclination() { return this->orbit->Inclination(); };

    // 偏心率
    double  getEccentricity() {return this->orbit->Eccentricity(); };

    // 近地点幅角？0°-360°
    double getArgPerigee()  {return this->orbit->ArgPerigee(); };

    // 升交点赤经The Right Ascension of Ascending Node,RAAN
    double getRAAN()  {return this->orbit->RAAN(); };

    // 平近角
    double getMeanAnomaly()  {return this->orbit->mnAnomaly(); };

    // 平均周期seconds
    double getPeriod() {return this->orbit->Period(); };

    // 平均角速度,每分钟多少弧度
    double getMeanMotion() { return this->orbit->mnMotionRec();};

    double getSemiMajor() {return this->orbit->SemiMajor(); };

    std::string getName1() {return this->line3;};

protected:
    virtual void handleMessage(cMessage* msg);

private:
    cEci eci;
    cJulian currentJulian;
    double gap;

    cTle* tle;
    cOrbit* orbit;
    cCoordGeo geoCoord;
    std::string line0;
    std::string line1;
    std::string line2;
    std::string line3;
};

#endif
