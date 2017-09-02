# makefile for iers.a

SRC      = ../src

F77      = gfortran
F77FLAGS = -O3 -ffixed-line-length-132

OBS = cal2jd.o dat.o dehanttideinel.o gmf.o gpt.o norm8.o sprod.o st1idiu.o \
      st1isem.o st1l1.o step2diu.o step2lon.o vmf1.o vmf1_ht.o zero_vec8.o

iers.a     : $(OBS)
	ar r iers.a $(OBS)
cal2jd.o   : $(SRC)/cal2jd.f
	$(F77) -c $(F77FLAGS) $(SRC)/cal2jd.f
dat.o      : $(SRC)/dat.f
	$(F77) -c $(F77FLAGS) $(SRC)/dat.f
dehanttideinel.o : $(SRC)/dehanttideinel.f
	$(F77) -c $(F77FLAGS) $(SRC)/dehanttideinel.f
gmf.o      : $(SRC)/gmf.f
	$(F77) -c $(F77FLAGS) $(SRC)/gmf.f
gpt.o      : $(SRC)/gpt.f
	$(F77) -c $(F77FLAGS) $(SRC)/gpt.f
norm8.o    : $(SRC)/norm8.f
	$(F77) -c $(F77FLAGS) $(SRC)/norm8.f
sprod.o    : $(SRC)/sprod.f
	$(F77) -c $(F77FLAGS) $(SRC)/sprod.f
st1idiu.o  : $(SRC)/st1idiu.f
	$(F77) -c $(F77FLAGS) $(SRC)/st1idiu.f
st1isem.o  : $(SRC)/st1isem.f
	$(F77) -c $(F77FLAGS) $(SRC)/st1isem.f
st1l1.o    : $(SRC)/st1l1.f
	$(F77) -c $(F77FLAGS) $(SRC)/st1l1.f
step2diu.o : $(SRC)/step2diu.f
	$(F77) -c $(F77FLAGS) $(SRC)/step2diu.f
step2lon.o : $(SRC)/step2lon.f
	$(F77) -c $(F77FLAGS) $(SRC)/step2lon.f
vmf1.o     : $(SRC)/vmf1.f
	$(F77) -c $(F77FLAGS) $(SRC)/vmf1.f
vmf1_ht.o  : $(SRC)/vmf1_ht.f
	$(F77) -c $(F77FLAGS) $(SRC)/vmf1_ht.f
zero_vec8.o: $(SRC)/zero_vec8.f
	$(F77) -c $(F77FLAGS) $(SRC)/zero_vec8.f
	
clean :
	rm -f *.o *.a

