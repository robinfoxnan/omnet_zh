/*
 * TleFile.cc
 *
 *  Created on: 2020年11月17日
 *      Author: 39001
 */

#include "TleFile.h"

#include <sstream>
#include <fstream>
#include <string>
using namespace std;//命名空间也要加

TleFile::TleFile() {
    // TODO Auto-generated constructor stub

}

TleFile::~TleFile() {
    // TODO Auto-generated destructor stub
}

int TleFile::ParseFile(std::string filename)
{
    // 只在第一个卫星初始化时候加载一次；
    if(count > 0)
        return count;

    std::ifstream tleFile;
    tleFile.open(filename.c_str());

    // Length 100 should be enough since lines are usually 70+'\n' char long
    char line0[200] = "";
    char line1[200] = "";
    char line2[200] = "";

    this->count = 0;

    std::string line_str;
    do {
        tleFile.getline(line0, 200);
        if (tleFile.eof() )
            break;

        tleFile.getline(line1, 200);
        if (tleFile.eof() )
           break;

        tleFile.getline(line2, 200);
        if (tleFile.eof() )
           break;

        this->vec0.push_back(line0);
        this->vec1.push_back(line1);
        this->vec2.push_back(line2);

        count ++;
    } while ( !tleFile.eof() );

    tleFile.close();

    std::cout << "加载了TLE组数：" << count << std::endl;
    return count;
}

bool TleFile::getInfo(int index, std::string & line0, std::string & line1, std::string & line2)
{
    if (index >= count)
        return false;

    line0 = vec0[index];
    line1 = vec1[index];
    line2 = vec2[index];

    return true;
}
