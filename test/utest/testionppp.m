function testionppp
%
% test RTCA/DO229C bug (A.4.4.10.1 A-22,23)
%

az=0:0.1:360;

figure, axes, hold on, box on, grid on;

pos=[80,0];
for i=1:length(az), posp(i,:)=ionppp(pos,[az(i),0]); end
plot(posp(:,2),posp(:,1),'.');

pos=[-75,170];
for i=1:length(az), posp(i,:)=ionppp(pos,[az(i),0]); end
plot(posp(:,2),posp(:,1),'.');

xlim([-180,180]);
ylim([-90,90]);

% pierce point -----------------------------------------------------------------
function posp=ionppp(pos,azel)
re=6378; hion=350;
pos=pos*pi/180;
azel=azel*pi/180;

rp=re/(re+hion)*cos(azel(2));
ap=pi/2-azel(2)-asin(rp);
posp(1)=asin(sin(pos(1))*cos(ap)+cos(pos(1))*sin(ap)*cos(azel(1)));

%if (pos(1)> 70.0*pi/180& tan(ap)*cos(azel(1))>tan(pi/2-pos(1)))|...
%   (pos(1)>-70.0*pi/180&-tan(ap)*cos(azel(1))>tan(pi/2-pos(1))) % DO229C

if (pos(1)> 70.0*pi/180& tan(ap)*cos(azel(1))>tan(pi/2-pos(1)))|...
   (pos(1)<-70.0*pi/180&-tan(ap)*cos(azel(1))>tan(pi/2+pos(1))) % corrected

    posp(2)=pos(2)+pi-asin(sin(ap)*sin(azel(1))/cos(posp(1)));
else
    posp(2)=pos(2)+asin(sin(ap)*sin(azel(1))/cos(posp(1)));
end
posp=posp*180/pi;
if posp(2)>180, posp(2)=posp(2)-360; end
