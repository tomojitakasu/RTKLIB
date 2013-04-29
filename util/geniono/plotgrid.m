function plotgrid(file)

if nargin<1, file='stec/iono.pos'; end

fn='times new roman';
fs=8;

[s,w,t,lat,lon]=textread(file,'%s %f %f %f %f');

figure('color','w');

gmt('mmap','proj','miller','cent',[137,38],'scale',13,'pos',[0,0,1,1],'fontname',fn,'fontsize',fs);
gmt('mcoast');
gmt('mgrid','gint',1,'lint',5,'color',[.5 .5 .5]);

for i=1:length(s)
    gmt('mplot',lon(i),lat(i),'r','marker','.');
end
