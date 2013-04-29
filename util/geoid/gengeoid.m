function gengeoid(range,file)
% read geoid model file and generate c-source code

if nargin<1, range=[120,155,20,50]; end
if nargin<2, file='WW15MGH.GRD'; end

ofile='geoid.c';

[lat1,lat2,lon1,lon2,dy,dx]=textread(file,'%f %f %f %f %f %f',1);
lon=lon1:dx:lon2;
lat=lat1:dy:lat2;
nx=(lon2-lon1)/dx+1;;
ny=(lat2-lat1)/dy+1;

x=dlmread(file,'',1,0);
data=[];
for n=1:181:size(x,1)
    xx=reshape(x(n:n+180,1:8)',1,181*8);
    data=[data;xx(1:nx)];
end
data=flipud(data);
i=find(range(3)<=lat&lat<=range(4));
j=find(range(1)<=lon&lon<=range(2));

f=fopen(ofile,'w');
fprintf(f,'#define DLON   %.2f                 /* longitude increment (deg) */ \n',dx);
fprintf(f,'#define DLAT   %.2f                 /* latitude increment (deg) */ \n',dy);
fprintf(f,'static const double range[4];       /* geoid area range {W,E,S,N} (deg) */\n');
fprintf(f,'static const float geoid[%d][%d]; /* geoid heights (m) (lon x lat) */\n',length(j),length(i));
fprintf(f,'\n\n');
fprintf(f,'/*------------------------------------------------------------------------------\n');
fprintf(f,'* geoid heights (derived from %s)\n',file);
fprintf(f,'*-----------------------------------------------------------------------------*/\n');
fprintf(f,'static const double range[]={%.2f,%.2f,%.2f,%.2f};\n\n',range);
fprintf(f,'static const float geoid[%d][%d]={',length(j),length(i));
for m=1:length(j)
    fprintf(f,'{\n');
    for n=1:length(i)
        fprintf(f,'%7.3ff',data(i(n),j(m)));
        if n~=length(i), fprintf(f,','); end
        if mod(n,10)==0, fprintf(f,'\n'); end
    end
    if m==length(j), fprintf(f,'\n}'); else fprintf(f,'\n},'); end
end
fprintf(f,'};\n');
fclose(f);
