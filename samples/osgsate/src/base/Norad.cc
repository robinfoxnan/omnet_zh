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

#include "Norad.h"

#include <ctime>
#include <fstream>

#include "libnorad/cTLE.h"
#include "libnorad/cOrbit.h"
#include "libnorad/cSite.h"

//Define_Module(Norad);

Norad::Norad()
{
    gap = 0.0;
    tle = nullptr;
    orbit = nullptr;
}

void Norad::finish()
{
    delete orbit;
    delete tle;
}

void Norad::initializeMobility(simtime_t& targetTime, std::string & line0, std::string & line1, std::string & line2)
{
    // Pretty up the satellites name
    this->line0 = line0.substr(0, line0.find("  "));
    this->line1 = line1;
    this->line2 = line2;

    cTle tle(this->line0, this->line1, this->line2);
    orbit = new cOrbit(tle);

    // Gap is needed to eliminate different start times
    gap = orbit->TPlusEpoch(currentJulian);
    std::cout << "时间差：" << gap << std::endl;

    updateTime(targetTime);

    // Set name from TLE file for icon name
    line3 = orbit->SatName(false);
    //getParentModule()->setName(satName.c_str());
}

void Norad::updateTime(simtime_t& targetTime)
{
    orbit->getPosition((gap + targetTime.dbl()) / 60, &eci);
    geoCoord = eci.toGeo();
}

// 获取当前的三维位置信息
void Norad::getXYZ(double &x, double &y, double &z)
{
    x = eci.getPos().m_x;
    y = eci.getPos().m_y;
    z = eci.getPos().m_z;
}

// 获取当前的速度分量
void Norad::getXYZ_V(double &xv, double &yv, double &zv)
{
    xv = eci.getVel().m_x;
    yv = eci.getVel().m_y;
    zv = eci.getVel().m_z;
}

// 获取当前的经纬度信息
void Norad::getLogLatAlt(double &Longitude, double &Latitude, double &Altitude)
{
    Longitude = rad2deg(geoCoord.m_Lon);
    Latitude = rad2deg(geoCoord.m_Lat);
    Altitude = geoCoord.m_Alt;
}


double Norad::getLongitude()
{
    return rad2deg(geoCoord.m_Lon);
}

double Norad::getLatitude()
{
    return rad2deg(geoCoord.m_Lat);
}

double Norad::getAltitude()
{
    geoCoord = eci.toGeo();
    return geoCoord.m_Alt;
}

// 计算到站点的仰角
double Norad::getElevation(const double& refLatitude, const double& refLongitude, const double& refAltitude)
{
    cSite siteEquator(refLatitude, refLongitude, refAltitude);
    cCoordTopo topoLook = siteEquator.getLookAngle(eci);
    if (topoLook.m_El == 0.0) {
        std::cout << "Error in Norad::getElevation(): Corrupted database." << std::endl;
    }
    return rad2deg(topoLook.m_El);
}

// 方位角
double Norad::getAzimuth(const double& refLatitude, const double& refLongitude, const double& refAltitude)
{
    cSite siteEquator(refLatitude, refLongitude, refAltitude);
    cCoordTopo topoLook = siteEquator.getLookAngle(eci);
    if (topoLook.m_El == 0.0) {
        std::cout <<"Error in Norad::getAzimuth(): Corrupted database."<< std::endl;
    }
    return rad2deg(topoLook.m_Az);
}



// 到基站的距离
double Norad::getDistance(const double& refLatitude, const double& refLongitude, const double& refAltitude)
{
    cSite siteEquator(refLatitude, refLongitude, refAltitude);
    cCoordTopo topoLook = siteEquator.getLookAngle(eci);
    double distance = topoLook.m_Range;
    return distance;
}

void Norad::handleMessage(cMessage* msg)
{
    std::cout <<"Error in Norad::handleMessage(): This module is not able to handle messages."<< std::endl;
}

void Norad::setJulian(std::tm* currentTime)
{
    currentJulian = cJulian(currentTime->tm_year + 1900,
                            currentTime->tm_mon + 1,
                            currentTime->tm_mday,
                            currentTime->tm_hour,
                            currentTime->tm_min, 0);
}
