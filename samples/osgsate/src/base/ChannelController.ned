package osgsate.src.base;

//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

//
// This module is responsible for tracking the distances of mobile nodes,
// and visualizing the connectivity graph via osgEarth.
//
simple ChannelController
{
    @display("i=block/network2");
    // 轨道内链路设置为绿色
    string satToSatColor = default("#00FF00BF");  
    double satToSatWidth = default(2);
    
    // 轨道间链路设置为紫色
    string satToSatColor1 = default("AA00AABF");
    double satToSatWidth1 = default(2);
    
    // 和地面站间颜色设置为橘色
    string satToGroundColor = default("#FFBF00BF");
    double satToGroundWidth = default(2);
    
    double lookAngleMin = default(30);

}
