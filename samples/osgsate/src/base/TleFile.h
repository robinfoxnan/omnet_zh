/*
 * TleFile.h
 *
 *  Created on: 2020年11月17日
 *      Author: 39001
 */

#ifndef BASE_TLEFILE_H_
#define BASE_TLEFILE_H_

#include <iostream>
#include <vector>

class TleFile {
public:
    TleFile();
    virtual ~TleFile();

    int ParseFile(std::string filename);
    bool getInfo(int index, std::string & line0, std::string & line1, std::string & line2);

protected:
    std::vector<std::string> vec0;
    std::vector<std::string> vec1;
    std::vector<std::string> vec2;

    int count;

};

#endif /* BASE_TLEFILE_H_ */
