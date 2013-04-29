function plotlexeph(file,sats,trange)
%
% plot lex ephemeris error
%
% 2010/12/09 0.1 new

if nargin<1, file='diffeph.out'; end
if nargin<2, sats=1:32; end
if nargin<3, trange=[0 24]; end % time range [tstart tend] (hr)
fn='Times New Roman';

v=textread(file);

figure('color','w'), hold on, box on, grid on

i=find(v(:,3)==2); % reference clock = PRN2
tclk=v(i,2)-floor(v(1,2)/86400)*86400;
clk0=v(i,5);
satlabel={};
rmserr=[];
range=3;

disp(sprintf('SAT  : %4s %4s %11s %11s %11s %11s %11s (m)','NE','NC','3D','Radial','AlongTrk','CrossTrk','Clock'));

for sat=sats
    i=find(v(:,3)==sat);
    
    if isempty(i), continue; end
    timep=v(i,2)-floor(v(1,2)/86400)*86400;
    poserr=[sqrt(sum(v(i,4:6).^2,2)),v(i,4:6)];
    
    [timec,j,k]=intersect(timep,tclk);
    clkerr=v(i(j),5)-clk0(k,:);
    
    i=find(trange(1)*3600<=timep&timep<trange(2)*3600);
    j=find(trange(1)*3600<=timec&timec<trange(2)*3600);
    
    timep=timep(i); poserr=poserr(i,:);
    timec=timec(j); clkerr=clkerr(j,:);
    
    plot(timep/3600,poserr(:,2:4),'-');
    plot(timec/3600,clkerr,'c-');
    
    satlabel={satlabel{:},sprintf('GPS%02d',sat)};
    rmserr=[rmserr;sqrt(mean(poserr.^2,1)),sqrt(mean(clkerr.^2,1))];
    
    disp(sprintf('GPS%02d: %4d %4d %11.4f %11.4f %11.4f %11.4f %11.4f m',sat,...
         length(timep),length(timec),rmserr(end,:)));
end
set(gca,'position',[0.08 0.1 0.86 0.8],'fontname',fn,'xtick',trange(1):3:trange(2));
xlim(trange);
ylim([-range,range]);
xlabel('Time (hr)');
ylabel('Error (m)');
title(['Ephemeris/Clock Error : ',file]);
legend({'Radial','AlongTrk','CrossTrk','Clock'});
text(0.02,0.98,sprintf('RMS 3D:%6.3fm R:%6.3fm,A:%6.3fm,C:%6.3fm,CLK:%6.3fm',...
     sqrt(mean(rmserr.^2))),...
     'units','normalized','horizontal','left','vertical','top','fontname',fn);
moveax;

figure('color','w'), hold on, box on
pos=[0.08,0.12,0.89,0.81];
h=ggt('barplot',rmserr,satlabel,'ylim',[0,range],'position',pos);
legend(h,{'3D','Radial','Along-Track','Cross-Track','Clock'})
ylabel('RMS Error (m)');
title(['Ephemeris/Clock Error: ',file]);
text(0.02,0.98,sprintf('RMS 3D:%6.3fm R:%6.3fm,A:%6.3fm,C:%6.3fm,CLK:%6.3fm',...
     sqrt(mean(rmserr.^2))),...
     'units','normalized','horizontal','left','vertical','top','fontname',fn);
