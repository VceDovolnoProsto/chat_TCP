all:    client server                                                                      

client: client.c
	gcc -std=gnu99 -pthread -Wall $< -o $@
 
server: server.c 
	gcc -std=gnu99 -Wall $< -o $@
       
clean:
	rm -f client server *.o *~ core
