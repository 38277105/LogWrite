target:=libLogWrite.so
objs:=Compitable.o CoLinux.o LogWrite.o
$(target):$(objs)
	g++ -fPIC -shared -D_linux_ -o $(target) $(objs)
$(objs):$(objs:.o=.cpp)	
	g++ -fPIC -D_linux_ -c $(@:.o=.cpp) 
.PHONY:clean cleanobj cleantarget
clean:cleanobj cleantarget
cleanobj:
	rm *.o
cleantarget:
	-rm -fr $(target)	
