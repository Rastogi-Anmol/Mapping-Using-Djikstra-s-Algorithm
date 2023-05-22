run:
	make -C server
	gnome-terminal -- ./server/server
	python3 client/client.py
	
clean:
	make clean -C server
	-rm -f inpipe outpipe
	echo Cleaned!
