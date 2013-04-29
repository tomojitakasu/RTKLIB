function testpeph1

C=299792458.0;
dirs='../data/sp3';
td=caltomjd([2010,7,1]);
time=0:30:86400*2-30;

ephs=textread('testpeph1.out');
ephs(ephs==0)=nan;

sat=sprintf('GPS%02d',ephs(1,1));

ephr=readeph(td,time,{sat},dirs,'igs',24,'interp');
clkr=readclk(td,time,{sat},dirs,'igs',24,'interp');

for i=1:length(time)
    clkr(i,1)=clkr(i,1)-2*ephr(i,1:3)*ephr(i,4:6)'/C^2;
end
dpos=ephs(:,3:6)-[ephr(:,1:3),clkr(:,1)*1E9];

figure('color','w'), hold on, box on, grid on
plot(time/3600,dpos)
xlabel('time (hr)');
ylabel('error (m)');
xlim(time([1,end])/3600);
ylim([-0.05,0.05]);
legend({'x','y','z','clk'})
text(0.02,0.95,sprintf('STD: X=%.4f Y=%.4f Z=%.4f CLK=%.4fm',...
     std(dpos(~isnan(dpos(:,1)),1:3)),std(dpos(~isnan(dpos(:,4)),4))),...
     'units','normalized')
title(sprintf('testpeph: interpolation error %s',sat));
moveax
