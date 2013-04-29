function plotigp

figure

mesh=readmesh;

gmt('mmap','proj','eq','cent',[135,35],'scale',10,'pos',[0,0,1,1]);
gmt('mcoast');
gmt('mgrid','gint',2,'lint',10,'color',[.5 .5 .5]);

for i=1:size(mesh,1)
    gmt('mplot',mesh(i,1),mesh(i,2),'r','marker','.','markersize',10);
end
plotarea([36,138],15);

% plot ipp area ----------------------------------------------------------------
function plotarea(pos,elmask)
posp=[];
for az=0:3:360
    posp=[posp;igppos(pos*pi/180,[az,elmask]*pi/180)*180/pi];
end
gmt('mplot',pos(2),pos(1),'b','marker','.','markersize',10);
gmt('mplot',posp(:,2),posp(:,1),'b');

% read mesh data ---------------------------------------------------------------
function mesh=readmesh
mesh=[];
fp=fopen('../../nicttec/vtec/2011/001.txt','r');
while 1
    s=fgets(fp); if ~ischar(s), break; end
    v=sscanf(s,' Mesh %d: (%f, %f)');
    if length(v)<2, continue; end
    mesh=[mesh;v(2:3)'];
end
fclose(fp);

% igp position -----------------------------------------------------------------
function posp=igppos(pos,azel)
re=6380; hion=350;
rp=re/(re+hion)*cos(azel(2));
ap=pi/2-azel(2)-asin(rp);
sinap=sin(ap);
tanap=tan(ap);
cosaz=cos(azel(1));
posp(1)=asin(sin(pos(1))*cos(ap)+cos(pos(1))*sinap*cosaz);
posp(2)=pos(2)+asin(sinap*sin(azel(1))/cos(posp(1)));  
