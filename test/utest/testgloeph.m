function testgloeph

C=299792458.0;
td=caltomjd([2009,4,1]);
time=0:30:86400*2-30;

err=textread('testgloeph.out');

sat=sprintf('SAT%02d',err(1,3));

time=err(:,2);
dpos=err(:,4:7);
dpos(:,4)=dpos(:,4)-mean(dpos(:,4));

figure('color','w'), hold on, box on, grid on
plot(time/3600,dpos)
plot(time(31:60:end)/3600,dpos(31:60:end,:),'.')

xlabel('time (hr)');
ylabel('error (m)');
xlim(time([1,end])/3600);
ylim([-10,10]);
legend({'x','y','z','clk'})
text(0.02,0.95,sprintf('STD: X=%.4f Y=%.4f Z=%.4f CLK=%.4fm',...
     std(dpos(~isnan(dpos(:,1)),1:3)),std(dpos(~isnan(dpos(:,4)),4))),...
     'units','normalized')
title(sprintf('testgloeph: brdc-prec ephemeris %s',sat));
moveax
