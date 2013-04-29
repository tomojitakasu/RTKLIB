function plotlexure(file,sats,trange)
%
% plot lex ure with ephemeris and clock
%
% 2010/12/09 0.1 new
%
if nargin<1, file='diffeph.out'; end
if nargin<2, sats=1:32; end
if nargin<3, trange=[0 24]; end % time range [tstart tend] (hr)

v=textread(file);

figure('color','w'), hold on, box on, grid on

range=3;
minel=15;

timv=v(:,2)-floor(v(:,2)/86400)*86400;
time=unique(timv);
v(v(:,end)<minel,end-1)=nan;

ure=repmat(nan,length(time),length(sats));
for i=1:length(sats)
    j=find(v(:,3)==sats(i)); if isempty(j), continue; end
    [tt,k]=intersect(time,timv(j));
    ure(k,i)=v(j,end-1);
end
for i=1:length(time)
    ure(i,:)=ure(i,:)-mean(ure(i,~isnan(ure(i,:))));
end
rmserr=[]; satlabel={};
for i=1:length(sats)
    
    plot(time/3600,ure(:,i),'-');
    
    satlabel={satlabel{:},sprintf('GPS%02d',sats(i))};
    rmserr=[rmserr;sqrt(mean(ure(~isnan(ure(:,i)),i).^2,1))];
    
    disp(sprintf('GPS%02d: %4d %8.4f m',sats(i),length(find(~isnan(ure(:,i)))),rmserr(end,:)));
end
xlim(trange);
ylim([-range,range]);
xlabel('Time (hr)');
ylabel('Error (m)');
title(['URE Error : ',file]);
legend({'URE'});
moveax;

figure('color','w'), hold on, box on
pos=[0.08,0.12,0.89,0.81];
h=ggt('barplot',rmserr,satlabel,'ylim',[0,range],'position',pos);
legend(h,{'URE'})
ylabel('URE RMS Error (m)');
title(['QZSS LEX Ephemeris/Clock URE Error: ',file]);

