//=========================================================================
//  GLOBALS.CC - part of
//
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//  Author: Andras Varga
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "omnetpp/globals.h"
#include "omnetpp/ccomponenttype.h"

namespace omnetpp {

//
// Global objects
//
cGlobalRegistrationList componentTypes("component types");     // ned解析后的各种ModuleTypes都存储在这里，首先SetupNetwork就要用
cGlobalRegistrationList nedFunctions("NED functions");
cGlobalRegistrationList classes("classes");                     // define_module后，注册到这里
cGlobalRegistrationList enums("enums");                         // 各种枚举类型，在sim_std.m中定义了使用
cGlobalRegistrationList classDescriptors("class descriptors");  // 类描述符，手工实现类似JAVA反射功能
cGlobalRegistrationList configOptions("supported configuration options");
cGlobalRegistrationList resultFilters("resultFilters");
cGlobalRegistrationList resultRecorders("resultRecorders");
cGlobalRegistrationList messagePrinters("messagePrinters");
std::map<std::string,std::string> figureTypes;

EXECUTE_ON_SHUTDOWN(
        componentTypes.clear();
        nedFunctions.clear();
        classes.clear();
        enums.clear();
        classDescriptors.clear();
        configOptions.clear();
        resultFilters.clear();
        resultRecorders.clear();
        messagePrinters.clear();
        figureTypes.clear();
);

}  // namespace omnetpp
