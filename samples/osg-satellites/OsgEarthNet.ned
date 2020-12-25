//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

//
// The osgEarth Demo network.
//
network OsgEarthNet
{
    parameters:
        @display("bgi=background/Basic1,s;bgb=1024,512");

        int numSats = default(0);
        int numStats = default(0);

		// 维度线
		@figure[grid](type=polyline; lineWidth=1; lineOpacity=0.6; lineColor=white;  lineStyle=dashed);
		
		@figure[box](type=group; transform=translate(0, 512); );
		@figure[box.bg](type=rectangle; pos=0,0; size=1026,20; lineColor=gray; fillColor=black; fillOpacity=0.7);
		@figure[box.v0](type=text; pos=0,0; text="-180°";  font=Arial, 8; color=white);
		@figure[box.v1](type=text; pos=85,0; text="-150°";  font=Arial, 8; color=white);
		@figure[box.v2](type=text; pos=171,0; text="-120°";  font=Arial, 8; color=white);
		@figure[box.v3](type=text; pos=256,0; text="-90°";  font=Arial, 8; color=white);
		@figure[box.v4](type=text; pos=341,0; text="-60°";  font=Arial, 8; color=white);
		@figure[box.v5](type=text; pos=426,0; text="-30°";  font=Arial, 8; color=white);
		@figure[box.v6](type=text; pos=513,0; text="0°";  font=Arial, 8; color=white);
		@figure[box.v7](type=text; pos=598,0; text="30°";  font=Arial, 8; color=white);
		@figure[box.v8](type=text; pos=684,0; text="60°";  font=Arial, 8; color=white);
		@figure[box.v9](type=text; pos=770,0; text="90°";  font=Arial, 8; color=white);
		@figure[box.v10](type=text; pos=855,0; text="120°";  font=Arial, 8; color=white);
		@figure[box.v11](type=text; pos=940,0; text="150°";  font=Arial, 8; color=white);
		//@figure[box.v12](type=text; pos=1024,0; text="180°";  font=Arial, 8; color=white);
		
		@figure[vbox](type=group; transform=translate(-20, -1); );
		@figure[vbox.bg](type=rectangle; pos=0,0; size=20,533; lineColor=gray; fillColor=#000000; fillOpacity=0.7);
		@figure[vbox.v0](type=text; pos=0,75; text="60°";  font=Arial, 8; color=white);
		@figure[vbox.v1](type=text; pos=0,160; text="30°";  font=Arial, 8; color=white);
		@figure[vbox.v2](type=text; pos=0,245; text="0°";  font=Arial, 8; color=white);
		@figure[vbox.v3](type=text; pos=0,330; text="-30°";  font=Arial, 8; color=white);
		@figure[vbox.v4](type=text; pos=0,415; text="-60°";  font=Arial, 8; color=white);
		

		
		
		@figure[IGSO1-1](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=yellow;  lineStyle=dashed; smooth=true);
		@figure[IGSO2-1](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=yellow;  lineStyle=dashed);
		@figure[IGSO3-1](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=yellow;  lineStyle=dashed);
		
		
		@figure[MEO1-1](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		@figure[MEO1-2](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		@figure[MEO1-3](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		@figure[MEO1-4](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		@figure[MEO1-5](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		@figure[MEO1-6](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		@figure[MEO1-7](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		@figure[MEO1-8](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		@figure[MEO1-9](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		
		@figure[MEO2-1](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		@figure[MEO2-2](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		@figure[MEO2-3](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		@figure[MEO2-4](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		@figure[MEO2-5](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		@figure[MEO2-6](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		@figure[MEO2-7](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		@figure[MEO2-8](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		@figure[MEO2-9](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		
		@figure[MEO3-1](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);
		@figure[MEO3-2](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);
		@figure[MEO3-3](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);
		@figure[MEO3-4](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);
		@figure[MEO3-5](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);
		@figure[MEO3-6](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);
		@figure[MEO3-7](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);
		@figure[MEO3-8](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);
		@figure[MEO3-9](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);
		
		
    submodules:
        osgEarthScene: OsgEarthScene {
            @display("is=s;p=0,-60");
        }
        sat[numSats]: Satellite {
            @display("p=204,150");
        }
        channelController: ChannelController {
            @display("p=250,-60");
        }
        stat[numStats]: GroundStation {
            @display("p=250,250");
        }
        clock: Clock {
            @display("p=150,-60");
        }
}
