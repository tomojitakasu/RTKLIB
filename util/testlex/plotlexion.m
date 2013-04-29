function plotlexion(file,index)
%
% plot lex ionosphere correction error
%
% 2010/12/09 0.1 new
%
if nargin<1, file='LEXION_20101204'; end
if nargin<2, index=2; end

eval(file);

td=caltomjd(epoch);
time=time(index);

ep=mjdtocal(td+(time+0.5)/86400);
ts=sprintf('%04.0f/%02.0f/%02.0f %02.0f:%02.0f',ep(1:5));

% plot lex ion
figure('color','w');
plotmap(tec(:,:,index),lons,lats,['LEX Vertical Ionosphere Delay (L1) (m): ',ts]);

% plot igs ion
for i=1:length(lons)
   for j=1:length(lats)
       ion(j,i)=ion_tec(td,time,[0 pi/2],[lats(j),lons(i),0],'../lexerrdata','igr');
   end
end
figure('color','w');
plotmap(ion,lons,lats,['IGR Vertical Ionosphere Delay (L1) (m): ',ts]);

% plot vion map ----------------------------------------------------------------
function plotmap(ion,lons,lats,ti)
fn='Times New Roman';
pos=[0.01 0.01 0.91 0.92];
cent=[137 35];
scale=8;
gray=[.5 .5 .5];
range=0:0.01:10;

gmt('mmap','proj','eq','cent',cent,'base',cent,'scale',6,'pos',pos,'fontname',fn);

[lon,lat]=meshgrid(lons,lats);
[xs,ys,zs]=gmt('lltoxy',lon,lat);
[c,h]=contourf(xs,ys,ion,range);
set(h,'edgecolor','none');
caxis(range([1,end]))

gmt('mcoast','lcolor','none','scolor','none','ccolor',gray);
gmt('mgrid','gint',5,'lint',5,'color',gray);

lonr=[141.0 129.0 126.7 146.0 146.0 141.0]; % lex tec coverage
latr=[ 45.5  34.7  26.0  26.0  45.5  45.5];
lonp=[130.0 118.0 115.7 157.0 157.0 130.0];
latp=[ 56.5  45.7  15.0  15.0  56.5  56.5];

gmt('mplot',lonr,latr,'k');
%gmt('mplot',lonp,latp,gray);

title(ti);

ggt('colorbarv',[0.94,0.015,0.015,0.92],range([1,end]),'',...
    'fontname',fn);
