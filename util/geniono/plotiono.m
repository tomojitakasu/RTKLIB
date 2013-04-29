function plotiono(ssat,file)

if nargin<1, ssat=1:32; end
if nargin<2, file='iono_0225.stec'; end

F1=1.57542E9;
F2=1.22760E9;
c_iono=1.0-F1^2/F2^2;

color='bgrcmyk';

[w,t,s,slip,iono,sig,ionor,sigr,bias,sigb,az,el,PG,LG]=...
    textread(file,'$STEC %d %f G%d %d %f %f %f %f %f %f %f %f %f %f',...
             'headerlines',1);

figure('color','w','Renderer','OpenGL'); hold on, grid on, box on;

for sat=ssat
%    i=find(s==sat&sig<0.1);
    i=find(s==sat);
%    plot(t(i)/3600,-LG(i)/c_iono,'g.-');
%    plot(t(i)/3600, PG(i)/c_iono,'m.-');
    
%    plot(t(i)/3600,bias(i),'k.');
    plot(t(i)/3600,iono(i),'b.');
%    plot(t(i)/3600,iono(i)+LG(i)/c_iono,'r-');
    
%    plot(t(i)/3600,iono(i)+sig(i),'r:');
%    plot(t(i)/3600,iono(i)-sig(i),'r:');
%    plot(t(i)/3600,bias(i)+sigb(i),'r:');
%    plot(t(i)/3600,bias(i)-sigb(i),'r:');
    
    i=find(s==sat&slip==1);
    plot(t(i)/3600,iono(i),'r.');
end
xlabel('TIME (H)');
ylabel('IONO DELAY (m)');
xlim([t(1),t(end)]/3600);
ylim([-2,18]);
moveax

%figure('color','w'); hold on, grid on, box on;
%for sat=8
%    i=find(s==sat);
%    plot(t(i)/3600,el(i),'.-');
%end
%moveax
