


OBJECTS = webserv.o socklib.o



webserver:	$(OBJECTS)	
	cc -o webserver $(OBJECTS) 

