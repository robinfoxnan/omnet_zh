package osgsate.demo;

import osgsate.src.base.*;

network OsgSateNet
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
		
		// or, for non-zooming status area: @figure[status](type=panel; pos=700,20);
        // 下面的部分是绘制的显示状态信息的文本方框
		@figure[status](type=group; transform=translate(510,10));
       
        @figure[status.bg](type=rectangle; pos=0,0; size=150,50; anchor=n; cornerRadius=10; fillColor=green; fillOpacity=0.4; lineColor=#fafdb9; lineOpacity=0.4);
        @figure[status.TimeTitle](type=text; pos=0,15; text="仿真时间"; anchor=center);
        @figure[status.TimeInfo](type=text; pos=0,35; text="2020-11-23 15:06:30"; anchor=center);
 
		
		@figure[GEO](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=yellow;  lineStyle=dashed; smooth=true);
		@figure[IGSO](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=yellow;  lineStyle=dashed; smooth=true);
		
		@figure[MEO1](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=red;  lineStyle=dashed);
		@figure[MEO2](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=MAGENTA;  lineStyle=dashed);
		@figure[MEO3](type=polyline; lineWidth=2; lineOpacity=0.6; lineColor=blue;  lineStyle=dashed);

		
		
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
