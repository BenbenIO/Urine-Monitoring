#TCP servor test to see if I can get it work...
import socket
from datetime import datetime
import time as T
TCP_IP='192.168.111.11'
TCP_PORT=6666
BUFFER_SIZE = 1024

s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
s.bind((TCP_IP,TCP_PORT))
s.listen(1)
while 1:
	print("Waiting for new connection on \n"+str(TCP_IP)+str(TCP_PORT))

	conn,addr=s.accept()
	print ("\nNew connection from:"+str(conn)+str(addr))

	while 1 :
		data=conn.recv(BUFFER_SIZE)
		if not data:
			#print("No data recieved\n")
			T.sleep(2)
			break
		print("\n*****\n")
		print(data)
		print("\n*****\n")
		conn.send("ERR_OK")
		print("\nData extraction\n")
		#carbage,clean=data.split("*")
		clean=data
		print(clean)
		#Export on txt ?
		datafile=open("data_file.txt","a")
		full=datetime.now()
		date=str(full.year)+'/'+str(full.month)+'/'+str(full.day)
		time=str(full.hour)+'h'+str(full.minute)
		line = str(date)+':'+str(time)+':'+clean+'\n'
		print(line)
		datafile.write(line)
		datafile.close()

	conn.close()
	print("Connection closed\n")
