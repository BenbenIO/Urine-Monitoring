#Script to get the WBGT forecast
import requests
from bs4 import BeautifulSoup
import pandas as pd
from datetime import datetime

#URL creation:
site_url='http://www.wbgt.env.go.jp/graph_ref_td.php?region=03&prefecture=44&point=44132'
page=requests.get(site_url)
soup=BeautifulSoup(page.text,'html.parser')

#Date processing:
full=datetime.now()
date=str(full.year)+"/"+str(full.month)+"/"+str(full.day)

#Extract the WBGT from the page (forecast table) // the code is really not sexy, can be done more smartly
i=0
for day in soup.find_all("tr",class_="day"):
	if i==0:
		D0=day.text
	if i==1:
		D1=day.text
	else:
		D2=day.text
	i=i+1

#Calcul the daily average for each day:
forecast=[D0,D1,D2]
average=[0,0,0]
d=0
for day in forecast:
	total=0
	for i in range(2,10):
		try:
			average[d]=int(forecast[d].splitlines()[i])+average[d]
			total=total+1
		except ValueError:
			#Empty data :(
			average[d]=0+average[d]
	average[d]=average[d]/total
	d=d+1
#Creation of the table:
try:
	print("Loading the database\n")
	df_wbgt=pd.read_csv("Weather_data.csv",index_col=0)
except OSError:
	print("First time creating the database\n")
	df_wbgt=pd.DataFrame(columns=["Date","D","D+1","D+2"])

df_wbgt.loc[(df_wbgt.shape[0])]=[date,average[0],average[1],average[2]]
df_wbgt.set_index('Date')
df_wbgt.to_csv("Weather_data.csv")


