# Application for the dataprocessing and the GUI ?
# called by the servor?

import numpy as np
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.figure import Figure
from matplotlib import style
import tkinter as tk
import tkinter.messagebox as tmb
from PIL import Image, ImageTk
import os
import time

#DATA BASE CREATION###########################
#Urine related data
df_urine = pd.read_csv('data_file.txt', sep=":",header=None)
df_urine.columns = ["Date","Hour","number","Absorbance","Conductivity"]
df_urine["NAbsorbance"]=df_urine.Absorbance/max(df_urine.Absorbance)
df_urine.dropna(axis=0,inplace=True)
#Weather related data:
df_weather = pd.read_csv('Weather_data.csv',index_col=0)
df_weather.dropna(axis=0,inplace=True)
#Fusion
df_full = df_urine.append(df_weather)
df_full.reset_index(inplace=True)
#Saving
df_full.to_csv('full_data.csv')

#DATA PRESENTATION############################
#Select the latest data:
last=df_urine.shape[0]-1
Ldate=df_urine.Date.iloc[last]
Lhour=df_urine.Hour.iloc[last]
Lnumber=df_urine.number.iloc[last]
Labs=round(df_urine.NAbsorbance[last],2)
Lcond=df_urine.Conductivity[last]
Lweather=df_weather.D.iloc[df_weather.shape[0]-1]

#Some usefull data extraction:
print(df_full.groupby('Date').sum()['number'])
print(df_full.groupby('Date').mean()[['Absorbance','Conductivity','D','D+1','D+2']])

#USER INTERFACE#################################
root = tk.Tk()
root.title('Heath System: HeatWave prevention')

#Lastest data display:
tk.Label(root, text=" Latest Data: ",fg="red").grid(row=1,column=0,sticky=tk.W)
tk.Label(root,text=" Date ").grid(row=0,column=1)
tk.Label(root,text=str(Ldate),bg="#3782B6",fg="white").grid(row=1,column=1,sticky=tk.W+tk.E+tk.N+tk.S)
tk.Label(root,text=" Absorbance ").grid(row=0,column=2)
tk.Label(root,text=str(Labs),bg="#3782B6",fg="white").grid(row=1,column=2,sticky=tk.W+tk.E+tk.N+tk.S)
tk.Label(root,text=" Conductivity ").grid(row=0,column=3)
tk.Label(root,text=str(Lcond),bg="#3782B6",fg="white").grid(row=1,column=3,sticky=tk.W+tk.E+tk.N+tk.S)
tk.Label(root,text=" Urination N* ").grid(row=0,column=4)
tk.Label(root,text=str(Lnumber),bg="#3782B6",fg="white").grid(row=1,column=4,sticky=tk.W+tk.E+tk.N+tk.S)
tk.Label(root,text=" WBGT ").grid(row=0,column=5)
tk.Label(root,text=str(Lweather),bg="#3782B6",fg="white").grid(row=1,column=5,sticky=tk.W+tk.E+tk.N+tk.S)

#MENU function:
def display_about(event=None):
    about_toplevel=tk.Toplevel(root)
    about_toplevel.title("About")
    about_display=tk.Label(about_toplevel,
                           text="Programed by Benjamin IOLLER,\n Contact: ioller.benjamin@gmail.com\n for the master thesis in KEIO :p")
    about_display.grid()
                           
def display_infoP(event=None):
    info_toplevel=tk.Toplevel(root)
    info_toplevel.title('Information Patient')
    name_label=tk.Label(info_toplevel,text="Name: ")
    name_label.grid(row=0,column=1)
    name_disp=tk.Label(info_toplevel,text="Kame Sennin",bg="#3782B6",fg="white")
    name_disp.grid(row=0,column=2,sticky='w',columnspan=2)
    ID_label=tk.Label(info_toplevel,text="Patient ID: ")
    ID_label.grid(row=1,column=1)
    ID_disp=tk.Label(info_toplevel,text="sp63****",bg="#3782B6",fg="white")
    ID_disp.grid(row=1,column=2,sticky='w',columnspan=2)
    weight_label=tk.Label(info_toplevel,text="Weight: ")
    weight_label.grid(row=2,column=1)
    weight_disp=tk.Label(info_toplevel,text="86kg",bg="#3782B6",fg="white")
    weight_disp.grid(row=2,column=2,sticky='w',columnspan=2)
    height_label=tk.Label(info_toplevel,text="Height: ")
    height_label.grid(row=3,column=1)
    height_disp=tk.Label(info_toplevel,text="1m53kg",bg="#3782B6",fg="white")
    height_disp.grid(row=3,column=2,sticky='w',columnspan=2)
    age_label=tk.Label(info_toplevel,text="Age: ")
    age_label.grid(row=4,column=1)
    age_disp=tk.Label(info_toplevel,text="103 years",bg="#3782B6",fg="white")
    age_disp.grid(row=4,column=2,sticky='w',columnspan=2)
    #Add picture:
    img = ImageTk.PhotoImage(Image.open("turtle.jpeg"))
    imgLabel = tk.Label(info_toplevel,image=img)
    imgLabel.image = img
    imgLabel.grid(row=0,column=5,sticky='e',rowspan=5,columnspan=5)
    #Add commnent section:
    comment_label=tk.Label(info_toplevel,text="Comments: ")
    comment_label.grid(row=6,column=1)
    comment_section=tk.Text(info_toplevel,height=5,width=40)
    comment_section.grid(row=6,column=2,rowspan=4,columnspan=20)
    comment_section.insert(tk.END,"Patient is a dragonball caractere :)")

#More graph pages:

def PageGraph(even=None):
    graph_toplevel=tk.Toplevel(root)
    graph_toplevel.title("Data monitor")
    scrollbar=tk.Scrollbar(graph_toplevel)
    scrollbar.grid(row=0,column=10,columnspan=10,sticky='e')
    #Daily data:
    df_daily=df_full.groupby("Date").mean()
    Dasb=df_daily["Absorbance"].dropna()
    Dcond=df_daily["Conductivity"].dropna()
    Dcompte=df_daily["number"].dropna()
    Dweather=df_daily["D"].dropna()
    #Graph time:    
    fig1,a=plt.subplots(2,2)
    a=a.ravel()
    Dasb.plot(ax=a[0],title="Daily Absorbance",rot=85)
    plt.xticks(rotation=45)
    Dcond.plot(ax=a[1],title="Daily Conductivity",rot=85)
    plt.xticks(rotation=45)
    Dcompte.plot(ax=a[2],kind='bar',title="Urination Frequency",rot=85)
    plt.xticks(rotation=45)
    Dweather.plot(ax=a[3],title="WGBT forecast",rot=85)
    plt.tight_layout(pad=0.5,w_pad=0.5,h_pad=1.0)
    #plt.xticks(rotation=90)
    plt.grid(True)
    #canvas:
    canvas1=FigureCanvasTkAgg(fig1,graph_toplevel)
    canvas1.show()
    canvas1.get_tk_widget().grid(row=0,column=0,columnspan=8,rowspan=8)
    #toolbar:
    toolbar_frame = tk.Frame(graph_toplevel)
    toolbar_frame.grid(row=10,column=1,rowspan=1,columnspan=8)
    toolbar = NavigationToolbar2TkAgg(canvas1,toolbar_frame)
      
#Creating the MENU:
menu_bar = tk.Menu(root)
menu_bar.add_command(label='Patient Information',command=display_infoP)
menu_bar.add_cascade(label='Setting')
menu_bar.add_command(label='About',command=display_about)

#Matplot into canvas:
df_daily=df_full.groupby('Date').mean()
#Absorbance example:
dailyABS=df_daily["Absorbance"].dropna()
fig,ax = plt.subplots()
dailyABS.plot(ax=ax,rot=45)
plt.title("Urine daily monitoring")
plt.axhline(y=1000,color='r',linestyle='-',label='TRIGGER')
plt.grid(True)
plt.ylabel("Urine quality indicator")
plt.xlabel("Date")
#Send to canvas:
canvas = FigureCanvasTkAgg(fig, master = root)
canvas.show()
canvas.get_tk_widget().grid(row=6,column=0,columnspan=8)
toolbar_frame = tk.Frame(root)
toolbar_frame.grid(row=10,column=1,rowspan=1,columnspan=8)
toolbar = NavigationToolbar2TkAgg(canvas,toolbar_frame)

#More graphique button:
more_graphe=tk.Button(root,text="More graphique",fg="red",command=PageGraph)
more_graphe.grid()

#Run the GUI
root.configure(menu=menu_bar)
print("END")
root.mainloop()

